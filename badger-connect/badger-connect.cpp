#include "pico/stdlib.h"
#include <stdio.h>
#include <cstring>
#include <string>
#include <algorithm>
#include "pico/time.h"
#include "pico/platform.h"

#include "common/pimoroni_common.hpp"
#include "lpbadger.hpp"

#include "examples/badger2040/badger2040_image_demo_images.hpp"

using std::min;
using std::max;

class Board {
  public:
    
    enum Piece : uint8_t {
      None,
      Cross,
      Square
    };

    void draw();
    void draw_piece(Piece piece, int x, int y);
    void select_column(Piece piece);
    void drop_piece(Piece piece, int i);
    Piece check_for_win();

  private:
    static constexpr int piece_size = 16;
    static constexpr int xoffset = 92;
    constexpr int getx(int i) { return xoffset + i * piece_size; }
    constexpr int gety(int j) { return j * piece_size; }
    Piece board[7][7] = {};
};

LowPowerBadger badger;
Board board;

void Board::draw()
{
  badger.pen(15);
  badger.clear();
  badger.pen(0);

  for (int x = getx(0); x <= getx(7); x += piece_size)
  {
    badger.line(x, gety(0), x, gety(7));
  }
  badger.line(getx(0), gety(7), getx(7) + 1, gety(7));

  for (int i = 0; i < 7; ++i)
  {
    for (int j = 0; j < 7; ++j)
    {
      draw_piece(board[j][i], getx(i), gety(j));
    }
  }
}

void Board::draw_piece(Piece piece, int x, int y)
{
  if (piece == None) return;

  if (piece == Cross) {
    badger.line(x + 2, y + 2, x + 15, y + 15);
    badger.line(x + 2, y + 14, x + 15, y + 1);
  }

  if (piece == Square) {
    badger.line(x + 3, y + 3, x + 14, y + 3);
    badger.line(x + 13, y + 3, x + 13, y + 14);
    badger.line(x + 3, y + 13, x + 14, y + 13);
    badger.line(x + 3, y + 3, x + 3, y + 14);
  }
}

void Board::select_column(Piece piece)
{
  int i = 3;

  badger.rectangle(getx(i)+4, gety(7)+4, 8, 2);
  badger.update_speed(3);
  badger.partial_update(getx(i)+4, gety(7), 8, 8, true);

  while(true) {
    badger.wait_for_press();
    badger.pen(15);
    badger.rectangle(getx(i)+4, gety(7)+4, 8, 2);
    badger.pen(0);
    int old_i = i;
    if (badger.pressed(badger.A)) {
      if (i > 0) --i;
    }
    else if (badger.pressed(badger.C)) {
      if (i < 7) ++i;
    }
    else if (badger.pressed(badger.B)) {
      badger.update_speed(2);
      drop_piece(piece, i);
      return;
    }
    else {
      return;
    }

    if (old_i != i)
    {
      badger.rectangle(getx(i)+4, gety(7)+4, 8, 2);
      badger.partial_update(getx(min(i, old_i))+4, gety(7), abs(i - old_i) * 16 + 8, 8, true);
    }
  }
}

void Board::drop_piece(Piece piece, int i)
{
  for (int j = 6; j >= 0; --j)
  {
    if (board[j][i] == None)
    {
      board[j][i] = piece;
      return;
    }
  }
}

Board::Piece Board::check_for_win()
{
  for (int i = 0; i < 7; ++i)
  {
    Piece p = board[6][i];
    int count = 1;
    for (int j = 5; j >= 0 && p != None; --j)
    {
      if (p == board[j][i]) {
        if (++count == 4) {
          return p;
        }
      }
      else {
        p = board[j][i];
        count = 1;
      }
    }
  }

  for (int j = 0; j < 7; ++j)
  {
    Piece p = board[j][0];
    int count = 1;
    for (int i = 1; i < 7; ++i)
    {
      if (p == board[j][i]) {
        if (++count == 4 && p != None) {
          return p;
        }
      }
      else {
        p = board[j][i];
        count = 1;
      }
    }
  }

  for (int sj = 3; sj < 7; ++sj)
  {
    for (int si = 0; si < 4; ++si)
    {
      Piece p = board[sj][si];
      int count = 1;
      for (int x = 1; sj - x >= 0 && si + x < 7; ++x)
      {
        int j = sj - x;
        int i = si + x;
        if (p == board[j][i]) {
          if (++count == 4 && p != None) {
            return p;
          }
        }
        else {
          p = board[j][i];
          count = 1;
        }
      }
    }
  }

  for (int sj = 0; sj < 4; ++sj)
  {
    for (int si = 0; si < 4; ++si)
    {
      Piece p = board[sj][si];
      int count = 1;
      for (int x = 1; sj + x < 7 && si + x < 7; ++x)
      {
        int j = sj + x;
        int i = si + x;
        if (p == board[j][i]) {
          if (++count == 4 && p != None) {
            return p;
          }
        }
        else {
          p = board[j][i];
          count = 1;
        }
      }
    }
  }

  return None;
}



int main() {
  badger.init();
  badger.led(100);

  Board::Piece cur_piece = Board::Square;
  board.draw();
  board.draw_piece(cur_piece, 220, 18);
  badger.update(true);
  badger.update_speed(2);

  while (true)
  {
    badger.led(200);
    board.select_column(cur_piece);

    if (board.check_for_win() != Board::None) {
      badger.pen(15);
      badger.clear();
      badger.pen(0);
      board.draw_piece(cur_piece, 100, 54);
      badger.text("wins!", 120, 60);
      badger.update_speed(1);
      badger.update(true);
      break;
    }

    badger.led(100);

    if (cur_piece == Board::Square) cur_piece = Board::Cross;
    else if (cur_piece == Board::Cross) cur_piece = Board::Square;

    if (badger.pressed(badger.DOWN))
    {
      break;
    }

    board.draw();
    board.draw_piece(cur_piece, 220, 18);
    badger.partial_update(92, 0, 144, 120, true);
  }

  badger.led(0);

  badger.halt();
}

