
#include <string>
#include <iostream>
#include <sstream>

#include "uci.h"
#include "movegen.h"
#include "search.h"
#include "tt.h"
#include "misc.h"

using namespace std;

namespace Spellfish
{
    StateListPtr states;
    // position() is called when engine receives the "position" UCI command.
// The function sets up the position described in the given FEN string ("fen")
// or the starting position ("startpos") and then makes the moves given in the
// following move list ("moves").
    namespace
    {
        void position(Position& pos, istringstream& is)
        {
            states = StateListPtr(new std::deque<StateInfo>(1)); // Drop old and create a new one
            Move m;
            string token, fen;

            is >> token;


            if (token == "startpos")
            {
                fen = START_FEN;
                is >> token; // Consume "moves" token if any
            }
            else if (token == "fen")
                while (is >> token && token != "moves")
                    fen += token + " ";
            else
                return;

            pos.set(fen, &states->back());

            //fen / startpos has been consumed by now
            if (token == "moves")
            {
                while (is >> token)
                {
                    Move move = UCI::to_move(pos, token);

                    if (move == MOVE_NONE)
                    {
                        std::cerr << "Invalid move" << std::endl;
                        break;
                    }

                    states->emplace_back();
                    pos.do_move(move, states->back());
                }
            }

       
            std::cout << "Loaded position " << pos.fen() << std::endl;
        }

        void setoption(istringstream& is)
        {
            
        }

        void genmoves(Position& pos, istringstream& is)
        {
            string token;
            Depth depth = 0;

            while (is >> token)
            {
                if (token == "depth") is >> depth;
            }

            if (depth > 0)
            {
                uint64_t t_s = timeSinceEpochMillisec();
                std::vector<ExtMove> moves;
                int count = Search::count_moves(pos, depth, moves);
                uint64_t t_e = timeSinceEpochMillisec();
                uint64_t elapsed = t_e - t_s;
                double elapsed_seconds = ((double)elapsed) / 1000;

                for (ExtMove move : moves)
                {
                    cout << UCI::move(pos, move.move) << ": " << move.value << endl;
                }

                std::cout << "Elapsed: " << elapsed_seconds << " Moves: " << count << " Moves/s: " << ((double)count) / elapsed_seconds << std::endl;
            }
        }

        void go(Position& pos, istringstream& is)
        {
            string token;
            SearchLimits limits;

            while (is >> token)
            {
                if (token == "depth") is >> limits.depth;
                else if (token == "movetime") is >> limits.timeMs;
            }

            //infinite - cap until we implement stop
            if (limits.depth == 0 && limits.timeMs == 0)
            {
                limits.timeMs = 2000;
            }

            Search::Stats stats;
            Eval eval = Search::search(pos, limits, stats);
            std::cout << "bestmove " << UCI::move(pos, eval.bestMove) << std::endl;
        }
    }

    void UCI::loop(int argc, char* argv[])
    {
        Position pos;
        StateInfo rootSt;
        string token, cmd;

        pos.set(START_FEN, &rootSt);



        for (int i = 1; i < argc; ++i)
            cmd += std::string(argv[i]) + " ";

        do
        {
            if (argc == 1 && !getline(cin, cmd)) // Block here waiting for input or EOF
                cmd = "quit";

            istringstream is(cmd);

            token.clear(); // Avoid a stale if getline() returns empty or blank line
            is >> skipws >> token;

            if (token == "uci")
            {
                cout << "id name Spellfish" << endl;
                cout << "id author mardarcu" << endl;

                cout << "uciok" << endl;
            }
            else if (token == "isready") cout << "readyok" << endl;
            else if (token == "ucinewgame") { istringstream ss("startpos"); position(pos, ss); TT.clear(); }
            else if (token == "setoption") setoption(is);
            else if (token == "position") { position(pos, is); } 
            else if (token == "gen") genmoves(pos, is);
            else if (token == "go") go(pos, is);

        } while (token != "quit" && argc == 1);
    }

    string UCI::square(Square s) {
        return  string{ char('a' + file_of(s)), char('1' + rank_of(s)) };
    }

    string UCI::square(const Position& pos, Square s) {
        return  string{ char('a' + file_of(s)), char('1' + rank_of(s)) };
    }

    string UCI::move(Move m) {

        Square from = from_sq(m);
        Square to = to_sq(m);

        if (m == MOVE_NULL)
            return "0000";

        string move = UCI::square(from) + UCI::square(to);

        return move;
    }

    string UCI::move(const Position& pos, Move m) {

        Square from = from_sq(m);
        Square to = to_sq(m);

        if (m == MOVE_NULL)
            return "0000";

        string move = UCI::square(pos, from) + UCI::square(pos, to);

        return move;
    }

    /// UCI::to_move() converts a string representing a move in coordinate notation
    /// (g1f3, a7a8q) to the corresponding legal Move, if any.

    Move UCI::to_move(const Position& pos, string& str) 
    {

        for (const auto& m : MoveList<LEGAL>(pos, pos.side_to_move()))
            if (str == UCI::move(pos, m))
                return m;

        return MOVE_NONE;
    }
}