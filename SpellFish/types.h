#pragma once

#include "stdint.h"
#include <cassert>

namespace Spellfish
{
	typedef uint64_t Bitboard;
	typedef uint64_t Key;
	typedef int Depth;

	constexpr int SQUARE_BITS = 6;
	constexpr int SPECIAL_BITS = 14;


		/// A move needs 16 bits to be stored
	///
	/// bit  0- 5: destination square (from 0 to 63)
	/// bit  6-11: origin square (from 0 to 63)
	/// bit 12-13: promotion piece type - 2 (from KNIGHT-2 to QUEEN-2)
	/// bit 14-15: special move flag: promotion (1), en passant (2), castling (3)
	/// bit 16-18: type of spell used (mardarcu) (8 spells max) (3 bit)
	/// bit 19 - 24: spell origin square (6 bit)
	/// NOTE: en passant bit is set only when a pawn can be captured
	///
	/// Special cases are MOVE_NONE and MOVE_NULL. We can sneak these in because in
	/// any normal move destination square is always different from origin square
	/// while MOVE_NONE and MOVE_NULL have the same origin and destination square.

	enum Move : int {
		MOVE_NONE,
		MOVE_NULL = 1 + (1 << SQUARE_BITS)
	};

	enum MoveType : int {
		NORMAL,
		PROMOTION = 1 << SPECIAL_BITS,
		EN_PASSANT = 2 << SPECIAL_BITS,
		CASTLING = 3 << SPECIAL_BITS,
		
	};
	constexpr int SPELL_ID_BIT = 16;
	constexpr int SPELL_ID_BITS = 3;
	constexpr int SPELL_ORIGIN_BIT = 19;
	constexpr int SPELL_ORIGIN_BITS = 6;

	constexpr int MOVE_TYPE_BITS = 4;

	enum CastlingRights {
		NO_CASTLING,
		WHITE_OO,
		WHITE_OOO = WHITE_OO << 1,
		BLACK_OO = WHITE_OO << 2,
		BLACK_OOO = WHITE_OO << 3,

		KING_SIDE = WHITE_OO | BLACK_OO,
		QUEEN_SIDE = WHITE_OOO | BLACK_OOO,
		WHITE_CASTLING = WHITE_OO | WHITE_OOO,
		BLACK_CASTLING = BLACK_OO | BLACK_OOO,
		ANY_CASTLING = WHITE_CASTLING | BLACK_CASTLING,

		CASTLING_RIGHT_NB = 16
	};


	enum Color {
		WHITE, BLACK, COLOR_NB = 2
	};

	constexpr int PIECE_TYPE_BITS = 6; // PIECE_TYPE_NB = pow(2, PIECE_TYPE_BITS)

	enum PieceType {
		NO_PIECE_TYPE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PIECE_TYPE_NB,

		//Aliases
		ALL_PIECES = 0
	};

	enum PieceSet : uint64_t {
		NO_PIECE_SET = 0,
		CHESS_PIECES = (1ULL << PAWN) | (1ULL << KNIGHT) | (1ULL << BISHOP) | (1ULL << ROOK) | (1ULL << QUEEN) | (1ULL << KING),
	};

	enum Piece {
		NO_PIECE,
		W_PAWN = PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING = KING,
		B_PAWN = PAWN + PIECE_TYPE_NB, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING = KING + PIECE_TYPE_NB,
		PIECE_NB = 2 * PIECE_TYPE_NB
	};

	enum Square : int {
		SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
		SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
		SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
		SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
		SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
		SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
		SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
		SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
		SQ_NONE,

		SQUARE_ZERO = 0,
		SQUARE_NB = 64,
		SQUARE_BIT_MASK = 63,

		SQ_MAX = SQUARE_NB - 1,
	};

	enum Direction : int {
		NORTH = 8,

		EAST = 1,
		SOUTH = -NORTH,
		WEST = -EAST,

		NORTH_EAST = NORTH + EAST,
		SOUTH_EAST = SOUTH + EAST,
		SOUTH_WEST = SOUTH + WEST,
		NORTH_WEST = NORTH + WEST
	};

	enum File : int {

		FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,

		FILE_NB,
		FILE_MAX = FILE_NB - 1
	};

	enum Rank : int {
		RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8,

		RANK_NB,
		RANK_MAX = RANK_NB - 1
	};

	enum Value : int {
		VALUE_ZERO = 0,
		VALUE_DRAW = 0,
		VALUE_MATE = 32000,
		


		PawnValueMg = 100, PawnValueEg = 208,
		KnightValueMg = 300, KnightValueEg = 854,
		BishopValueMg = 350, BishopValueEg = 915,
		RookValueMg = 500, RookValueEg = 1380,
		QueenValueMg = 900, QueenValueEg = 2682,
		

		MidgameLimit = 15258, EndgameLimit = 3915
	};

	/// Score enum stores a middlegame and an endgame value in a single integer (enum).
	/// The least significant 16 bits are used to store the middlegame value and the
	/// upper 16 bits are used to store the endgame value. We have to take care to
	/// avoid left-shifting a signed int to avoid undefined behavior.
	enum Score : int { SCORE_ZERO };

	constexpr Score make_score(int mg, int eg) {
		return Score((int)((unsigned int)eg << 16) + mg);
	}

	/// Extracting the signed lower and upper 16 bits is not so trivial because
	/// according to the standard a simple cast to short is implementation defined
	/// and so is a right shift of a signed integer.
	inline Value eg_value(Score s) {
		union { uint16_t u; int16_t s; } eg = { uint16_t(unsigned(s + 0x8000) >> 16) };
		return Value(eg.s);
	}

	inline Value mg_value(Score s) {
		union { uint16_t u; int16_t s; } mg = { uint16_t(unsigned(s)) };
		return Value(mg.s);
	}

	enum Phase {
		PHASE_ENDGAME,
		PHASE_MIDGAME = 128,
		MG = 0, EG = 1, PHASE_NB = 2
	};




#define ENABLE_BIT_OPERATORS_ON(T)                                        \
constexpr T operator~ (T d) { return (T)~(int)d; }                        \
constexpr T operator| (T d1, T d2) { return (T)((int)d1 | (int)d2); }     \
constexpr T operator& (T d1, T d2) { return (T)((int)d1 & (int)d2); }     \
constexpr T operator^ (T d1, T d2) { return (T)((int)d1 ^ (int)d2); }     \
inline T& operator|= (T& d1, T d2) { return (T&)((int&)d1 |= (int)d2); }  \
inline T& operator&= (T& d1, T d2) { return (T&)((int&)d1 &= (int)d2); }  \
inline T& operator^= (T& d1, T d2) { return (T&)((int&)d1 ^= (int)d2); }

#define ENABLE_BASE_OPERATORS_ON(T)                                \
constexpr T operator+(T d1, int d2) { return T(int(d1) + d2); }    \
constexpr T operator-(T d1, int d2) { return T(int(d1) - d2); }    \
constexpr T operator-(T d) { return T(-int(d)); }                  \
inline T& operator+=(T& d1, int d2) { return d1 = d1 + d2; }       \
inline T& operator-=(T& d1, int d2) { return d1 = d1 - d2; }

#define ENABLE_INCR_OPERATORS_ON(T)                                \
inline T& operator++(T& d) { return d = T(int(d) + 1); }           \
inline T& operator--(T& d) { return d = T(int(d) - 1); }

#define ENABLE_FULL_OPERATORS_ON(T)                                \
ENABLE_BASE_OPERATORS_ON(T)                                        \
constexpr T operator*(int i, T d) { return T(i * int(d)); }        \
constexpr T operator*(T d, int i) { return T(int(d) * i); }        \
constexpr T operator/(T d, int i) { return T(int(d) / i); }        \
constexpr int operator/(T d1, T d2) { return int(d1) / int(d2); }  \
constexpr int operator%(T d1, T d2) { return int(d1) % int(d2); }   \
inline T& operator*=(T& d, int i) { return d = T(int(d) * i); }    \
inline T& operator/=(T& d, int i) { return d = T(int(d) / i); }

		ENABLE_FULL_OPERATORS_ON(Value)
		ENABLE_FULL_OPERATORS_ON(Direction)

		ENABLE_INCR_OPERATORS_ON(Piece)
		ENABLE_INCR_OPERATORS_ON(PieceType)
		ENABLE_INCR_OPERATORS_ON(Square)
		ENABLE_INCR_OPERATORS_ON(File)
		ENABLE_INCR_OPERATORS_ON(Rank)
		ENABLE_INCR_OPERATORS_ON(Color)
		//ENABLE_INCR_OPERATORS_ON(CheckCount)

		ENABLE_BASE_OPERATORS_ON(Score)

		ENABLE_BASE_OPERATORS_ON(PieceType)
		//ENABLE_BIT_OPERATORS_ON(RiderType)
		//ENABLE_BASE_OPERATORS_ON(RiderType)

#undef ENABLE_FULL_OPERATORS_ON
#undef ENABLE_INCR_OPERATORS_ON
#undef ENABLE_BASE_OPERATORS_ON
#undef ENABLE_BIT_OPERATORS_ON

		constexpr PieceSet piece_set(PieceType pt) {
			return PieceSet(1ULL << pt);
		}

		constexpr PieceSet operator~ (PieceSet ps) { return (PieceSet)~(uint64_t)ps; }
		constexpr PieceSet operator| (PieceSet ps1, PieceSet ps2) { return (PieceSet)((uint64_t)ps1 | (uint64_t)ps2); }
		constexpr PieceSet operator| (PieceSet ps, PieceType pt) { return ps | piece_set(pt); }
		constexpr PieceSet operator& (PieceSet ps1, PieceSet ps2) { return (PieceSet)((uint64_t)ps1 & (uint64_t)ps2); }
		constexpr PieceSet operator& (PieceSet ps, PieceType pt) { return ps & piece_set(pt); }
		constexpr PieceSet operator^ (PieceSet ps1, PieceSet ps2) { return (PieceSet)((uint64_t)ps1 ^ (uint64_t)ps2); }
		constexpr PieceSet operator^ (PieceSet ps, PieceType pt) { return ps ^ piece_set(pt); }
		inline PieceSet& operator|= (PieceSet& ps1, PieceSet ps2) { return (PieceSet&)((uint64_t&)ps1 |= (uint64_t)ps2); }
		inline PieceSet& operator|= (PieceSet& ps, PieceType pt) { return ps |= piece_set(pt); }
		inline PieceSet& operator&= (PieceSet& ps1, PieceSet ps2) { return (PieceSet&)((uint64_t&)ps1 &= (uint64_t)ps2); }
		//inline PieceSet& operator&= (PieceSet& ps, PieceType pt) does not make sense
		inline PieceSet& operator^= (PieceSet& ps1, PieceSet ps2) { return (PieceSet&)((uint64_t&)ps1 ^= (uint64_t)ps2); }
		inline PieceSet& operator^= (PieceSet& ps, PieceType pt) { return ps ^= piece_set(pt); }

	/*	static_assert(piece_set(PAWN)& PAWN);
		static_assert(piece_set(KING)& KING);*/

	



	/// Additional operators to add a Direction to a Square
	constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
	constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
	inline Square& operator+=(Square& s, Direction d) { return s = s + d; }
	inline Square& operator-=(Square& s, Direction d) { return s = s - d; }

	constexpr bool operator<(Value a, Value b) { return ((int)a) < ((int)b); }

	constexpr Color operator~(Color c) {
		return Color(c ^ BLACK); // Toggle color
	}

	inline Color color_of(Piece pc) {
		assert(pc != NO_PIECE);
		return pc < PIECE_TYPE_NB ? WHITE : BLACK;
	}

	constexpr PieceType type_of(Piece pc) {
		return pc < PIECE_TYPE_NB ? PieceType(pc) : PieceType(pc - PIECE_TYPE_NB);
	}

	constexpr MoveType type_of(Move m) {
		return MoveType(m & (7 << SPECIAL_BITS));
	}

	template<MoveType T>
	constexpr Move make_move(Square from, Square to, PieceType promotionPc = NO_PIECE_TYPE) {
		int promoPart = (promotionPc != NO_PIECE_TYPE ? KNIGHT - promotionPc : 0) << (2 * SQUARE_BITS);
		return Move(T + (from << SQUARE_BITS) + to + promoPart);
	}

	constexpr Square make_square(File f, Rank r) {
		return Square(r * FILE_NB + f);
	}

	constexpr Square to_sq(Move m) {
		return Square(m & (int) SQUARE_BIT_MASK);
	}

	constexpr Square from_sq(Move m) {
		return Square((m >> SQUARE_BITS) & (int) SQUARE_BIT_MASK);
	}

	inline int from_to(Move m) {
		return to_sq(m) + (from_sq(m) << SQUARE_BITS);
	}

	constexpr bool is_ok(Square s) {
		return s >= SQ_A1 && s <= SQ_MAX;
	}

	inline bool is_ok(Move m) {
		return from_sq(m) != to_sq(m);
	}

	/// Only declared but not defined. We don't want to multiply two scores due to
	/// a very high risk of overflow. So user should explicitly convert to integer.
	Score operator*(Score, Score) = delete;

	/// Division of a Score must be handled separately for each term
	inline Score operator/(Score s, int i) {
		return make_score(mg_value(s) / i, eg_value(s) / i);
	}

	/// Multiplication of a Score by an integer. We check for overflow in debug mode.
	inline Score operator*(Score s, int i) {

		Score result = Score(int(s) * i);

		assert(eg_value(result) == (i * eg_value(s)));
		assert(mg_value(result) == (i * mg_value(s)));
		assert((i == 0) || (result / i) == s);

		return result;
	}

	/// Multiplication of a Score by a boolean
	inline Score operator*(Score s, bool b) {
		return b ? s : SCORE_ZERO;
	}

	constexpr File file_of(Square s) {
		return File(s % FILE_NB);
	}

	constexpr Rank rank_of(Square s) {
		return Rank(s / FILE_NB);
	}

	constexpr Rank relative_rank(Color c, Rank r, Rank maxRank = RANK_8) {
		return Rank(c == WHITE ? r : maxRank - r);
	}

	constexpr Rank relative_rank(Color c, Square s, Rank maxRank = RANK_8) {
		return relative_rank(c, rank_of(s), maxRank);
	}

	constexpr Square relative_square(Color c, Square s, Rank maxRank = RANK_8) {
		return make_square(file_of(s), relative_rank(c, s, maxRank));
	}

	constexpr Piece make_piece(Color c, PieceType pt) {
		return Piece(pt + 7 * c);
	}

}