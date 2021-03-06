#ifndef FMIndex_h
#define FMIndex_h

#include <vector>
#include <cstdint>
#include <string>
#include <iostream>
#include "WaveletTree.h"
#include "RankBitvector.h"

class FMIndex
{
public:
	FMIndex();
	FMIndex(std::string&& seq, const size_t sampleRate);
	std::pair<size_t, size_t> advance(size_t start, size_t end, uint8_t c) const;
	void initialize(std::string&& seq, const size_t sampleRate);
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
	uint8_t getNext(size_t i) const;
	bool built;
	std::array<uint64_t, 6> startIndices;
	uint64_t sampleRate;
	WaveletTree tree;
	bool lowSample;
	std::vector<uint32_t> lowSampledPositions;
	std::vector<uint64_t> sampledPositions;
	RankBitvector hasPosition;
};

#endif
