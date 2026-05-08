#include "movegen.h"
#include <algorithm>

namespace MoveGen {

Bitboard get_sliding_attacks(Square sq, Bitboard occupied, PieceType pt) {
    Bitboard attacks = 0;
    static const Direction rook_dirs[] = { NORTH, SOUTH, EAST, WEST };
    static const Direction bishop_dirs[] = { NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST };
    
    const Direction* dirs = (pt == ROOK) ? rook_dirs : (pt == BISHOP) ? bishop_dirs : nullptr;
    int num_dirs = 4;
    
    if (pt == QUEEN) {
        dirs = nullptr; // special case handled below
    }

    auto generate_for_dirs = [&](const Direction* d_list, int count) {
        for (int i = 0; i < count; ++i) {
            Direction d = d_list[i];
            Bitboard b = 1ULL << sq;
            while (true) {
                b = Bitboards::shift_one(b, d);
                if (b == 0) break;
                attacks |= b;
                if (b & occupied) break;
            }
        }
    };

    if (pt == ROOK) generate_for_dirs(rook_dirs, 4);
    else if (pt == BISHOP) generate_for_dirs(bishop_dirs, 4);
    else if (pt == QUEEN) {
        generate_for_dirs(rook_dirs, 4);
        generate_for_dirs(bishop_dirs, 4);
    }

    return attacks;
}

bool is_attacked(const BoardState& board, Square sq, Color byColor) {
    Bitboard occupied = board.pieces();
    
    // Knights
    if (Bitboards::KnightAttacks[sq] & board.pieces(byColor, KNIGHT)) return true;
    
    // King
    if (Bitboards::KingAttacks[sq] & board.pieces(byColor, KING)) return true;
    
    // Pawns
    Color us = ~byColor;
    if (Bitboards::PawnAttacks[us][sq] & board.pieces(byColor, PAWN)) return true;
    
    // Sliding pieces
    if (get_sliding_attacks(sq, occupied, ROOK) & (board.pieces(byColor, ROOK) | board.pieces(byColor, QUEEN))) return true;
    if (get_sliding_attacks(sq, occupied, BISHOP) & (board.pieces(byColor, BISHOP) | board.pieces(byColor, QUEEN))) return true;
    
    return false;
}

bool is_in_check(const BoardState& board) {
    Square king_sq = Bitboards::lsb(board.pieces(board.side_to_move(), KING));
    return is_attacked(board, king_sq, ~board.side_to_move());
}

std::vector<Move> generate_pseudo_legal_moves(const BoardState& board) {
    std::vector<Move> moves;
    Color us = board.side_to_move();
    Color them = ~us;
    Bitboard occupied = board.pieces();
    Bitboard empty = ~occupied;
    Bitboard targets = ~board.pieces(us);

    // Pawns
    Bitboard pawns = board.pieces(us, PAWN);
    while (pawns) {
        Square from = Bitboards::lsb(Bitboards::pop_lsb(pawns));
        Bitboard b = 1ULL << from;
        
        // Push
        Bitboard single_push = (us == WHITE ? b << 8 : b >> 8) & empty;
        if (single_push) {
            Square to = Bitboards::lsb(single_push);
            if ((us == WHITE && to >= SQ_A8) || (us == BLACK && to <= SQ_H1)) {
                moves.emplace_back(from, to, PROMOTION);
            } else {
                moves.emplace_back(from, to, NORMAL);
                // Double push
                Bitboard double_push = (us == WHITE ? single_push << 8 : single_push >> 8) & empty & (us == WHITE ? Bitboards::Rank4BB : Bitboards::Rank5BB);
                if (double_push) moves.emplace_back(from, Bitboards::lsb(double_push), NORMAL);
            }
        }
        
        // Captures
        Bitboard attacks = Bitboards::PawnAttacks[us][from] & board.pieces(them);
        while (attacks) {
            Square to = Bitboards::lsb(Bitboards::pop_lsb(attacks));
            if ((us == WHITE && to >= SQ_A8) || (us == BLACK && to <= SQ_H1)) moves.emplace_back(from, to, PROMOTION | CAPTURE);
            else moves.emplace_back(from, to, CAPTURE);
        }
    }

    // Knights, King
    auto gen_fixed = [&](PieceType pt, Bitboard* table) {
        Bitboard pc = board.pieces(us, pt);
        while (pc) {
            Square from = Bitboards::lsb(Bitboards::pop_lsb(pc));
            Bitboard attacks = table[from] & targets;
            while (attacks) {
                Square to = Bitboards::lsb(Bitboards::pop_lsb(attacks));
                moves.emplace_back(from, to, (Bitboards::test_bit(board.pieces(them), to) ? CAPTURE : NORMAL));
            }
        }
    };
    gen_fixed(KNIGHT, Bitboards::KnightAttacks);
    gen_fixed(KING, Bitboards::KingAttacks);

    // Sliding pieces
    auto gen_sliding = [&](PieceType pt) {
        Bitboard pc = board.pieces(us, pt);
        while (pc) {
            Square from = Bitboards::lsb(Bitboards::pop_lsb(pc));
            Bitboard attacks = get_sliding_attacks(from, occupied, pt) & targets;
            while (attacks) {
                Square to = Bitboards::lsb(Bitboards::pop_lsb(attacks));
                moves.emplace_back(from, to, (Bitboards::test_bit(board.pieces(them), to) ? CAPTURE : NORMAL));
            }
        }
    };
    gen_sliding(ROOK);
    gen_sliding(BISHOP);
    gen_sliding(QUEEN);

    return moves;
}

std::vector<Move> generate_legal_moves(const BoardState& board) {
    std::vector<Move> pseudo = generate_pseudo_legal_moves(board);
    std::vector<Move> legal;
    
    for (Move m : pseudo) {
        BoardState temp_board = board;
        StateInfo st;
        temp_board.do_move(m, st);
        
        // After we move, it's the other player's turn, so we check if OUR king is attacked
        Square king_sq = Bitboards::lsb(temp_board.pieces(board.side_to_move(), KING));
        if (!is_attacked(temp_board, king_sq, temp_board.side_to_move())) {
            legal.push_back(m);
        }
    }
    return legal;
}

} // namespace MoveGen
