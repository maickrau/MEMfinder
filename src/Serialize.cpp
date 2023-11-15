#include "Serialize.h"

void serialize(std::ostream& stream, uint32_t value)
{
	stream.write((const char*)&value, sizeof(uint32_t));
}

void serialize(std::ostream& stream, uint64_t value)
{
	stream.write((const char*)&value, sizeof(uint64_t));
}

void serialize(std::ostream& stream, __uint128_t value)
{
	stream.write((const char*)&value, sizeof(__uint128_t));
}

void serialize(std::ostream& stream, bool value)
{
	uint8_t c = value ? 1 : 0;
	stream.write((const char*)&c, sizeof(uint8_t));
}

void deserialize(std::istream& stream, uint32_t& value)
{
	stream.read((char*)&value, sizeof(uint32_t));
}

void deserialize(std::istream& stream, uint64_t& value)
{
	stream.read((char*)&value, sizeof(uint64_t));
}

void deserialize(std::istream& stream, __uint128_t& value)
{
	stream.read((char*)&value, sizeof(__uint128_t));
}

void deserialize(std::istream& stream, bool& value)
{
	uint8_t c = 0;
	stream.read((char*)&c, sizeof(uint8_t));
	value = (c == 1);
}
