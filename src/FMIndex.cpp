#include <string>
#include <cassert>
#include <limits>
#include "libsais64.h"
#include "FMIndex.h"
#include "Serialize.h"
#include "PartSortBWT.h"

FMIndex::FMIndex() :
built(false),
sampleRate(0),
tree(),
lowSample(false),
lowSampledPositions(),
sampledPositions(),
hasPosition()
{
}

FMIndex::FMIndex(std::string&& seq, const size_t sampleRate, const bool lowMemoryConstruction, const bool useWaveletTree) :
built(false),
sampleRate(0),
tree(),
lowSample(false),
lowSampledPositions(),
sampledPositions(),
hasPosition()
{
	if (lowMemoryConstruction)
	{
		initializeLowMemory(std::move(seq), sampleRate, useWaveletTree);
	}
	else
	{
		initialize(std::move(seq), sampleRate, useWaveletTree);
	}
}

void FMIndex::initialize(std::string&& seq, const size_t sampleRate, const bool useWaveletTree)
{
	hasWaveletTree = useWaveletTree;
	assert(!built);
	hasPosition.resize(seq.size());
	this->sampleRate = sampleRate;
	assert(sampleRate >= 1);
	assert(sampleRate < seq.size());
	for (size_t i = 0; i < seq.size()-1; i++)
	{
		assert(seq[i] <= 5);
		assert(seq[i] >= 1);
	}
	assert(seq[seq.size()-1] == 0);
	{
		std::vector<int64_t> tmp;
		tmp.resize(seq.size());
		size_t status = libsais64((uint8_t*)seq.data(), (int64_t*)tmp.data(), seq.size(), 0, nullptr);
		assert(status == 0);
		std::string bwt;
		bwt.resize(seq.size(), 0);
		bwt[0] = seq[seq.size()];
		lowSample = (size_t)((seq.size() + sampleRate - 1) / sampleRate) < (size_t)std::numeric_limits<uint32_t>::max();
		if (lowSample)
		{
			lowSampledPositions.reserve((seq.size() + sampleRate - 1) / sampleRate);
		}
		else
		{
			sampledPositions.reserve((seq.size() + sampleRate - 1) / sampleRate);
		}
		for (size_t i = 0; i < seq.size(); i++)
		{
			if (tmp[i] > 0)
			{
				bwt[i] = seq[tmp[i]-1];
				if ((tmp[i]-1) % sampleRate == 0)
				{
					hasPosition.set(i, true);
					if (lowSample)
					{
						lowSampledPositions.push_back((tmp[i]-1) / sampleRate);
					}
					else
					{
						sampledPositions.push_back((tmp[i]-1) / sampleRate);
					}
				}
			}
			else
			{
				bwt[i] = seq[seq.size()-1];
				if ((seq.size()-1) % sampleRate == 0)
				{
					hasPosition.set(i, true);
					if (lowSample)
					{
						lowSampledPositions.push_back((seq.size()-1) / sampleRate);
					}
					else
					{
						sampledPositions.push_back((seq.size()-1) / sampleRate);
					}
				}
			}
		}
		assert(lowSample || sampledPositions.size() == (seq.size() + sampleRate - 1) / sampleRate);
		assert(!lowSample || lowSampledPositions.size() == (seq.size() + sampleRate - 1) / sampleRate);
		if (useWaveletTree)
		{
			tree.initialize(bwt);
		}
		else
		{
			flatRanks.initialize(bwt);
		}
	}
	startIndices[0] = 0;
	if (useWaveletTree)
	{
		startIndices[1] = tree.charCount(0);
		startIndices[2] = tree.charCount(1) + startIndices[1];
		startIndices[3] = tree.charCount(2) + startIndices[2];
		startIndices[4] = tree.charCount(3) + startIndices[3];
		startIndices[5] = tree.charCount(4) + startIndices[4];
	}
	else
	{
		startIndices[1] = flatRanks.charCount(0);
		startIndices[2] = flatRanks.charCount(1) + startIndices[1];
		startIndices[3] = flatRanks.charCount(2) + startIndices[2];
		startIndices[4] = flatRanks.charCount(3) + startIndices[3];
		startIndices[5] = flatRanks.charCount(4) + startIndices[4];
	}
	assert(startIndices[5] < size());
	hasPosition.buildRanks();
	assert(lowSample || sampledPositions.size() == hasPosition.rankOne(hasPosition.size()));
	assert(!lowSample || lowSampledPositions.size() == hasPosition.rankOne(hasPosition.size()));
	built = true;
}

void FMIndex::initializeLowMemory(std::string&& seq, const size_t sampleRate, const bool useWaveletTree)
{
	for (size_t i = 0; i < seq.size()-1; i++)
	{
		assert(seq[i] <= 5);
		assert(seq[i] >= 1);
	}
	assert(seq[seq.size()-1] == 0);
	partSortBWT(seq, seq);
	initializeFromBWT(std::move(seq), sampleRate, useWaveletTree);
}

void FMIndex::initializeFromBWT(std::string&& bwt, const size_t sampleRate, const bool useWaveletTree)
{
	hasWaveletTree = useWaveletTree;
	assert(!built);
	lowSample = (size_t)((bwt.size() + sampleRate - 1) / sampleRate) < (size_t)std::numeric_limits<uint32_t>::max();
	if (lowSample)
	{
		lowSampledPositions.reserve((bwt.size() + sampleRate - 1) / sampleRate);
	}
	else
	{
		sampledPositions.reserve((bwt.size() + sampleRate - 1) / sampleRate);
	}
	if (useWaveletTree)
	{
		tree.initialize(bwt);
		assert(tree.size() == bwt.size());
	}
	else
	{
		flatRanks.initialize(bwt);
		assert(flatRanks.size() == bwt.size());
	}
	{
		std::string tmp;
		std::swap(tmp, bwt);
	}
	hasPosition.resize(size());
	this->sampleRate = sampleRate;
	assert(sampleRate >= 1);
	assert(sampleRate < size());
	startIndices[0] = 0;
	if (useWaveletTree)
	{
		startIndices[1] = tree.charCount(0);
		startIndices[2] = tree.charCount(1) + startIndices[1];
		startIndices[3] = tree.charCount(2) + startIndices[2];
		startIndices[4] = tree.charCount(3) + startIndices[3];
		startIndices[5] = tree.charCount(4) + startIndices[4];
	}
	else
	{
		startIndices[1] = flatRanks.charCount(0);
		startIndices[2] = flatRanks.charCount(1) + startIndices[1];
		startIndices[3] = flatRanks.charCount(2) + startIndices[2];
		startIndices[4] = flatRanks.charCount(3) + startIndices[3];
		startIndices[5] = flatRanks.charCount(4) + startIndices[4];
	}
	assert(startIndices[5] < size());
	size_t index = 0;
	for (size_t i = 0; i < size(); i++)
	{
		assert(hasPosition.get(index) == false);
		size_t seqPos = (2*size()-2-i) % size();
		if (seqPos % sampleRate == 0)
		{
			hasPosition.set(index, true);
		}
		index = advance(index, get(index));
	}
	hasPosition.buildRanks();
	assert(hasPosition.rankOne(hasPosition.size()) == ((size() + sampleRate - 1) / sampleRate));
	if (lowSample)
	{
		lowSampledPositions.resize(hasPosition.rankOne(hasPosition.size()), std::numeric_limits<uint32_t>::max());
	}
	else
	{
		sampledPositions.resize(hasPosition.rankOne(hasPosition.size()), std::numeric_limits<size_t>::max());
	}
	index = 0;
	for (size_t i = 0; i < size(); i++)
	{
		size_t seqPos = (2*size()-2-i) % size();
		if (seqPos % sampleRate == 0)
		{
			assert(hasPosition.get(index) == true);
			size_t samplePos = hasPosition.rankOne(index);
			if (lowSample)
			{
				assert(lowSampledPositions[samplePos] == std::numeric_limits<uint32_t>::max());
				lowSampledPositions[samplePos] = seqPos / sampleRate;
			}
			else
			{
				assert(sampledPositions[samplePos] == std::numeric_limits<size_t>::max());
				sampledPositions[samplePos] = seqPos / sampleRate;
			}
		}
		else
		{
			assert(hasPosition.get(index) == false);
		}
		index = advance(index, get(index));
	}
	for (size_t i = 0; i < lowSampledPositions.size(); i++)
	{
		assert(lowSampledPositions[i] != std::numeric_limits<uint32_t>::max());
	}
	for (size_t i = 0; i < sampledPositions.size(); i++)
	{
		assert(sampledPositions[i] != std::numeric_limits<size_t>::max());
	}
	built = true;
}

size_t FMIndex::advance(size_t pos, uint8_t c) const
{
	assert(c <= 5);
	assert(startIndices[c] < size());
	size_t rank;
	if (hasWaveletTree)
	{
		rank = tree.rank(pos, c);
	}
	else
	{
		rank = flatRanks.rank(pos, c);
	}
	assert(rank < size());
	size_t result = startIndices[c] + rank;
	assert(result <= size());
	return result;
}

std::pair<size_t, size_t> FMIndex::advance(size_t start, size_t end, uint8_t c) const
{
	start = advance(start, c);
	end = advance(end, c);
	return std::make_pair(start, end);
}

size_t FMIndex::size() const
{
	if (hasWaveletTree)
	{
		return tree.size();
	}
	else
	{
		return flatRanks.size();
	}
}

size_t FMIndex::charCount(uint8_t c) const
{
	if (hasWaveletTree)
	{
		return tree.charCount(c);
	}
	else
	{
		return flatRanks.charCount(c);
	}
}

size_t FMIndex::charStart(uint8_t c) const
{
	return startIndices[c];
}

size_t FMIndex::locate(size_t pos) const
{
	assert(built);
	size_t offset = 0;
	while (!hasPosition.get(pos))
	{
		pos = advance(pos, get(pos));
		offset += 1;
	}
	assert(offset < sampleRate);
	if (lowSample)
	{
		offset += (size_t)lowSampledPositions[hasPosition.rankOne(pos)] * sampleRate;
	}
	else
	{
		offset += (size_t)sampledPositions[hasPosition.rankOne(pos)] * sampleRate;
	}
	return offset;
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
	if (hasWaveletTree)
	{
		return tree.get(i);
	}
	else
	{
		return flatRanks.get(i);
	}
}

bool FMIndex::initialized() const
{
	return built;
}

void FMIndex::save(std::ostream& stream) const
{
	assert(built);
	for (size_t i = 0; i < 6; i++)
	{
		serialize(stream, startIndices[i]);
	}
	serialize(stream, sampleRate);
	tree.save(stream);
	serialize(stream, lowSample);
	serialize(stream, lowSampledPositions);
	serialize(stream, sampledPositions);
	hasPosition.save(stream);
	serialize(stream, hasWaveletTree);
	flatRanks.save(stream);
}

void FMIndex::load(std::istream& stream)
{
	assert(!built);
	for (size_t i = 0; i < 6; i++)
	{
		deserialize(stream, startIndices[i]);
	}
	deserialize(stream, sampleRate);
	tree.load(stream);
	deserialize(stream, lowSample);
	deserialize(stream, lowSampledPositions);
	deserialize(stream, sampledPositions);
	hasPosition.load(stream);
	deserialize(stream, hasWaveletTree);
	flatRanks.load(stream);
	built = true;
}

bool FMIndex::operator==(const FMIndex& other) const
{
	if (built != other.built) return false;
	for (size_t i = 0; i < 6; i++)
	{
		if (startIndices[i] != other.startIndices[i]) return false;
	}
	if (sampleRate != other.sampleRate) return false;
	if (tree != other.tree) return false;
	if (lowSample != other.lowSample) return false;
	if (lowSampledPositions != other.lowSampledPositions) return false;
	if (sampledPositions != other.sampledPositions) return false;
	if (hasPosition != other.hasPosition) return false;
	if (hasWaveletTree != other.hasWaveletTree) return false;
	if (flatRanks != other.flatRanks) return false;
	return true;
}

bool FMIndex::operator!=(const FMIndex& other) const
{
	return !(*this == other);
}
