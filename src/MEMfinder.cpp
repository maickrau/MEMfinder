#include <queue>
#include "FMIndex.h"
#include "MEMfinder.h"

namespace MEMfinder
{
	Match::Match(size_t refPos, size_t queryPos, size_t length) :
	refPos(refPos),
	queryPos(queryPos),
	length(length)
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

	std::vector<Match> getBestMEMs(const FMIndex& index, const std::string& seq, const size_t minLen, const size_t maxCount)
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
		std::vector<Match> result;
		result.reserve(chosenCount);
		while (chosen.size() > 0)
		{
			auto top = chosen.top();
			chosen.pop();
			iterateMEMs(index, seq, top, [&result](Match&& match)
			{
				result.emplace_back(match);
			});
		}
		assert(result.size() == chosenCount);
		return result;
	}

}
