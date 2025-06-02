#pragma once

#include "types.h"
#include "bitboard.h"
#include <string>
#include <cassert>
#include <deque>
#include <memory> // For std::unique_ptr

namespace Spellfish
{
	
	const std::string PIECE_TO_CHAR = " PNBRQK pnbrqk";
	//const std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	const std::string START_FEN = "4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1";

	struct StateInfo
	{
		//Copied
		int castleRights;
	
	

		//Not copied, recomputed
		Move move;
		Key key;
		Piece capturedPiece;
		StateInfo* previous;

		bool inCheck;
		Bitboard checkersBB;
		Bitboard attacksBB[COLOR_NB][PIECE_TYPE_NB];
		Square epSquare;
	
	};

	class Position
	{
	public:
		Position() = default;
		static void init();

		void set(const std::string& fenStr, StateInfo* st);
		std::string fen() const;

		const std::string& piece_to_char() const;
		const Color side_to_move() const;
		bool in_check() const;
		bool is_game_end(Value& val) const;
		void calculate_attacks(Color us);


		Bitboard pieces(PieceType pt = ALL_PIECES) const;
		Bitboard pieces(PieceType pt1, PieceType pt2) const;
		Bitboard pieces(Color c) const;
		Bitboard pieces(Color c, PieceType pt) const;
		Square piece_square(Color c, PieceType pt) const;
		Square piece_square(Piece piece) const;
		Bitboard checkers() const;
		Bitboard attackers_to(Color us, Square sq);
		Bitboard attacks_from(Piece piece, Square sq);
		Bitboard moves_from(Piece piece, Square sq);
		bool has_attack_on(Piece piece, Square piece_sq, Square attack_sq);
		Piece piece_on(Square s) const;
		Piece moved_piece(Move m) const;

		Direction pawn_push(Color c) const;

		void do_move(Move m, StateInfo& newSt);
		void undo_move(Move m);

		void put_piece(Piece pc, Square s, bool isPromoted = false, Piece unpromotedPc = NO_PIECE);
		void remove_piece(Square s);

		StateInfo* state() const;
		int ply() const;

	private:
		void move_piece(Square from, Square to);

		Color sideToMove;
		//bool isChecked;
		StateInfo* st;

		Piece board[SQUARE_NB];
		Bitboard byTypeBB[PIECE_TYPE_NB];
		Bitboard byColorBB[COLOR_NB];
		//Bitboard checkersBB;

		int gamePly;
	};


	/// A list to keep track of the position states along the setup moves (from the
	/// start position to the position just before the search starts). Needed by
	/// 'draw by repetition' detection. Use a std::deque because pointers to
	/// elements are not invalidated upon list resizing.
	typedef std::unique_ptr<std::deque<StateInfo>> StateListPtr;

	inline int Position::ply() const
	{
		return gamePly;
	}


	inline StateInfo* Position::state() const
	{
		return st;
	}

	inline Bitboard Position::checkers() const
	{
		return st->checkersBB;
	}

	inline bool Position::is_game_end(Value& val) const
	{
		if (st->move == MOVE_NONE)
			return false;

		if (st->capturedPiece != NO_PIECE)
		{
			PieceType pt = type_of(st->capturedPiece);
			Color pt_color = color_of(st->capturedPiece);

			if (pt == KING)
			{
				val = VALUE_MATE * (pt_color == WHITE ? -1 : 1);
				return true;
			}
		}

		const Square to = to_sq(st->move);
		const Piece pc = piece_on(to);

		if (type_of(pc) == PAWN && ((rank_bb(relative_rank(~sideToMove, RANK_8)) & to) != 0))
		{
			val = VALUE_MATE * (~sideToMove == WHITE ? 1 : -1);
			return true;
		}

		return false;
	}

	inline bool Position::in_check() const
	{
		return st->inCheck;
	}

	

	inline const Color Position::side_to_move() const
	{
		return sideToMove;
	}

	inline const std::string& Position::piece_to_char() const {
		return PIECE_TO_CHAR;
	}

	inline void Position::put_piece(Piece pc, Square s, bool isPromoted, Piece unpromotedPc) {

		board[s] = pc;
		byTypeBB[ALL_PIECES] |= byTypeBB[type_of(pc)] |= s;
		byColorBB[color_of(pc)] |= s;
		/*pieceCount[pc]++;
		pieceCount[make_piece(color_of(pc), ALL_PIECES)]++;
		psq += PSQT::psq[pc][s];
		if (isPromoted)
			promotedPieces |= s;
		unpromotedBoard[s] = unpromotedPc;*/
	}

	inline void Position::remove_piece(Square s) {

		Piece pc = board[s];
		byTypeBB[ALL_PIECES] ^= s;
		byTypeBB[type_of(pc)] ^= s;
		byColorBB[color_of(pc)] ^= s;
		board[s] = NO_PIECE;
	/*	pieceCount[pc]--;
		pieceCount[make_piece(color_of(pc), ALL_PIECES)]--;
		psq -= PSQT::psq[pc][s];
		promotedPieces -= s;
		unpromotedBoard[s] = NO_PIECE;*/
	}

	inline void Position::move_piece(Square from, Square to) {

		Piece pc = board[from];
		Bitboard fromTo = square_bb(from) ^ to; // from == to needs to cancel out
		byTypeBB[ALL_PIECES] ^= fromTo;
		byTypeBB[type_of(pc)] ^= fromTo;
		byColorBB[color_of(pc)] ^= fromTo;
		board[from] = NO_PIECE;
		board[to] = pc;
		//psq += PSQT::psq[pc][to] - PSQT::psq[pc][from];
		/*if (is_promoted(from))
			promotedPieces ^= fromTo;
		unpromotedBoard[to] = unpromotedBoard[from];
		unpromotedBoard[from] = NO_PIECE;*/
	}


	inline Bitboard Position::pieces(PieceType pt) const {
		return byTypeBB[pt];
	}

	inline Bitboard Position::pieces(PieceType pt1, PieceType pt2) const {
		return pieces(pt1) | pieces(pt2);
	}

	inline Bitboard Position::pieces(Color c) const {
		return byColorBB[c];
	}

	inline Bitboard Position::pieces(Color c, PieceType pt) const {
		return pieces(c) & pieces(pt);
	}

	inline Direction Position::pawn_push(Color c) const
	{
		return c == WHITE ? NORTH : SOUTH;
	}

	inline Piece Position::piece_on(Square s) const {
		assert(is_ok(s));
		return board[s];
	}

	inline Piece Position::moved_piece(Move m) const {
		return piece_on(from_sq(m));
	}
}