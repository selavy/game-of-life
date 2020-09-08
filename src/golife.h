#pragma once

#include <vector>
#include <iosfwd>

namespace gol {

struct Board
{
    Board(int nrows, int ncols) noexcept;
    int ix(int x, int y) const noexcept;
    bool live(int x, int y) const noexcept;
    bool dead(int x, int y) const noexcept;
    void set_live(int x, int y) noexcept;
    void set_dead(int x, int y) noexcept;
    Board tick() const noexcept;
    int live_neighbors(int x, int y) const noexcept;

    int nrows;
    int ncols;
    std::vector<int> brd;

    static constexpr int LIVE = 1;
    static constexpr int DEAD = 0;

private:
    friend std::ostream& operator<<(std::ostream& os, const Board& b);
};

} // namespace gol
