#include "data_storage.h"
#include <fstream>
#include <iostream>

namespace Training {

bool save_games_to_file(const std::vector<GameRecord>& games, const std::string& filename) {
    std::ofstream out(filename);
    if (!out) return false;

    // Simple JSON-lines format
    for (const auto& game : games) {
        out << "{";
        out << "\"outcome\": " << game.outcome << ", ";
        
        out << "\"fens\": [";
        for (size_t i = 0; i < game.fens.size(); ++i) {
            out << "\"" << game.fens[i] << "\"";
            if (i < game.fens.size() - 1) out << ", ";
        }
        out << "], ";
        
        out << "\"moves\": [";
        for (size_t i = 0; i < game.moves.size(); ++i) {
            out << game.moves[i];
            if (i < game.moves.size() - 1) out << ", ";
        }
        out << "]";
        
        // Probability saving is simplified for now
        out << "}\n";
    }
    
    return true;
}

} // namespace Training
