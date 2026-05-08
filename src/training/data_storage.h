#pragma once
#include "self_play.h"
#include <string>
#include <vector>

namespace Training {

// Save generated games to a basic JSON-like text file for easy Python parsing
bool save_games_to_file(const std::vector<GameRecord>& games, const std::string& filename);

} // namespace Training
