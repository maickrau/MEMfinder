#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "PartSortBWT.h"

int main(int, char** argv)
{
	std::string seq;
	{
		std::ifstream file { argv[1] };
		getline(file, seq);
	}
	for (size_t i = 0; i < seq.size(); i++)
	{
		switch(seq[i])
		{
			case 'A':
				seq[i] = 2;
				break;
			case 'C':
				seq[i] = 3;
				break;
			case 'G':
				seq[i] = 4;
				break;
			case 'T':
				seq[i] = 5;
				break;
			default:
				seq[i] = 1;
				break;
		}
	}
	seq.push_back(0);
	std::cerr << "size " << seq.size() << std::endl;
	partSortBWT(seq, seq);
	for (size_t i = 0; i < seq.size(); i++)
	{
		std::cout << "$_ACGT"[(size_t)seq[i]];
	}
	std::cout << std::endl;
}
