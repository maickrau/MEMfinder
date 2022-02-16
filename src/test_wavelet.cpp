#include <iostream>
#include <fstream>
#include <cassert>
#include "WaveletTree.h"

int main(int argc, char** argv)
{
	std::string seq;
	{
		std::ifstream file { argv[1] };
		getline(file, seq);
	}
	for (size_t i = 0; i < seq.size(); i++)
	{
		switch(seq[i])
		{
			case 'a':
			case 'A':
				seq[i] = 1;
				break;
			case 'c':
			case 'C':
				seq[i] = 2;
				break;
			case 'g':
			case 'G':
				seq[i] = 3;
				break;
			case 't':
			case 'T':
				seq[i] = 4;
				break;
			default:
				seq[i] = 0;
				break;
		}
	}
	WaveletTree tree { seq };
	assert(tree.size() == seq.size());
	std::array<size_t, 5> counts { 0, 0, 0, 0, 0 };
	for (size_t i = 0; i < seq.size(); i++)
	{
		assert(tree.get(i) == seq[i]);
		for (size_t j = 0; j < 5; j++)
		{
			assert(tree.rank(i, j) == counts[j]);
		}
		counts[seq[i]] += 1;
	}
	for (size_t j = 0; j < 5; j++)
	{
		assert(tree.rank(seq.size(), j) == counts[j]);
	}
	assert(counts[0] + counts[1] + counts[2] + counts[3] + counts[4] == seq.size());
	std::cerr << "N: " << counts[0] << std::endl;
	std::cerr << "A: " << counts[1] << std::endl;
	std::cerr << "C: " << counts[2] << std::endl;
	std::cerr << "G: " << counts[3] << std::endl;
	std::cerr << "T: " << counts[4] << std::endl;
}
