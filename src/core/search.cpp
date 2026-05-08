#include "search.h"
#include <algorithm>
#include <limits>

namespace Search {

constexpr int INFINITY_SCORE = 32000;
constexpr int MATE_SCORE = 30000;

int evaluate(const BoardState& board) {
    int score = 0;
    
    score += 100 * (Bitboards::popcount(board.pieces(WHITE, PAWN))   - Bitboards::popcount(board.pieces(BLACK, PAWN)));
    score += 320 * (Bitboards::popcount(board.pieces(WHITE, KNIGHT)) - Bitboards::popcount(board.pieces(BLACK, KNIGHT)));
    score += 330 * (Bitboards::popcount(board.pieces(WHITE, BISHOP)) - Bitboards::popcount(board.pieces(BLACK, BISHOP)));
    score += 500 * (Bitboards::popcount(board.pieces(WHITE, ROOK))   - Bitboards::popcount(board.pieces(BLACK, ROOK)));
    score += 900 * (Bitboards::popcount(board.pieces(WHITE, QUEEN))  - Bitboards::popcount(board.pieces(BLACK, QUEEN)));
    
    return board.side_to_move() == WHITE ? score : -score;
}

int alphabeta(BoardState& board, int depth, int alpha, int beta) {
    if (depth == 0) {
        return evaluate(board);
    }
    
    std::vector<Move> moves = MoveGen::generate_legal_moves(board);
    
    if (moves.empty()) {
        if (MoveGen::is_in_check(board)) {
            return -MATE_SCORE + (100 - depth); // Prefer faster mates
        }
        return 0; // Stalemate
    }
    
    int max_eval = -INFINITY_SCORE;
    
    for (Move m : moves) {
        StateInfo st;
        board.do_move(m, st);
        
        int eval = -alphabeta(board, depth - 1, -beta, -alpha);
        
        board.undo_move(m);
        
        if (eval > max_eval) max_eval = eval;
        if (eval > alpha) alpha = eval;
        if (alpha >= beta) break;
    }
    
    return max_eval;
}

Move get_best_move(BoardState& board, int depth) {
    std::vector<Move> moves = MoveGen::generate_legal_moves(board);
    if (moves.empty()) return Move(); // No legal moves
    
    Move best_move = moves[0];
    int max_eval = -INFINITY_SCORE;
    int alpha = -INFINITY_SCORE;
    int beta = INFINITY_SCORE;
    
    for (Move m : moves) {
        StateInfo st;
        board.do_move(m, st);
        
        int eval = -alphabeta(board, depth - 1, -beta, -alpha);
        
        board.undo_move(m);
        
        if (eval > max_eval) {
            max_eval = eval;
            best_move = m;
        }
        if (eval > alpha) alpha = eval;
    }
    
    return best_move;
}

} // namespace Search
