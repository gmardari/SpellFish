#pragma once

#include "types.h"
#include "position.h"


namespace Spellfish
{
	constexpr Value mobilityScore = Value(3);
	constexpr Value blockedPawnScore = Value(-10);
	constexpr Value doubledPawnScore = Value(-10);
	constexpr Value isolatedPawnScore = Value(-10);


	Value evaluate_pos_mg(const Position& pos);
}