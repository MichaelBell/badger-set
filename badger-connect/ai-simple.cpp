#include <stdlib.h>
#include <algorithm>

#include "game.hpp"

using std::min, std::max;

namespace
{
  enum MoveValue {
    FULL,    // Column is full - can't play
    LOSS,    // Known that playing here results in a loss
    UNKNOWN, // Unknown
    VALID_3, // Results in 3 in a row with a space
    WIN_NEXT,// Results in guaranteed win
    BLOCK,   // Blocks a guaranteed win
    WIN      // Wins
  };

  using Piece = GameState::Piece;

  MoveValue get_value_of_column(int col, const GameState& state)
  {
    int row;
    Piece my_piece = state.get_cur_piece();
    GameState next_state = state.get_state_if_drop(col, row);

    MoveValue value = UNKNOWN;
    int next_col_for_win_3 = -1;

    // Vertical
    if (row < Game::HEIGHT - 2)
    {
      int count = 1;
      for (int j = row + 1; j < Game::HEIGHT; ++j)
      {
        if (next_state.get(col, j) != my_piece)
          break;
        ++count;
      }
      if (count == 4) 
        return WIN;

      if (count == 3 && row != 0)
      {        
        value = VALID_3;
        next_col_for_win_3 = col;
      }
    }

    // Horizontal
    {
      int count = 1;
      int space_col[2] = { -1, -1 };
      for (int i = col - 1; i >= max(0, col - 3); --i)
      {
        Piece p = next_state.get(i, row);
        if (p != my_piece)
        {
          if (p == Piece::None) 
            space_col[0] = i;
          break;
        }
        ++count;
      }
      for (int i = col + 1; i < min(Game::WIDTH, col + 4); ++i)
      {
        Piece p = next_state.get(i, row);
        if (p != my_piece)
        {
          if (p == Piece::None) 
            space_col[1] = i;
          break;
        }
        ++count;
      }
      
      if (count == 4)
        return WIN;

      if (count == 3)
      {
        if (space_col[0] != -1 && space_col[1] != -1)
        {
          value = WIN_NEXT;
        }
        else
        {
          int free_col = space_col[0] + space_col[1] + 1;
          if (free_col >= 0)
          {
            if (free_col != next_col_for_win_3)
            {
              value = WIN_NEXT;
            }
            else
            {
              value = VALID_3;
              next_col_for_win_3 = free_col;
            }
          }
        }
      }
    }


    // TODO Diagonal
    if (next_state.check_for_win() != Piece::None)
    {
      return WIN;
    }

    // Check for block
    {
      GameState state_copy(state);
      state_copy.switch_cur_piece();
      state_copy.drop_piece(col);
      if (state_copy.check_for_win() != Piece::None)
      {
        return BLOCK;
      }
    }

    // Check for allowing a win
    next_state.drop_piece(col);
    if (next_state.check_for_win() != Piece::None)
    {
      return LOSS;
    }

    return value;
  }
}


int ai_simple_choose_column(const GameState& state)
{
  MoveValue value[Game::WIDTH];

  for (int col = 0; col < Game::WIDTH; ++col)
  {
    if (state.get(col, 0) == Piece::None)
    {
      value[col] = get_value_of_column(col, state);
    }
    else
    {
      value[col] = FULL;
    }
  }

  for (MoveValue test_value = MoveValue::WIN; (int)test_value > (int)MoveValue::FULL; test_value = MoveValue((int)test_value - 1))
  {
    int valid_cols[Game::WIDTH];
    int num_valid = 0;
    for (int col = 0; col < Game::WIDTH; ++col)
    {
      if (value[col] == test_value)
      {
        valid_cols[num_valid++] = col;
      }
    }
    if (num_valid > 0)
    {
      return valid_cols[rand() % num_valid];
    }
  }

  return rand() % 7;
}
