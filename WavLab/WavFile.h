#pragma once

#include <string>
#include <vector>

class WavFile
{
public:
	WavFile(std::string path);
	
	~WavFile();

private:
	size_t lengthInBytes;
};

void checkHeader(std::vector<char> buffer, std::string expectedHeader);