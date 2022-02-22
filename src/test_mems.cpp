#include <tuple>
#include <iostream>
#include <fstream>
#include <set>
#include <cassert>
#include <chrono>
#include "FMIndex.h"
#include "MEMfinder.h"

int main(int argc, char** argv)
{
	size_t indexDensity = std::stoi(argv[3]);
	size_t minLength = std::stoi(argv[4]);
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
	std::vector<std::set<std::pair<size_t, size_t>>> matches;
	matches.resize(query.size());
	size_t matchCount = 0;
	MEMfinder::iterateMEMGroups(index, query, minLength, [&matches, &index, &query, &matchCount](MEMfinder::MatchGroup group) {
		size_t oldSize = matchCount;
		MEMfinder::iterateMEMs(index, query, group, [&matches, &matchCount](MEMfinder::Match&& m)
		{
			matches[m.queryPos].emplace(m.refPos, m.length);
			matchCount += 1;
		});
		assert(matchCount == oldSize + group.count());
	});
	size_t countedMatches = 0;
	for (size_t i = 0; i < query.size(); i++)
	{
		countedMatches += matches[i].size();
		// matches good
		for (auto match : matches[i])
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
				if (k+1 < minLength) continue;
				if (i+k+1 != query.size() && j+k+1 != seqCopy.size() && query[i+k+1] == seqCopy[j+k+1]) continue;
				assert(matches[i].count(std::make_pair(j, k+1)) == 1);
			}
		}
	}
	assert(countedMatches == matchCount);
	std::cerr << "match count: " << matchCount << std::endl;
}
