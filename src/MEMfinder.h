#ifndef MEMfinder_h
#define MEMfinder_h

#include <cassert>
#include <string>
#include "FMIndex.h"

namespace MEMfinder
{
	class Match
	{
	public:
		Match(size_t refPos, size_t queryPos, size_t length);
		size_t refPos;
		size_t queryPos;
		size_t length;
	};

	class MatchGroup
	{
	public:
		size_t count() const;
		size_t queryPos() const;
		size_t matchLength() const;
	private:
		MatchGroup(size_t matchCount, size_t lowStart, size_t lowEnd, size_t highStart, size_t highEnd, size_t seqPos, size_t matchLen);
		size_t matchCount;
		size_t lowStart;
		size_t lowEnd;
		size_t highStart;
		size_t highEnd;
		size_t seqPos;
		size_t matchLen;
		template <typename F>
		friend void iterateMEMGroupsInternal(const FMIndex&, const std::string&, const size_t, size_t, size_t, size_t, size_t, size_t, size_t, F);
		template <typename F>
		friend void iterateMEMGroups(const FMIndex&, const std::string&, const size_t, F);
		template <typename F>
		friend void iterateMEMs(const FMIndex&, const std::string&, const MatchGroup&, F);
	};

	uint8_t mapChar(const char);
	std::vector<Match> getBestMEMs(const FMIndex& index, const std::string& seq, const size_t minLen, const size_t maxCount);

	template <typename F>
	void iterateMEMGroupsInternal(const FMIndex& index, const std::string& seq, const size_t minLen, size_t lowStart, size_t lowEnd, size_t highStart, size_t highEnd, size_t end, size_t length, F callback)
	{
		while (length <= end)
		{
			size_t oldSize = (lowEnd - lowStart) + (highEnd - highStart);
			if (oldSize == 0) break;
			uint8_t c = mapChar(seq[end-length]);
			size_t newLowStart = 0, newLowEnd = 0, newHighStart = 0, newHighEnd = 0;
			if (lowEnd > lowStart)
			{
				newLowStart = index.advance(lowStart, c);
				newLowEnd = index.advance(lowEnd, c);
			}
			if (highEnd > highStart)
			{
				newHighStart = index.advance(highStart, c);
				newHighEnd = index.advance(highEnd, c);
			}
			size_t newSize = (newLowEnd - newLowStart) + (newHighEnd - newHighStart);
			assert(newSize <= oldSize);
			if (length >= minLen && newSize < oldSize)
			{
				callback(MatchGroup { oldSize - newSize, lowStart, lowEnd, highStart, highEnd, end - length + 1, length });
			}
			lowStart = newLowStart;
			lowEnd = newLowEnd;
			highStart = newHighStart;
			highEnd = newHighEnd;
			assert(lowEnd >= lowStart);
			assert(highEnd >= highStart);
			if (length == end)
			{
				callback(MatchGroup { newSize, lowStart, lowEnd, highStart, highEnd, end - length, length+1 });
			}
			length += 1;
			if (newSize == 0) break;
		}
	}

	template <typename F>
	void iterateMEMGroups(const FMIndex& index, const std::string& seq, const size_t minLen, F callback)
	{
		assert(minLen >= 2);
		size_t lastLow = 0;
		size_t lastHigh = 0;
		for (size_t end = seq.size()-1; end >= minLen-1; end--)
		{
			if (mapChar(seq[end]) == 0)
			{
				lastLow = 0;
				lastHigh = 0;
				continue;
			}
			size_t low = 0;
			size_t high = index.size();
			for (size_t i = 0; i < minLen-1; i++)
			{
				size_t c = mapChar(seq[end-i]);
				low = index.advance(low, c);
				high = index.advance(high, c);
				if (high == low)
				{
					lastLow = 0;
					lastHigh = 0;
					break;
				}
			}
			if (high == low)
			{
				lastLow = 0;
				lastHigh = 0;
				continue;
			}
			if (lastLow != lastHigh)
			{
				size_t c = mapChar(seq[end-(minLen-2)]);
				lastLow = index.advance(lastLow, c);
				lastHigh = index.advance(lastHigh, c);
				if (lastLow != lastHigh)
				{
					assert(lastLow >= low);
					assert(lastHigh <= high);
					if (lastLow == low && lastHigh == high) continue;
					iterateMEMGroupsInternal(index, seq, minLen, low, lastLow, lastHigh, high, end, minLen-1, callback);
				}
			}
			if (lastLow == lastHigh)
			{
				iterateMEMGroupsInternal(index, seq, minLen, low, high, 0, 0, end, minLen-1, callback);
			}
			lastLow = low;
			lastHigh = high;
		}
	}

	template <typename F>
	void iterateMEMs(const FMIndex& index, const std::string& seq, const MatchGroup& matches, F callback)
	{
		if (matches.queryPos() == 0)
		{
			for (size_t i = matches.lowStart; i < matches.lowEnd; i++)
			{
				size_t pos = index.locate(i);
				pos += 1;
				pos %= index.size();
				callback(Match { pos, matches.queryPos(), matches.matchLength() });
			}
			for (size_t i = matches.highStart; i < matches.highEnd; i++)
			{
				size_t pos = index.locate(i);
				pos += 1;
				pos %= index.size();
				callback(Match { pos, matches.queryPos(), matches.matchLength() });
			}
			return;
		}
		for (uint8_t c = 0; c <= 5; c++)
		{
			if (c == mapChar(seq[matches.queryPos()-1])) continue;
			if (matches.lowEnd > matches.lowStart)
			{
				size_t newStart = index.advance(matches.lowStart, c);
				size_t newEnd = index.advance(matches.lowEnd, c);
				for (size_t i = newStart; i < newEnd; i++)
				{
					size_t pos = index.locate(i);
					pos += 2;
					pos %= index.size();
					callback(Match { pos, matches.queryPos(), matches.matchLength() });
				}
			}
			if (matches.highEnd > matches.highStart)
			{
				size_t newStart = index.advance(matches.highStart, c);
				size_t newEnd = index.advance(matches.highEnd, c);
				for (size_t i = newStart; i < newEnd; i++)
				{
					size_t pos = index.locate(i);
					pos += 2;
					pos %= index.size();
					callback(Match { pos, matches.queryPos(), matches.matchLength() });
				}
			}
		}
	}

}

#endif
