#include <cassert>
#include "FlatRanks.h"
#include "Serialize.h"

FlatRanks::FlatRanks() :
counts({0, 0, 0, 0, 0, 0}),
charBitvector(),
built(false)
{
}

FlatRanks::FlatRanks(size_t size) :
counts({0, 0, 0, 0, 0, 0}),
built(false)
{
	charBitvector[0] = {size};
	charBitvector[1] = {size};
	charBitvector[2] = {size};
	charBitvector[3] = {size};
	charBitvector[4] = {size};
	charBitvector[5] = {size}; 
}

FlatRanks::FlatRanks(const std::string& seq) :
counts({0, 0, 0, 0, 0, 0}),
built(false)
{
	charBitvector[0] = {seq.size()};
	charBitvector[1] = {seq.size()};
	charBitvector[2] = {seq.size()};
	charBitvector[3] = {seq.size()};
	charBitvector[4] = {seq.size()};
	charBitvector[5] = {seq.size()};
	initialize(seq);
}

void FlatRanks::initialize(const std::string& seq)
{
	assert(!built);
	if (charBitvector[0].size() == 0)
	{
		assert(charBitvector[1].size() == 0);
		assert(charBitvector[2].size() == 0);
		assert(charBitvector[3].size() == 0);
		assert(charBitvector[4].size() == 0);
		assert(charBitvector[5].size() == 0);
		charBitvector[0].resize(seq.size());
		charBitvector[1].resize(seq.size());
		charBitvector[2].resize(seq.size());
		charBitvector[3].resize(seq.size());
		charBitvector[4].resize(seq.size());
		charBitvector[5].resize(seq.size());
	}
	else
	{
		assert(charBitvector[0].size() == seq.size());
		assert(charBitvector[1].size() == seq.size());
		assert(charBitvector[2].size() == seq.size());
		assert(charBitvector[3].size() == seq.size());
		assert(charBitvector[4].size() == seq.size());
		assert(charBitvector[5].size() == seq.size());
	}
	for (size_t i = 0; i < seq.size(); i++)
	{
		switch(seq[i])
		{
			case 0:
			case '$':
				counts[0] += 1;
				charBitvector[0].set(i, true);
				break;
			case 1:
			case 'N':
			case 'n':
				counts[1] += 1;
				charBitvector[1].set(i, true);
				break;
			case 2:
			case 'A':
			case 'a':
				counts[2] += 1;
				charBitvector[2].set(i, true);
				break;
			case 3:
			case 'C':
			case 'c':
				counts[3] += 1;
				charBitvector[3].set(i, true);
				break;
			case 4:
			case 'G':
			case 'g':
				counts[4] += 1;
				charBitvector[4].set(i, true);
				break;
			case 5:
			case 'T':
			case 't':
				counts[5] += 1;
				charBitvector[5].set(i, true);
				break;
			default:
				assert(false);
		}
	}
	charBitvector[0].buildRanks();
	charBitvector[1].buildRanks();
	charBitvector[2].buildRanks();
	charBitvector[3].buildRanks();
	charBitvector[4].buildRanks();
	charBitvector[5].buildRanks();
	assert(charBitvector[0].rankOne(seq.size()) == counts[0]);
	assert(charBitvector[1].rankOne(seq.size()) == counts[1]);
	assert(charBitvector[2].rankOne(seq.size()) == counts[2]);
	assert(charBitvector[3].rankOne(seq.size()) == counts[3]);
	assert(charBitvector[4].rankOne(seq.size()) == counts[4]);
	assert(charBitvector[5].rankOne(seq.size()) == counts[5]);
	built = true;
}

uint8_t FlatRanks::get(size_t index) const
{
	assert(built);
	assert(index < size());
	if (charBitvector[0].get(index)) return 0;
	if (charBitvector[1].get(index)) return 1;
	if (charBitvector[2].get(index)) return 2;
	if (charBitvector[3].get(index)) return 3;
	if (charBitvector[4].get(index)) return 4;
	if (charBitvector[5].get(index)) return 5;
	assert(false);
	return 0;
}

size_t FlatRanks::rank(size_t index, uint8_t c) const
{
	assert(built);
	assert(index <= size());
	return charBitvector[c].rankOne(index);
}

size_t FlatRanks::size() const
{
	return charBitvector[0].size();
}

size_t FlatRanks::charCount(uint8_t c) const
{
	assert(built);
	assert(c <= 5);
	return counts[c];
}

void FlatRanks::save(std::ostream& stream) const
{
	assert(built);
	for (size_t i = 0; i < 6; i++)
	{
		serialize(stream, counts[i]);
	}
	charBitvector[0].save(stream);
	charBitvector[1].save(stream);
	charBitvector[2].save(stream);
	charBitvector[3].save(stream);
	charBitvector[4].save(stream);
	charBitvector[5].save(stream);
}

void FlatRanks::load(std::istream& stream)
{
	assert(!built);
	for (size_t i = 0; i < 6; i++)
	{
		deserialize(stream, counts[i]);
	}
	charBitvector[0].load(stream);
	charBitvector[1].load(stream);
	charBitvector[2].load(stream);
	charBitvector[3].load(stream);
	charBitvector[4].load(stream);
	charBitvector[5].load(stream);
	built = true;
}

bool FlatRanks::operator==(const FlatRanks& other) const
{
	for (size_t i = 0; i < 6; i++)
	{
		if (counts[i] != other.counts[i]) return false;
		if (charBitvector[i] != other.charBitvector[i]) return false;
	}
	return true;
}

bool FlatRanks::operator!=(const FlatRanks& other) const
{
	return !(*this == other);
}
