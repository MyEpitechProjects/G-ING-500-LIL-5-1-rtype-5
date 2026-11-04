/*
** EPITECH PROJECT, 2026
** Shape
** File description:
** Shape def
*/

#ifndef SHAPE
    #define SHAPE

enum class ShapeType {
    Rectangle,
    Circle
};

struct Shape {
    ShapeType type;
    float width;
    float height;
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
    int layer;
};

#endif
