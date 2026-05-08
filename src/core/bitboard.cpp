#include "bitboard.h"

namespace Bitboards {

Bitboard PawnAttacks[COLOR_NB][SQUARE_NB];
Bitboard KnightAttacks[SQUARE_NB];
Bitboard KingAttacks[SQUARE_NB];

void init() {
    for (int s = 0; s < SQUARE_NB; ++s) {
        Bitboard b = 1ULL << s;
        
        // Knights
        KnightAttacks[s] = shift<NORTH_NORTH_EAST>(b) | shift<NORTH_NORTH_WEST>(b) |
                           shift<SOUTH_SOUTH_EAST>(b) | shift<SOUTH_SOUTH_WEST>(b) |
                           shift<EAST_EAST_NORTH>(b) | shift<EAST_EAST_SOUTH>(b) |
                           shift<WEST_WEST_NORTH>(b) | shift<WEST_WEST_SOUTH>(b);
        
        // King
        KingAttacks[s] = shift<NORTH>(b) | shift<SOUTH>(b) | shift<EAST>(b) | shift<WEST>(b) |
                         shift<NORTH_EAST>(b) | shift<NORTH_WEST>(b) | 
                         shift<SOUTH_EAST>(b) | shift<SOUTH_WEST>(b);
                         
        // Pawns
        PawnAttacks[WHITE][s] = shift<NORTH_EAST>(b) | shift<NORTH_WEST>(b);
        PawnAttacks[BLACK][s] = shift<SOUTH_EAST>(b) | shift<SOUTH_WEST>(b);
    }
}

} // namespace Bitboards
