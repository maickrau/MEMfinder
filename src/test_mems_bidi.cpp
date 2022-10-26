#include <tuple>
#include <iostream>
#include <fstream>
#include <set>
#include <cassert>
#include <chrono>
#include "FMIndex.h"
#include "MEMfinder.h"

char complement(char c)
{
	switch(c)
	{
		case 'A':
			return 'T';
		case 'C':
			return 'G';
		case 'G':
			return 'C';
		case 'T':
			return 'A';
		default:
			return 'N';
	}
}

int main(int argc, char** argv)
{
	size_t indexDensity = std::stoi(argv[3]);
	size_t minLength = std::stoi(argv[4]);
	size_t maxCount = std::stoi(argv[5]);
	std::string seq;
	{
		std::ifstream file { argv[1] };
		getline(file, seq);
	}
	std::string seqCopy = seq;
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
	FMIndex index { std::move(seq), indexDensity };
	std::string query;
	{
		std::ifstream file { argv[2] };
		getline(file, query);
	}
	auto preCount = std::chrono::system_clock::now();
	auto result = MEMfinder::getBestFwBwMEMs(index, query, minLength, maxCount, 1);
	auto postCount = std::chrono::system_clock::now();
	std::cerr << "getting best matches took " << std::chrono::duration_cast<std::chrono::milliseconds>(postCount - preCount).count() << "ms" << std::endl;
	std::vector<std::set<std::pair<size_t, size_t>>> fwmatches;
	std::vector<std::set<std::pair<size_t, size_t>>> bwmatches;
	size_t shortestMatch = std::numeric_limits<size_t>::max();
	size_t longestMatch = 0;
	fwmatches.resize(query.size());
	bwmatches.resize(query.size());
	for (auto match : result)
	{
		shortestMatch = std::min(shortestMatch, match.length);
		longestMatch = std::max(longestMatch, match.length);
		if (match.fw)
		{
			fwmatches[match.queryPos].emplace(match.refPos, match.length);
		}
		else
		{
			bwmatches[match.queryPos].emplace(match.refPos, match.length);
		}
	}
	std::cerr << "match count: " << result.size() << std::endl;
	std::cerr << "shortest match: " << shortestMatch << std::endl;
	std::cerr << "longest match: " << longestMatch << std::endl;
	for (size_t i = 0; i < query.size(); i++)
	{
		// matches good
		for (auto match : fwmatches[i])
		{
			assert(i == 0 || match.first == 0 || query[i-1] != seqCopy[match.first-1]);
			for (size_t k = 0; k < match.second; k++)
			{
				assert(query[i+k] == seqCopy[match.first+k]);
			}
			assert(i+match.second == query.size() || match.first+match.second == seqCopy.size() || query[i+match.second] != seqCopy[match.first+match.second]);
		}
		// matches complete
		for (size_t j = 0; j < seqCopy.size(); j++)
		{
			if (i != 0 && j != 0 && query[i-1] == seqCopy[j-1]) continue;
			for (size_t k = 0; j+k < seqCopy.size() && i+k < query.size(); k++)
			{
				if (query[i+k] != seqCopy[j+k]) break;
				if (k+1 < shortestMatch) continue;
				if (i+k+1 != query.size() && j+k+1 != seqCopy.size() && query[i+k+1] == seqCopy[j+k+1]) continue;
				assert(fwmatches[i].count(std::make_pair(j, k+1)) == 1);
			}
		}
		// matches good
		for (auto match : bwmatches[i])
		{
			assert(i+match.second == query.size() || match.first == 0 || query[i+match.second] != complement(seqCopy[match.first-1]));
			for (size_t k = 0; k < match.second; k++)
			{
				assert(query[i+match.second-1-k] == complement(seqCopy[match.first+k]));
			}
			assert(i == 0 || match.first+match.second == seqCopy.size() || query[i-1] != complement(seqCopy[match.first+match.second]));
		}
		// matches complete
		for (size_t j = 0; j < seqCopy.size(); j++)
		{
			if (i < query.size()-1 && j != 0 && query[i+1] == complement(seqCopy[j-1])) continue;
			for (size_t k = 0; j+k < seqCopy.size() && k <= i; k++)
			{
				if (query[i-k] != complement(seqCopy[j+k])) break;
				if (k+1 < shortestMatch) continue;
				if (k < i && j+k+1 != seqCopy.size() && query[i-k-1] == complement(seqCopy[j+k+1])) continue;
				assert(bwmatches[i-k].count(std::make_pair(j, k+1)) == 1);
			}
		}
	}
	std::cerr << "match count: " << result.size() << std::endl;
}
