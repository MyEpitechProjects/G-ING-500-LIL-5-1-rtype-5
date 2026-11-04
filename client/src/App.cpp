#include "App.hpp"
#include "Constants.hpp"
#include <algorithm>
#include <iostream>

namespace {
    constexpr std::size_t VISIBLE_ROWS = 6;
    const sf::Color BLUE(50, 110, 200);
    const sf::Color GRAY(160, 160, 170);
    const sf::Color RED(235, 90, 90);
}

rtype::App::App(Options options)
    : _options(std::move(options)), _assets("assets.pak"), _resources(_assets), _sync(_resources) {
    if (!_assets.is_valid())
        throw std::runtime_error("assets.pak not found");
    _window.create(sf::VideoMode({config::WINDOW_WIDTH, config::WINDOW_HEIGHT}), "R-Type");
    _window.setVerticalSyncEnabled(true);
    registerComponents();
    _autoPending = _options.autoConnect;
    setScene(Scene::Connect);
    if (_options.autoConnect)
        connectClicked();
}

void rtype::App::registerComponents() {
    _engine.registerComponent<ecs::Transform>();
    _engine.registerComponent<ecs::Renderable>();
    _engine.registerComponent<ecs::NetId>();
    _engine.registerComponent<ecs::Interpolated>();
    _engine.registerComponent<ecs::LocalPlayer>();
    _engine.registerComponent<ecs::ShipAnimation>();
    _engine.registerComponent<ecs::ChargeEffect>();
    _engine.registerComponent<ecs::UiBox>();
    _engine.registerComponent<ecs::UiText>();
    _engine.registerComponent<ecs::UiInput>();
    _engine.registerComponent<ecs::UiButton>();
    _engine.registerComponent<ecs::UiRow>();
}


void rtype::App::clearScene() {
    const std::vector<Entity> entities = _engine.getEntities();

    for (const Entity &entity : entities)
        _engine.killEntity(entity);
    _ui.focus.reset();
    _addressField.reset();
    _portField.reset();
    _connectButton.reset();
    _statusLabel.reset();
    _gameOverLabel.reset();
}

void rtype::App::setScene(Scene scene) {
    clearScene();
    _scene = scene;
    _seenState = _net.state();
    _seenRoomsVersion = _net.roomsVersion();
    switch (scene) {
        case Scene::Connect:
            buildConnect();
            break;
        case Scene::Lobby:
            _scroll = 0;
            _seenScroll = 0;
            buildLobby();
            break;
        case Scene::Game:
            buildGame();
            break;
    }
}

Entity rtype::App::addLabel(float x, float y, const std::string &text, unsigned size, sf::Color color, bool centered) {
    Registry &registry = _engine.getRegistry();
    Entity entity = registry.spawnEntity();

    registry.addComponent(entity, ecs::Transform{x, y});
    registry.addComponent(entity, ecs::UiText{text, size, color, centered});
    return entity;
}

Entity rtype::App::addButton(float x, float y, float width, float height, const std::string &label,
    sf::Color fill, std::function<void()> onClick, bool enabled) {
    Registry &registry = _engine.getRegistry();
    Entity entity = registry.spawnEntity();

    registry.addComponent(entity, ecs::Transform{x, y});
    registry.addComponent(entity, ecs::UiBox{{width, height}, fill, sf::Color(150, 190, 255)});
    registry.addComponent(entity, ecs::UiText{label, 18, sf::Color::White, true});
    registry.addComponent(entity, ecs::UiButton{std::move(onClick), enabled});
    return entity;
}

Entity rtype::App::addInput(float x, float y, float width, float height, const std::string &value,
    const std::string &placeholder, bool digitsOnly, std::size_t maxLength) {
    Registry &registry = _engine.getRegistry();
    Entity entity = registry.spawnEntity();

    registry.addComponent(entity, ecs::Transform{x, y});
    registry.addComponent(entity, ecs::UiBox{{width, height}});
    registry.addComponent(entity, ecs::UiText{"", 20, sf::Color::White, false});
    registry.addComponent(entity, ecs::UiInput{value, placeholder, maxLength, digitsOnly});
    return entity;
}

void rtype::App::setLabel(Entity entity, const std::string &text, sf::Color color) {
    ecs::UiText &label = *_engine.getComponents<ecs::UiText>()[entity.getId()];

    label.text = text;
    label.color = color;
}

std::string rtype::App::fieldValue(const std::optional<Entity> &field) {
    if (!field || !_engine.hasComponent<ecs::UiInput>(*field))
        return {};
    return _engine.getComponents<ecs::UiInput>()[field->getId()]->value;
}

void rtype::App::buildConnect() {
    addLabel(400, 55, "R-TYPE", 56, sf::Color::White, true);
    addLabel(400, 135, "Connect to server", 20, GRAY, true);
    addLabel(200, 195, "Server's address", 16, GRAY, false);
    _addressField = addInput(200, 220, 400, 44, _options.address, "127.0.0.1", false, 40);
    addLabel(200, 285, "Port", 16, GRAY, false);
    _portField = addInput(200, 310, 400, 44, _options.port, "4242", true, 5);
    _connectButton = addButton(300, 385, 200, 48, "Login", BLUE, [this]() { connectClicked(); });
    _statusLabel = addLabel(400, 455, "", 18, GRAY, true);
    _ui.focus = _addressField;
}

void rtype::App::buildLobby() {
    addLabel(400, 35, "Rooms list", 40, sf::Color::White, true);
    addLabel(400, 92, "Server " + _net.serverName(), 16, GRAY, true);
    _statusLabel = addLabel(400, 125, "", 16, RED, true);
    addButton(250, 525, 300, 48, "Create new room", BLUE, [this]() { _net.createRoom(); },
        _net.state() == rtype::ClientNet::State::Lobby);
    rebuildRooms();
    if (_autoPending) {
        _autoPending = false;
        if (_options.autoJoin != 0)
            _net.joinRoom(_options.autoJoin);
        else if (_options.autoCreate)
            _net.createRoom();
    }
}

void rtype::App::rebuildRooms() {
    Registry &registry = _engine.getRegistry();
    const SparseArray<ecs::UiRow> &marks = registry.getComponents<ecs::UiRow>();
    std::vector<Entity> stale;

    for (std::size_t index = 0; index < marks.size(); ++index)
        if (marks[index])
            stale.emplace_back(index);
    for (const Entity &entity : stale)
        registry.killEntity(entity);

    const auto &rooms = _net.rooms();
    const bool canJoin = _net.state() == rtype::ClientNet::State::Lobby;
    const std::size_t maxScroll = rooms.size() > VISIBLE_ROWS ? rooms.size() - VISIBLE_ROWS : 0;

    _scroll = std::min(_scroll, maxScroll);
    _seenScroll = _scroll;
    _seenRoomsVersion = _net.roomsVersion();
    _seenState = _net.state();
    if (rooms.empty()) {
        registry.addComponent(addLabel(400, 290, "No room right now", 20, GRAY, true), ecs::UiRow{});
        registry.addComponent(addLabel(400, 322, "Create one with this button", 16, GRAY, true), ecs::UiRow{});
        return;
    }
    for (std::size_t row = 0; row < VISIBLE_ROWS && _scroll + row < rooms.size(); ++row) {
        const rtype::ClientNet::Room &room = rooms[_scroll + row];
        const float y = 165.0f + static_cast<float>(row) * 56.0f;
        const std::string players = std::to_string(room.players) + (room.players > 1 ? " players" : " player");
        const std::uint32_t id = room.id;
        Entity box = registry.spawnEntity();

        registry.addComponent(box, ecs::Transform{100, y});
        registry.addComponent(box, ecs::UiBox{{600, 48}});
        registry.addComponent(box, ecs::UiText{"Room #" + std::to_string(id) + "   -   " + players, 20, sf::Color::White, false});
        registry.addComponent(box, ecs::UiRow{});
        registry.addComponent(addButton(574, y + 6, 116, 36, "Join", BLUE,
            [this, id]() { _net.joinRoom(id); }, canJoin), ecs::UiRow{});
    }
    if (maxScroll > 0)
        registry.addComponent(addLabel(400, 500, "Scroll wheel: scroll (" + std::to_string(_scroll + 1) + "-"
            + std::to_string(std::min(_scroll + VISIBLE_ROWS, rooms.size())) + " / " + std::to_string(rooms.size()) + ")",
            14, GRAY, true), ecs::UiRow{});
}

void rtype::App::buildGame() {
    _sync.reset();
    _sentKeys = {};
    _window.setTitle("R-Type - room " + std::to_string(_net.instanceId()));
    addLabel(8, 6, "Room #" + std::to_string(_net.instanceId()), 14, GRAY, false);
}

void rtype::App::connectClicked() {
    std::string error;

    if (_addressField)
        _options.address = fieldValue(_addressField);
    if (_portField)
        _options.port = fieldValue(_portField);
    _formError.clear();
    if (!_net.connect(_options.address, _options.port, error))
        _formError = error;
}

void rtype::App::updateConnect(bool enter) {
    using State = rtype::ClientNet::State;
    const State state = _net.state();

    if (state == State::Lobby) {
        setScene(Scene::Lobby);
        return;
    }
    if (enter && state != State::Connecting)
        connectClicked();
    if (_connectButton)
        _engine.getComponents<ecs::UiButton>()[_connectButton->getId()]->enabled = _net.state() != State::Connecting;
    if (!_statusLabel)
        return;
    if (!_formError.empty())
        setLabel(*_statusLabel, _formError, RED);
    else if (_net.state() == State::Connecting)
        setLabel(*_statusLabel, "Connecting to " + _net.serverName() + "...", GRAY);
    else if (_net.state() == State::Failed)
        setLabel(*_statusLabel, _net.error(), RED);
    else
        setLabel(*_statusLabel, "", GRAY);
}

void rtype::App::updateLobby(const ecs::FrameInput &input) {
    using State = rtype::ClientNet::State;
    const State state = _net.state();

    if (state == State::InGame) {
        setScene(Scene::Game);
        return;
    }
    if (state == State::Failed || state == State::Idle) {
        _formError = _net.error();
        setScene(Scene::Connect);
        return;
    }
    if (input.wheel != 0.0f) {
        const std::size_t rooms = _net.rooms().size();
        const std::size_t maxScroll = rooms > VISIBLE_ROWS ? rooms - VISIBLE_ROWS : 0;
        if (input.wheel > 0.0f)
            _scroll = _scroll > 0 ? _scroll - 1 : 0;
        else
            _scroll = std::min(_scroll + 1, maxScroll);
    }
    if (_net.roomsVersion() != _seenRoomsVersion || state != _seenState || _scroll != _seenScroll) {
        rebuildRooms();
        const bool canJoin = state == State::Lobby;
        for (auto &slot : _engine.getComponents<ecs::UiButton>())
            if (slot)
                slot->enabled = canJoin;
    }
    if (_statusLabel) {
        if (state == State::Joining)
            setLabel(*_statusLabel, "Connecting to the room...", GRAY);
        else
            setLabel(*_statusLabel, _net.error(), RED);
    }
}

void rtype::App::updateGame(float dt) {
    using State = rtype::ClientNet::State;

    if (_net.state() == State::Failed) {
        _formError = _net.error();
        _window.setTitle("R-Type");
        setScene(Scene::Connect);
        return;
    }
    _keys = readKeys();
    if (_keys != _sentKeys) {
        _net.sendInput(ecs::toButtons(_keys));
        _sentKeys = _keys;
    }
    _engine.applySystem(_sync, _net);
    _engine.applySystem(ecs::InterpolationSystem{}, _sync.renderTime(_net.now()));
    _engine.applySystem(ecs::PredictionSystem{}, _keys, dt);
    _engine.applySystem(ecs::ShipAnimationSystem{});
    _engine.applySystem(ecs::ChargeEffectSystem{}, _keys, dt);
    _engine.applySystem(ecs::AnimationSystem{}, dt);
    if (_sync.localPlayerDead() && !_gameOverLabel) {
        _gameOverLabel = addLabel(400, 270, "Game Over", 32, RED, true);
        addLabel(400, 320, "Escape to quit", 18, GRAY, true);
    }
}

rtype::ecs::Keys rtype::App::readKeys() const {
    ecs::Keys keys;

    if (!_window.hasFocus())
        return keys;
    keys.up = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
    keys.down = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
    keys.left = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
    keys.right = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
    keys.shoot = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X);
    return keys;
}

rtype::ecs::FrameInput rtype::App::pollEvents(bool &enter) {
    ecs::FrameInput input;

    enter = false;
    while (const std::optional event = _window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            _window.close();
        } else if (const auto *key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape)
                _window.close();
            else if (key->code == sf::Keyboard::Key::Backspace)
                input.backspace = true;
            else if (key->code == sf::Keyboard::Key::Tab)
                input.tab = true;
            else if (key->code == sf::Keyboard::Key::Enter)
                enter = true;
        } else if (const auto *text = event->getIf<sf::Event::TextEntered>()) {
            if (text->unicode >= 32 && text->unicode < 127)
                input.typed.push_back(static_cast<char>(text->unicode));
        } else if (const auto *click = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (click->button == sf::Mouse::Button::Left)
                input.clicked = true;
        } else if (const auto *wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
            input.wheel += wheel->delta;
        }
    }
    input.mouse = _window.mapPixelToCoords(sf::Mouse::getPosition(_window));
    return input;
}

void rtype::App::run() {
    sf::Clock clock;

    while (_window.isOpen()) {
        bool enter = false;
        const ecs::FrameInput input = pollEvents(enter);
        const float dt = std::min(clock.restart().asSeconds(), 0.1f);

        _ui.time += dt;
        _net.update();
        _engine.applySystem(ecs::UiInputSystem{}, _ui, input);
        switch (_scene) {
            case Scene::Connect:
                updateConnect(enter);
                break;
            case Scene::Lobby:
                updateLobby(input);
                break;
            case Scene::Game:
                updateGame(dt);
                break;
        }
        _window.clear(sf::Color(30, 30, 30));
        _engine.applySystem(ecs::RenderSystem{}, _window);
        _engine.applySystem(ecs::UiRenderSystem{}, _window, _resources.font(), _ui);
        _window.display();
    }
}
