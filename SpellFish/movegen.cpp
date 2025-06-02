#include "movegen.h"
#include "bitboard.h"
#include "types.h"

namespace Spellfish
{
	namespace
	{
		template<GenType T>
		ExtMove* generate_king_moves(const Position& pos, Color side, ExtMove* movelist)
		{
			Color us = side;
			Color them = ~us;
			const Bitboard theirPieces = pos.pieces(them);
			const Bitboard ourPieces = pos.pieces(us);
			const Square ksq = pos.piece_square(us, KING);

			const Bitboard theirAttacks = pos.state()->attacksBB[them][ALL_PIECES];

			Bitboard moves = Bitbases::KingAttacks[ksq] & (~ourPieces & ~theirAttacks);
			Bitboard quiets = moves & ~theirPieces;
			Bitboard attacks = moves & theirPieces;

			if constexpr(T == LEGAL)
				while (moves)
				{
					Square to = pop_lsb(moves);

					*movelist++ = make_move<NORMAL>(ksq, to);
				}
			else if constexpr (T == CAPTURES)
			{
				while (attacks)
				{
					Square to = pop_lsb(attacks);
					
					*movelist++ = make_move<NORMAL>(ksq, to);
				}
			}

			return movelist;
		}

		template<GenType T>
		ExtMove* generate_pawn_moves(const Position& pos, Color side, ExtMove* movelist, Bitboard target)
		{
			Color us = side;
			Color them = ~us;
			Bitboard lastRank = us == WHITE ? Rank8BB : Rank1BB;
			Bitboard promoRank = us == WHITE ? Rank7BB : Rank2BB;
			Bitboard pieces = pos.pieces(us, PAWN) & ~lastRank; //Todo: remove rank8bb
			Bitboard theirPieces = pos.pieces(them);
			Bitboard allPieces = pos.pieces();
			const Bitboard doubleJumpBB = us == WHITE ? Rank2BB : Rank7BB;
			const Direction pawn_push = pos.pawn_push(us);
			const Bitboard ep_bb = pos.state()->epSquare <= SQ_MAX ? square_bb(pos.state()->epSquare) : Bitboard(0);

			const Bitboard blocked = pawn_push == NORTH ? shift<SOUTH>(allPieces) : shift<NORTH>(allPieces);
			const Bitboard blocked2 = blocked | (pawn_push == NORTH ? shift<SOUTH>(blocked) : shift<NORTH>(blocked));

			Bitboard jump1 = pieces & ~(blocked);
			Bitboard jump2 = (pieces & doubleJumpBB) & ~(blocked2);


			Bitboard b1 = shift(pawn_push, jump1) & target;
			Bitboard b2 = shift(pawn_push, shift(pawn_push, jump2)) & target;
			Bitboard b3 = pieces;

			Bitboard promos_move = (b1 & lastRank);
			Bitboard promos_captures = b3 & promoRank;

			b1 &= ~promos_move;
			b3 &= ~promos_captures;

			if constexpr (T == LEGAL || T == QUIETS)
			{
				while (b1)
				{
					Square to = pop_lsb(b1);
					Square sq = to - pawn_push;

					ExtMove move;
					move = make_move<NORMAL>(sq, to);

					*movelist++ = move;
				}

				
				while (b2)
				{
					Square to = pop_lsb(b2);
					Square sq = to - pawn_push - pawn_push;

					/*if ((target & to) == 0)
						continue;*/

					ExtMove move;
					move = make_move<NORMAL>(sq, to);

					*movelist++ = move;
				}
			}
			
			if constexpr (T == PROMOTIONS || T == LEGAL)
			{
				while (promos_move)
				{
					Square to = pop_lsb(promos_move);
					Square sq = to - pawn_push;

					*movelist++ = make_move<PROMOTION>(sq, to, QUEEN);
				}

				while (promos_captures)
				{
					Square sq = pop_lsb(promos_captures);
					Bitboard attacks = Bitbases::PawnAttacks[us][sq] & theirPieces & target;

					while (attacks)
					{
						Square attackSq = pop_lsb(attacks);
						*movelist++ = make_move<PROMOTION>(sq, attackSq, QUEEN);
					}
				}
			}

			if constexpr (T == LEGAL || T == CAPTURES)
			{
				while (b3)
				{
					Square sq = pop_lsb(b3);
					Bitboard attacks = Bitbases::PawnAttacks[us][sq] & (theirPieces | ep_bb) & target;
					//Bitboard captureEast = shift(pawn_push + EAST, square_bb(sq)); //shift used to make sure we dont overflow the board dimensions
					//Bitboard captureWest = shift(pawn_push + WEST, square_bb(sq));

					while (attacks)
					{
						Square attackSq = pop_lsb(attacks);

						ExtMove move;
						move = make_move<NORMAL>(sq, attackSq);

						*movelist++ = move;
					}

				}
			}
			

			return movelist;
		}

		template<GenType T>
		ExtMove* generate_all(const Position& pos, Color side, ExtMove* movelist)
		{
			Bitboard target = pos.in_check() ? pos.checkers() : AllSquares; //TODO: Make in check() check for which side
			movelist = generate_pawn_moves<T>(pos, side, movelist, target);

			if constexpr (T != PROMOTIONS)
			{
				movelist = generate_king_moves<T>(pos, side, movelist);
			}

			return movelist;
		}

		
	}



 

	


	template<GenType T>
	ExtMove* Movegen::generate(const Position& pos, Color side, ExtMove* movelist)
	{
		return generate_all<T>(pos, side, movelist);
	}
	
	// Explicit template instantiations
	template ExtMove* Movegen::generate<CAPTURES>(const Position&, Color, ExtMove*);
	template ExtMove* Movegen::generate<QUIETS>(const Position&, Color, ExtMove*);
	template ExtMove* Movegen::generate<EVASIONS>(const Position&, Color, ExtMove*);
	template ExtMove* Movegen::generate<NON_EVASIONS>(const Position&, Color, ExtMove*);
	template ExtMove* Movegen::generate<LEGAL>(const Position&, Color, ExtMove*);
	template ExtMove* Movegen::generate<PROMOTIONS>(const Position&, Color, ExtMove*);





}