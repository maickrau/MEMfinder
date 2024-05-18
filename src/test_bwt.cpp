#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "libsais64.h"

int main(int argc, char** argv)
{
	std::string seq;
	{
		std::ifstream file { argv[1] };
		getline(file, seq);
	}
	std::cerr << "size " << seq.size() << std::endl;
	std::vector<int64_t> tmp;
	tmp.resize(seq.size(), 0);
	size_t result = libsais64_bwt((uint8_t*)seq.data(), (uint8_t*)seq.data(), (int64_t*)tmp.data(), seq.size(), 0, nullptr);
	std::cerr << "result " << result << std::endl;
	std::cout << seq.substr(0, result);
	std::cout << "$";
	std::cout << seq.substr(result);
	std::cout << std::endl;
}
