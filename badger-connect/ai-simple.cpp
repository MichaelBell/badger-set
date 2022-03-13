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

  struct MoveData
  {
    MoveValue value;
    uint8_t valid3_space_col;
    bool valid3_forces;
  };

  MoveData get_value_of_column(int col, const GameState& state)
  {
    int row;
    Piece my_piece = state.get_cur_piece();
    GameState next_state = state.get_state_if_drop(col, row);

    MoveData result;
    MoveValue& value = result.value;
    value = UNKNOWN;
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
        return MoveData{ WIN };

      if (count == 3 && row != 0)
      {        
        value = VALID_3;
        result.valid3_space_col = col;
        result.valid3_forces = true;
        next_col_for_win_3 = col;
      }
    }

    // Horizontal and Diagonal
    for (int dir = -1; dir <= 1; ++dir)
    {
      int count = 1;
      int space_col[2] = { -1, -1 };
      bool space_forces[2] = { false, false };
      for (int i = col - 1; i >= max(0, col - 3); --i)
      {
        int j = row + dir * (col - i);
        if (j < 0 || j >= Game::HEIGHT) break;

        Piece p = next_state.get(i, j);
        if (p != my_piece)
        {
          if (p == Piece::None) {
            space_col[0] = i;
            space_forces[0] = (j == Game::HEIGHT - 1 || next_state.get(i, j + 1) != Piece::None);
          }
          break;
        }
        ++count;
      }
      for (int i = col + 1; i < min(Game::WIDTH, col + 4); ++i)
      {
        int j = row - dir * (i - col);
        if (j < 0 || j >= Game::HEIGHT) break;

        Piece p = next_state.get(i, j);
        if (p != my_piece)
        {
          if (p == Piece::None) {
            space_col[1] = i;
            space_forces[1] = (j == Game::HEIGHT - 1 || next_state.get(i, j + 1) != Piece::None);
          }
          break;
        }
        ++count;
      }
      
      if (count >= 4)
        return MoveData{ WIN };

      if (count == 3)
      {
        if (space_col[0] != -1 && space_col[1] != -1 && space_forces[0] && space_forces[1])
        {
          value = WIN_NEXT;
        }
        else 
        {
          for (int i = 0; i < 2; ++i)
          {
            if (space_col[i] != -1 && space_forces[i])
            {
              if (next_col_for_win_3 != -1 && space_col[i] != next_col_for_win_3)
              {
                value = WIN_NEXT;
                break;
              }
              else if (value <= VALID_3)
              {
                value = VALID_3;
                result.valid3_space_col = space_col[i];
                next_col_for_win_3 = space_col[i];
                result.valid3_forces = true;
              }
            }
            else if (value < VALID_3 && space_col[i] != -1)
            {
              value = VALID_3;
              result.valid3_space_col = space_col[i];
              result.valid3_forces = false;
            }
          }
        }
      }
    }

    // Check for block
    {
      GameState state_copy(state);
      state_copy.switch_cur_piece();
      state_copy.drop_piece(col);
      if (state_copy.check_for_win() != Piece::None)
      {
        return MoveData{BLOCK};
      }
    }

    // Check for allowing a win
    next_state.drop_piece(col);
    if (next_state.check_for_win() != Piece::None)
    {
      return MoveData{LOSS};
    }

    return result;
  }

  // Called to choose between multiple columns that would
  // result in row of 3 + a space.  If 
  int choose_from_valid3(int* cols, int num_cols, MoveData* data, const GameState& state, int* debug_move_value)
  {
    MoveValue best_value = LOSS;
    int best_col = cols[0];
    int chosen = 0;
    for (int i = 0; i < num_cols; ++i) {
      if (data->valid3_forces) {
        // Assume we play in this col and then opponent plays in the space
        int row;
        GameState next_state = state.get_state_if_drop(cols[i], row);

        next_state.drop_piece(data->valid3_space_col);
        next_state.switch_cur_piece();
        for (int j = 0; j < Game::WIDTH; ++j) {
          MoveValue value = get_value_of_column(j, next_state).value;
          if ((int)value > (int)best_value) {
            best_value = value;
            best_col = cols[i];
            chosen = 1;
          }
          else if (value == best_value) {
            if ((rand() % ++chosen) == 0) {
              best_col = cols[i];
            }
          }
        }
      }
      else {
        if ((int)UNKNOWN > (int)best_value) {
          best_value = UNKNOWN;
          best_col = cols[i];
          chosen = 1;
        }
        else if (UNKNOWN == best_value) {
          if ((rand() % ++chosen) == 0) {
            best_col = cols[i];
          }
        }
      }
    }

    if (debug_move_value) *debug_move_value |= best_value;

    return best_col;
  }

  int choose_from_unknown(int* cols, int num_cols, MoveData* data, const GameState& state, int* debug_move_value)
  {
    MoveValue best_value = WIN;
    int best_col = cols[0];
    int chosen = 0;
    for (int i = 0; i < num_cols; ++i) {
      // Assume we play in this col then what does that leave opponent with?
      int row;
      GameState next_state = state.get_state_if_drop(cols[i], row);

      MoveValue opp_best_value = LOSS;
      for (int j = 0; j < Game::WIDTH; ++j) {
        MoveValue value = get_value_of_column(j, next_state).value;
        if ((int)value > (int)opp_best_value) 
          opp_best_value = value;
      }
      if ((int)opp_best_value < (int)best_value) {
        best_value = opp_best_value;
        best_col = cols[i];
        chosen = 1;
      }
      else if (opp_best_value == best_value) {
        if ((rand() % ++chosen) == 0) {
          best_col = cols[i];
        }
      }
    }

    if (debug_move_value) *debug_move_value |= best_value;

    return best_col;
  }
}


int ai_simple_choose_column(const GameState& state, int* debug_move_value)
{
  MoveData move_data[Game::WIDTH];

  for (int col = 0; col < Game::WIDTH; ++col)
  {
    if (state.get(col, 0) == Piece::None)
    {
      move_data[col] = get_value_of_column(col, state);
    }
    else
    {
      move_data[col].value = FULL;
    }
  }

  for (MoveValue test_value = MoveValue::WIN; (int)test_value > (int)MoveValue::FULL; test_value = MoveValue((int)test_value - 1))
  {
    if (debug_move_value) *debug_move_value = test_value << 8;

    int valid_cols[Game::WIDTH];
    int num_valid = 0;
    for (int col = 0; col < Game::WIDTH; ++col)
    {
      if (move_data[col].value == test_value)
      {
        valid_cols[num_valid++] = col;
      }
    }
    if (num_valid > 0)
    {
      if (test_value == VALID_3 && num_valid > 1) {
        return choose_from_valid3(valid_cols, num_valid, move_data, state, debug_move_value);
      }
      else if (test_value == UNKNOWN && num_valid > 1) {
        return choose_from_unknown(valid_cols, num_valid, move_data, state, debug_move_value);
      }
      return valid_cols[rand() % num_valid];
    }
  }

  return rand() % 7;
}
