#pragma once

#include  <unordered_map>
#include "misc.h"
#include "types.h"

namespace Spellfish
{

	constexpr int TT_SIZE_MB = 64;

	class TTEntry
	{
	public:
		Key key64 = 0;
		Value eval = VALUE_ZERO;
		Depth depth = 0;
		Move bestMove = MOVE_NONE;
		uint8_t generation8 = 0;

		void save(Key k, Value eval, Depth d, Move m);
	};

	class TranspositionTable
	{
		static constexpr int ClusterSize = 5;

		struct Cluster {
			TTEntry entry[ClusterSize];
		};

	public:
		static void init();
		void allocate(size_t mbSize);
		~TranspositionTable() { delete[] table; } 
		void new_search() { generation8 += 1; } 
		TTEntry* probe(const Key key, bool& found) const;
		int hash_occupancy() const;
		int hashfull() const;
		//void resize(size_t mbSize);
		void clear();

		TTEntry* first_entry(const Key key) const {
			return &table[mul_hi64(key, clusterCount)].entry[0];
		}

	private:
		friend struct TTEntry;

		size_t clusterCount;
		Cluster* table;
		uint8_t generation8; // Size must be not bigger than TTEntry::genBound8
		size_t filledEntries;
	};

	extern TranspositionTable TT;
}