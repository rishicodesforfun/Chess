#pragma once
#include "types.h"

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace Bitboards {

extern void init();

inline int popcount(Bitboard b) {
#if defined(__clang__) || defined(__GNUC__)
    return __builtin_popcountll(b);
#elif defined(_WIN64)
    return static_cast<int>(__popcnt64(b));
#else
    return static_cast<int>(__popcnt(static_cast<unsigned int>(b)) + __popcnt(static_cast<unsigned int>(b >> 32)));
#endif
}

inline Square lsb(Bitboard b) {
#if defined(__clang__) || defined(__GNUC__)
    return static_cast<Square>(__builtin_ctzll(b));
#elif defined(_WIN64)
    unsigned long idx;
    _BitScanForward64(&idx, b);
    return static_cast<Square>(idx);
#else
    unsigned long idx;
    if (_BitScanForward(&idx, static_cast<unsigned long>(b))) return static_cast<Square>(idx);
    _BitScanForward(&idx, static_cast<unsigned long>(b >> 32));
    return static_cast<Square>(idx + 32);
#endif
}

inline Bitboard pop_lsb(Bitboard& b) {
    Square s = lsb(b);
    b &= b - 1;
    return static_cast<Bitboard>(1ULL << s);
}

inline void set_bit(Bitboard& b, Square s) {
    b |= (1ULL << s);
}

inline void clear_bit(Bitboard& b, Square s) {
    b &= ~(1ULL << s);
}

inline bool test_bit(Bitboard b, Square s) {
    return (b & (1ULL << s)) != 0;
}

constexpr Bitboard FileABB = 0x0101010101010101ULL;
constexpr Bitboard FileBBB = FileABB << 1;
constexpr Bitboard FileCBB = FileABB << 2;
constexpr Bitboard FileDBB = FileABB << 3;
constexpr Bitboard FileEBB = FileABB << 4;
constexpr Bitboard FileFBB = FileABB << 5;
constexpr Bitboard FileGBB = FileABB << 6;
constexpr Bitboard FileHBB = FileABB << 7;

constexpr Bitboard Rank1BB = 0xFFULL;
constexpr Bitboard Rank2BB = Rank1BB << (8 * 1);
constexpr Bitboard Rank3BB = Rank1BB << (8 * 2);
constexpr Bitboard Rank4BB = Rank1BB << (8 * 3);
constexpr Bitboard Rank5BB = Rank1BB << (8 * 4);
constexpr Bitboard Rank6BB = Rank1BB << (8 * 5);
constexpr Bitboard Rank7BB = Rank1BB << (8 * 6);
constexpr Bitboard Rank8BB = Rank1BB << (8 * 7);

template<Direction D>
constexpr Bitboard shift(Bitboard b) {
    if constexpr (D == NORTH) return b << 8;
    else if constexpr (D == SOUTH) return b >> 8;
    else if constexpr (D == EAST) return (b & ~FileHBB) << 1;
    else if constexpr (D == WEST) return (b & ~FileABB) >> 1;
    else if constexpr (D == NORTH_EAST) return (b & ~FileHBB) << 9;
    else if constexpr (D == NORTH_WEST) return (b & ~FileABB) << 7;
    else if constexpr (D == SOUTH_EAST) return (b & ~FileHBB) >> 7;
    else if constexpr (D == SOUTH_WEST) return (b & ~FileABB) >> 9;
    else return 0;
}

inline Bitboard shift_one(Bitboard b, Direction d) {
    switch (d) {
        case NORTH: return b << 8;
        case SOUTH: return b >> 8;
        case EAST:  return (b & ~FileHBB) << 1;
        case WEST:  return (b & ~FileABB) >> 1;
        case NORTH_EAST: return (b & ~FileHBB) << 9;
        case NORTH_WEST: return (b & ~FileABB) << 7;
        case SOUTH_EAST: return (b & ~FileHBB) >> 7;
        case SOUTH_WEST: return (b & ~FileABB) >> 9;
        default: return 0;
    }
}

extern Bitboard PawnAttacks[COLOR_NB][SQUARE_NB];
extern Bitboard KnightAttacks[SQUARE_NB];
extern Bitboard KingAttacks[SQUARE_NB];

} // namespace Bitboards
