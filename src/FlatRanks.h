#ifndef FlatRanks_h
#define FlatRanks_h

#include <string>
#include <cstdint>
#include <vector>
#include <array>
#include <iostream>
#include "RankBitvector.h"

class FlatRanks
{
public:
	FlatRanks();
	FlatRanks(size_t size);
	FlatRanks(const std::string& seq);
	void initialize(const std::string& seq);
	uint8_t get(size_t index) const;
	size_t rank(size_t index, uint8_t c) const;
	size_t size() const;
	size_t charCount(uint8_t c) const;
	void save(std::ostream& stream) const;
	void load(std::istream& stream);
	bool operator==(const FlatRanks& other) const;
	bool operator!=(const FlatRanks& other) const;
private:
	std::array<uint64_t, 6> counts;
	RankBitvector charBitvector[6];
	bool built;
};

#endif
