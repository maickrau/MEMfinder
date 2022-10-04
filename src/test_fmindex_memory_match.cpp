#include <tuple>
#include <iostream>
#include <fstream>
#include <cassert>
#include "FMIndex.h"

int main(int argc, char** argv)
{
	size_t sampleRate = std::stoi(argv[2]);
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
	std::string tmp = seq;
	FMIndex index { std::move(seq), sampleRate };
	FMIndex index2 { std::move(tmp), sampleRate, true };
	assert(index == index2);
}
