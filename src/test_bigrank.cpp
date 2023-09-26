#include <iostream>
#include <fstream>
#include <cassert>
#include "WaveletTree.h"

int main(int argc, char** argv)
{
	size_t count = std::stoull(argv[1]); // 3822108160
	RankBitvector bitvector;
	bitvector.resize(count);
	srand(time(NULL));
	size_t finalCount = 0;
	for (size_t i = 0; i < count; i += 3)
	{
		bitvector.set(i, true);
		finalCount += 1;
	}
	bitvector.buildRanks();
	for (size_t i = 0; i < count; i++)
	{
		size_t realCount = (i+2)/3;
		assert(bitvector.rankOne(i) == realCount);
	}
	assert(bitvector.rankOne(count) == finalCount);
}
