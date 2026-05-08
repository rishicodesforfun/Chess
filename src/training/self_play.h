#pragma once
#include "engine/chess_engine.h"
#include <vector>

namespace Training {

struct GameRecord {
    std::vector<std::string> fens;           // Board state after each half-move
    std::vector<uint16_t> moves;             // Move played
    std::vector<std::vector<double>> probs;  // Move probabilities
    int outcome;                             // -1 (black win), 0 (draw), 1 (white win)
};

// Play a full game of the engine against itself
GameRecord play_self_game(ChessEngine& engine, int search_depth);

} // namespace Training
