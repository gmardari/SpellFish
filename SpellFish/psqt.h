#pragma once

#include "types.h"


namespace Spellfish
{
	namespace PSQT
	{
		auto constexpr S = make_score;

		const Score pieceValues[PIECE_TYPE_NB] = {
			S(0,0), S(PawnValueMg, PawnValueEg), S(KnightValueMg, KnightValueEg), S(BishopValueMg, BishopValueEg), S(RookValueMg, RookValueEg), S(QueenValueMg, QueenValueEg), S(VALUE_MATE, VALUE_MATE)
		};

		const Score psqt[PIECE_TYPE_NB][FILE_NB/2][RANK_NB] = {
			{},
			//Pawns
			{
				{ S(0, 0), S(0, 0), S(10, 10), S(15, 15), S(25, 20), S(60, 25), S(100, 30), S(0, 35) },
				{ S(0, 0), S(0, 0), S(10, 10), S(15, 15), S(25, 20), S(60, 25), S(100, 30), S(0, 35) },
				{ S(0, 0), S(0, 0), S(10, 10), S(15, 15), S(25, 20), S(60, 25), S(100, 30), S(0, 35) },
				{ S(0, 0), S(0, 0), S(10, 10), S(15, 15), S(25, 20), S(60, 25), S(100, 30), S(0, 35) }
			},
			{},
			{},
			{},
			{},
			{	{ S(0, 0), S(0, 0), S(5, 10), S(10, 15), S(15, 20), S(20, 25), S(25, 30), S(25, 35) },
				{ S(0, 0), S(0, 0), S(5, 10), S(10, 15), S(15, 20), S(20, 25), S(25, 30), S(25, 35) },
				{ S(0, 0), S(0, 0), S(5, 10), S(10, 15), S(15, 20), S(20, 25), S(25, 30), S(25, 35) },
				{ S(0, 0), S(0, 0), S(5, 10), S(10, 15), S(15, 20), S(20, 25), S(25, 30), S(25, 35) }
			}
		};
	}
}