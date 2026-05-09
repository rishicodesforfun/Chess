#pragma once
#include "types.h"
#include "bitboard.h"
#include <string>
#include <vector>

enum CastlingRight {
    NO_CASTLING = 0,
    WHITE_OO    = 1,
    WHITE_OOO   = 2,
    BLACK_OO    = 4,
    BLACK_OOO   = 8,
    ALL_CASTLING= 15
};

// Flags for Move (4 bits)
enum MoveFlags {
    QUIET = 0,
    DOUBLE_PUSH = 1,
    OO = 2,
    OOO = 3,
    EN_PASSANT = 5,
    PR_KNIGHT = 8,
    PR_BISHOP = 9,
    PR_ROOK = 10,
    PR_QUEEN = 11,
    CAPTURE = 4,
    PC_KNIGHT = 12,
    PC_BISHOP = 13,
    PC_ROOK = 14,
    PC_QUEEN = 15
};

inline bool is_promotion(int flags) { return (flags & 8) != 0; }
inline bool is_capture(int flags) { return (flags & 4) != 0; }
inline PieceType promotion_type(int flags) {
    return static_cast<PieceType>((flags & 3) + 1); // KNIGHT=1, BISHOP=2, ROOK=3, QUEEN=4
}

// AlphaZero move encoding (4672 total moves)
int encode_move(Move m);
Move decode_move(int encoded, const BoardState& board);

struct Move {
    uint16_t data;

    Move() : data(0) {}
    Move(uint16_t d) : data(d) {}
    Move(Square from, Square to, int flags = 0) {
        data = (from & 0x3F) | ((to & 0x3F) << 6) | ((flags & 0xF) << 12);
    }

    Square from() const { return static_cast<Square>(data & 0x3F); }
    Square to() const   { return static_cast<Square>((data >> 6) & 0x3F); }
    int flags() const   { return (data >> 12) & 0xF; }
    
    bool is_ok() const { return data != 0; }
    
    bool operator==(const Move& m) const { return data == m.data; }
    bool operator!=(const Move& m) const { return data != m.data; }
};

struct StateInfo {
    Bitboard piece_bb[COLOR_NB][PIECE_TYPE_NB];
    Bitboard color_bb[COLOR_NB];
    Piece captured_piece;
    int castling_rights;
    Square ep_square;
    int halfmove_clock;
    uint64_t zobrist_key; // For repetition tracking
    Move last_move;
    StateInfo* previous;
};

class BoardState {
public:
    BoardState();
    BoardState(const BoardState& other);
    BoardState& operator=(const BoardState& other);

    void set_fen(const std::string& fen);
    std::string get_fen() const;

    Bitboard pieces(Color c, PieceType pt) const { return piece_bb[c][pt]; }
    Bitboard pieces(Color c) const { return color_bb[c]; }
    Bitboard pieces() const { return color_bb[WHITE] | color_bb[BLACK]; }
    
    Piece piece_on(Square sq) const { return board[sq]; }
    
    Color side_to_move() const { return stm; }
    Square en_passant() const { return st->ep_square; }
    int castling_rights() const { return st->castling_rights; }
    int halfmove_clock() const { return st->halfmove_clock; }
    int fullmove_number() const { return fullmove; }
    const StateInfo* get_state_info() const { return st; }

    void do_move(Move m, StateInfo& new_st);
    void undo_move(Move m);

    // Get current hash
    uint64_t key() const { return st->zobrist_key; }

private:
    void clear();
    void place_piece(Piece p, Square sq);
    void remove_piece(Square sq);

    Bitboard piece_bb[COLOR_NB][PIECE_TYPE_NB];
    Bitboard color_bb[COLOR_NB];
    Piece board[SQUARE_NB];

    Color stm;
    int fullmove;

    StateInfo* st;
    StateInfo setup_state; // Base state for root position
};
