
#include "tt.h"

namespace Spellfish
{
	TranspositionTable TT;

	void TTEntry::save(Key k, Value eval, Depth d, Move m) 
	{
		if(key64 == 0)
			TT.filledEntries++;


		key64 = k;
		this->eval = eval;
		this->depth = d;
		this->bestMove = m;
		this->generation8 = TT.generation8;
	}

	void TranspositionTable::allocate(size_t mbSize)
	{
		size_t byteValue = mbSize * 1024 * 1024;

		clusterCount = byteValue / sizeof(Cluster);
		table = new Cluster[clusterCount];
		generation8 = 0;
	}

	int TranspositionTable::hash_occupancy() const
	{
		size_t maxEntries = clusterCount * ClusterSize;

		return (100 * filledEntries) / maxEntries;
	}

	TTEntry* TranspositionTable::probe(const Key key, bool& found) const
	{
		TTEntry* tte = first_entry(key);

		for (int i = 0; i < ClusterSize; ++i)
		{
			if (tte[i].key64 == key)
			{
				found = true;
				return &tte[i];
			}
		}


		TTEntry* replace = tte;
		for (int i = 1; i < ClusterSize; ++i)
		{
			if (tte[i].generation8 < replace->generation8)
			{
				replace = &tte[i];
				//break;
			}
		}
			
				

		found = false;
	

		return replace;
	}

	void TranspositionTable::clear()
	{
		std::memset(table, 0, clusterCount * sizeof(Cluster));
		TT.filledEntries = 0;
	}

	void TranspositionTable::init()
	{
		TT.allocate(TT_SIZE_MB);
	}
}