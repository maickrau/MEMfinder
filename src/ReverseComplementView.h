#ifndef ReverseComplementView_h
#define ReverseComplementView_h

#include <string>

class ReverseComplementView
{
public:
	ReverseComplementView(const std::string& seq);
	size_t size() const;
	const char operator[](size_t pos) const;
private:
	const std::string& seq;
};

#endif
