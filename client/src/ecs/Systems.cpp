#include "ecs/Systems.hpp"
#include "Constants.hpp"
#include "Zipper.hpp"
#include <algorithm>
#include <cmath>
#include <tuple>
#include <vector>

namespace rtype::ecs {

namespace {
    constexpr const char *SHIP_STAND = "images/animations/ship/blue/stand.anim";
    constexpr const char *SHIP_UP = "images/animations/ship/blue/up.anim";
    constexpr const char *SHIP_DOWN = "images/animations/ship/blue/down.anim";
    constexpr const char *ENEMY_ANIM = "images/animations/enemy.anim";
    constexpr const char *BULLET_ANIM = "images/animations/bullet.anim";
    constexpr const char *CHARGE_ANIM = "images/animations/shoot_load.anim";

    constexpr int LAYER_WORLD = 0;
    constexpr int LAYER_SHIP = 1;
    constexpr int LAYER_EFFECT = 2;
}

// network sync systems

NetSyncSystem::NetSyncSystem(rtype::Resources &resources)
    : _resources(resources), _clock(config::SERVER_TICK_RATE) {
}

void NetSyncSystem::reset() {
    _clock.reset();
    _entities.clear();
    _destroyed.clear();
    _myEntity.reset();
    _charge.reset();
    _lastTick.reset();
    _localDead = false;
    _localHealth = 3;
    _score = 0;
}

double NetSyncSystem::renderTime(double localNow) const {
    if (!_clock.valid())
        return 0.0;
    return _clock.serverTimeAt(localNow) - config::INTERP_DELAY;
}

void NetSyncSystem::operator()(Registry &registry, rtype::ClientNet &net) {
    for (const auto &event : net.takeEvents())
        applyEvent(registry, event);
    for (const auto &snapshot : net.takeSnapshots())
        applySnapshot(registry, snapshot);
}

Entity NetSyncSystem::spawn(Registry &registry, const Net::EntityState &state) {
    Entity entity = registry.spawnEntity();
    Renderable renderable;

    registry.addComponent(entity, Transform{state.x, state.y});
    registry.addComponent(entity, NetId{state.id, state.kind});
    registry.addComponent(entity, Interpolated{});
    switch (state.kind) {
        case Net::EntityKind::ENEMY:
            renderable.animation = _resources.animation(ENEMY_ANIM);
            renderable.loop = true;
            renderable.layer = LAYER_WORLD;
            break;
        case Net::EntityKind::PROJECTILE:
            renderable.animation = _resources.animation(BULLET_ANIM);
            renderable.layer = LAYER_WORLD;
            break;
        case Net::EntityKind::PLAYER:
            renderable.animation = _resources.animation(SHIP_STAND);
            renderable.layer = LAYER_SHIP;
            registry.addComponent(entity, ShipAnimation{
                _resources.animation(SHIP_UP), _resources.animation(SHIP_DOWN), state.y, 0});
            break;
    }
    registry.addComponent(entity, renderable);
    _entities.emplace(state.id, entity);
    if (state.kind == Net::EntityKind::PLAYER && _myEntity && *_myEntity == state.id) {
        promoteToLocal(registry, entity);
        _localHealth = state.health;
    }
    return entity;
}

void NetSyncSystem::promoteToLocal(Registry &registry, Entity entity) {
    if (registry.hasComponent<LocalPlayer>(entity))
        return;
    registry.addComponent(entity, LocalPlayer{});
    if (_charge)
        return;
    Entity charge = registry.spawnEntity();
    Renderable renderable;

    renderable.animation = _resources.animation(CHARGE_ANIM);
    renderable.loop = true;
    renderable.visible = false;
    renderable.layer = LAYER_EFFECT;
    registry.addComponent(charge, Transform{});
    registry.addComponent(charge, renderable);
    registry.addComponent(charge, ChargeEffect{});
    _charge = charge;
}

void NetSyncSystem::applySnapshot(Registry &registry, const rtype::ClientNet::Snapshot &snapshot) {
    if (_lastTick && snapshot.tick <= *_lastTick)
        return;
    _lastTick = snapshot.tick;
    _score = snapshot.score;
    _clock.onSnapshot(snapshot.tick, snapshot.receivedAt);

    const double time = _clock.tickTime(snapshot.tick);
    for (const Net::EntityState &state : snapshot.entities) {
        if (_destroyed.contains(state.id))
            continue;
        auto found = _entities.find(state.id);
        Entity entity = found != _entities.end() ? found->second : spawn(registry, state);
        registry.getComponents<Interpolated>()[entity.getId()]->buffer.push(time, state.x, state.y);
        if (_myEntity && state.id == *_myEntity)
            _localHealth = state.health;
    }
}

void NetSyncSystem::applyEvent(Registry &registry, const rtype::ClientNet::Event &event) {
    if (event.type == Net::EventType::YOUR_ENTITY) {
        if (_destroyed.contains(event.entityId))
            return;
        _myEntity = event.entityId;
        auto found = _entities.find(event.entityId);
        if (found != _entities.end())
            promoteToLocal(registry, found->second);
        return;
    }
    if (event.type != Net::EventType::DESTROY)
        return;
    _destroyed.insert(event.entityId);
    if (_myEntity && *_myEntity == event.entityId) {
        _localDead = true;
        _localHealth = 0;
        if (_charge) {
            registry.killEntity(*_charge);
            _charge.reset();
        }
    }
    auto found = _entities.find(event.entityId);
    if (found == _entities.end())
        return;
    registry.killEntity(found->second);
    _entities.erase(found);
}

// gameplay systems

void InterpolationSystem::operator()(Registry &registry, double renderTime) const {
    using Zip = Containers::Zipper<SparseArray<Transform>, SparseArray<Interpolated>>;
    Zip zipped(registry.getComponents<Transform>(), registry.getComponents<Interpolated>());

    for (Zip::iterator it = zipped.begin(); it != zipped.end(); ++it) {
        if (registry.hasComponent<LocalPlayer>(Entity(it.getIndex())))
            continue;
        Transform &transform = std::get<0>(*it);
        const Interpolated &interpolated = std::get<1>(*it);
        if (interpolated.buffer.empty())
            continue;
        const interp::Point point = interpolated.buffer.sample(renderTime, config::MAX_EXTRAPOLATION);
        transform.x = point.x;
        transform.y = point.y;
    }
}

void PredictionSystem::operator()(Registry &registry, const Keys &keys, float dt) const {
    using Zip = Containers::Zipper<SparseArray<Transform>, SparseArray<LocalPlayer>, SparseArray<Interpolated>>;
    Zip zipped(registry.getComponents<Transform>(), registry.getComponents<LocalPlayer>(),
        registry.getComponents<Interpolated>());
    const float vx = (static_cast<float>(keys.right) - static_cast<float>(keys.left)) * config::PLAYER_SPEED;
    const float vy = (static_cast<float>(keys.down) - static_cast<float>(keys.up)) * config::PLAYER_SPEED;
    const bool moving = vx != 0.0f || vy != 0.0f;

    for (Zip::iterator it = zipped.begin(); it != zipped.end(); ++it) {
        Transform &transform = std::get<0>(*it);
        LocalPlayer &local = std::get<1>(*it);
        const Interpolated &interpolated = std::get<2>(*it);

        transform.x = std::clamp(transform.x + vx * dt, 0.0f, config::FIELD_WIDTH - 1.0f);
        transform.y = std::clamp(transform.y + vy * dt, 0.0f, config::FIELD_HEIGHT - 1.0f);
        local.idleTime = moving ? 0.0f : local.idleTime + dt;
        if (interpolated.buffer.empty())
            continue;

        const interp::Point server = interpolated.buffer.latest();
        const float dx = server.x - transform.x;
        const float dy = server.y - transform.y;
        if (std::hypot(dx, dy) > config::SNAP_DISTANCE) {
            transform.x = server.x;
            transform.y = server.y;
        } else if (!moving && local.idleTime > config::IDLE_BEFORE_RECONCILE && interpolated.buffer.still()) {
            const float k = 1.0f - std::exp(-config::RECONCILE_RATE * dt);
            transform.x += dx * k;
            transform.y += dy * k;
        }
    }
}

void ShipAnimationSystem::operator()(Registry &registry) const {
    using Zip = Containers::Zipper<SparseArray<Transform>, SparseArray<Renderable>, SparseArray<ShipAnimation>>;
    Zip zipped(registry.getComponents<Transform>(), registry.getComponents<Renderable>(),
        registry.getComponents<ShipAnimation>());

    for (Zip::iterator it = zipped.begin(); it != zipped.end(); ++it) {
        const Transform &transform = std::get<0>(*it);
        Renderable &renderable = std::get<1>(*it);
        ShipAnimation &ship = std::get<2>(*it);
        const float dy = transform.y - ship.lastY;
        const int direction = dy < -0.2f ? -1 : (dy > 0.2f ? 1 : 0);

        ship.lastY = transform.y;
        if (direction == ship.direction)
            continue;
        if (direction != 0) {
            renderable.animation = direction < 0 ? ship.up : ship.down;
            renderable.reversed = false;
        } else {
            renderable.reversed = true;
        }
        renderable.loop = false;
        renderable.time = 0.0;
        ship.direction = direction;
    }
}

void ChargeEffectSystem::operator()(Registry &registry, const Keys &keys, float dt) const {
    using PlayerZip = Containers::Zipper<const SparseArray<Transform>, const SparseArray<LocalPlayer>>;
    using EffectZip = Containers::Zipper<SparseArray<Transform>, SparseArray<Renderable>, SparseArray<ChargeEffect>>;
    const SparseArray<Transform> &transforms = registry.getComponents<Transform>();
    const SparseArray<LocalPlayer> &locals = registry.getComponents<LocalPlayer>();
    PlayerZip players(transforms, locals);
    std::optional<Transform> player;

    for (PlayerZip::iterator it = players.begin(); it != players.end(); ++it) {
        player = std::get<0>(*it);
        break;
    }
    EffectZip zipped(registry.getComponents<Transform>(), registry.getComponents<Renderable>(),
        registry.getComponents<ChargeEffect>());
    for (EffectZip::iterator it = zipped.begin(); it != zipped.end(); ++it) {
        Transform &transform = std::get<0>(*it);
        Renderable &renderable = std::get<1>(*it);
        ChargeEffect &charge = std::get<2>(*it);

        if (!player || !keys.shoot) {
            charge.pressed = false;
            charge.held = 0.0f;
            renderable.visible = false;
            continue;
        }
        transform.x = player->x + 30.0f;
        transform.y = player->y - 5.0f;
        if (!charge.pressed) {
            charge.pressed = true;
            charge.held = 0.0f;
            renderable.time = 0.0;
        }
        charge.held += dt;
        renderable.visible = charge.held > 0.25f;
    }
}

void AnimationSystem::operator()(Registry &registry, float dt) const {
    for (auto &slot : registry.getComponents<Renderable>())
        if (slot)
            slot->time += dt;
}

void RenderSystem::operator()(Registry &registry, sf::RenderWindow &window) const {
    using Zip = Containers::Zipper<const SparseArray<Transform>, const SparseArray<Renderable>>;
    const SparseArray<Transform> &transforms = registry.getComponents<Transform>();
    const SparseArray<Renderable> &renderables = registry.getComponents<Renderable>();
    Zip zipped(transforms, renderables);
    struct Item {
        int layer;
        std::size_t index;
        const Transform *transform;
        const Renderable *renderable;
    };
    std::vector<Item> items;

    for (Zip::iterator it = zipped.begin(); it != zipped.end(); ++it) {
        const Renderable &renderable = std::get<1>(*it);
        if (renderable.visible && renderable.animation)
            items.push_back({renderable.layer, it.getIndex(), &transforms[it.getIndex()].value(), &renderable});
    }
    std::sort(items.begin(), items.end(), [](const Item &a, const Item &b) {
        return a.layer != b.layer ? a.layer < b.layer : a.index < b.index;
    });
    for (const Item &item : items) {
        const Renderable &renderable = *item.renderable;
        const rtype::Frame &frame = renderable.animation->frameAt(renderable.time, renderable.loop, renderable.reversed);
        sf::Sprite sprite(renderable.animation->texture());

        sprite.setTextureRect(sf::IntRect(
            {static_cast<int>(frame.x), static_cast<int>(frame.y)},
            {static_cast<int>(frame.w), static_cast<int>(frame.h)}));
        sprite.setPosition({std::round(item.transform->x), std::round(item.transform->y)});
        window.draw(sprite);
    }
}

// ui

void UiInputSystem::operator()(Registry &registry, UiState &ui, const FrameInput &input) const {
    using InputZip = Containers::Zipper<SparseArray<Transform>, SparseArray<UiBox>, SparseArray<UiInput>>;
    using ButtonZip = Containers::Zipper<SparseArray<Transform>, SparseArray<UiBox>, SparseArray<UiButton>>;
    std::function<void()> action;

    ui.mouse = input.mouse;
    InputZip inputs(registry.getComponents<Transform>(), registry.getComponents<UiBox>(),
        registry.getComponents<UiInput>());

    if (input.clicked) {
        ui.focus.reset();
        for (InputZip::iterator it = inputs.begin(); it != inputs.end(); ++it) {
            const Transform &transform = std::get<0>(*it);
            const UiBox &box = std::get<1>(*it);
            if (sf::FloatRect({transform.x, transform.y}, box.size).contains(input.mouse))
                ui.focus = Entity(it.getIndex());
        }
        ButtonZip buttons(registry.getComponents<Transform>(), registry.getComponents<UiBox>(),
            registry.getComponents<UiButton>());
        for (ButtonZip::iterator it = buttons.begin(); it != buttons.end(); ++it) {
            const Transform &transform = std::get<0>(*it);
            const UiBox &box = std::get<1>(*it);
            const UiButton &button = std::get<2>(*it);
            if (button.enabled && sf::FloatRect({transform.x, transform.y}, box.size).contains(input.mouse))
                action = button.onClick;
        }
    }
    if (input.tab) {
        std::vector<std::size_t> order;
        for (InputZip::iterator it = inputs.begin(); it != inputs.end(); ++it)
            order.push_back(it.getIndex());
        if (!order.empty()) {
            auto next = ui.focus ? std::upper_bound(order.begin(), order.end(), ui.focus->getId()) : order.begin();
            ui.focus = Entity(next == order.end() ? order.front() : *next);
        }
    }
    if (ui.focus && registry.hasComponent<UiInput>(*ui.focus)) {
        UiInput &field = *registry.getComponents<UiInput>()[ui.focus->getId()];

        for (char character : input.typed) {
            if (field.value.size() >= field.maxLength)
                break;
            if (field.digitsOnly && (character < '0' || character > '9'))
                continue;
            field.value.push_back(character);
        }
        if (input.backspace && !field.value.empty())
            field.value.pop_back();
    }
    if (action)
        action();
}

void UiRenderSystem::operator()(Registry &registry, sf::RenderWindow &window, const sf::Font &font, const UiState &ui) const {
    using BoxZip = Containers::Zipper<const SparseArray<Transform>, const SparseArray<UiBox>>;
    using TextZip = Containers::Zipper<const SparseArray<Transform>, const SparseArray<UiText>>;
    const SparseArray<Transform> &transforms = registry.getComponents<Transform>();
    const SparseArray<UiBox> &boxes = registry.getComponents<UiBox>();
    const SparseArray<UiText> &texts = registry.getComponents<UiText>();
    const SparseArray<UiInput> &inputs = registry.getComponents<UiInput>();
    const SparseArray<UiButton> &buttons = registry.getComponents<UiButton>();

    BoxZip boxZip(transforms, boxes);
    for (BoxZip::iterator it = boxZip.begin(); it != boxZip.end(); ++it) {
        const std::size_t index = it.getIndex();
        const Transform &transform = std::get<0>(*it);
        const UiBox &box = std::get<1>(*it);
        sf::RectangleShape shape(box.size);
        sf::Color fill = box.fill;
        sf::Color outline = box.outline;

        if (index < buttons.size() && buttons[index]) {
            const bool hover = sf::FloatRect({transform.x, transform.y}, box.size).contains(ui.mouse);
            if (!buttons[index]->enabled) {
                fill = sf::Color(50, 50, 55);
                outline = sf::Color(70, 70, 75);
            } else if (hover) {
                fill = sf::Color(std::min(255, fill.r + 30), std::min(255, fill.g + 30), std::min(255, fill.b + 30));
            }
        }
        if (ui.focus && ui.focus->getId() == index)
            outline = sf::Color(110, 170, 255);
        shape.setPosition({transform.x, transform.y});
        shape.setFillColor(fill);
        shape.setOutlineColor(outline);
        shape.setOutlineThickness(2.0f);
        window.draw(shape);
    }

    TextZip textZip(transforms, texts);
    for (TextZip::iterator it = textZip.begin(); it != textZip.end(); ++it) {
        const std::size_t index = it.getIndex();
        const Transform &transform = std::get<0>(*it);
        const UiText &label = std::get<1>(*it);
        std::string content = label.text;
        sf::Color color = label.color;

        if (index < inputs.size() && inputs[index]) {
            const bool focused = ui.focus && ui.focus->getId() == index;
            if (inputs[index]->value.empty() && !focused) {
                content = inputs[index]->placeholder;
                color = sf::Color(120, 120, 130);
            } else {
                content = inputs[index]->value;
                if (focused && std::fmod(ui.time, 1.0f) < 0.5f)
                    content += '|';
            }
            if (index < boxes.size() && boxes[index]) {
                const float maxTextWidth = std::max(0.0f, boxes[index]->size.x - 24.0f);
                while (content.size() > 1) {
                    sf::Text measure(font, sf::String::fromUtf8(content.begin(), content.end()), label.size);
                    if (measure.getLocalBounds().size.x <= maxTextWidth)
                        break;
                    content.erase(0, 1);
                }
            }
        }
        sf::Text text(font, sf::String::fromUtf8(content.begin(), content.end()), label.size);
        const sf::FloatRect bounds = text.getLocalBounds();
        float x = transform.x;
        float y = transform.y;

        text.setFillColor(color);
        if (index < boxes.size() && boxes[index]) {
            const sf::Vector2f &size = boxes[index]->size;
            y += (size.y - font.getLineSpacing(label.size)) / 2.0f;
            x += label.centered ? (size.x - bounds.size.x) / 2.0f - bounds.position.x : 12.0f;
        } else if (label.centered) {
            x -= bounds.size.x / 2.0f + bounds.position.x;
        }
        text.setPosition({std::round(x), std::round(y)});
        window.draw(text);
    }
}

}
