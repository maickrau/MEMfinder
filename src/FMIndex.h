#ifndef FMIndex_h
#define FMIndex_h

#include <vector>
#include <cstdint>
#include <string>
#include <iostream>
#include "WaveletTree.h"
#include "RankBitvector.h"
#include "FlatRanks.h"
#include "DNAPrefixSumIndex.h"

class FMIndex
{
public:
	enum IndexPrefixStructureType
	{
		useFlatRanks = 0,
		useWaveletTree = 1,
		useDNAPrefixSumIndex = 2
	};
	FMIndex();
	FMIndex(std::string&& seq, const size_t sampleRate, const bool lowMemoryConstruction = false, const IndexPrefixStructureType prefixStructureType = IndexPrefixStructureType::useDNAPrefixSumIndex);
	std::pair<size_t, size_t> advance(size_t start, size_t end, uint8_t c) const;
	void initialize(std::string&& seq, const size_t sampleRate, const IndexPrefixStructureType prefixStructureType = IndexPrefixStructureType::useDNAPrefixSumIndex);
	void initializeLowMemory(std::string&& seq, const size_t sampleRate, const IndexPrefixStructureType prefixStructureType = IndexPrefixStructureType::useDNAPrefixSumIndex);
	size_t size() const;
	size_t charCount(uint8_t c) const;
	size_t charStart(uint8_t c) const;
	size_t locate(size_t pos) const;
	uint8_t get(size_t i) const;
	size_t advance(size_t pos, uint8_t c) const;
	bool initialized() const;
	void save(std::ostream& stream) const;
	void load(std::istream& stream);
	bool operator==(const FMIndex& other) const;
	bool operator!=(const FMIndex& other) const;
private:
	void initializeFromBWT(std::string&& bwt, const size_t sampleRate, const IndexPrefixStructureType prefixStructureType);
	uint8_t getNext(size_t i) const;
	bool built;
	std::array<uint64_t, 6> startIndices;
	uint64_t sampleRate;
	IndexPrefixStructureType prefixStructureType;
	WaveletTree tree;
	FlatRanks flatRanks;
	DNAPrefixSumIndex prefixSumIndex;
	bool lowSample;
	std::vector<uint32_t> lowSampledPositions;
	std::vector<uint64_t> sampledPositions;
	RankBitvector hasPosition;
};

#endif
