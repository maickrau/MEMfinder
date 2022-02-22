#include <tuple>
#include <iostream>
#include <fstream>
#include <cassert>
#include <chrono>
#include "FMIndex.h"
#include "MEMfinder.h"

int main(int argc, char** argv)
{
	size_t sampleRate = std::stoi(argv[3]);
	size_t minLength = std::stoi(argv[4]);
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
	auto preIndex = std::chrono::system_clock::now();
	FMIndex index { std::move(seq), sampleRate };
	auto postIndex = std::chrono::system_clock::now();
	std::cerr << "indexing took " << std::chrono::duration_cast<std::chrono::milliseconds>(postIndex - preIndex).count() << "ms" << std::endl;
	std::string query;
	{
		std::ifstream file { argv[2] };
		getline(file, query);
	}
	size_t matchCount = 0;
	auto preCount = std::chrono::system_clock::now();
	MEMfinder::iterateMEMGroups(index, query, minLength, [&matchCount](MEMfinder::MatchGroup group) {
		matchCount += group.count();
	});
	auto postCount = std::chrono::system_clock::now();
	std::cerr << "counting took " << std::chrono::duration_cast<std::chrono::milliseconds>(postCount - preCount).count() << "ms" << std::endl;
	std::cerr << "match count: " << matchCount << std::endl;
}
