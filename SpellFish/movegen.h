#pragma once

// Include the vector library
#include <vector>
#include <algorithm>
#include "types.h"
#include "position.h"
#include "psqt.h"

#define MAX_MOVES 128

namespace Spellfish
{
	
	enum GenType {
		CAPTURES,
		QUIETS,
		EVASIONS,
		NON_EVASIONS,
		PROMOTIONS,
		LEGAL
	};


	struct ExtMove {
		Move move;
		int value = 0;

		operator Move() const { return move; }
		void operator=(Move m) { move = m; }

		// Inhibit unwanted implicit conversions to Move
		// with an ambiguity that yields to a compile error.
		operator float() const = delete;
	};

	namespace
	{
		bool cmp_moves(ExtMove a, ExtMove b) { return a.value > b.value; }
	}

	namespace Movegen
	{
		template<GenType T>
		ExtMove* generate(const Position& pos, Color side, ExtMove* movelist);
	}

	template<GenType T>
	struct MoveList
	{
	private:
		const Position& pos;
		Move ttmove;
		Color side;
		//std::vector<ExtMove> list;
		ExtMove* list;
		ExtMove* last;
		bool allocated_list;

		
		
	public:
		//explicit MoveList(const Position& pos, GenType type, Move ttmove = MOVE_NONE, ExtMove* list = nullptr) : MoveList(pos, pos.side_to_move(), type, ttmove, list) {};
		//explicit MoveList(const Position& pos, Color side, GenType type, Move ttmove = MOVE_NONE, ExtMove* list = nullptr);
		explicit MoveList(const Position& pos, Move ttmove = MOVE_NONE, ExtMove* list = nullptr) : MoveList(pos, pos.side_to_move(), ttmove, list) {};
		explicit MoveList(const Position& pos, Color side, Move ttmove = MOVE_NONE, ExtMove* list = nullptr) : pos(pos), ttmove(ttmove), side(side)
		{
			if (list != nullptr)
			{
				this->list = list;
				this->last = list;

				this->allocated_list = false;
			}
			else
			{
				this->list = this->last = new ExtMove[MAX_MOVES];
				this->allocated_list = true;
			}

			this->last = Movegen::generate<T>(pos, side, this->list);
		}
		~MoveList() { if (allocated_list) delete[] list; }

		void sort()
		{
			for (auto it = list; it != last; ++it)
			{
				ExtMove move = *it;
				Square from = from_sq(move);
				Square to = to_sq(move);

				const Piece moved_piece = pos.moved_piece(move);
				const Piece captured = pos.piece_on(to);

				if (ttmove != MOVE_NONE && move.move == ttmove)
				{
					it->value += 10000;
				}

				if (captured != NO_PIECE_TYPE)
				{
					//it->value += 10;

					it->value += 5 * PSQT::pieceValues[type_of(captured)] - PSQT::pieceValues[type_of(moved_piece)];
				}

				if ((pos.state()->attacksBB[~side][PAWN] & to) != 0)
				{
					it->value -= PSQT::pieceValues[type_of(moved_piece)];
				}

			}

			std::sort(begin(), end(), cmp_moves);
		}
	
		//std::vector<ExtMove>::const_iterator begin() { return list.begin(); }
		//std::vector<ExtMove>::const_iterator end() { return list.end(); }
		ExtMove* begin() const { return list; }
		ExtMove* end() const { return last; }
		ExtMove at(int i) { return list[i]; }
		int size() const { return last - list; }
		bool contains(Move move) { return std::find(begin(), end(), move) != end(); }
		void merge(const ExtMove* it_begin, const ExtMove* it_end)
		{
			ExtMove* curr = this->last;

			for (auto it = it_begin; it != it_end; ++it)
			{
				Move m = *it;

				if(!contains(m))
					*curr++ = m;
			}

			this->last = curr;
		}
	};

	
}