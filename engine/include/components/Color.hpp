/*
** EPITECH PROJECT, 2026
** Color
** File description:
** Color def
*/

#ifndef COLOR
    #define COLOR

    #include <cstdint>

struct Color {
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
    std::uint8_t alpha = 255;
};

#endif
