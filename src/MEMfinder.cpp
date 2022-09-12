#include <queue>
#include <algorithm>
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
			uniqueBonus(1)
		{
		}
		MatchGroupComparer(size_t uniqueBonus) :
			uniqueBonus(uniqueBonus)
		{
		}
		bool operator()(const MatchGroup& left, const MatchGroup& right) const
		{
			return left.prioritizedMatchLength(uniqueBonus) > right.prioritizedMatchLength(uniqueBonus);
		}
		const size_t uniqueBonus;
	};

	template <typename String>
	std::pair<size_t, std::priority_queue<MatchGroup, std::vector<MatchGroup>, MatchGroupComparer>> getBestMatchGroups(const FMIndex& index, const String& seq, const size_t minLen, const size_t maxCount, const double uniqueBonus)
	{
		std::priority_queue<MatchGroup, std::vector<MatchGroup>, MatchGroupComparer> chosen { MatchGroupComparer { uniqueBonus } };
		size_t chosenCount = 0;
		size_t lengthFloor = minLen;
		iterateMEMGroups(index, seq, minLen, [maxCount, minLen, &chosenCount, &lengthFloor, &chosen, uniqueBonus](MatchGroup&& group)
		{
			assert(chosenCount <= maxCount);
			assert(chosen.size() == 0 || chosen.top().prioritizedMatchLength(uniqueBonus) >= lengthFloor);
			assert(group.matchLength() >= minLen);
			if (group.prioritizedMatchLength(uniqueBonus) < lengthFloor) return;
			if (group.count() > maxCount)
			{
				lengthFloor = group.prioritizedMatchLength(uniqueBonus)+1;
				while (chosen.size() > 0 && chosen.top().prioritizedMatchLength(uniqueBonus) < lengthFloor)
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
			if (group.prioritizedMatchLength(uniqueBonus) < chosen.top().prioritizedMatchLength(uniqueBonus))
			{
				assert(lengthFloor <= group.prioritizedMatchLength(uniqueBonus));
				lengthFloor = group.prioritizedMatchLength(uniqueBonus)+1;
				assert(chosen.top().prioritizedMatchLength(uniqueBonus) >= lengthFloor);
				return;
			}
			if (group.prioritizedMatchLength(uniqueBonus) == chosen.top().prioritizedMatchLength(uniqueBonus))
			{
				lengthFloor = group.prioritizedMatchLength(uniqueBonus)+1;
				while (chosen.size() > 0 && chosen.top().prioritizedMatchLength(uniqueBonus) < lengthFloor)
				{
					assert(chosenCount >= chosen.top().count());
					chosenCount -= chosen.top().count();
					chosen.pop();
				}
				return;
			}
			assert(group.prioritizedMatchLength(uniqueBonus) > chosen.top().prioritizedMatchLength(uniqueBonus));
			assert(group.count() + chosenCount > maxCount);
			chosenCount += group.count();
			chosen.emplace(group);
			while (chosenCount > maxCount)
			{
				assert(chosen.size() > 0);
				lengthFloor = chosen.top().prioritizedMatchLength(uniqueBonus)+1;
				while (chosen.size() > 0 && chosen.top().prioritizedMatchLength(uniqueBonus) < lengthFloor)
				{
					assert(chosenCount >= chosen.top().count());
					chosenCount -= chosen.top().count();
					chosen.pop();
				}
			}
		});
		assert(chosenCount <= maxCount);
		assert(chosen.size() == 0 || chosen.top().prioritizedMatchLength(uniqueBonus) >= lengthFloor);
		return std::make_pair(chosenCount, chosen);
	}

	std::vector<Match> getBestMEMs(const FMIndex& index, const std::string& seq, const size_t minLen, const size_t maxCount, const double uniqueBonus)
	{
		auto chosen = getBestMatchGroups(index, seq, minLen, maxCount, uniqueBonus);
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
		auto chosenFw = getBestMatchGroups(index, seq, minLen, maxCount, uniqueBonus);
		ReverseComplementView revComp { seq };
		auto chosenBw = getBestMatchGroups(index, revComp, minLen, maxCount, uniqueBonus);
		size_t totalChosen = chosenFw.first + chosenBw.first;
		while (totalChosen > maxCount)
		{
			assert(chosenFw.second.size() > 0 || chosenBw.second.size() > 0);
			size_t floor = seq.size() * uniqueBonus + 1;
			if (chosenFw.second.size() > 0) floor = std::min(floor, chosenFw.second.top().prioritizedMatchLength(uniqueBonus));
			if (chosenBw.second.size() > 0) floor = std::min(floor, chosenBw.second.top().prioritizedMatchLength(uniqueBonus));
			assert(floor > 0);
			assert(floor < seq.size() * uniqueBonus + 1);
			while (chosenFw.second.size() > 0 && chosenFw.second.top().prioritizedMatchLength(uniqueBonus) <= floor)
			{
				totalChosen -= chosenFw.second.top().count();
				chosenFw.second.pop();
			}
			while (chosenBw.second.size() > 0 && chosenBw.second.top().prioritizedMatchLength(uniqueBonus) <= floor)
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

}
