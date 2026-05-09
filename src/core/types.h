#pragma once
#include <cstdint>

using Bitboard = uint64_t;

enum Color { WHITE, BLACK, COLOR_NB };

inline Color operator~(Color c) {
    return static_cast<Color>(c ^ 1);
}

enum PieceType {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_TYPE_NB
};

enum Piece {
    W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    PIECE_NB, NO_PIECE
};

inline Piece make_piece(Color c, PieceType pt) {
    return static_cast<Piece>((c << 3) + pt);
}

inline PieceType type_of(Piece p) {
    return static_cast<PieceType>(p & 7);
}

inline Color color_of(Piece p) {
    return static_cast<Color>(p >> 3);
}

enum Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    SQUARE_NB, NO_SQUARE
};

inline Square make_square(int rank, int file) {
    return static_cast<Square>(rank * 8 + file);
}

inline int rank_of(Square sq) { return sq / 8; }
inline int file_of(Square sq) { return sq % 8; }

// Directions for sliding pieces
enum Direction {
    NORTH =  8,
    SOUTH = -8,
    EAST  =  1,
    WEST  = -1,
    NORTH_EAST =  9,
    NORTH_WEST =  7,
    SOUTH_EAST = -7,
    SOUTH_WEST = -9,
    
    // Knight moves
    NORTH_NORTH_EAST = 17,
    NORTH_NORTH_WEST = 15,
    SOUTH_SOUTH_EAST = -15,
    SOUTH_SOUTH_WEST = -17,
    EAST_EAST_NORTH = 10,
    EAST_EAST_SOUTH = -6,
    WEST_WEST_NORTH = 6,
    WEST_WEST_SOUTH = -10
};

namespace Zobrist {
    extern uint64_t piece[COLOR_NB][PIECE_TYPE_NB][SQUARE_NB];
    extern uint64_t castling[16];
    extern uint64_t en_passant[8];
    extern uint64_t side_to_move;

    void init();
}

// Square constants with SQ_ prefix
#define SQ_A1 A1
#define SQ_H1 H1
#define SQ_A8 A8
#define SQ_H8 H8
