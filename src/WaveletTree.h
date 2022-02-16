#include <cstdint>
#include <vector>
#include <array>
#include "RankBitvector.h"

class WaveletTree
{
public:
	WaveletTree(const std::string& seq);
	uint8_t get(size_t index) const;
	size_t rank(size_t index, uint8_t c) const;
	size_t size() const;
private:
	std::array<size_t, 5> counts;
	RankBitvector layer1;
	RankBitvector layer2;
	RankBitvector layer3;
};
