#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "WavFile.h"

WavFile::WavFile(std::string path)
{
	std::vector<char> buffer;

	infile.open(path, std::ofstream::binary);

	if (infile.good() != 1)
	{
		std::cout << "File " << path << " does not exist" << std::endl;
		exit(1);
	}

	infile.seekg(0, std::ios::end);
	lengthInBytes = infile.tellg();
	infile.seekg(0, std::ios::beg);

	fileChunkId = readString(infile, 4);
	fileChunkSize = readInt32(infile);
	format = readString(infile, 4);

	formatChunkId = readString(infile, 4);
	formatChunkSize = readInt32(infile);
	audioFormat = readInt16(infile);
	numberOfChannels = readInt16(infile);
	sampleRate = readInt32(infile);
	byteRate = readInt32(infile);
	blockAlign = readInt16(infile);
	bitsPerSample = readInt16(infile);

	//TODO: BWAV fields.

	dataChunkId = readString(infile, 4);
	dataChunkSize = readInt32(infile);

	validate();
}

//Public functions
void WavFile::validate()
{
	if (fileChunkId != "RIFF")
	{
		std::cout << "File chunk is not valid for Wav file. Expected: RIFF, Actual: " << fileChunkId << std::endl;
		exit(1);
	}

	if (fileChunkSize != lengthInBytes - 8)
	{
		std::cout << "File chunk size is not valid. Expected:" << lengthInBytes - 8 << ", Actual: " << fileChunkSize << std::endl;
		exit(1);
	}

	if (format != "WAVE")
	{
		std::cout << "Format is not valid for Wav file. Expected: WAVE, Actual: " << format << std::endl;
		exit(1);
	}

	if (formatChunkId != "fmt ")
	{
		std::cout << "Format chunk is not valid for Wav file. Expected: fmt , Actual: " << formatChunkId << std::endl;
		exit(1);
	}

	if (audioFormat != 1)
	{
		std::cout << "File is not PCM format and is not currently supported" << std::endl;
		exit(1);
	}

	if (numberOfChannels != 1 && numberOfChannels != 2)
	{
		std::cout << "File is not mono or stereo and is not currently supported" << std::endl;
		exit(1);
	}

	if (bitsPerSample != 16 && bitsPerSample != 24 && bitsPerSample == 32)
	{
		std::cout << "File is not 16, 24 or 32 bit and is not supported" << std::endl;
		exit(1);
	}
}

void WavFile::loadSamplesFromFile()
{
	numberOfSamples = dataChunkSize / blockAlign * numberOfChannels;
	samples.resize(numberOfSamples);

	if (bitsPerSample == 16)
	{
		for (size_t i = 0; i < numberOfSamples; i++)
		{
			samples[i] = sixteenBitToDouble(readInt16(infile));
		}
	}
	else if (bitsPerSample == 24)
	{
		for (size_t i = 0; i < numberOfSamples; i++)
		{
			samples[i] = twentyfourBitToDouble(readInt24(infile));
		}
	}
	else if (bitsPerSample == 32)
	{
		for (size_t i = 0; i < numberOfSamples; i++)
		{
			samples[i] = thirtytwoBitToDouble(readInt32(infile));
		}
	}
}

std::vector<double> WavFile::getSamples()
{
	return samples;
}

void WavFile::printDetails()
{
	std::cout << "lengthInBytes: " << lengthInBytes << std::endl;

	std::cout << "fileChunkId: " << fileChunkId << std::endl;
	std::cout << "fileChunkSize: " << fileChunkSize << std::endl;
	std::cout << "format: " << format << std::endl;

	std::cout << "formatChunkId: " << formatChunkId << std::endl;
	std::cout << "formatChunkSize: " << formatChunkSize << std::endl;
	std::cout << "audioFormat: " << audioFormat << std::endl;
	std::cout << "numberOfChannels: " << numberOfChannels << std::endl;
	std::cout << "sampleRate: " << sampleRate << std::endl;
	std::cout << "byteRate: " << byteRate << std::endl;
	std::cout << "blockAlign: " << blockAlign << std::endl;
	std::cout << "bitsPerSample: " << bitsPerSample << std::endl;

	std::cout << "dataChunkId: " << dataChunkId << std::endl;
	std::cout << "dataChunkSize: " << dataChunkSize << std::endl;
}

//Destructor
WavFile::~WavFile()
{
	//TODO: Implement destructor
}


//Private functions
std::string WavFile::readString(std::ifstream& file, int length)
{
	std::vector<char> buffer;
	buffer.resize(length);
	file.read(&buffer[0], length);
	std::string header(buffer.begin(), buffer.end());
	return header;
}

short WavFile::readInt16(std::ifstream& file)
{
	//TODO: Don't trust these, seems like we are not getting negative numbers.
	union uni
	{
		short s;
		struct bytes
		{
			char b0;
			char b1;
		} b;
	} conversionUnion;

	file.read((char*)&conversionUnion.b.b0, 1);
	file.read((char*)&conversionUnion.b.b1, 1);

	return conversionUnion.s;
}

int WavFile::readInt24(std::ifstream& file)
{
	//TODO: Don't trust these, seems like we are not getting negative numbers.
	union uni
	{
		int i;
		struct bytes
		{
			char b0;
			char b1;
			char b2;
			char b3;
		} b;
	} conversionUnion;

	file.read((char*)&conversionUnion.b.b0, 1);
	file.read((char*)&conversionUnion.b.b1, 1);
	file.read((char*)&conversionUnion.b.b2, 1);
	conversionUnion.b.b3 = 0;

	return conversionUnion.i;
}

int WavFile::readInt32(std::ifstream& file)
{
	//TODO: Don't trust these, seems like we are not getting negative numbers.
	union uni
	{
		int i;
		struct bytes
		{
			char b0;
			char b1;
			char b2;
			char b3;
		} b;
	} conversionUnion;

	file.read((char*)&conversionUnion.b.b0, 1);
	file.read((char*)&conversionUnion.b.b1, 1);
	file.read((char*)&conversionUnion.b.b2, 1);
	file.read((char*)&conversionUnion.b.b3, 1);

	return conversionUnion.i;
}

void WavFile::writeString(std::ofstream& file, std::string value)
{
	file.write(value.c_str(), value.size());
}

void WavFile::writeInt16(std::ofstream& file, short value)
{
	file.write((char*)&value, 2);
}

void WavFile::writeInt24(std::ofstream& file, int value)
{
	file.write((char*)&value, 3);
}

void WavFile::writeInt32(std::ofstream& file, int value)
{
	file.write((char*)&value, 4);
}

double WavFile::sixteenBitToDouble(short input)
{
	double min = 32767.0;
	double max = 32768.0;
	double sample = 0.0;

	if (input > 0)
	{
		sample = static_cast<double>(input) / max;
	}
	else
	{
		sample = static_cast<double>(input) / min;
	}

	return sample - 1;
}

short WavFile::doubleToSixteenBit(double input)
{
	double max = 32767.0;
	double min = 32768.0;
	double sample = 0.0;

	if (input > 0)
	{
		sample = input * max;
	}
	else
	{
		sample = input * min;
	}
	return static_cast<short>(sample);
}

double WavFile::twentyfourBitToDouble(int input)
{
	double min = 8388608.0;
	double max = 8388607.0;
	double sample = 0.0;

	if (input > 0)
	{
		sample = static_cast<double>(input) / max;
	}
	else
	{
		sample = static_cast<double>(input) / min;
	}

	return sample;
}

int WavFile::doubleToTwentyFourBit(double input)
{
	double max = 8388607.0;
	double min = 8388608.0;
	double sample = 0.0;

	input = input + 1;

	if (input > 0)
	{
		sample = input * max;
	}
	else
	{
		sample = input * min;
	}
	return static_cast<int>(sample);
}


double WavFile::thirtytwoBitToDouble(int input)
{
	double min = 2147483648.0;
	double max = 2147483647.0;
	double sample = 0.0;

	if (input > 0)
	{
		sample = static_cast<double>(input) / max;
	}
	else
	{
		sample = static_cast<double>(input) / min;
	}

	return sample;
}

int WavFile::doubleToThirtyTwoBit(double input)
{
	double max = 2147483647.0;
	double min = 2147483648.0;
	double sample = 0.0;

	if (input > 0)
	{
		sample = input * max;
	}
	else
	{
		sample = input * min;
	}
	return static_cast<int>(sample);
}