#ifndef Serialize_h
#define Serialize_h

#include <cstdint>
#include <iostream>
#include <vector>

void serialize(std::ostream& stream, uint32_t value);
void serialize(std::ostream& stream, uint64_t value);
void serialize(std::ostream& stream, __uint128_t value);
void serialize(std::ostream& stream, bool value);
template<typename T>
void serialize(std::ostream& stream, const std::vector<T>& value)
{
	serialize(stream, (uint64_t)value.size());
	for (size_t i = 0; i < value.size(); i++)
	{
		serialize(stream, value[i]);
	}
}

void deserialize(std::istream& stream, uint32_t& value);
void deserialize(std::istream& stream, uint64_t& value);
void deserialize(std::istream& stream, __uint128_t& value);
void deserialize(std::istream& stream, bool& value);
template<typename T>
void deserialize(std::istream& stream, std::vector<T>& value)
{
	uint64_t size = 0;
	deserialize(stream, size);
	value.resize(size);
	for (size_t i = 0; i < value.size(); i++)
	{
		deserialize(stream, value[i]);
	}
}

#endif
