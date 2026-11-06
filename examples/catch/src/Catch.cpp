/*
** EPITECH PROJECT, 2026
** Catch
** File description:
** Catch
*/

#include "Catch.hpp"
#include "Hitbox.hpp"
#include "Position.hpp"
#include "Shape.hpp"
#include "Velocity.hpp"

#include <algorithm>
#include <iostream>
#include <memory>

demo::Catch::Catch()
    : _width(800), _height(600), _paddleSpeed(7.0f), _spawnInterval(45), _window(nullptr),
    _paddle(_engine.spawnEntity()), _tick(0), _score(0), _left(false), _right(false) {
    std::unique_ptr<PhysicsModule> physics = std::make_unique<PhysicsModule>(-50, -100, _width + 50, _height + 50, 0.1f,
        [this](Entity entity) {
            if (_engine.hasComponent<Fruit>(entity))
                loseLife();
            _engine.killEntity(entity);
        }
    );
    std::unique_ptr<RenderModule> render = std::make_unique<RenderModule>(_width, _height, "Catch - engine demo");

    physics->onCollision<Fruit, Paddle>([this](Registry &registry, Entity fruit, [[maybe_unused]] Entity paddle) {
        registry.killEntity(fruit);
        _score++;
        std::cout << "Score: " << _score << std::endl;
    });
    physics->onCollision<Bomb, Paddle>([this](Registry &registry, Entity bomb, [[maybe_unused]] Entity paddle) {
        registry.killEntity(bomb);
        loseLife();
    });
    _window = render.get();
    render->onEvent([this](const Event &event) {
        if (event.type == EventType::FocusLost) {
            _left = false;
            _right = false;
        }
        if (event.type != EventType::KeyPressed && event.type != EventType::KeyReleased)
            return;
        if (event.key == Key::Escape)
            _window->close();
        if (event.key == Key::Left)
            _left = event.type == EventType::KeyPressed;
        if (event.key == Key::Right)
            _right = event.type == EventType::KeyPressed;
    });
    _engine.addModule(std::move(physics));
    _engine.addModule(std::move(render));
    _engine.registerComponent<Paddle>();
    _engine.registerComponent<Fruit>();
    _engine.registerComponent<Bomb>();
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    spawnPaddle();
    spawnLives(3);
}

void demo::Catch::spawnPaddle() {
    _engine.addComponent(_paddle, Position{_width / 2.0f, _height - 30.0f});
    _engine.addComponent(_paddle, Hitbox{50, 10});
    _engine.addComponent(_paddle, Shape{ShapeType::Rectangle, 100.0f, 20.0f, Color{80, 160, 255}, 1});
    _engine.addComponent(_paddle, Paddle{});
}

void demo::Catch::spawnLives(int count) {
    for (int i = 0; i < count; ++i) {
        Entity life = _engine.spawnEntity();

        _engine.addComponent(life, Position{20.0f + i * 25.0f, 20.0f});
        _engine.addComponent(life, Shape{ShapeType::Rectangle, 16.0f, 16.0f, Color{230, 60, 60}, 2});
        _lives.push_back(life);
    }
}

void demo::Catch::spawnFalling() {
    Entity entity = _engine.spawnEntity();
    float x = static_cast<float>(20 + std::rand() % (_width - 40));
    float speed = 0.5f + static_cast<float>(std::rand() % 21) / 10.0f;

    _engine.addComponent(entity, Position{x, -20.0f});
    _engine.addComponent(entity, Velocity{0.0f, speed});
    _engine.addComponent(entity, Hitbox{10, 10});
    if (std::rand() % 4 == 0) {
        _engine.addComponent(entity, Shape{ShapeType::Rectangle, 20.0f, 20.0f, Color{230, 60, 60}, 1});
        _engine.addComponent(entity, Bomb{});
        return;
    }
    _engine.addComponent(entity, Shape{ShapeType::Circle, 20.0f, 20.0f, Color{90, 220, 90}, 1});
    _engine.addComponent(entity, Fruit{});
}

void demo::Catch::movePaddle() {
    SparseArray<Position> &positions = _engine.getComponents<Position>();
    Position &position = positions[_paddle.getId()].value();

    if (_left)
        position.x -= _paddleSpeed;
    if (_right)
        position.x += _paddleSpeed;
    position.x = std::clamp(position.x, 50.0f, _width - 50.0f);
}

void demo::Catch::loseLife() {
    if (_lives.empty())
        return;
    _engine.killEntity(_lives.back());
    _lives.pop_back();
    if (!_lives.empty())
        return;
    Shape &paddle = _engine.getComponents<Shape>()[_paddle.getId()].value();

    paddle.fill = Color{120, 120, 120};
    std::cout << "Game over - final score: " << _score << std::endl;
}

void demo::Catch::update() {
    _tick++;
    if (!_lives.empty()) {
        movePaddle();
        if (_tick % _spawnInterval == 0)
            spawnFalling();
    }
    _engine.update(1.0 / 60.0);
}
