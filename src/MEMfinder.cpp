#include <queue>
#include "FMIndex.h"
#include "MEMfinder.h"
#include "ReverseComplementView.h"

namespace MEMfinder
{
	Match::Match(size_t refPos, size_t queryPos, size_t length, bool fw) :
	refPos(refPos),
	queryPos(queryPos),
	length(length),
	fw(fw)
	{
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
		bool operator()(const MatchGroup& left, const MatchGroup& right) const
		{
			return left.matchLength() > right.matchLength();
		}
	};

	template <typename String>
	std::pair<size_t, std::priority_queue<MatchGroup, std::vector<MatchGroup>, MatchGroupComparer>> getBestMatchGroups(const FMIndex& index, const String& seq, const size_t minLen, const size_t maxCount)
	{
		std::priority_queue<MatchGroup, std::vector<MatchGroup>, MatchGroupComparer> chosen;
		size_t chosenCount = 0;
		size_t lengthFloor = minLen;
		iterateMEMGroups(index, seq, minLen, [maxCount, minLen, &chosenCount, &lengthFloor, &chosen](MatchGroup&& group)
		{
			assert(chosenCount <= maxCount);
			assert(chosen.size() == 0 || chosen.top().matchLength() >= lengthFloor);
			assert(group.matchLength() >= minLen);
			if (group.matchLength() < lengthFloor) return;
			if (group.count() > maxCount)
			{
				lengthFloor = group.matchLength()+1;
				while (chosen.size() > 0 && chosen.top().matchLength() < lengthFloor)
				{
					assert(chosenCount >= chosen.top().count());
					chosenCount -= chosen.top().count();
					chosen.pop();
				}
				return;
			}
			if (chosenCount + group.count() <= maxCount)
			{
				chosenCount += group.count();
				chosen.emplace(group);
				return;
			}
			assert(chosen.size() > 0);
			if (group.matchLength() < chosen.top().matchLength())
			{
				assert(lengthFloor <= group.matchLength());
				lengthFloor = group.matchLength()+1;
				assert(chosen.top().matchLength() >= lengthFloor);
				return;
			}
			if (group.matchLength() == chosen.top().matchLength())
			{
				lengthFloor = group.matchLength()+1;
				while (chosen.size() > 0 && chosen.top().matchLength() < lengthFloor)
				{
					assert(chosenCount >= chosen.top().count());
					chosenCount -= chosen.top().count();
					chosen.pop();
				}
				return;
			}
			assert(group.matchLength() > chosen.top().matchLength());
			assert(group.count() + chosenCount > maxCount);
			chosenCount += group.count();
			chosen.emplace(group);
			while (chosenCount > maxCount)
			{
				assert(chosen.size() > 0);
				lengthFloor = chosen.top().matchLength()+1;
				while (chosen.size() > 0 && chosen.top().matchLength() < lengthFloor)
				{
					assert(chosenCount >= chosen.top().count());
					chosenCount -= chosen.top().count();
					chosen.pop();
				}
			}
		});
		assert(chosenCount <= maxCount);
		assert(chosen.size() == 0 || chosen.top().matchLength() >= lengthFloor);
		return std::make_pair(chosenCount, chosen);
	}

	std::vector<Match> getBestMEMs(const FMIndex& index, const std::string& seq, const size_t minLen, const size_t maxCount)
	{
		auto chosen = getBestMatchGroups(index, seq, minLen, maxCount);
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

	std::vector<Match> getBestFwBwMEMs(const FMIndex& index, const std::string& seq, const size_t minLen, const size_t maxCount)
	{
		auto chosenFw = getBestMatchGroups(index, seq, minLen, maxCount);
		ReverseComplementView revComp { seq };
		auto chosenBw = getBestMatchGroups(index, revComp, minLen, maxCount);
		size_t totalChosen = chosenFw.first + chosenBw.first;
		while (totalChosen > maxCount)
		{
			assert(chosenFw.second.size() > 0 || chosenBw.second.size() > 0);
			size_t floor = seq.size()+1;
			if (chosenFw.second.size() > 0) floor = std::min(floor, chosenFw.second.top().matchLength());
			if (chosenBw.second.size() > 0) floor = std::min(floor, chosenBw.second.top().matchLength());
			assert(floor > 0);
			assert(floor < seq.size()+1);
			while (chosenFw.second.size() > 0 && chosenFw.second.top().matchLength() <= floor)
			{
				totalChosen -= chosenFw.second.top().count();
				chosenFw.second.pop();
			}
			while (chosenBw.second.size() > 0 && chosenBw.second.top().matchLength() <= floor)
			{
				totalChosen -= chosenBw.second.top().count();
				chosenBw.second.pop();
			}
		}
		std::vector<Match> result;
		while (chosenFw.second.size() > 0)
		{
			auto top = chosenFw.second.top();
			chosenFw.second.pop();
			iterateMEMs(index, seq, top, [&result](Match&& match)
			{
				result.emplace_back(match);
			});
		}
		while (chosenBw.second.size() > 0)
		{
			auto top = chosenBw.second.top();
			chosenBw.second.pop();
			iterateMEMs(index, revComp, top, [&result, &seq](Match&& match)
			{
				match.fw = false;
				match.queryPos = seq.size() - match.queryPos - match.length;
				result.emplace_back(match);
			});
		}
		assert(result.size() <= maxCount);
		return result;
	}

}
