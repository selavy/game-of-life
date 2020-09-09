#!/usr/bin/env python

import copy


DEAD = '   '
LIVE = ' * '

class Board:
    def __init__(self, nrows, ncols):
        self._nrows = nrows
        self._ncols = ncols
        self._board = [[DEAD for x in range(ncols)] for y in range(nrows)]
        self._rowdiv = '-'*int(ncols*3+6)
        self._rowsep = f'|\n{self._rowdiv}\n|'

    def __str__(self):
        return f'{self._rowdiv}\n|' + self._rowsep.join('|'.join(row) for row in self._board) + '|\n' + self._rowdiv

    def get(self, x, y):
        return self._board[y][x]

    def islive(self, x, y):
        return self.get(x, y) == LIVE

    def isdead(self, x, y):
        return self.get(x, y) == DEAD

    def activate(self, x, y):
        self._board[y][x] = LIVE

    def kill(self, x, y):
        self._board[y][x] = DEAD

    def neighbors(self, x, y):
        offs = (-1, 0, 1)
        for off_x in offs:
            for off_y in offs:
                if off_x == 0 and off_y == 0:
                    continue
                cur_x = x + off_x
                cur_y = y + off_y
                try:
                    _ = self.get(cur_x, cur_y)
                    yield (cur_x, cur_y)
                except IndexError:
                    continue

    def live_neighbors(self, x, y):
        return sum(self.islive(nx, ny) for nx, ny in self.neighbors(x, y))

    def step(self):
        nb = copy.deepcopy(self._board)

        for y in range(self._nrows):
            for x in range(self._ncols):
                ns = self.live_neighbors(x, y)
                # print(f"{x}, {y} = {ns}")
                if ns == 3:
                    # print(f"Activateing {x}, {y}")
                    nb[y][x] = LIVE
                elif self.islive(x, y) and ns == 2:
                    nb[y][x] = LIVE
                else:
                    nb[y][x] = DEAD
        self._board = nb



if __name__ == '__main__':
    M = 5
    N = 5
    b = Board(nrows=M, ncols=N)

    print(b)
    print(b._board)

    b.activate(2, 1)
    b.activate(2, 2)
    b.activate(2, 3)

    for x, y in [
            (2,0),
            (2,1),
            (2,2),
            (2,3),
            (2,4),
            (3,2),
    ]:
        print(f"{x},{y}? {b.islive(x, y)} -> {b.live_neighbors(x, y)}")

    # print(b.live_neighbors(2, 2))

    print(b)
    b.step()
    print(b)
    b.step()
    print(b)
    b.step()
    print(b)
    b.step()
    print(b)
