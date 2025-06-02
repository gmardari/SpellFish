#pragma once

#include "types.h"
#include "evaluate.h"
#include "position.h"
#include "movegen.h"

namespace Spellfish
{
	constexpr int MAX_PLY = 64; //Includes qsearch depth

	struct SearchLimits
	{
		Depth depth = 0;
		uint64_t timeMs = 0;
	};

	struct SearchVars
	{
		uint64_t finishTime;

		SearchVars() : finishTime(0) {}
		//SearchVars(uint64_t finishTime) : finishTime(finishTime) {}
	};

	extern SearchVars svars;

	struct Eval
	{
		Value eval;
		Move bestMove;

		Eval() : eval(VALUE_ZERO), bestMove(MOVE_NONE) {}
		Eval(Value _eval, Move _bestMove) : eval(_eval), bestMove(_bestMove) {}
	};

	struct Stack
	{
		Move currentMove;
		StateInfo st;
		ExtMove moves[MAX_MOVES];
		int pvIndex;
		int ply;
	};

	struct PVList
	{
		Move moves[MAX_PLY];
		Depth maxPly;
	};

	
	extern Stack search_stack[MAX_PLY + 2];

	namespace Search
	{
		struct Stats
		{
			int nodesSearched = 0;
			int nodesPruned = 0;
			int cutoffs = 0;
			int alphaCutoffs = 0;
			int betaCutoffs = 0;
			int tt_hits = 0;
			int reductions = 0;
			int qnodes = 0;
		};
		
		void init();
		Eval search(Position& pos, SearchLimits limits);
		Eval search(Position& pos, SearchLimits limits, Stats& stats);
		//Eval minimax(Position& pos, Depth depth, Value alpha, Value beta, bool maximizingPlayer, Stack* ss, Stats& stats);
		int count_moves(Position& pos, Depth depth, std::vector<ExtMove>& vec);
		int count_moves(Position& pos, Depth depth, Stack* ss);
	}
}