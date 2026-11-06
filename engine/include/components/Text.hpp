/*
** EPITECH PROJECT, 2026
** Text
** File description:
** Text def
*/

#ifndef TEXT
    #define TEXT

    #include "Color.hpp"
    #include <string>

struct Text {
    std::string content;
    std::string font;
    unsigned int size;
    Color color;
    int layer;
    bool centered = false;
    bool visible = true;
};

#endif
