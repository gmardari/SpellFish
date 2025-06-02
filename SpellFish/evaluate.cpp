
#include "evaluate.h"
#include "types.h"
#include "bitboard.h"
#include "psqt.h"
#include "movegen.h"

namespace Spellfish
{
	Value score_pawn_structure_mg(const Position& pos, Color us)
	{
		Value score = VALUE_ZERO;

		Color them = ~us;
		Bitboard pieces = pos.pieces(us, PAWN);
		Bitboard allPieces = pos.pieces();
		const Direction pawn_push = pos.pawn_push(us);

		const Bitboard blocked = (pawn_push == NORTH ? shift<SOUTH>(allPieces) : shift<NORTH>(allPieces)) & pieces;
		const int numBlocked = count_bits(blocked);
		score -= numBlocked * blockedPawnScore;
		bool pawns[FILE_MAX + 1];

		for (File file = FILE_A; file <= FILE_MAX; ++file)
		{
			const Bitboard bb = file_bb(file);
			const int file_pawns = count_bits(bb & pieces);

			pawns[file] = (file_pawns > 0);
		}

		for (int i = 0; i < FILE_MAX + 1; i++)
		{
			if (pawns[i])
			{
				bool hasPrevPawn = i == 0 ? false : pawns[i - 1];
				bool hasAfterPawn = i == 7 ? false : pawns[i + 1];

				if (!hasPrevPawn && !hasAfterPawn)
					score -= isolatedPawnScore;
			}
		}

		return score;
	}

	Value score_mobility_mg(const Position& pos, Color us)
	{
		MoveList<LEGAL> moves(pos, us, MOVE_NONE);

		return moves.size() * mobilityScore;

	}

	Value score_piece_mg(const Position& pos, Color us, PieceType pt)
	{
		Value score = VALUE_ZERO;
		const Value piece_score = mg_value(PSQT::pieceValues[pt]);
		Bitboard pieces = pos.pieces(us, pt);

		while (pieces)
		{
			Square sq = pop_lsb(pieces);
				
			File file = File(file_of(sq) % 4);
			Rank rank = relative_rank(us, sq);
			
			score += piece_score + mg_value(PSQT::psqt[pt][file][rank]);
		}

		return score;
	}


	Value evaluate_pos_mg(const Position& pos)
	{
		Value score = VALUE_ZERO;

		//score pawns
		score += score_piece_mg(pos, WHITE, PAWN) - score_piece_mg(pos, BLACK, PAWN);
		score += score_piece_mg(pos, WHITE, KING) - score_piece_mg(pos, BLACK, KING);
		score += score_mobility_mg(pos, WHITE) - score_mobility_mg(pos, BLACK);
		score += score_pawn_structure_mg(pos, WHITE) - score_pawn_structure_mg(pos, BLACK);

		return score;
	}
}