#include "movegen.h"

namespace MoveGen {

bool is_attacked(const BoardState& board, Square sq, Color byColor) {
    // To be implemented: lookup magic bitboards, pawn attacks, knight attacks, king attacks
    return false;
}

bool is_in_check(const BoardState& board) {
    Square king_sq = Bitboards::lsb(board.pieces(board.side_to_move(), KING));
    return is_attacked(board, king_sq, ~board.side_to_move());
}

std::vector<Move> generate_pseudo_legal_moves(const BoardState& board) {
    std::vector<Move> moves;
    
    // To be implemented: Generate moves using bitboards
    
    return moves;
}

std::vector<Move> generate_legal_moves(const BoardState& board) {
    std::vector<Move> pseudo = generate_pseudo_legal_moves(board);
    std::vector<Move> legal;
    legal.reserve(pseudo.size());
    
    BoardState temp_board = board;
    
    for (Move m : pseudo) {
        StateInfo st;
        temp_board.do_move(m, st);
        
        Color moved_color = ~temp_board.side_to_move();
        Square king_sq = Bitboards::lsb(temp_board.pieces(moved_color, KING));
        
        if (!is_attacked(temp_board, king_sq, temp_board.side_to_move())) {
            legal.push_back(m);
        }
        
        temp_board.undo_move(m);
    }
    
    return legal;
}

} // namespace MoveGen
