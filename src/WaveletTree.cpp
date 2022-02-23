#include <cassert>
#include "WaveletTree.h"
#include "Serialize.h"

WaveletTree::WaveletTree() :
counts({0, 0, 0, 0, 0, 0}),
layer1(),
layer2(),
layer3(),
built(false)
{
}

WaveletTree::WaveletTree(size_t size) :
counts({0, 0, 0, 0, 0, 0}),
layer1(size),
layer2(size),
layer3(size),
built(false)
{
}

WaveletTree::WaveletTree(const std::string& seq) :
counts({0, 0, 0, 0, 0, 0}),
layer1(seq.size()),
layer2(seq.size()),
layer3(seq.size()),
built(false)
{
	initialize(seq);
}

void WaveletTree::initialize(const std::string& seq)
{
	assert(!built);
	if (layer1.size() == 0)
	{
		assert(layer2.size() == 0);
		assert(layer3.size() == 0);
		layer1.resize(seq.size());
		layer2.resize(seq.size());
		layer3.resize(seq.size());
	}
	else
	{
		assert(layer1.size() == seq.size());
		assert(layer2.size() == seq.size());
		assert(layer3.size() == seq.size());
	}
	for (size_t i = 0; i < seq.size(); i++)
	{
		switch(seq[i])
		{
			case 0:
			case '$':
				counts[0] += 1;
				break;
			case 1:
			case 'N':
			case 'n':
				counts[1] += 1;
				break;
			case 2:
			case 'A':
			case 'a':
				counts[2] += 1;
				break;
			case 3:
			case 'C':
			case 'c':
				counts[3] += 1;
				break;
			case 4:
			case 'G':
			case 'g':
				counts[4] += 1;
				break;
			case 5:
			case 'T':
			case 't':
				counts[5] += 1;
				break;
			default:
				assert(false);
		}
	}
	size_t leftPos = 0;
	size_t rightPos = 0;
	size_t thirdPos = 0;
	size_t thirdRightPos = 0;
	for (size_t i = 0; i < seq.size(); i++)
	{
		switch(seq[i])
		{
			case 0:
			case '$':
				layer1.set(i, false);
				layer2.set(leftPos, false);
				layer3.set(thirdPos, false);
				thirdPos += 1;
				leftPos += 1;
				break;
			case 1:
			case 'N':
			case 'n':
				layer1.set(i, false);
				layer2.set(leftPos, false);
				layer3.set(thirdPos, true);
				thirdPos += 1;
				leftPos += 1;
				break;
			case 2:
			case 'A':
			case 'a':
				layer1.set(i, false);
				layer2.set(leftPos, true);
				layer3.set(counts[0] + counts[1] + thirdRightPos, false);
				thirdRightPos += 1;
				leftPos += 1;
				break;
			case 3:
			case 'C':
			case 'c':
				layer1.set(i, false);
				layer2.set(leftPos, true);
				layer3.set(counts[0] + counts[1] + thirdRightPos, true);
				thirdRightPos += 1;
				leftPos += 1;
				break;
			case 4:
			case 'G':
			case 'g':
				layer1.set(i, true);
				layer2.set(counts[0] + counts[1] + counts[2] + counts[3] + rightPos, false);
				rightPos += 1;
				break;
			case 5:
			case 'T':
			case 't':
				layer1.set(i, true);
				layer2.set(counts[0] + counts[1] + counts[2] + counts[3] + rightPos, true);
				rightPos += 1;
				break;
			default:
				assert(false);
		}
	}
	assert(leftPos == counts[0] + counts[1] + counts[2] + counts[3]);
	assert(thirdPos == counts[0] + counts[1]);
	assert(thirdRightPos == counts[2] + counts[3]);
	assert(rightPos == counts[4] + counts[5]);
	layer1.buildRanks();
	layer2.buildRanks();
	layer3.buildRanks();
	assert(layer1.rankZero(seq.size()) == counts[0] + counts[1] + counts[2] + counts[3]);
	assert(layer1.rankOne(seq.size()) == counts[4] + counts[5]);
	assert(layer2.rankZero(seq.size()) == counts[0] + counts[1] + counts[4]);
	assert(layer2.rankOne(seq.size()) == counts[2] + counts[3] + counts[5]);
	assert(layer2.rankZero(counts[0] + counts[1] + counts[2] + counts[3]) == counts[0] + counts[1]);
	assert(layer2.rankOne(counts[0] + counts[1] + counts[2] + counts[3]) == counts[2] + counts[3]);
	assert(layer3.rankZero(counts[0] + counts[1]) == counts[0]);
	assert(layer3.rankOne(counts[0] + counts[1]) == counts[1]);
	assert(layer3.rankOne(layer3.size()) == counts[1] + counts[3]);
	built = true;
}

uint8_t WaveletTree::get(size_t index) const
{
	assert(built);
	assert(index < size());
	bool bit1 = layer1.get(index);
	if (bit1)
	{
		bool bit2 = layer2.get(counts[0] + counts[1] + counts[2] + counts[3] + layer1.rankOne(index));
		if (!bit2) return 4;
		if (bit2) return 5;
	}
	size_t layer2pos = layer1.rankZero(index);
	bool bit2 = layer2.get(layer2pos);
	if (bit2)
	{
		bool bit3 = layer3.get(counts[0] + counts[1] + layer2.rankOne(layer2pos));
		if (!bit3) return 2;
		if (bit3) return 3;
	}
	bool bit3 = layer3.get(layer2.rankZero(layer2pos));
	if (!bit3) return 0;
	if (bit3) return 1;
	assert(false);
	return 0;
}

size_t WaveletTree::rank(size_t index, uint8_t c) const
{
	assert(built);
	assert(index <= size());
	size_t layer2pos, layer3pos;
	switch(c)
	{
		case 0:
			layer2pos = layer1.rankZero(index);
			layer3pos = layer2.rankZero(layer2pos);
			return layer3.rankZero(layer3pos);
		case 1:
			layer2pos = layer1.rankZero(index);
			layer3pos = layer2.rankZero(layer2pos);
			return layer3.rankOne(layer3pos);
		case 2:
			layer2pos = layer1.rankZero(index);
			layer3pos = layer2.rankOne(layer2pos);
			return layer3.rankZero(counts[0] + counts[1] + layer3pos) - counts[0];
		case 3:
			layer2pos = layer1.rankZero(index);
			layer3pos = layer2.rankOne(layer2pos);
			return layer3.rankOne(counts[0] + counts[1] + layer3pos) - counts[1];
		case 4:
			layer2pos = layer1.rankOne(index);
			return layer2.rankZero(counts[0] + counts[1] + counts[2] + counts[3] + layer2pos) - counts[0] - counts[1];
		case 5:
			layer2pos = layer1.rankOne(index);
			return layer2.rankOne(counts[0] + counts[1] + counts[2] + counts[3] + layer2pos) - counts[2] - counts[3];
		default:
			assert(false);
			return 0;
	}
	return 0;
}

size_t WaveletTree::size() const
{
	return layer1.size();
}

size_t WaveletTree::charCount(uint8_t c) const
{
	assert(built);
	assert(c <= 5);
	return counts[c];
}

void WaveletTree::save(std::ostream& stream) const
{
	assert(built);
	for (size_t i = 0; i < 6; i++)
	{
		serialize(stream, counts[i]);
	}
	layer1.save(stream);
	layer2.save(stream);
	layer3.save(stream);
}

void WaveletTree::load(std::istream& stream)
{
	assert(!built);
	for (size_t i = 0; i < 6; i++)
	{
		deserialize(stream, counts[i]);
	}
	layer1.load(stream);
	layer2.load(stream);
	layer3.load(stream);
	built = true;
}

bool WaveletTree::operator==(const WaveletTree& other) const
{
	for (size_t i = 0; i < 6; i++)
	{
		if (counts[i] != other.counts[i]) return false;
	}
	if (layer1 != other.layer1) return false;
	if (layer2 != other.layer2) return false;
	if (layer3 != other.layer3) return false;
	if (built != other.built) return false;
	return true;
}

bool WaveletTree::operator!=(const WaveletTree& other) const
{
	return !(*this == other);
}
