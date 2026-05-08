#pragma once
#include "core/board.h"
#include "core/search.h"
#include "mcts/mcts.h"
#include <string>
#include <vector>
#include <memory>

class ChessEngine {
public:
    ChessEngine();

    void set_position(const std::string& fen);
    std::string get_fen() const;
    
    // Returns move in algebraic notation (e.g. e2e4)
    std::string get_best_move_algebraic(int depth);
    Move get_best_move(int depth);
    
    // Play move in algebraic notation
    bool play_move(const std::string& algebraic);
    void play_move(Move m);
    
    bool is_game_over();
    std::string get_game_result(); // "1-0", "0-1", "1/2-1/2", "*"
    
    // Raw board access for training data extraction
    const BoardState& get_board_state() const { return board; }

private:
    BoardState board;
    std::vector<std::unique_ptr<StateInfo>> state_history;
    
    // MCTS components
    std::unique_ptr<MCTS::NeuralNetworkEvaluator> nn_evaluator;
    std::unique_ptr<MCTS::MCTSEngine> mcts_engine;
    
    // Helper to parse string to Move object
    Move parse_algebraic(const std::string& algebraic);
    std::string to_algebraic(Move m);
};
