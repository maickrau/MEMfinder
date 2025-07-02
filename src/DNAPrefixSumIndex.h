#ifndef DNAPrefixSumIndex_h
#define DNAPrefixSumIndex_h

#include <string>
#include <cstdint>
#include <vector>
#include <array>
#include <iostream>
#include <unordered_map>
#include "RankBitvector.h"

// alphabet is $NACGT in that order
// assumption is that most chars are ACGT so $N have a less efficient representation to make ACGT more efficient
class DNAPrefixSumIndex
{
public:
	DNAPrefixSumIndex();
	DNAPrefixSumIndex(const std::string& seq);
	void initialize(const std::string& seq);
	uint8_t get(size_t index) const;
	size_t rank(size_t index, uint8_t c) const;
	size_t size() const;
	size_t charCount(uint8_t c) const;
	void save(std::ostream& stream) const;
	void load(std::istream& stream);
	bool operator==(const DNAPrefixSumIndex& other) const;
	bool operator!=(const DNAPrefixSumIndex& other) const;
private:
	struct MidChunk
	{
		uint64_t firstBits[4]; // small chunk refers to these 64-bit chunks of firstBits and secondBits
		uint64_t secondBits[4]; // only ACGT are encoded here, $N are encoded in special char masks
		uint16_t midPrefixSums[6]; // one per each alphabet at start of 256 char boundary
		uint8_t smallPrefixSums[12]; // three per ACGT alphabet at 64, 128 and 192 char boundaries
		// 704 bits -> 2.75 bits per char
	};
	struct BigChunk
	{
		uint64_t bigPrefixSums[6]; // one per each alphabet
		uint64_t specialCharMaskStartIndex;
		uint64_t which64BitChunksHaveSpecialChar[16]; // one bit per 64-bit small chunk within big chunk
		// 1472 bits -> ~0.02 bits per char
	};
	const size_t countBasesPerMidChunk = 256;
	const size_t countMidChunksPerBigChunk = 255;
	const size_t countBasesPerBigChunk = countMidChunksPerBigChunk * countBasesPerMidChunk; // 65280
	const size_t countUint64sPerMidChunk = sizeof(MidChunk)/8;
	const size_t countUint64sPerBigChunk = sizeof(BigChunk)/8 + countMidChunksPerBigChunk*countUint64sPerMidChunk;
	const size_t countSmallChunksPerBigChunk = countMidChunksPerBigChunk*4;
	std::vector<uint64_t> bytes;
	std::array<uint64_t, 6> counts;
	std::vector<std::pair<uint64_t, uint64_t>> specialCharMasks;
	uint64_t realSize;
	bool built;
	inline std::pair<uint64_t, uint64_t> getSpecialMasks(const BigChunk * const bigChunkPtr, const size_t smallChunk) const;
};

#endif
