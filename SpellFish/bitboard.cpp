
#include "bitboard.h"
using namespace Spellfish::Bitbases;

namespace Spellfish
{
	namespace Bitbases
	{
		Bitboard PawnAttacks[COLOR_NB][SQUARE_NB];
		Bitboard KingAttacks[SQUARE_NB];

		void init()
		{
			//Pawn attacks
			for (Color us = WHITE; us <= BLACK; ++us)
			{
				const Direction pawn_push = us == WHITE ? NORTH : SOUTH;
				const Bitboard undefinedRanks = (Rank1BB | Rank8BB);

				for (Square sq = SQ_A1; sq <= SQ_MAX; ++sq)
				{
					Bitboard sq_bb = square_bb(sq);
					Bitboard captureEast = shift(pawn_push + EAST, sq_bb); //shift used to make sure we dont overflow the board dimensions
					Bitboard captureWest = shift(pawn_push + WEST, sq_bb);


					if ((undefinedRanks & sq) != 0)
					{
						PawnAttacks[us][sq] = 0;
					}
					else
					{
						PawnAttacks[us][sq] = captureEast | captureWest;
					}

				}
			}

			//King attacks
			for (Square sq = SQ_A1; sq <= SQ_MAX; ++sq)
			{
				KingAttacks[sq] = adjacent_bb(sq);

			}
		}

		Bitboard attacks_from_bb(Piece piece, Square sq)
		{
			const PieceType pt = type_of(piece);

			if (pt == PAWN)
			{
				Color us = color_of(piece);
				return PawnAttacks[us][sq];
			}
			else if (pt == KING)
			{
				return KingAttacks[sq];
			}

			return Bitboard(0);
		}
	}
}