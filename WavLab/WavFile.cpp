#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "WavFile.h"

WavFile::WavFile(std::string path)
{
	std::ifstream file;
	std::vector<char> buffer;

	file.open(path, std::ofstream::binary);

	if (file.good() != 1)
	{
		std::cout << "File " << path << " does not exist" << std::endl;
		exit(1);
	}

	file.seekg(0, std::ios::end);
	lengthInBytes = file.tellg();
	file.seekg(0, std::ios::beg);

	//Read first 4 bytes and check it is RIFF
	buffer.resize(4);
	file.read(&buffer[0], 4);
	checkHeader(buffer, "RIFF");

	//Skip ahead to position of WAVE header, read and check it
	file.seekg(8);
	file.read(&buffer[0], 4);
	checkHeader(buffer, "WAVE");

	//Read format header and check
	file.read(&buffer[0], 4);
	checkHeader(buffer, "fmt ");
}



WavFile::~WavFile()
{
}

void checkHeader(std::vector<char> buffer, std::string expectedHeader)
{
	std::string header(buffer.begin(), buffer.end());

	if (header != expectedHeader)
	{
		std::cout << "File is not a valid Wav - does not contain" << expectedHeader << " header" << std::endl;
		exit(1);
	}
}
