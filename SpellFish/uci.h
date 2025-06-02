#pragma once

#include "position.h"
#include "types.h"

namespace Spellfish
{
namespace UCI
{
	void loop(int argc, char* argv[]);
	std::string square(Square s);
	std::string square(const Position& pos, Square s);
	std::string move(Move m);
	std::string move(const Position& pos, Move m);
	Move to_move(const Position& pos, std::string& str);
}
}