/*
** EPITECH PROJECT, 2026
** Catch
** File description:
** Catch def
*/

#ifndef CATCH
    #define CATCH

    #include "Engine.hpp"
    #include "PhysicsModule.hpp"
    #include "RenderModule.hpp"
    #include <vector>

namespace demo {
struct Paddle {};
struct Fruit {};
struct Bomb {};

class Catch {
    private:
        int _width;
        int _height;
        float _paddleSpeed;
        int _spawnInterval;

        Engine _engine;
        RenderModule *_window;
        Entity _paddle;
        std::vector<Entity> _lives;
        unsigned int _tick;
        unsigned int _score;
        bool _left;
        bool _right;

        void spawnPaddle();
        void spawnLives(int count);
        void spawnFalling();
        void movePaddle();
        void loseLife();

    public:
        Catch();
        ~Catch() = default;

        Catch(const Catch &) = delete;
        Catch &operator=(const Catch &) = delete;

        bool isRunning() const { return _window->isOpen(); }
        void update();
};
}

#endif
