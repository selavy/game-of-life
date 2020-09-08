#include "golife.h"
#include <iostream>
#include <cassert>

namespace gol {

Board::Board(int xs, int ys) noexcept
    : nrows{xs}, ncols{ys}, brd(nrows * ncols, DEAD) {}

int Board::ix(int x, int y) const noexcept
{
    assert(0 <= x && x < ncols);
    assert(0 <= y && y < nrows);
    return y*ncols + x;
}

bool Board::live(int x, int y) const noexcept
{
    return brd[ix(x, y)] == LIVE;
}

bool Board::dead(int x, int y) const noexcept
{
    return brd[ix(x, y)] == DEAD;
}

void Board::set_live(int x, int y) noexcept
{
    brd[ix(x, y)] = LIVE;
}

void Board::set_dead(int x, int y) noexcept
{
    brd[ix(x, y)] = DEAD;
}

Board Board::tick() const noexcept
{
    Board nb = *this;
    for (int y = 0; y < nrows; ++y) {
        for (int x = 0; x < ncols; ++x) {
            const int ns = live_neighbors(x, y);
            if (ns == 3) {
                nb.set_live(x, y);
            } else if (live(x, y) && ns == 2) {
                nb.set_live(x, y);
            } else {
                nb.set_dead(x, y);
            }
        }
    }
    return nb;
}

int Board::live_neighbors(const int x, const int y) const noexcept
{
    int result = 0;
    for (int xoff : { -1, 0, 1 }) {
        for (int yoff : { -1, 0, 1 }) {
            const int xcur = x + xoff;
            const int ycur = y + yoff;
            if (xoff == 0 && yoff == 0) {
                continue;
            }
            if (!(0 <= xcur && xcur < ncols)) {
                continue;
            }
            if (!(0 <= ycur && ycur < nrows)) {
                continue;
            }
            result += live(xcur, ycur) ? 1 : 0;
        }
    }
    return result;
}

std::ostream& operator<<(std::ostream& os, const Board& b)
{
    const std::string live(" * ");
    const std::string dead("   ");
    const std::string rowdiv(4*b.ncols+1, '-');
    os << rowdiv << "\n";
    for (int y = 0; y < b.nrows; ++y) {
        for (int x = 0; x < b.ncols; ++x) {
            os << "|" << (b.live(x, y) ? live : dead);
        }
        os << "|\n" << rowdiv << "\n";
    }
    return os;
}

} // namespace gol
