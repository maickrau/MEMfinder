#include <tuple>
#include <iostream>
#include <fstream>
#include <cassert>
#include "FMIndex.h"

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
	FMIndex index { seq, 32 };
	assert(index.size() == seq.size());
	std::string tmp;
	std::cout << "done" << std::endl;
	std::cin >> tmp;
	// size_t start = 0;
	// size_t end = index.size();
	// for (size_t i = seq.size()-1; i < seq.size(); i--)
	// {
	// 	assert(end > start);
	// 	assert(i != 0 || end == start+1);
	// 	if (end == start+1)
	// 	{
	// 		assert(index.locate(start) == i);
	// 	}
	// 	std::tie(start, end) = index.advance(start, end, seq[i]);
	// }
}
