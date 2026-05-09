#include "types.h"
#include <random>

namespace Zobrist {
    uint64_t piece[COLOR_NB][PIECE_TYPE_NB][SQUARE_NB];
    uint64_t castling[16];
    uint64_t en_passant[8];
    uint64_t side_to_move;

    void init() {
        std::mt19937_64 rng(1070372); // Fixed seed for reproducibility

        for (int c = 0; c < COLOR_NB; ++c) {
            for (int pt = 0; pt < PIECE_TYPE_NB; ++pt) {
                for (int s = 0; s < SQUARE_NB; ++s) {
                    piece[c][pt][s] = rng();
                }
            }
        }

        for (int i = 0; i < 16; ++i) castling[i] = rng();
        for (int i = 0; i < 8; ++i) en_passant[i] = rng();
        side_to_move = rng();
    }
}
