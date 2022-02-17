#include <string>
#include <cassert>
#include "libsais64.h"
#include "FMIndex.h"

FMIndex::FMIndex(std::string seq, const size_t sampleRate) :
sampleRate(sampleRate),
tree(seq.size()),
sampledPositions(),
hasPosition(seq.size())
{
	assert(sampleRate >= 1);
	assert(sampleRate < seq.size());
	size_t lastIndex;
	for (size_t i = 0; i < seq.size()-1; i++)
	{
		assert(seq[i] <= 5);
		assert(seq[i] >= 1);
	}
	assert(seq[seq.size()-1] == 0);
	{
		std::vector<int64_t> tmp;
		tmp.resize(seq.size());
		lastIndex = libsais64((uint8_t*)seq.data(), (int64_t*)tmp.data(), seq.size(), 0, nullptr);
		assert(lastIndex < seq.size());
		std::string bwt;
		bwt.resize(seq.size(), 0);
		bwt[0] = seq[seq.size()];
		for (size_t i = 0; i < seq.size(); i++)
		{
			if (tmp[i] > 0)
			{
				bwt[i] = seq[tmp[i]-1];
			}
			else
			{
				lastIndex = i;
				bwt[i] = seq[seq.size()-1];
			}
		}
		tree.initialize(bwt);
	}
	startIndices[0] = 0;
	startIndices[1] = tree.charCount(0);
	startIndices[2] = tree.charCount(1) + startIndices[1];
	startIndices[3] = tree.charCount(2) + startIndices[2];
	startIndices[4] = tree.charCount(3) + startIndices[3];
	startIndices[5] = tree.charCount(4) + startIndices[4];
	assert(startIndices[5] < size());
	size_t pos = lastIndex;
	hasPosition.clear();
	for (size_t i = size()-1; i < size(); i--)
	{
		assert(!hasPosition.get(pos));
		if (i % sampleRate == 0) hasPosition.set(pos, true);
		pos = advance(pos, get(pos));
	}
	hasPosition.buildRanks();
	sampledPositions.resize((size()+sampleRate-1) / sampleRate, std::numeric_limits<size_t>::max());
	assert(sampledPositions.size() == hasPosition.rankOne(hasPosition.size()));
	pos = lastIndex;
	for (size_t i = size()-1; i < size(); i--)
	{
		if (i % sampleRate == 0)
		{
			assert(hasPosition.get(pos));
			size_t index = hasPosition.rankOne(pos);
			assert(sampledPositions[index] == std::numeric_limits<size_t>::max());
			sampledPositions[index] = i / sampleRate;
		}
		else
		{
			assert(!hasPosition.get(pos));
		}
		pos = advance(pos, get(pos));
	}
	for (size_t i = 0; i < sampledPositions.size(); i++)
	{
		assert(sampledPositions[i] != std::numeric_limits<size_t>::max());
	}
}

size_t FMIndex::advance(size_t pos, uint8_t c) const
{
	assert(c <= 5);
	assert(startIndices[c] < size());
	assert(tree.rank(pos, c) < size());
	size_t result = startIndices[c] + tree.rank(pos, c);
	assert(result < size());
	return result;
}

std::pair<size_t, size_t> FMIndex::advance(size_t start, size_t end, uint8_t c) const
{
	start = startIndices[c] + tree.rank(start, c);
	end = startIndices[c] + tree.rank(end, c);
	return std::make_pair(start, end);
}

size_t FMIndex::size() const
{
	return tree.size();
}

size_t FMIndex::charCount(uint8_t c) const
{
	return tree.charCount(c);
}

size_t FMIndex::locate(size_t pos) const
{
	size_t extraOffset = 0;
	while (!hasPosition.get(pos))
	{
		pos = advance(pos, get(pos));
		extraOffset += 1;
	}
	return sampledPositions[hasPosition.rankOne(pos)] * sampleRate + extraOffset;
}

uint8_t FMIndex::getNext(size_t i) const
{
	if (i < charCount(0)) return 0;
	i -= charCount(0);
	if (i < charCount(1)) return 1;
	i -= charCount(1);
	if (i < charCount(2)) return 2;
	i -= charCount(2);
	if (i < charCount(3)) return 3;
	i -= charCount(3);
	if (i < charCount(4)) return 4;
	i -= charCount(4);
	assert(i < charCount(5));
	return 5;
}

uint8_t FMIndex::get(size_t i) const
{
	return tree.get(i);
}
