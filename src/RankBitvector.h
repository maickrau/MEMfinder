#include <cstdint>
#include <vector>

// "Optimized Succinct Data Structures for Massive Data", Simon Gog, Matthias Petri 2014.
// Section 3: A CACHE FRIENDLY RANK IMPLEMENTATION FOR UNCOMPRESSED BITVECTORS
class RankBitvector
{
public:
	RankBitvector(size_t size);
	void buildRanks();
	size_t size() const;
	bool get(size_t index) const;
	void set(size_t index, bool value);
	size_t rankOne(size_t index) const;
	size_t rankZero(size_t index) const;
private:
	std::vector<uint64_t> values;
	bool ranksBuilt;
	size_t realSize;
};
