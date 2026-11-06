/*
** EPITECH PROJECT, 2026
** Sprite
** File description:
** Sprite def
*/

#ifndef SPRITE
    #define SPRITE

    #include <string>

struct Sprite {
    std::string texture;
    int left;
    int top;
    int width;
    int height;
    int layer;
    bool centered = true;
    bool visible = true;
};

#endif
