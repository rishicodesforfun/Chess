#include "engine/chess_engine.h"
#include "training/self_play.h"
#include "training/data_storage.h"
#include <iostream>
#include <string>

using namespace Training;

void training_iteration(int iteration_number) {
    std::cout << "Starting self-play iteration " << iteration_number << std::endl;
    
    ChessEngine engine;
    std::vector<GameRecord> games;
    
    // Play 10 games per iteration for testing (will scale up later)
    for (int i = 0; i < 10; i++) {
        GameRecord game = play_self_game(engine, 4); // depth 4
        games.push_back(game);
        std::cout << "Game " << i << " outcome: " << game.outcome << " (moves: " << game.moves.size() << ")" << std::endl;
    }
    
    std::string filename = "games_iteration_" + std::to_string(iteration_number) + ".jsonl";
    if (save_games_to_file(games, filename)) {
        std::cout << "Saved " << games.size() << " games to " << filename << std::endl;
    } else {
        std::cerr << "Failed to save games to " << filename << std::endl;
    }
}

int main() {
    std::cout << "AlphaZero Self-Play Data Generation" << std::endl;
    
    // Initialize move generation tables
    Bitboards::init();
    
    // Run 3 iterations for demonstration
    for (int iter = 1; iter <= 3; iter++) {
        training_iteration(iter);
    }
    
    std::cout << "Self-play data collection complete!" << std::endl;
    return 0;
}
