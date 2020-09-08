#include <iostream>
#include "golife.h"

int main(int argc, char** argv)
{
    constexpr int nrows = 5;
    constexpr int ncols = 5;
    gol::Board b(nrows, ncols);

    b.set_live(2, 1);
    b.set_live(2, 2);
    b.set_live(2, 3);

    std::cout << b << std::endl;

    b = b.tick();
    std::cout << b << std::endl;
    b = b.tick();
    std::cout << b << std::endl;
    b = b.tick();
    std::cout << b << std::endl;
    b = b.tick();
    std::cout << b << std::endl;

    return 0;
}
