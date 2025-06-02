
#include <iostream>
#include <limits>
#include <string>
#include "search.h"
#include "movegen.h"
#include "evaluate.h"
#include "tt.h"
#include "uci.h"
#include "misc.h"


namespace Spellfish
{
	Depth iterationMaxPly;
	Depth searchMaxPly;
	Stack search_stack[MAX_PLY + 2];
	SearchVars svars;
	PVList pv_list;
	Move* pvArray;

	namespace
	{
		void movcpy(Move* pTarget, const Move* pSource, int n) {
			while (n--)
			{
				Move m = *pSource;
				*pTarget++ = *pSource++;

				if (m == MOVE_NONE)
					break;
			}
		}

		bool confirm_pv_validity(Position& pos, Move* pvArray, int length, Stack* ss)
		{
			bool confirmed = true;
			int i;
			int ply = pos.ply();
			std::string fen = pos.fen();

			for (i = 0; i < length; ++i)
			{
				Move m = pvArray[i];
				MoveList<LEGAL> moves(pos, MOVE_NONE, (ss + i)->moves);

				if (!moves.contains(m))
				{
					confirmed = true;
					break;
				}

				pos.do_move(m, (ss + i + 1)->st);
			}
			//undo moves
			for (int j = i - 1; j >= 0; --j)
			{
				Move m = pvArray[j];
				pos.undo_move(m);
			}

			assert(ply == pos.ply() && fen == pos.fen());

			return confirmed;
		}

	

		Value qsearch(Position& pos, Depth depth, Value alpha, Value beta, bool maximizingPlayer, Stack* ss, Search::Stats& stats)
		{
			Value gameValue = VALUE_ZERO;
			bool game_end = pos.is_game_end(gameValue);

			if (game_end)
				return gameValue;

			Value stand_pat = evaluate_pos_mg(pos);
			Value bestValue = stand_pat;

			if (depth < 0)
				stats.qnodes++;

			if (maximizingPlayer)
			{
				alpha = std::max(alpha, bestValue);

				if (beta <= alpha)
				{
					return bestValue;
				}

				
				MoveList<CAPTURES> moves(pos, MOVE_NONE, ss->moves);
				MoveList<PROMOTIONS> promotions(pos);
				int s1 = moves.size(), s2 = promotions.size();
				moves.merge(promotions.begin(), promotions.end());
				moves.sort();


				for (int i = 0; i < moves.size(); i++)
				{
					Move m = moves.at(i);
					Stack* next = ss + 1;
					pos.do_move(m, next->st);
					Value eval = qsearch(pos, depth - 1, alpha, beta, false, next, stats);
					pos.undo_move(m);

					bestValue = std::max(bestValue, eval);

					//minEval = std::min(eval, minEval);
					alpha = std::max(alpha, eval);

					if (beta <= alpha)
					{
						break;
					}

					if (svars.finishTime != 0 && timeSinceEpochMillisec() >= svars.finishTime)
						break;
				}
			}
			else
			{
				beta = std::min(beta, bestValue);

				if (beta <= alpha)
				{
					return bestValue;
				}
				MoveList<CAPTURES> moves(pos, MOVE_NONE, ss->moves);
				MoveList<PROMOTIONS> promotions(pos);
				int s1 = moves.size(), s2 = promotions.size();
				moves.merge(promotions.begin(), promotions.end());
				moves.sort();

				for (int i = 0; i < moves.size(); i++)
				{
					Move m = moves.at(i);
					Stack* next = ss + 1;
					pos.do_move(m, next->st);
					Value eval = qsearch(pos, depth - 1, alpha, beta, true, next, stats);
					pos.undo_move(m);

					bestValue = std::min(bestValue, eval);

					//minEval = std::min(eval, minEval);
					beta = std::min(beta, eval);

					if (beta <= alpha)
					{
						break;
					}

					if (svars.finishTime != 0 && timeSinceEpochMillisec() >= svars.finishTime)
						break;
				}
			}

			return bestValue;
		}

		Eval minimax(Position& pos, Depth depth, Value alpha, Value beta, bool maximizingPlayer, Stack* ss, Search::Stats& stats)
		{
			/*	std::vector<ExtMove> movelist;
				movelist.reserve(128);
				Movegen::generate_legal(pos, movelist);*/


			Value gameValue = VALUE_ZERO;
			bool game_end = pos.is_game_end(gameValue);


			stats.nodesSearched++;

			if (depth <= 0 || game_end)
			{
				if (game_end)
				{
					//std::cout << "found game end " << gameValue << std::endl;
					return Eval(gameValue, MOVE_NONE);
				}

				//return Eval(evaluate_pos_mg(pos), MOVE_NONE);
				return Eval(qsearch(pos, depth, alpha, beta, maximizingPlayer, ss + 1, stats), MOVE_NONE);
			}


			//Check TT if depth != 0 and not game end
			bool tt_hit = false;
			Key ttkey = pos.state()->key;
			TTEntry* tte = TT.probe(ttkey, tt_hit);
			tt_hit = false;

			if (tt_hit && tte->depth >= depth)
			{
				stats.tt_hits++;
			/*	int nextPvIndex = ss->pvIndex + iterationMaxPly - ss->ply;

				if (maximizingPlayer)
				{
					if (tte->eval > alpha)
					{
						alpha = tte->eval;
						pvArray[ss->pvIndex] = tte->bestMove;
						movcpy(pvArray + ss->pvIndex + 1, pvArray + nextPvIndex, iterationMaxPly - ss->ply - 1);
					}
				}
				else
				{
					if (tte->eval < beta)
					{
						beta = tte->eval;
						pvArray[ss->pvIndex] = tte->bestMove;
						movcpy(pvArray + ss->pvIndex + 1, pvArray + nextPvIndex, iterationMaxPly - ss->ply - 1);
					}
				}*/

				return Eval(tte->eval, tte->bestMove);
			}

			MoveList<LEGAL> movelist(pos, tt_hit ? tte->bestMove : MOVE_NONE, ss->moves); //Ordered move list
			movelist.sort();

			//Stalemate
			if (movelist.size() == 0)
			{
				return Eval(VALUE_DRAW, MOVE_NONE);
			}

			if (maximizingPlayer)
			{
				Value maxEval = Value(std::numeric_limits<int>::min());
				Move bmove = MOVE_NONE;
				int numChildren = 0;
				/*int cutoffs = stats.alphaCutoffs;*/
				//bool doLMR = false;
				
			/*	if (tt_hit)
				{
					Stack* next = (ss + 1);
					next->ply = ss->ply + 1;
					next->pvIndex = ss->pvIndex + iterationMaxPly - ss->ply;


					if (tte->eval > alpha)
					{
						alpha = tte->eval;
						pvArray[ss->pvIndex] = tte->bestMove;
						movcpy(pvArray + ss->pvIndex + 1, pvArray + next->pvIndex, iterationMaxPly - ss->ply - 1);
					}

					if (beta <= alpha)
					{
						stats.nodesPruned += movelist.size() - numChildren;
						stats.cutoffs++;
						stats.alphaCutoffs++;

						return Eval(tte->eval, tte->bestMove);
					}

				
					pos.do_move(tte->bestMove, next->st);
					minimax(pos, depth - 1, alpha, beta, !maximizingPlayer, next, stats);
					pos.undo_move(tte->bestMove);

					return Eval(tte->eval, tte->bestMove);
				}*/


				for (Move m : movelist)
				{

					//std::string moveStr = UCI::move(pos, m);
					numChildren++;
					//bool lastRank = (Rank8BB & to_sq(m)) != 0;

				/*	if (!doLMR)
						doLMR = (numChildren > 2);*/

					Depth R = (depth >= 3 && numChildren > 2) ? (numChildren > 5 ? depth / 3 : 1) : 0;
					//int prevCutoff = stats.alphaCutoffs;


					Stack* next = (ss + 1);
					next->ply = ss->ply + 1;
					next->pvIndex = ss->pvIndex + iterationMaxPly - ss->ply;
					std::string fen = pos.fen();
					pos.do_move(m, next->st);
					Eval eval = minimax(pos, depth - 1 - R, alpha, beta, false, next, stats);

					//Reduced search failed high
					if (R > 0 && (beta <= std::max(alpha, eval.eval)))
					{
						eval = minimax(pos, depth - 1, alpha, beta, false, next, stats);
					}
					else
						stats.reductions += R;

					pos.undo_move(m);

					assert(pos.fen() == fen);

					if (eval.eval > maxEval)
					{
						maxEval = eval.eval;
						bmove = m;
					}

					//maxEval = std::max(eval, maxEval);
					if (eval.eval > alpha)
					{
						alpha = eval.eval;
						pvArray[ss->pvIndex] = m;
						movcpy(pvArray + ss->pvIndex + 1, pvArray + next->pvIndex, iterationMaxPly - ss->ply - 1);
					}

					if (beta <= alpha)
					{
						stats.nodesPruned += movelist.size() - numChildren;
						stats.cutoffs++;
						stats.alphaCutoffs++;
						break;
					}

					if (svars.finishTime != 0 && timeSinceEpochMillisec() >= svars.finishTime)
						break;
				}

				tte->save(ttkey, maxEval, depth, bmove);

				return Eval(maxEval, bmove);
			}
			else
			{
				Value minEval = Value(std::numeric_limits<int>::max());
				Move bmove = MOVE_NONE;
				int numChildren = 0;
				/*	int cutoffs = stats.betaCutoffs;
					bool doLMR = false;*/



				for (Move m : movelist)
				{

					//std::string moveStr = UCI::move(pos, m);
					numChildren++;

					/*if (!doLMR)
						doLMR = (numChildren > 2 && (stats.cutoffs - cutoffs == 0));*/

					Depth R = (depth >= 3 && numChildren > 2) ? (numChildren > 5 ? depth / 3 : 1) : 0;
					//int prevCutoff = stats.betaCutoffs;

					Stack* next = (ss + 1);
					next->ply = ss->ply + 1;
					next->pvIndex = ss->pvIndex + iterationMaxPly - ss->ply;
					std::string fen = pos.fen();

					pos.do_move(m, next->st);
					Eval eval = minimax(pos, depth - 1 - R, alpha, beta, true, next, stats);

					//Reduced search failed high
					if (R > 0 && (std::min(beta, eval.eval) <= alpha))
					{
						eval = minimax(pos, depth - 1, alpha, beta, false, next, stats);
					}
					else
						stats.reductions += R;

					pos.undo_move(m);

					assert(pos.fen() == fen);

					if (eval.eval < minEval)
					{
						minEval = eval.eval;
						bmove = m;
					}

					//minEval = std::min(eval, minEval);
					if (eval.eval < beta)
					{
						beta = eval.eval;
						pvArray[ss->pvIndex] = m;
						movcpy(pvArray + ss->pvIndex + 1, pvArray + next->pvIndex, iterationMaxPly - ss->ply - 1);
					}
					

					if (beta <= alpha)
					{
						stats.nodesPruned += movelist.size() - numChildren;
						stats.cutoffs++;
						stats.betaCutoffs++;
						break;
					}

					if (svars.finishTime != 0 && timeSinceEpochMillisec() >= svars.finishTime)
						break;
				}

				tte->save(ttkey, minEval, depth, bmove);

				return Eval(minEval, bmove);
			}

		}

		

		
	}

	void Search::init()
	{
		
	}


	Eval Search::search(Position& pos, SearchLimits limits)
	{
		Stats stats;
		return search(pos, limits, stats);
	}

	Eval Search::search(Position& pos, SearchLimits limits, Search::Stats& stats)
	{
		
		Eval eval;
		Value mate = pos.side_to_move() == WHITE ? VALUE_MATE : -VALUE_MATE;

		const int max_ply = limits.depth > 0 ? std::min(limits.depth, MAX_PLY) : MAX_PLY;
		svars.finishTime = limits.timeMs > 0 ? timeSinceEpochMillisec() + limits.timeMs : 0;

		searchMaxPly = max_ply;

		for (int ply = 1; ply <= max_ply && eval.eval != mate; ++ply)
		{
			/*for (int i = 0; i < MAX_PLY + 2; i++)
			{
				std::memset(&search_stack[i], 0, sizeof(StateInfo));
			}*/

			//Stack* ss = new Stack[depth + 2];
			stats = Stats();
			TT.new_search();
			iterationMaxPly = ply;
			pv_list.maxPly = 0;

			int N = ply;
			pvArray = new Move[(N * N + N) / 2];
			eval = minimax(pos, ply, -VALUE_MATE, VALUE_MATE, pos.side_to_move() == WHITE, search_stack, stats);
		

			

			std::cout << "info depth " << ply << " score cp " << eval.eval << " nodes " << stats.nodesSearched << " qnodes " << stats.qnodes << " prunes " << stats.nodesPruned << " reductions " << stats.reductions << " tthits " << stats.tt_hits << " tt_occuppancy " << TT.hash_occupancy();
			std::cout << " pv ";

			//Print pv 
			for (int i = 0; i < ply; ++i)
			{
				std::cout << UCI::move(pos, pvArray[i]) << " ";
			}

			std::cout << std::endl;

			if (!confirm_pv_validity(pos, pvArray, N, search_stack))
				std::cout << "PV Corrupted!" << std::endl;
			
			//std::memset(&pv_list, 0, sizeof(PVList));

			if (svars.finishTime != 0 && timeSinceEpochMillisec() > svars.finishTime)
				break;

			delete[] pvArray;
		}

		

		return eval;
	}
	
	

	int Search::count_moves(Position& pos, Depth depth, std::vector<ExtMove>& vec)
	{
		Stack* ss = new Stack[depth + 2 - 1];
		MoveList<LEGAL> moves(pos, MOVE_NONE, ss->moves);
		int totalCount = 0; //We count terminal nodes

		for (int i = 0; i < moves.size(); ++i)
		{
			ExtMove move = moves.at(i);
			pos.do_move(move, ss->st);
			int dt = count_moves(pos, depth - 1, ss + 1);
			pos.undo_move(move);
			
			move.value = dt;
			vec.push_back(move);
			totalCount += dt;
		}
		

		delete[] ss;
		return totalCount;
	}

	int Search::count_moves(Position& pos, Depth depth, Stack* ss)
	{
		if (depth <= 0)
			return 1;

		MoveList<LEGAL> moves(pos, MOVE_NONE, ss->moves);

	

		if (depth == 1)
		{
			return moves.size();
		}

		int count = 0;

		for (auto it = moves.begin(); it != moves.end(); ++it)
		{
			pos.do_move(*it, (ss + 1)->st);
			count += count_moves(pos, depth - 1, ss + 1);
			pos.undo_move(*it);
		}

		return count;
	}
	
}