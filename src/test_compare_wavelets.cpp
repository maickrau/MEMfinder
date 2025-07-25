#include <chrono>
#include <iostream>
#include <fstream>
#include <cassert>
#include "FlatRanks.h"
#include "WaveletTree.h"
#include "DNAPrefixSumIndex.h"

template <typename T>
void measure(const std::string& seq)
{
	auto startTime = std::chrono::steady_clock::now();
	T tree { seq };
	auto midTime = std::chrono::steady_clock::now();
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
	auto endTime = std::chrono::steady_clock::now();
	size_t totalmilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime-startTime).count();
	size_t constructionmilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(midTime-startTime).count();
	size_t querymilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime-midTime).count();
	std::cerr << "took " << std::to_string(totalmilliseconds / 1000) + "," + std::to_string(totalmilliseconds % 1000) + " s" << std::endl;
	std::cerr << "construction " << std::to_string(constructionmilliseconds / 1000) + "," + std::to_string(constructionmilliseconds % 1000) + " s" << std::endl;
	std::cerr << "query " << std::to_string(querymilliseconds / 1000) + "," + std::to_string(querymilliseconds % 1000) + " s" << std::endl;
}

int main(int, char** argv)
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
	std::cerr << "try wavelet tree" << std::endl;
	measure<WaveletTree>(seq);
	std::cerr << "try DNA prefix sum index" << std::endl;
	measure<DNAPrefixSumIndex>(seq);
	std::cerr << "try flat ranks" << std::endl;
	measure<FlatRanks>(seq);
}
