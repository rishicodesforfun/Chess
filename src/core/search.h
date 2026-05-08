#pragma once
#include "board.h"
#include "movegen.h"

namespace Search {

// Simple static evaluation function for alpha-beta search
int evaluate(const BoardState& board);

// Alpha-beta pruning search
// Returns the best move found within the given depth
Move get_best_move(BoardState& board, int depth);

} // namespace Search
