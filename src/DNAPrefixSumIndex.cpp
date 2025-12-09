#include <limits>
#include <cstring>
#include <cassert>
#include "DNAPrefixSumIndex.h"
#include "Serialize.h"

int popcount(uint64_t x);

DNAPrefixSumIndex::DNAPrefixSumIndex() :
bytes(),
counts({0, 0, 0, 0, 0, 0}),
specialCharMasks(),
realSize(0),
built(false)
{
}

DNAPrefixSumIndex::DNAPrefixSumIndex(const std::string& seq) :
bytes(),
counts({0, 0, 0, 0, 0, 0}),
specialCharMasks(),
realSize(0),
built(false)
{
	initialize(seq);
}

void DNAPrefixSumIndex::initialize(const std::string& seq)
{
	static_assert(sizeof(MidChunk)*8 == 11*64);
	static_assert(sizeof(BigChunk)*8 == 23*64);
	realSize = seq.size();
	bytes.clear();
	for (size_t i = 0; i < 6; i++)
	{
		counts[i] = 0;
	}
	size_t countBigChunks = (seq.size()+countBasesPerBigChunk-1)/countBasesPerBigChunk;
	size_t countMidChunks = (seq.size()+countBasesPerMidChunk-1)/countBasesPerMidChunk;
	bytes.resize(countBigChunks * 23 + countMidChunks * 11, 0);
	for (size_t bigChunk = 0; bigChunk < countBigChunks; bigChunk++)
	{
		std::array<uint64_t, 6> countsBeforeMidChunks = counts;
		BigChunk bigChunkUnderConstruction;
		for (size_t i = 0; i < 6; i++)
		{
			bigChunkUnderConstruction.bigPrefixSums[i] = counts[i];
		}
		bigChunkUnderConstruction.specialCharMaskStartIndex = specialCharMasks.size();
		for (size_t i = 0; i < 16; i++)
		{
			bigChunkUnderConstruction.which64BitChunksHaveSpecialChar[i] = 0;
		}
		for (size_t midChunk = 0; midChunk < countMidChunksPerBigChunk && bigChunk*countMidChunksPerBigChunk+midChunk < countMidChunks; midChunk++)
		{
			std::array<uint64_t, 6> countsBeforeSmallChunks = counts;
			MidChunk midChunkUnderConstruction;
			for (size_t i = 0; i < 6; i++)
			{
				assert(counts[i] - countsBeforeMidChunks[i] < std::numeric_limits<uint16_t>::max());
				midChunkUnderConstruction.midPrefixSums[i] = counts[i] - countsBeforeMidChunks[i];
			}
			for (size_t i = 0; i < 4; i++)
			{
				midChunkUnderConstruction.firstBits[i] = 0;
				midChunkUnderConstruction.secondBits[i] = 0;
			}
			for (size_t i = 0; i < 12; i++)
			{
				midChunkUnderConstruction.smallPrefixSums[i] = 0;
			}
			for (size_t smallChunk = 0; smallChunk < 4; smallChunk++)
			{
				if (smallChunk > 0)
				{
					for (size_t i = 0; i < 4; i++)
					{
						assert(counts[i+2] - countsBeforeSmallChunks[i+2] < std::numeric_limits<uint8_t>::max());
						midChunkUnderConstruction.smallPrefixSums[(smallChunk-1)*4+i] = counts[i+2] - countsBeforeSmallChunks[i+2];
					}
				}
				bool current64BitChunkHasSpecialCharacter = false;
				uint64_t currentSpecialMaskDollar = 0;
				uint64_t currentSpecialMaskN = 0;
				for (size_t index = 0; index < 64 && bigChunk*countBasesPerBigChunk+midChunk*countBasesPerMidChunk+smallChunk*64+index < seq.size(); index++)
				{
					size_t i = bigChunk*countBasesPerBigChunk+midChunk*countBasesPerMidChunk+smallChunk*64+index;
					switch(seq[i])
					{
						case 0:
						case '$':
							counts[0] += 1;
							current64BitChunkHasSpecialCharacter = true;
							currentSpecialMaskDollar |= 1ull << index;
							break;
						case 1:
						case 'n':
						case 'N':
							counts[1] += 1;
							current64BitChunkHasSpecialCharacter = true;
							currentSpecialMaskN |= 1ull << index;
							break;
						case 2:
						case 'a':
						case 'A':
							counts[2] += 1;
//							midChunkUnderConstruction.firstBits[smallChunk] |= 1ull << index;
//							midChunkUnderConstruction.secondBits[smallChunk] |= 1ull << index;
							break;
						case 3:
						case 'c':
						case 'C':
							counts[3] += 1;
//							midChunkUnderConstruction.firstBits[smallChunk] |= 1ull << index;
							midChunkUnderConstruction.secondBits[smallChunk] |= 1ull << index;
							break;
						case 4:
						case 'g':
						case 'G':
							counts[4] += 1;
							midChunkUnderConstruction.firstBits[smallChunk] |= 1ull << index;
//							midChunkUnderConstruction.secondBits[smallChunk] |= 1ull << index;
							break;
						case 5:
						case 't':
						case 'T':
							counts[5] += 1;
							midChunkUnderConstruction.firstBits[smallChunk] |= 1ull << index;
							midChunkUnderConstruction.secondBits[smallChunk] |= 1ull << index;
							break;
					}
				}
				if (current64BitChunkHasSpecialCharacter)
				{
					bigChunkUnderConstruction.which64BitChunksHaveSpecialChar[(midChunk*4+smallChunk)/64] |= 1ull << (size_t)((midChunk*4+smallChunk)%64);
					specialCharMasks.emplace_back(currentSpecialMaskDollar, currentSpecialMaskN);
				}
			}
			std::memcpy((void*)(bytes.data()+bigChunk*countUint64sPerBigChunk+sizeof(BigChunk)/8+midChunk*countUint64sPerMidChunk), (void*)&midChunkUnderConstruction, sizeof(MidChunk));
		}
		std::memcpy((void*)(bytes.data()+bigChunk*countUint64sPerBigChunk), (void*)&bigChunkUnderConstruction, sizeof(BigChunk));
	}
	built = true;
}

uint8_t DNAPrefixSumIndex::get(size_t index) const
{
	assert(built);
	size_t bigChunk = index / countBasesPerBigChunk;
	size_t midChunkWithinBigChunk = (index - (bigChunk * countBasesPerBigChunk)) / countBasesPerMidChunk;
	MidChunk* midChunkPtr = (MidChunk*)(bytes.data()+bigChunk*countUint64sPerBigChunk+sizeof(BigChunk)/8+midChunkWithinBigChunk*countUint64sPerMidChunk);
	size_t smallChunk = (index % 256) / 64;
	assert(smallChunk < 4);
	size_t bitOffset = index % 64;
	size_t result = 0;
	result += ((((midChunkPtr->firstBits)[smallChunk]) >> bitOffset) & 1) * 2;
	result += (((midChunkPtr->secondBits)[smallChunk]) >> bitOffset) & 1;
	if (result > 0) return result+2;
	BigChunk* bigChunkPtr = (BigChunk*)(bytes.data()+bigChunk*countUint64sPerBigChunk);
	if ((((bigChunkPtr->which64BitChunksHaveSpecialChar)[(midChunkWithinBigChunk*4+smallChunk)/64] >> ((midChunkWithinBigChunk*4+smallChunk)%64)) & 1ull) == 1)
	{
		auto masks = getSpecialMasks(bigChunkPtr, midChunkWithinBigChunk*4 + smallChunk);
		if (masks.first & (1ull << bitOffset)) return 0;
		if (masks.second & (1ull << bitOffset)) return 1;
	}
	return 2;
}

size_t DNAPrefixSumIndex::rank(size_t index, uint8_t c) const
{
	assert(built);
	size_t bigChunk = index / countBasesPerBigChunk;
	BigChunk* bigChunkPtr = (BigChunk*)(bytes.data()+bigChunk*countUint64sPerBigChunk);
	size_t midChunkWithinBigChunk = (index - (bigChunk * countBasesPerBigChunk)) / countBasesPerMidChunk;
	MidChunk* midChunkPtr = (MidChunk*)(bytes.data()+bigChunk*countUint64sPerBigChunk+sizeof(BigChunk)/8+midChunkWithinBigChunk*countUint64sPerMidChunk);
	size_t smallChunk = (index - (bigChunk * countBasesPerBigChunk) - midChunkWithinBigChunk * countBasesPerMidChunk) / 64;
	assert(smallChunk < 4);
	size_t bitOffset = index % 64;
	size_t result = (bigChunkPtr->bigPrefixSums)[c];
	result += (midChunkPtr->midPrefixSums)[c];
	if (c >= 2)
	{
		switch(smallChunk)
		{
			case 0:
				break;
			case 1:
				result += midChunkPtr->smallPrefixSums[c-2];
				break;
			case 2:
				result += midChunkPtr->smallPrefixSums[4+c-2];
				break;
			case 3:
				result += midChunkPtr->smallPrefixSums[8+c-2];
				break;
		}
		uint64_t matchBases = 0;
		switch(c)
		{
			case 2:
				matchBases = ~((midChunkPtr->firstBits)[smallChunk]) & ~((midChunkPtr->secondBits)[smallChunk]);
				break;
			case 3:
				matchBases = ~((midChunkPtr->firstBits)[smallChunk]) & ((midChunkPtr->secondBits)[smallChunk]);
				break;
			case 4:
				matchBases = ((midChunkPtr->firstBits)[smallChunk]) & ~((midChunkPtr->secondBits)[smallChunk]);
				break;
			case 5:
				matchBases = ((midChunkPtr->firstBits)[smallChunk]) & ((midChunkPtr->secondBits)[smallChunk]);
				break;
		}
		uint64_t mask = (1ull << bitOffset) - 1;
		if (c == 2)
		{
			if ((((bigChunkPtr->which64BitChunksHaveSpecialChar)[(midChunkWithinBigChunk*4+smallChunk)/64] >> ((midChunkWithinBigChunk*4+smallChunk)%64)) & 1ull) == 1)
			{
				auto masks = getSpecialMasks(bigChunkPtr, midChunkWithinBigChunk*4 + smallChunk);
				matchBases &= ~masks.first;
				matchBases &= ~masks.second;
			}
		}
		matchBases &= mask;
		result += popcount(matchBases);
		return result;
	}
	else
	{
		for (size_t i = 0; i <= smallChunk; i++)
		{
			if ((((bigChunkPtr->which64BitChunksHaveSpecialChar)[(midChunkWithinBigChunk*4+i)/64] >> ((midChunkWithinBigChunk*4+i)%64)) & 1ull) == 1)
			{
				uint64_t matchBases;
				auto masks = getSpecialMasks(bigChunkPtr, midChunkWithinBigChunk*4 + i);
				if (c == 0)
				{
					matchBases = masks.first;
				}
				else
				{
					matchBases = masks.second;
				}
				if (i == smallChunk)
				{
					uint64_t mask = (1ull << bitOffset) - 1;
					matchBases &= mask;
				}
				result += popcount(matchBases);
			}
		}
		return result;
	}
}

size_t DNAPrefixSumIndex::size() const
{
	return realSize;
}

size_t DNAPrefixSumIndex::charCount(uint8_t c) const
{
	assert(built);
	assert(c <= 5);
	return counts[c];
}

void DNAPrefixSumIndex::save(std::ostream& stream) const
{
	assert(built);
	serialize(stream, realSize);
	serialize(stream, bytes);
	serialize(stream, counts);
	serialize(stream, specialCharMasks);
}

void DNAPrefixSumIndex::load(std::istream& stream)
{
	assert(!built);
	deserialize(stream, realSize);
	deserialize(stream, bytes);
	deserialize(stream, counts);
	deserialize(stream, specialCharMasks);
	built = true;
}

bool DNAPrefixSumIndex::operator==(const DNAPrefixSumIndex& other) const
{
	if (realSize != other.realSize) return false;
	if (built != other.built) return false;
	if (bytes != other.bytes) return false;
	if (counts != other.counts) return false;
	if (specialCharMasks != other.specialCharMasks) return false;
	return true;
}

bool DNAPrefixSumIndex::operator!=(const DNAPrefixSumIndex& other) const
{
	return !(*this == other);
}

inline std::pair<uint64_t, uint64_t> DNAPrefixSumIndex::getSpecialMasks(const BigChunk * const bigChunkPtr, const size_t smallChunkWithinBigChunk) const
{
	size_t specialStart = bigChunkPtr->specialCharMaskStartIndex;
	for (size_t block = 0; block < smallChunkWithinBigChunk/64; block++)
	{
		specialStart += popcount((bigChunkPtr->which64BitChunksHaveSpecialChar)[block]);
	}
	uint64_t mask = (1ull << (smallChunkWithinBigChunk % 64)) - 1;
	specialStart += (popcount((bigChunkPtr->which64BitChunksHaveSpecialChar)[smallChunkWithinBigChunk / 64] & mask));
	return specialCharMasks[specialStart];
}
