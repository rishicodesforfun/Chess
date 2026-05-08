#pragma once
#include "board.h"
#include <vector>

namespace MoveGen {

// Generate all pseudo-legal moves
std::vector<Move> generate_pseudo_legal_moves(const BoardState& board);

// Generate all strictly legal moves (pseudo-legal filtered for king safety)
std::vector<Move> generate_legal_moves(const BoardState& board);

// Check if a square is attacked by a given color
bool is_attacked(const BoardState& board, Square sq, Color byColor);

// Check if current side to move is in check
bool is_in_check(const BoardState& board);

} // namespace MoveGen
