#pragma once

#include "types.h"

namespace Spellfish 
{


    constexpr Bitboard FileABB = 0x0101010101010101ULL;

    constexpr Bitboard FileBBB = FileABB << 1;
    constexpr Bitboard FileCBB = FileABB << 2;
    constexpr Bitboard FileDBB = FileABB << 3;
    constexpr Bitboard FileEBB = FileABB << 4;
    constexpr Bitboard FileFBB = FileABB << 5;
    constexpr Bitboard FileGBB = FileABB << 6;
    constexpr Bitboard FileHBB = FileABB << 7;

    constexpr Bitboard Rank1BB = 0xFF;
    constexpr Bitboard Rank2BB = Rank1BB << (FILE_NB * 1);
    constexpr Bitboard Rank3BB = Rank1BB << (FILE_NB * 2);
    constexpr Bitboard Rank4BB = Rank1BB << (FILE_NB * 3);
    constexpr Bitboard Rank5BB = Rank1BB << (FILE_NB * 4);
    constexpr Bitboard Rank6BB = Rank1BB << (FILE_NB * 5);
    constexpr Bitboard Rank7BB = Rank1BB << (FILE_NB * 6);
    constexpr Bitboard Rank8BB = Rank1BB << (FILE_NB * 7);
    
    constexpr Bitboard AllSquares = ~Bitboard(0);


    namespace Bitbases
    {
        extern Bitboard PawnAttacks[COLOR_NB][SQUARE_NB];
        extern Bitboard KingAttacks[SQUARE_NB];

        void init();
        Bitboard attacks_from_bb(Piece piece, Square sq);
    }
   


    inline Bitboard square_bb(Square s) {
        assert(is_ok(s));
        return Bitboard(1) << s;
    }

 


    /// Overloads of bitwise operators between a Bitboard and a Square for testing
    /// whether a given bit is set in a bitboard, and for setting and clearing bits.

    inline Bitboard  operator&(Bitboard  b, Square s) { return b & square_bb(s); }
    inline Bitboard  operator|(Bitboard  b, Square s) { return b | square_bb(s); }
    inline Bitboard  operator^(Bitboard  b, Square s) { return b ^ square_bb(s); }
    inline Bitboard& operator|=(Bitboard& b, Square s) { return b |= square_bb(s); }
    inline Bitboard& operator^=(Bitboard& b, Square s) { return b ^= square_bb(s); }

    inline Bitboard  operator-(Bitboard  b, Square s) { return b & ~square_bb(s); }
    inline Bitboard& operator-=(Bitboard& b, Square s) { return b &= ~square_bb(s); }

    inline Bitboard  operator&(Square s, Bitboard b) { return b & s; }
    inline Bitboard  operator|(Square s, Bitboard b) { return b | s; }
    inline Bitboard  operator^(Square s, Bitboard b) { return b ^ s; }

    inline Bitboard  operator|(Square s1, Square s2) { return square_bb(s1) | s2; }

    /// rank_bb() and file_bb() return a bitboard representing all the squares on
    /// the given file or rank.

    constexpr Bitboard rank_bb(Rank r) {
        return Rank1BB << (FILE_NB * r);
    }

    constexpr Bitboard rank_bb(Square s) {
        return rank_bb(rank_of(s));
    }

    constexpr Bitboard file_bb(File f) {
        return FileABB << f;
    }

    constexpr Bitboard file_bb(Square s) {
        return file_bb(file_of(s));
    }

    template<Direction D>
    constexpr Bitboard shift(Bitboard b) {
        return  D == NORTH ? b << NORTH : D == SOUTH ? b >> NORTH
            : D == NORTH + NORTH ? b << (2 * NORTH) : D == SOUTH + SOUTH ? b >> (2 * NORTH)
            : D == EAST ? (b & ~file_bb(FILE_MAX)) << EAST : D == WEST ? (b & ~FileABB) >> EAST
            : D == NORTH_EAST ? (b & ~file_bb(FILE_MAX)) << NORTH_EAST : D == NORTH_WEST ? (b & ~FileABB) << NORTH_WEST
            : D == SOUTH_EAST ? (b & ~file_bb(FILE_MAX)) >> NORTH_WEST : D == SOUTH_WEST ? (b & ~FileABB) >> NORTH_EAST
            : Bitboard(0);
    }


    /// shift() moves a bitboard one step along direction D (mainly for pawns)

    constexpr Bitboard shift(Direction D, Bitboard b) {
        return  D == NORTH ? b << NORTH : D == SOUTH ? b >> NORTH
            : D == NORTH + NORTH ? b << (2 * NORTH) : D == SOUTH + SOUTH ? b >> (2 * NORTH)
            : D == EAST ? (b & ~file_bb(FILE_MAX)) << EAST : D == WEST ? (b & ~FileABB) >> EAST
            : D == NORTH_EAST ? (b & ~file_bb(FILE_MAX)) << NORTH_EAST : D == NORTH_WEST ? (b & ~FileABB) << NORTH_WEST
            : D == SOUTH_EAST ? (b & ~file_bb(FILE_MAX)) >> NORTH_WEST : D == SOUTH_WEST ? (b & ~FileABB) >> NORTH_EAST
            : Bitboard(0);
    }

    //Does not include the square s
    inline Bitboard adjacent_bb(Square s)
    {
        Bitboard bb = square_bb(s);
        assert(is_ok(s));

        return shift<NORTH>(bb) | shift<SOUTH>(bb) | shift<EAST>(bb) | shift<WEST>(bb) |
            shift<NORTH_EAST>(bb) | shift<SOUTH_EAST>(bb) | shift<NORTH_WEST>(bb) | shift<SOUTH_WEST>(bb);
    }

  


    /// lsb() and msb() return the least/most significant bit in a non-zero bitboard

#if defined(__GNUC__)  // GCC, Clang, ICC

    inline Square lsb(Bitboard b) {
        assert(b);
#ifdef LARGEBOARDS
        if (!(b << 64))
            return Square(__builtin_ctzll(b >> 64) + 64);
#endif
        return Square(__builtin_ctzll(b));
    }

    inline Square msb(Bitboard b) {
        assert(b);
#ifdef LARGEBOARDS
        if (b >> 64)
            return Square(int(SQUARE_BIT_MASK) ^ __builtin_clzll(b >> 64));
        return Square(int(SQUARE_BIT_MASK) ^ (__builtin_clzll(b) + 64));
#else
        return Square(int(SQUARE_BIT_MASK) ^ __builtin_clzll(b));
#endif
    }

#elif defined(_MSC_VER)  // MSVC

#ifdef _WIN64  // MSVC, WIN64

    inline Square lsb(Bitboard b) {
        assert(b);
        unsigned long idx;
#ifdef LARGEBOARDS
        if (uint64_t(b))
        {
            _BitScanForward64(&idx, uint64_t(b));
            return Square(idx);
        }
        else
        {
            _BitScanForward64(&idx, uint64_t(b >> 64));
            return Square(idx + 64);
        }
#else
        _BitScanForward64(&idx, b);
        return (Square)idx;
#endif
    }

    inline Square msb(Bitboard b) {
        assert(b);
        unsigned long idx;
#ifdef LARGEBOARDS
        if (b >> 64)
        {
            _BitScanReverse64(&idx, uint64_t(b >> 64));
            return Square(idx + 64);
        }
        else
        {
            _BitScanReverse64(&idx, uint64_t(b));
            return Square(idx);
        }
#else
        _BitScanReverse64(&idx, b);
        return (Square)idx;
#endif
    }

#else  // MSVC, WIN32

    inline Square lsb(Bitboard b) {
        assert(b);
        unsigned long idx;

#ifdef LARGEBOARDS
        if (b << 96) {
            _BitScanForward(&idx, uint32_t(b));
            return Square(idx);
        }
        else if (b << 64) {
            _BitScanForward(&idx, uint32_t(b >> 32));
            return Square(idx + 32);
        }
        else if (b << 32) {
            _BitScanForward(&idx, uint32_t(b >> 64));
            return Square(idx + 64);
        }
        else {
            _BitScanForward(&idx, uint32_t(b >> 96));
            return Square(idx + 96);
        }
#else
        if (b & 0xffffffff) {
            _BitScanForward(&idx, uint32_t(b));
            return Square(idx);
        }
        else {
            _BitScanForward(&idx, uint32_t(b >> 32));
            return Square(idx + 32);
        }
#endif
    }

    inline Square msb(Bitboard b) {
        assert(b);
        unsigned long idx;

#ifdef LARGEBOARDS
        if (b >> 96) {
            _BitScanReverse(&idx, uint32_t(b >> 96));
            return Square(idx + 96);
        }
        else if (b >> 64) {
            _BitScanReverse(&idx, uint32_t(b >> 64));
            return Square(idx + 64);
        }
        else
#endif
            if (b >> 32) {
                _BitScanReverse(&idx, uint32_t(b >> 32));
                return Square(idx + 32);
            }
            else {
                _BitScanReverse(&idx, uint32_t(b));
                return Square(idx);
            }
    }

#endif

#else  // Compiler is neither GCC nor MSVC compatible

#error "Compiler not supported."

#endif

/// least_significant_square_bb() returns the bitboard of the least significant
/// square of a non-zero bitboard. It is equivalent to square_bb(lsb(bb)).

   /* inline Bitboard least_significant_square_bb(Bitboard b) {
        assert(b);
        return b & -b;
    }*/

    /// pop_lsb() finds and clears the least significant bit in a non-zero bitboard

    inline Square pop_lsb(Bitboard& b) {
        assert(b);
        const Square s = lsb(b);
        b &= b - 1;
        return s;
    }



#if defined(__GNUC__)  // GCC, Clang, ICC

    inline PieceType lsb(PieceSet ps) {
        assert(ps);
        return PieceType(__builtin_ctzll(ps));
    }

    inline PieceType msb(PieceSet ps) {
        assert(ps);
        return PieceType((PIECE_TYPE_NB - 1) ^ __builtin_clzll(ps));
    }

#elif defined(_MSC_VER)  // MSVC

#ifdef _WIN64  // MSVC, WIN64

    inline PieceType lsb(PieceSet ps) {
        assert(ps);
        unsigned long idx;
        _BitScanForward64(&idx, ps);
        return (PieceType)idx;
    }

    inline PieceType msb(PieceSet ps) {
        assert(ps);
        unsigned long idx;
        _BitScanReverse64(&idx, ps);
        return (PieceType)idx;
    }

#else  // MSVC, WIN32

    inline PieceType lsb(PieceSet ps) {
        assert(ps);
        unsigned long idx;

        if (ps & 0xffffffff) {
            _BitScanForward(&idx, uint32_t(ps));
            return PieceType(idx);
        }
        else {
            _BitScanForward(&idx, uint32_t(ps >> 32));
            return PieceType(idx + 32);
        }
    }

    inline PieceType msb(PieceSet ps) {
        assert(ps);
        unsigned long idx;
        if (ps >> 32) {
            _BitScanReverse(&idx, uint32_t(ps >> 32));
            return PieceType(idx + 32);
        }
        else {
            _BitScanReverse(&idx, uint32_t(ps));
            return PieceType(idx);
        }
    }

#endif

#else  // Compiler is neither GCC nor MSVC compatible

#error "Compiler not supported."

#endif

    inline PieceType pop_lsb(PieceSet& ps) {
        assert(ps);
        const PieceType pt = lsb(ps);
        ps &= PieceSet(ps - 1);
        return pt;
    }

    inline PieceType pop_msb(PieceSet& ps) {
        assert(ps);
        const PieceType pt = msb(ps);
        ps &= ~piece_set(pt);
        return pt;
    }

    inline int count_bits(Bitboard b)
    {
        int count = 0;

        while (b)
        {
            pop_lsb(b);
            count++;
        }

        return count;
    }

}