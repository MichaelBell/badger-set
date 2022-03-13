#pragma once

#include "common/pimoroni_common.hpp"
#include "lpbadger.hpp"

class GameState {
  public:
    static constexpr int HEIGHT = 7;
    static constexpr int WIDTH = 7;

    enum Piece : uint8_t {
      None,
      Cross,
      Square
    };

    Piece get(int x, int y) const {
      return board[y][x];
    }

    Piece get_cur_piece() const { return cur_piece; }

    Piece check_for_win() const;

    void switch_cur_piece();
    int drop_piece(int i);

    // Copy of current state with a piece dropped in selected row
    GameState get_state_if_drop(int column, int& row) const;

  protected:
    Piece board[HEIGHT][WIDTH] = {};
    Piece cur_piece = Square;
};

class Game : public GameState {
  public:
    Game(LowPowerBadger& badger_)
      : badger(badger_)
    {}

    void switch_ai();
    bool using_ai() const { return use_ai; }
    void draw();
    void draw_piece(Piece piece, int x, int y, bool clear = false);
    void select_column();
    int drop_piece(int i);

    bool restore_state();
    void save_state();
    void clear_state();

    const GameState& get_game_state() { return *this; }

  private:
    static constexpr int piece_size = 16;
    static constexpr int xoffset = 92;
    constexpr int getx(int i) { return xoffset + i * piece_size; }
    constexpr int gety(int j) { return j * piece_size; }
    LowPowerBadger& badger;
    bool use_ai = false;
};

