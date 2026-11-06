/*
** EPITECH PROJECT, 2026
** SpriteAnimation
** File description:
** SpriteAnimation def
*/

#ifndef SPRITEANIMATION
    #define SPRITEANIMATION

    #include <string>

struct SpriteAnimation {
    std::string animation;
    int layer;
    double time = 0.0;
    bool loop = true;
    bool reversed = false;
    bool centered = true;
    bool visible = true;
};

#endif
