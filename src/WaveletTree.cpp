#include <cassert>
#include "WaveletTree.h"

WaveletTree::WaveletTree(const std::string& seq) :
counts({0, 0, 0, 0, 0}),
layer1(seq.size()),
layer2(seq.size()),
layer3(seq.size())
{
	for (size_t i = 0; i < seq.size(); i++)
	{
		switch(seq[i])
		{
			case 0:
			case 'N':
			case 'n':
				counts[0] += 1;
				break;
			case 1:
			case 'A':
			case 'a':
				counts[1] += 1;
				break;
			case 2:
			case 'C':
			case 'c':
				counts[2] += 1;
				break;
			case 3:
			case 'G':
			case 'g':
				counts[3] += 1;
				break;
			case 4:
			case 'T':
			case 't':
				counts[4] += 1;
				break;
			default:
				assert(false);
		}
	}
	size_t leftPos = 0;
	size_t rightPos = 0;
	size_t thirdPos = 0;
	for (size_t i = 0; i < seq.size(); i++)
	{
		switch(seq[i])
		{
			case 0:
			case 'N':
			case 'n':
				layer1.set(i, false);
				layer2.set(leftPos, false);
				layer3.set(thirdPos, false);
				thirdPos += 1;
				leftPos += 1;
				break;
			case 1:
			case 'A':
			case 'a':
				layer1.set(i, false);
				layer2.set(leftPos, false);
				layer3.set(thirdPos, true);
				thirdPos += 1;
				leftPos += 1;
				break;
			case 2:
			case 'C':
			case 'c':
				layer1.set(i, false);
				layer2.set(leftPos, true);
				leftPos += 1;
				break;
			case 3:
			case 'G':
			case 'g':
				layer1.set(i, true);
				layer2.set(counts[0] + counts[1] + counts[2] + rightPos, false);
				rightPos += 1;
				break;
			case 4:
			case 'T':
			case 't':
				layer1.set(i, true);
				layer2.set(counts[0] + counts[1] + counts[2] + rightPos, true);
				rightPos += 1;
				break;
			default:
				assert(false);
		}
	}
	assert(leftPos == counts[0] + counts[1] + counts[2]);
	assert(thirdPos == counts[0] + counts[1]);
	assert(rightPos == counts[3] + counts[4]);
	layer1.buildRanks();
	layer2.buildRanks();
	layer3.buildRanks();
	assert(layer1.rankZero(seq.size()) == counts[0] + counts[1] + counts[2]);
	assert(layer1.rankOne(seq.size()) == counts[3] + counts[4]);
	assert(layer2.rankZero(counts[0] + counts[1] + counts[2]) == counts[0] + counts[1]);
	assert(layer2.rankOne(counts[0] + counts[1] + counts[2]) == counts[2]);
	assert(layer3.rankOne(layer3.size()) == counts[1]);
}

uint8_t WaveletTree::get(size_t index) const
{
	assert(index < size());
	bool bit1 = layer1.get(index);
	if (bit1)
	{
		bool bit2 = layer2.get(counts[0] + counts[1] + counts[2] + layer1.rankOne(index));
		if (!bit2) return 3;
		if (bit2) return 4;
	}
	size_t layer2pos = layer1.rankZero(index);
	bool bit2 = layer2.get(layer2pos);
	if (bit2) return 2;
	bool bit3 = layer3.get(layer2.rankZero(layer2pos));
	if (!bit3) return 0;
	if (bit3) return 1;
	assert(false);
	return 0;
}

size_t WaveletTree::rank(size_t index, uint8_t c) const
{
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
			return layer2.rankOne(layer2pos);
		case 3:
			layer2pos = layer1.rankOne(index);
			return layer2.rankZero(counts[0] + counts[1] + counts[2] + layer2pos) - counts[0] - counts[1];
		case 4:
			layer2pos = layer1.rankOne(index);
			return layer2.rankOne(counts[0] + counts[1] + counts[2] + layer2pos) - counts[2];
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
