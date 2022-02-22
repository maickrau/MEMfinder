#include <vector>
#include "ReverseComplementView.h"

std::vector<char> getComplements()
{
	std::vector<char> result;
	result.resize(128, 'N');
	result['a'] = 't';
	result['A'] = 'T';
	result['c'] = 'g';
	result['C'] = 'G';
	result['g'] = 'c';
	result['G'] = 'C';
	result['t'] = 'a';
	result['T'] = 'A';
	return result;
}

std::vector<char> complement = getComplements();

ReverseComplementView::ReverseComplementView(const std::string& seq) :
seq(seq)
{

}

size_t ReverseComplementView::size() const
{
	return seq.size();
}

const char ReverseComplementView::operator[](size_t pos) const
{
	return complement[seq[seq.size()-1-pos]];
}
