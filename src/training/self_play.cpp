#include "self_play.h"
#include "core/movegen.h"
#include <iostream>

namespace Training {

GameRecord play_self_game(ChessEngine& engine, int search_depth) {
    GameRecord game;
    engine.set_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    while (!engine.is_game_over()) {
        // Record position FEN
        game.fens.push_back(engine.get_fen());
        
        // Search for best move
        Move best_move = engine.get_best_move(search_depth);
        game.moves.push_back(best_move.data);
        
        // Before MCTS is implemented, use uniform move probabilities
        std::vector<Move> legal = MoveGen::generate_legal_moves(engine.get_board_state());
        std::vector<double> probs(legal.size(), 1.0 / legal.size());
        game.probs.push_back(probs);
        
        engine.play_move(best_move);
    }
    
    // Determine outcome
    std::string result = engine.get_game_result();
    if (result == "1-0") game.outcome = 1;
    else if (result == "0-1") game.outcome = -1;
    else game.outcome = 0;
    
    return game;
}

} // namespace Training
