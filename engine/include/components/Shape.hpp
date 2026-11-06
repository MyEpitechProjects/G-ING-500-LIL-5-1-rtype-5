/*
** EPITECH PROJECT, 2026
** Shape
** File description:
** Shape def
*/

#ifndef SHAPE
    #define SHAPE

    #include "Color.hpp"

enum class ShapeType {
    Rectangle,
    Circle
};

struct Shape {
    ShapeType type;
    float width;
    float height;
    Color fill;
    int layer;
    Color outline = {0, 0, 0, 0};
    float outlineThickness = 0.0f;
    bool centered = true;
    bool visible = true;
};

#endif
