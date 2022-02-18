#include "FMIndex.h"
#include "MEMfinder.h"

namespace MEMfinder
{
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
}
