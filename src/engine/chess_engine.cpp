#include "chess_engine.h"
#include "core/movegen.h"
#include <iostream>

ChessEngine::ChessEngine() {
    set_position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void ChessEngine::set_position(const std::string& fen) {
    state_history.clear();
    board.set_fen(fen);
}

std::string ChessEngine::get_fen() const {
    return board.get_fen();
}

std::string ChessEngine::to_algebraic(Move m) {
    if (!m.is_ok()) return "0000";
    
    std::string res = "";
    res += ('a' + file_of(m.from()));
    res += ('1' + rank_of(m.from()));
    res += ('a' + file_of(m.to()));
    res += ('1' + rank_of(m.to()));
    
    if (m.flags() == PROMOTION) {
        res += "q"; // Simplified: always promote to queen in notation for now
    }
    return res;
}

Move ChessEngine::parse_algebraic(const std::string& algebraic) {
    std::vector<Move> legal_moves = MoveGen::generate_legal_moves(board);
    for (Move m : legal_moves) {
        if (to_algebraic(m) == algebraic || 
            (algebraic.length() == 4 && to_algebraic(m).substr(0, 4) == algebraic)) {
            return m;
        }
    }
    return Move(); // Invalid move
}

Move ChessEngine::get_best_move(int depth) {
    return Search::get_best_move(board, depth);
}

std::string ChessEngine::get_best_move_algebraic(int depth) {
    Move best = get_best_move(depth);
    return to_algebraic(best);
}

void ChessEngine::play_move(Move m) {
    auto st = std::make_unique<StateInfo>();
    board.do_move(m, *st);
    state_history.push_back(std::move(st));
}

bool ChessEngine::play_move(const std::string& algebraic) {
    Move m = parse_algebraic(algebraic);
    if (!m.is_ok()) return false;
    
    play_move(m);
    return true;
}

bool ChessEngine::is_game_over() {
    return MoveGen::generate_legal_moves(board).empty() || board.halfmove_clock() >= 100;
}

std::string ChessEngine::get_game_result() {
    if (!is_game_over()) return "*";
    
    if (MoveGen::generate_legal_moves(board).empty()) {
        if (MoveGen::is_in_check(board)) {
            return board.side_to_move() == WHITE ? "0-1" : "1-0";
        }
        return "1/2-1/2"; // Stalemate
    }
    
    return "1/2-1/2"; // Draw by 50-move rule
}
