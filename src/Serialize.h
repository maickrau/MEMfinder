#ifndef Serialize_h
#define Serialize_h

#include <iostream>
#include <vector>

void serialize(std::ostream& stream, uint32_t value);
void serialize(std::ostream& stream, uint64_t value);
void serialize(std::ostream& stream, size_t value);
void serialize(std::ostream& stream, bool value);
template<typename T>
void serialize(std::ostream& stream, const std::vector<T>& value)
{
	serialize(stream, value.size());
	for (size_t i = 0; i < value.size(); i++)
	{
		serialize(stream, value[i]);
	}
}

void deserialize(std::istream& stream, uint32_t& value);
void deserialize(std::istream& stream, uint64_t& value);
void deserialize(std::istream& stream, size_t& value);
void deserialize(std::istream& stream, bool& value);
template<typename T>
void deserialize(std::istream& stream, std::vector<T>& value)
{
	size_t size = 0;
	deserialize(stream, size);
	value.resize(size);
	for (size_t i = 0; i < value.size(); i++)
	{
		deserialize(stream, value[i]);
	}
}

#endif
