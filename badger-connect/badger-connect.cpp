#include "pico/stdlib.h"
#include <stdio.h>
#include <cstring>
#include <string>
#include <algorithm>
#include "pico/time.h"
#include "pico/platform.h"

#include "common/pimoroni_common.hpp"
#include "lpbadger.hpp"
//#include "images/badger_crop.h"
#include "images/duck_crop.h"

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
    void draw_piece(Piece piece, int x, int y, bool clear = false);
    void select_column();
    bool drop_piece(int i);
    Piece get_cur_piece() { return cur_piece; }
    void switch_cur_piece();
    Piece check_for_win();

    bool restore_state();
    void save_state();
    void clear_state();

  private:
    static constexpr int piece_size = 16;
    static constexpr int xoffset = 92;
    constexpr int getx(int i) { return xoffset + i * piece_size; }
    constexpr int gety(int j) { return j * piece_size; }
    Piece board[7][7] = {};
    Piece cur_piece = Square;
};

LowPowerBadger badger;
Board board;

void Board::draw()
{
  badger.fast_clear();
  badger.image(duck_crop, 88, 125, 0, 1);

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

void Board::draw_piece(Piece piece, int x, int y, bool clear)
{
  if (clear) {
    badger.pen(15);
    badger.rectangle(x+1, y+1, 15, 15);
    badger.pen(0);
  }

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

void Board::select_column()
{
  int i = 3;

  badger.rectangle(getx(i)+4, gety(7)+4, 8, 2);
  badger.update_speed(3);
  badger.partial_update(getx(i)+4, gety(7), 8, 8, false);

  while(true) {
    if (!badger.wait_for_press(20))
    {
      save_state();
      badger.led(0);
      badger.halt();
    } 
    badger.pen(15);
    badger.rectangle(getx(i)+4, gety(7)+4, 8, 2);
    badger.pen(0);
    int old_i = i;
    if (badger.pressed(badger.A)) {
      if (i > 0) --i;
    }
    else if (badger.pressed(badger.C)) {
      if (i < 6) ++i;
    }
    else if (badger.pressed(badger.B)) {
      if (drop_piece(i)) {
        badger.update_speed(2);
        return;
      }
    }
    else if (badger.pressed(badger.UP)) {
      badger.led(100);
      badger.update_speed(1);
      badger.rectangle(getx(i)+4, gety(7)+4, 8, 2);
      badger.update(true);
      badger.update_speed(3);
      badger.led(200);
    }
    else {
      return;
    }

    if (old_i != i)
    {
      badger.rectangle(getx(i)+4, gety(7)+4, 8, 2);
      badger.wait_for_idle();
      badger.partial_update(getx(min(i, old_i))+4, gety(7), abs(i - old_i) * 16 + 8, 8, false);
      badger.wait_for_no_press();
    }
  }
}

bool Board::drop_piece(int i)
{
  for (int j = 6; j >= 0; --j)
  {
    if (board[j][i] == None)
    {
      board[j][i] = cur_piece;
      draw_piece(cur_piece, getx(i), gety(j));
      return true;
    }
  }

  return false;
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

void Board::switch_cur_piece()
{
  if (cur_piece == Board::Square) cur_piece = Board::Cross;
  else if (cur_piece == Board::Cross) cur_piece = Board::Square;
}

void Board::save_state()
{
  uint8_t data[51];
  memcpy(data, board, 49);
  data[49] = cur_piece;
  data[50] = 0xd2;

  badger.store_persistent_data(data, 51);
}

bool Board::restore_state()
{
  const uint8_t* data = badger.get_persistent_data();
  if (data[50] == 0xd2)
  {
    memcpy(board, data, 49);
    cur_piece = (Piece)data[49];
    return true;
  }

  return false;
}

void Board::clear_state()
{
  uint8_t data[51] = {};
  badger.store_persistent_data(data, 51);

  memset(board, 0, 49);
  cur_piece = Square;
}

int main() {
  badger.init();
  badger.led(100);

  bool restored = false;
  if (!badger.pressed_to_wake(badger.DOWN))
  {
    restored = board.restore_state();
  }

  board.draw();
  board.draw_piece(board.get_cur_piece(), 220, 18);
  if (restored)
  {
    badger.update_speed(2);
    badger.update(true);
  }
  else
  {
    badger.update(true);
    badger.update_speed(2);
  }

  while (true)
  {
    badger.led(200);
    board.select_column();

    if (board.check_for_win() != Board::None) {
      board.draw_piece(Board::None, 220, 16, true);
      badger.partial_update(92, 0, 144, 120, true);
      badger.fast_clear();
      board.draw_piece(board.get_cur_piece(), 100, 54);
      badger.text("wins!", 120, 60);
      badger.update_speed(1);
      badger.update();
      break;
    }

    board.switch_cur_piece();

    if (badger.pressed(badger.DOWN))
    {
      board.clear_state();
      break;
    }

    badger.led(100);
    board.draw_piece(board.get_cur_piece(), 220, 16, true);
    badger.partial_update(92, 0, 144, 120, true);
  }

  board.clear_state();

  badger.led(0);

  if (badger.pressed(badger.DOWN))
  {
    board.draw();
    badger.update_speed(2);
    badger.update();
  }
  badger.wait_for_idle();

  badger.halt();
}

