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
				seq[i] = 2;
				break;
			case 'c':
			case 'C':
				seq[i] = 3;
				break;
			case 'g':
			case 'G':
				seq[i] = 4;
				break;
			case 't':
			case 'T':
				seq[i] = 5;
				break;
			default:
				seq[i] = 1;
				break;
		}
	}
	seq.push_back(0);
	WaveletTree tree { seq };
	assert(tree.size() == seq.size());
	std::array<size_t, 6> counts { 0, 0, 0, 0, 0, 0 };
	for (size_t i = 0; i < seq.size(); i++)
	{
		assert(tree.get(i) == seq[i]);
		for (size_t j = 0; j < 6; j++)
		{
			assert(tree.rank(i, j) == counts[j]);
		}
		counts[seq[i]] += 1;
	}
	for (size_t j = 0; j < 6; j++)
	{
		assert(tree.rank(seq.size(), j) == counts[j]);
	}
	assert(counts[0] + counts[1] + counts[2] + counts[3] + counts[4] + counts[5] == seq.size());
	std::cerr << "$: " << counts[0] << std::endl;
	std::cerr << "N: " << counts[1] << std::endl;
	std::cerr << "A: " << counts[2] << std::endl;
	std::cerr << "C: " << counts[3] << std::endl;
	std::cerr << "G: " << counts[4] << std::endl;
	std::cerr << "T: " << counts[5] << std::endl;
}
