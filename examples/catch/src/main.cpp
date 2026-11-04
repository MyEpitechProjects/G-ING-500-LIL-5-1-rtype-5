/*
** EPITECH PROJECT, 2026
** main
** File description:
** Catch demo entry point
*/

#include "Catch.hpp"

#include <chrono>
#include <thread>

int main() {
    demo::Catch game;
    std::chrono::steady_clock::time_point nextFrame = std::chrono::steady_clock::now();

    while (game.isRunning()) {
        game.update();
        nextFrame += std::chrono::microseconds(16667);
        std::this_thread::sleep_until(nextFrame);
    }
    return 0;
}
