#ifndef COMPONENTS_HPP
    #define COMPONENTS_HPP

    #include <iostream>

struct Position {
    int x;
    int y;
};

struct Velocity {
    int x;
    int y;
};

struct Drawable {
    int color;
};

struct Controllable {
    bool up;
    bool down;
    bool right;
    bool left;
    bool fire;
};

struct Health {
    int value;
};

struct Enemy {
};

struct Projectile {
    bool fromPlayer;
};

struct CoolDown {
    double shootCooldown;
};

struct Player {
    int id;
};

#endif 
