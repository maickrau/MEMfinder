#include <tuple>
#include <cmath>
#include <queue>
#include <algorithm>
#include "FMIndex.h"
#include "MEMfinder.h"
#include "ReverseComplementView.h"
#include <set>

namespace MEMfinder
{
	Match::Match(size_t refPos, size_t queryPos, size_t length, bool fw) :
	refPos(refPos),
	queryPos(queryPos),
	length(length),
	fw(fw)
	{
	}

	bool MatchGroup::operator==(const MatchGroup& other) const
	{
		return matchCount == other.matchCount && lowStart == other.lowStart && lowEnd == other.lowEnd && highStart == other.highStart && highEnd == other.highEnd && seqPos == other.seqPos && matchLen == other.matchLen;
	}

	bool MatchGroup::operator<(const MatchGroup& other) const
	{
		if (matchCount < other.matchCount) return true;
		if (matchCount > other.matchCount) return false;
		if (lowStart < other.lowStart) return true;
		if (lowStart > other.lowStart) return false;
		if (lowEnd < other.lowEnd) return true;
		if (lowEnd > other.lowEnd) return false;
		if (highStart < other.highStart) return true;
		if (highStart > other.highStart) return false;
		if (highEnd < other.highEnd) return true;
		if (highEnd > other.highEnd) return false;
		if (seqPos < other.seqPos) return true;
		if (seqPos > other.seqPos) return false;
		if (matchLen < other.matchLen) return true;
		if (matchLen > other.matchLen) return false;
		return false;
	}

	size_t MatchGroup::count() const
	{
		return matchCount;
	}

	size_t MatchGroup::queryPos() const
	{
		return seqPos;
	}

	size_t MatchGroup::matchLength() const
	{
		return matchLen;
	}

	size_t MatchGroup::prioritizedMatchLength(const double uniqueBonus) const
	{
		return matchLen * (count() == 1 ? uniqueBonus : 1.0);
	}

	MatchGroup::MatchGroup(size_t matchCount, size_t lowStart, size_t lowEnd, size_t highStart, size_t highEnd, size_t seqPos, size_t matchLen) :
	matchCount(matchCount),
	lowStart(lowStart),
	lowEnd(lowEnd),
	highStart(highStart),
	highEnd(highEnd),
	seqPos(seqPos),
	matchLen(matchLen)
	{
	}

	uint8_t mapChar(const char c)
	{
		switch(c)
		{
			case 'a':
			case 'A':
				return 2;
			case 'c':
			case 'C':
				return 3;
			case 'g':
			case 'G':
				return 4;
			case 't':
			case 'T':
				return 5;
			default:
				return 0;
		}
	}

	class MatchGroupComparer
	{
	public:
		MatchGroupComparer() :
			uniqueBonus(1.0)
		{
		}
		MatchGroupComparer(double uniqueBonus) :
			uniqueBonus(uniqueBonus)
		{
		}
		bool operator()(const MatchGroup& left, const MatchGroup& right) const
		{
			return left.prioritizedMatchLength(uniqueBonus) > right.prioritizedMatchLength(uniqueBonus);
		}
		const double uniqueBonus;
	};

	template <typename String>
	std::vector<std::pair<size_t, std::priority_queue<MatchGroup, std::vector<MatchGroup>, MatchGroupComparer>>> getBestMatchGroups(const FMIndex& index, const String& seq, const size_t minLen, const size_t maxCount, const double uniqueBonus, const std::vector<std::pair<size_t, size_t>>& prefixIndex, const size_t prefixLen, const size_t windowSize)
	{
		size_t windowCount = (seq.size()+windowSize-1)/windowSize;
		size_t bpPerWindow = (seq.size() + windowCount - 1) / windowCount;
		std::vector<std::priority_queue<MatchGroup, std::vector<MatchGroup>, MatchGroupComparer>> chosen;
		std::vector<size_t> chosenCount;
		std::vector<size_t> lengthFloor;
		for (size_t i = 0; i < windowCount; i++) chosen.emplace_back(MatchGroupComparer { uniqueBonus });
		chosenCount.resize(windowCount, 0);
		lengthFloor.resize(windowCount, minLen);
		iterateMEMGroups(index, seq, minLen, prefixIndex, prefixLen, [maxCount, minLen, &chosenCount, &lengthFloor, &chosen, uniqueBonus, windowSize, windowCount, bpPerWindow](MatchGroup group)
		{
			size_t minWindow = group.queryPos() / bpPerWindow;
			size_t maxWindow = (group.queryPos() + group.matchLength() - 1) / bpPerWindow;
			assert(minWindow <= maxWindow);
			assert(maxWindow < windowCount);
			for (size_t window = minWindow; window <= maxWindow; window++)
			{
				assert(chosenCount[window] <= maxCount);
				assert(chosen[window].size() == 0 || chosen[window].top().prioritizedMatchLength(uniqueBonus) >= lengthFloor[window]);
				assert(group.matchLength() >= minLen);
				if (group.prioritizedMatchLength(uniqueBonus) < lengthFloor[window]) continue;
				if (group.count() > maxCount)
				{
					lengthFloor[window] = group.prioritizedMatchLength(uniqueBonus)+1;
					while (chosen[window].size() > 0 && chosen[window].top().prioritizedMatchLength(uniqueBonus) < lengthFloor[window])
					{
						assert(chosenCount[window] >= chosen[window].top().count());
						chosenCount[window] -= chosen[window].top().count();
						chosen[window].pop();
					}
					continue;
				}
				if (chosenCount[window] + group.count() <= maxCount)
				{
					chosenCount[window] += group.count();
					chosen[window].emplace(group);
					continue;
				}
				assert(chosen[window].size() > 0);
				if (group.prioritizedMatchLength(uniqueBonus) < chosen[window].top().prioritizedMatchLength(uniqueBonus))
				{
					assert(lengthFloor[window] <= group.prioritizedMatchLength(uniqueBonus));
					lengthFloor[window] = group.prioritizedMatchLength(uniqueBonus)+1;
					assert(chosen[window].top().prioritizedMatchLength(uniqueBonus) >= lengthFloor[window]);
					continue;
				}
				if (group.prioritizedMatchLength(uniqueBonus) == chosen[window].top().prioritizedMatchLength(uniqueBonus))
				{
					lengthFloor[window] = group.prioritizedMatchLength(uniqueBonus)+1;
					while (chosen[window].size() > 0 && chosen[window].top().prioritizedMatchLength(uniqueBonus) < lengthFloor[window])
					{
						assert(chosenCount[window] >= chosen[window].top().count());
						chosenCount[window] -= chosen[window].top().count();
						chosen[window].pop();
					}
					continue;
				}
				assert(group.prioritizedMatchLength(uniqueBonus) > chosen[window].top().prioritizedMatchLength(uniqueBonus));
				assert(group.count() + chosenCount[window] > maxCount);
				chosenCount[window] += group.count();
				chosen[window].emplace(group);
				while (chosenCount[window] > maxCount)
				{
					assert(chosen[window].size() > 0);
					lengthFloor[window] = chosen[window].top().prioritizedMatchLength(uniqueBonus)+1;
					while (chosen[window].size() > 0 && chosen[window].top().prioritizedMatchLength(uniqueBonus) < lengthFloor[window])
					{
						assert(chosenCount[window] >= chosen[window].top().count());
						chosenCount[window] -= chosen[window].top().count();
						chosen[window].pop();
					}
				}
			}
		});
		std::vector<std::pair<size_t, std::priority_queue<MatchGroup, std::vector<MatchGroup>, MatchGroupComparer>>> result;
		for (size_t window = 0; window < windowCount; window++)
		{
			assert(chosenCount[window] <= maxCount);
			assert(chosen[window].size() == 0 || chosen[window].top().prioritizedMatchLength(uniqueBonus) >= lengthFloor[window]);
			result.emplace_back(chosenCount[window], std::move(chosen[window]));
		}
		return result;
	}

	std::vector<Match> getBestMEMs(const FMIndex& index, const std::string& seq, const size_t minLen, const size_t maxCount, const double uniqueBonus)
	{
		auto chosen = getBestMatchGroups(index, seq, minLen, maxCount, uniqueBonus, std::vector<std::pair<size_t, size_t>>{}, 0, seq.size())[0];
		std::vector<Match> result;
		result.reserve(chosen.first);
		while (chosen.second.size() > 0)
		{
			auto top = chosen.second.top();
			chosen.second.pop();
			iterateMEMs(index, seq, top, [&result](Match&& match)
			{
				result.emplace_back(match);
			});
		}
		assert(result.size() == chosen.first);
		return result;
	}

	std::vector<Match> getBestFwBwMEMs(const FMIndex& index, const std::string& seq, const size_t minLen, const size_t maxCount, const double uniqueBonus)
	{
		return getBestFwBwMEMs(index, seq, minLen, maxCount, uniqueBonus, seq.size());
	}

	std::vector<Match> getBestFwBwMEMs(const FMIndex& index, const std::string& seq, const size_t minLen, const size_t maxCount, const double uniqueBonus, const size_t windowSize)
	{
		std::vector<std::pair<size_t, size_t>> fakePrefixIndex;
		return getBestFwBwMEMs(index, seq, minLen, maxCount, uniqueBonus, fakePrefixIndex, 0, windowSize);
	}

	std::vector<Match> getBestFwBwMEMs(const FMIndex& index, const std::string& seq, const size_t minLen, const size_t maxCount, const double uniqueBonus, const std::vector<std::pair<size_t, size_t>>& prefixIndex, const size_t prefixLen, const size_t windowSize)
	{
		size_t numWindows = (seq.size() + windowSize - 1) / windowSize;
		size_t maxCountPerWindow = (maxCount + numWindows-1) / numWindows;
		auto chosenFw = getBestMatchGroups(index, seq, minLen, maxCountPerWindow, uniqueBonus, prefixIndex, prefixLen, windowSize);
		ReverseComplementView revComp { seq };
		auto chosenBw = getBestMatchGroups(index, revComp, minLen, maxCountPerWindow, uniqueBonus, prefixIndex, prefixLen, windowSize);
		assert(chosenFw.size() == numWindows);
		assert(chosenBw.size() == numWindows);
		std::set<MatchGroup> uniqueFwGroups;
		std::set<MatchGroup> uniqueBwGroups;
		for (size_t windowindex = 0; windowindex < numWindows; windowindex++)
		{
			size_t fwWindow = windowindex;
			size_t bwWindow = numWindows-1-windowindex;
			size_t totalChosen = chosenFw[fwWindow].first + chosenBw[bwWindow].first;
			while (totalChosen > maxCountPerWindow)
			{
				assert(chosenFw[fwWindow].second.size() > 0 || chosenBw[bwWindow].second.size() > 0);
				size_t floor = seq.size() * uniqueBonus + 1;
				if (chosenFw[fwWindow].second.size() > 0) floor = std::min(floor, chosenFw[fwWindow].second.top().prioritizedMatchLength(uniqueBonus));
				if (chosenBw[bwWindow].second.size() > 0) floor = std::min(floor, chosenBw[bwWindow].second.top().prioritizedMatchLength(uniqueBonus));
				assert(floor > 0);
				assert(floor < seq.size() * uniqueBonus + 1);
				while (chosenFw[fwWindow].second.size() > 0 && chosenFw[fwWindow].second.top().prioritizedMatchLength(uniqueBonus) <= floor)
				{
					totalChosen -= chosenFw[fwWindow].second.top().count();
					chosenFw[fwWindow].second.pop();
				}
				while (chosenBw[bwWindow].second.size() > 0 && chosenBw[bwWindow].second.top().prioritizedMatchLength(uniqueBonus) <= floor)
				{
					totalChosen -= chosenBw[bwWindow].second.top().count();
					chosenBw[bwWindow].second.pop();
				}
			}
			size_t foundChosen = 0;
			while (chosenFw[fwWindow].second.size() > 0)
			{
				auto top = chosenFw[fwWindow].second.top();
				foundChosen += top.count();
				uniqueFwGroups.emplace(top);
				chosenFw[fwWindow].second.pop();
			}
			while (chosenBw[bwWindow].second.size() > 0)
			{
				auto top = chosenBw[bwWindow].second.top();
				foundChosen += top.count();
				uniqueBwGroups.emplace(top);
				chosenBw[bwWindow].second.pop();
			}
			assert(foundChosen == totalChosen);
			assert(foundChosen <= maxCountPerWindow);
		}
		std::vector<Match> result;
		for (auto group : uniqueFwGroups)
		{
			iterateMEMs(index, seq, group, [&result](Match&& match)
			{
				result.emplace_back(match);
			});
		}
		for (auto group : uniqueBwGroups)
		{
			iterateMEMs(index, revComp, group, [&result, &seq](Match&& match)
			{
				match.fw = false;
				match.queryPos = seq.size() - match.queryPos - match.length;
				result.emplace_back(match);
			});
		}
		assert(result.size() <= maxCount);
		return result;
	}

	std::vector<Match> getBestFwBwMUMs(const FMIndex& index, const std::string& seq, const size_t minLen, const size_t maxCount)
	{
		std::vector<Match> maybeMums;
		iterateMUMs(index, seq, minLen, [&maybeMums](Match m)
		{
			maybeMums.push_back(m);
		});
		ReverseComplementView revComp { seq };
		iterateMUMs(index, revComp, minLen, [&maybeMums, &seq](Match m)
		{
			m.queryPos = seq.size() - m.queryPos - m.length;
			m.fw = false;
			maybeMums.push_back(m);
		});
		std::sort(maybeMums.begin(), maybeMums.end(), [](Match left, Match right)
		{
			if (left.queryPos < right.queryPos) return true;
			if (left.queryPos > right.queryPos) return false;
			if (left.length > right.length) return true;
			if (left.length < right.length) return false;
			return false;
		});
		std::vector<Match> mums;
		if (maybeMums.size() == 0) return mums;
		mums.push_back(maybeMums[0]);
		size_t longestEnd = maybeMums[0].queryPos + maybeMums[0].length;
		size_t longestStart = maybeMums[0].queryPos;
		for (size_t i = 1; i < maybeMums.size(); i++)
		{
			size_t endHere = maybeMums[i].queryPos + maybeMums[i].length;
			if (endHere < longestEnd) continue;
			if (endHere == longestEnd)
			{
				assert(longestStart <= maybeMums[i].queryPos);
				if (maybeMums[i].queryPos > longestStart) continue;
				assert(longestStart == maybeMums[i].queryPos);
				if (mums.size() > 0)
				{
					assert(mums.back().queryPos < longestStart || (mums.back().queryPos == longestStart && mums.back().queryPos + mums.back().length == longestEnd));
					if (mums.back().queryPos == longestStart)
					{
						assert(mums.back().queryPos + mums.back().length == longestEnd);
						mums.pop_back();
					}
				}
				continue;
			}
			if (endHere > longestEnd)
			{
				assert(longestStart < maybeMums[i].queryPos);
				assert(mums.size() == 0 || mums.back().queryPos < maybeMums[i].queryPos);
				assert(mums.size() == 0 || mums.back().queryPos + mums.back().length < maybeMums[i].queryPos + maybeMums[i].length);
				mums.push_back(maybeMums[i]);
				longestStart = maybeMums[i].queryPos;
				longestEnd = maybeMums[i].queryPos + maybeMums[i].length;
			}
		}
		for (size_t i = 1; i < mums.size(); i++)
		{
			assert(mums[i].queryPos > mums[i-1].queryPos);
			assert(mums[i].queryPos + mums[i].length > mums[i-1].queryPos + mums[i-1].length);
		}
		if (mums.size() > maxCount)
		{
			std::sort(mums.begin(), mums.end(), [](Match left, Match right){
				if (left.length > right.length) return true;
				if (left.length < right.length) return false;
				return false;
			});
			while (mums.size() > maxCount)
			{
				size_t floor = mums.back().length;
				while (mums.size() > 0 && mums.back().length == floor)
				{
					mums.pop_back();
				}
				assert(mums.size() == 0 || mums.back().length > floor);
			}
			std::sort(mums.begin(), mums.end(), [](Match left, Match right)
			{
				if (left.queryPos < right.queryPos) return true;
				if (left.queryPos > right.queryPos) return false;
				assert(false);
			});
		}
		return mums;
	}

	std::vector<std::pair<size_t, size_t>> buildPrefixIndex(const FMIndex& index, const size_t len)
	{
		std::vector<std::pair<size_t, size_t>> result;
		result.resize(pow(4, len));
		for (size_t i = 0; i < result.size(); i++)
		{
			uint64_t kmer = i;
			size_t start = 0;
			size_t end = index.size();
			for (size_t j = 0; j < len; j++)
			{
				std::tie(start, end) = index.advance(start, end, (kmer & 3) + 2);
				kmer >>= 2;
			}
			result[i].first = start;
			result[i].second = end;
		}
		return result;
	}

}
