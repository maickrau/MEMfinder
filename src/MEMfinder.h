#ifndef MEMfinder_h
#define MEMfinder_h

#include <cassert>
#include <string>
#include "FMIndex.h"

namespace MEMfinder
{

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
		friend void iterateMEMGroups(const FMIndex&, const std::string&, const size_t, F);
		template <typename F>
		friend void iterateMEMs(const FMIndex&, const std::string&, const MatchGroup&, F);
	};

	uint8_t mapChar(const char);

	template <typename F>
	void iterateMEMGroups(const FMIndex& index, const std::string& seq, const size_t minLen, F callback)
	{
		for (size_t end = seq.size()-1; end >= minLen-1; end--)
		{
			if (end != seq.size()-1 && mapChar(seq[end]) == 0) continue;
			size_t lowStart;
			size_t lowEnd;
			size_t highStart;
			size_t highEnd;
			if (end == seq.size()-1 || mapChar(seq[end+1]) == 0)
			{
				lowStart = 0;
				lowEnd = index.size();
				highStart = 0;
				highEnd = 0;
			}
			else if (mapChar(seq[end+1]) == 5)
			{
				lowStart = 0;
				lowEnd = index.charStart(mapChar(seq[end+1]));
				highStart = 0;
				highEnd = 0;
			}
			else
			{
				lowStart = 0;
				lowEnd = index.charStart(mapChar(seq[end+1]));
				highStart = index.charStart(mapChar(seq[end+1])+1);
				highEnd = index.size();
			}
			size_t length = 0;
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
				callback(pos, matches.queryPos(), matches.matchLength());
			}
			for (size_t i = matches.highStart; i < matches.highEnd; i++)
			{
				size_t pos = index.locate(i);
				pos += 1;
				pos %= index.size();
				callback(pos, matches.queryPos(), matches.matchLength());
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
					callback(pos, matches.queryPos(), matches.matchLength());
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
					callback(pos, matches.queryPos(), matches.matchLength());
				}
			}
		}
	}

}

#endif
