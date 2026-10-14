#include "Graphic.hpp"

int main() {
    rtype::Graphic graphic(800, 600, "R-Type", "assets.pak");
    graphic.run();
    return 0;
}