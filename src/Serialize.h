#ifndef Serialize_h
#define Serialize_h

#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

void serialize(std::ostream& stream, uint8_t value);
void serialize(std::ostream& stream, uint32_t value);
void serialize(std::ostream& stream, uint64_t value);
void serialize(std::ostream& stream, __uint128_t value);
void serialize(std::ostream& stream, bool value);
void serialize(std::ostream& stream, std::pair<uint64_t, uint64_t> value);
template<typename T>
void serialize(std::ostream& stream, const std::vector<T>& value)
{
	serialize(stream, (uint64_t)value.size());
	for (size_t i = 0; i < value.size(); i++)
	{
		serialize(stream, value[i]);
	}
}
template<typename T, size_t Length>
void serialize(std::ostream& stream, const std::array<T, Length>& value)
{
	for (size_t i = 0; i < value.size(); i++)
	{
		serialize(stream, value[i]);
	}
}

void deserialize(std::istream& stream, uint8_t& value);
void deserialize(std::istream& stream, uint32_t& value);
void deserialize(std::istream& stream, uint64_t& value);
void deserialize(std::istream& stream, __uint128_t& value);
void deserialize(std::istream& stream, bool& value);
void deserialize(std::istream& stream, std::pair<uint64_t, uint64_t>& value);
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
template<typename T, size_t Length>
void deserialize(std::istream& stream, std::array<T, Length>& value)
{
	for (size_t i = 0; i < value.size(); i++)
	{
		deserialize(stream, value[i]);
	}
}

#endif
