/*
** EPITECH PROJECT, 2026
** Input
** File description:
** Input def
*/

#ifndef INPUT
    #define INPUT

enum class Key {
    Unknown,
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Left,
    Right,
    Up,
    Down,
    Space,
    Enter,
    Escape,
    Backspace,
    Tab
};

enum class MouseButton {
    Left,
    Right,
    Middle
};

enum class EventType {
    Closed,
    FocusLost,
    KeyPressed,
    KeyReleased,
    TextEntered,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseWheelScrolled
};

struct Event {
    EventType type;
    Key key = Key::Unknown;
    char32_t character = 0;
    MouseButton button = MouseButton::Left;
    float x = 0.0f;
    float y = 0.0f;
    float wheel = 0.0f;
};

#endif
