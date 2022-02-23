#include <tuple>
#include <iostream>
#include <fstream>
#include <cassert>
#include <fstream>
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
	FMIndex index { std::move(seq), sampleRate };
	{
		std::ofstream file { "tmp.fmi", std::ios::binary };
		index.save(file);
	}
	FMIndex index2;
	{
		std::ifstream file { "tmp.fmi", std::ios::binary };
		index2.load(file);
	}
	assert(index2 == index);
	std::cerr << index2.size() << std::endl;
}
