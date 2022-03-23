#ifndef WaveletTree_h
#define WaveletTree_h

#include <string>
#include <cstdint>
#include <vector>
#include <array>
#include <iostream>
#include "RankBitvector.h"

class WaveletTree
{
public:
	WaveletTree();
	WaveletTree(size_t size);
	WaveletTree(const std::string& seq);
	void initialize(const std::string& seq);
	uint8_t get(size_t index) const;
	size_t rank(size_t index, uint8_t c) const;
	size_t size() const;
	size_t charCount(uint8_t c) const;
	void save(std::ostream& stream) const;
	void load(std::istream& stream);
	bool operator==(const WaveletTree& other) const;
	bool operator!=(const WaveletTree& other) const;
private:
	std::array<uint64_t, 6> counts;
	RankBitvector layer1;
	RankBitvector layer2;
	RankBitvector layer3;
	bool built;
};

#endif
