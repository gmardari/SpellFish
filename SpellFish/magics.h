#pragma once


namespace Spellfish
{



	namespace Magics
	{
#define U64 unsigned long long

		void init();

		void init_sliders_attacks(bool is_bishop);
		void print_bitboard(U64 bitboard);

		U64 get_bishop_attacks(int square, U64 occupancy);
		U64 get_rook_attacks(int square, U64 occupancy);
	}
}