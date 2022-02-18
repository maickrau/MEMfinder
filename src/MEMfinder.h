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
	};

	uint8_t mapChar(const char);

	template <typename F>
	void iterateMEMGroups(const FMIndex& index, const std::string& seq, const size_t minLen, F callback)
	{
		for (size_t end = seq.size()-1; end >= minLen; end--)
		{
			if (end != seq.size()-1 && mapChar(seq[end+1]) == 0) continue;
			size_t lowStart;
			size_t lowEnd;
			size_t highStart;
			size_t highEnd;
			if (end == seq.size()-1 || mapChar(seq[end+1]) == 0 || mapChar(seq[end+1]) == 5)
			{
				lowStart = 0;
				lowEnd = index.size();
				highStart = 0;
				highEnd = 0;
			}
			else
			{
				lowStart = 0;
				lowEnd = index.advance(index.size(), mapChar(seq[end+1]-1));
				highStart = index.advance(index.size(), mapChar(seq[end+1]-1));
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

}

#endif
