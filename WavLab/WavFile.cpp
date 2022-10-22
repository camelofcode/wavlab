#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "BinaryHelpers.h"
#include "ConversionHelpers.h"
#include "WavFile.h"

WavFile::WavFile(std::string path)
{
	infile.open(path, std::ofstream::binary);

	if (infile.good() != 1)
	{
		std::cout << "File " << path << " does not exist" << std::endl;
		exit(1);
	}

	infile.seekg(0, std::ios::end);
	lengthInBytes = infile.tellg();
	infile.seekg(0, std::ios::beg);

	fileChunkId = BinaryHelpers::readString(infile, 4);
	fileChunkSize = BinaryHelpers::readInt32(infile);
	format = BinaryHelpers::readString(infile, 4);

	formatChunkId = BinaryHelpers::readString(infile, 4);
	formatChunkSize = BinaryHelpers::readInt32(infile);
	audioFormat = BinaryHelpers::readInt16(infile);
	numberOfChannels = BinaryHelpers::readInt16(infile);
	sampleRate = BinaryHelpers::readInt32(infile);
	byteRate = BinaryHelpers::readInt32(infile);
	blockAlign = BinaryHelpers::readInt16(infile);
	bitsPerSample = BinaryHelpers::readInt16(infile);

	auto nextChunk = BinaryHelpers::readString(infile, 4);
	if(nextChunk == "bext")
	{
		//TODO: BWAV fields.
		isBroadcastWav = true;

	}
	else if(nextChunk == "data")
	{
		isBroadcastWav = false;
		dataChunkId = nextChunk;
		dataChunkSize = BinaryHelpers::readInt32(infile);
	}

	//TODO: Random text at end of file?

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

	if (bitsPerSample != 16 && bitsPerSample != 24 && bitsPerSample != 32)
	{
		std::cout << "File is not 16, 24 or 32 bit and is not supported" << std::endl;
		exit(1);
	}

	if (isBroadcastWav)
	{
		std::cout << "File is a broadcast wav, which is not currently supported (but will be like really soon)" << std::endl;
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
			samples[i] = ConversionHelpers::sixteenBitToDouble(BinaryHelpers::readInt16(infile));
		}
	}
	else if (bitsPerSample == 24)
	{
		for (size_t i = 0; i < numberOfSamples; i++)
		{
			samples[i] = ConversionHelpers::twentyfourBitToDouble(BinaryHelpers::readInt24(infile));
		}
	}
	else if (bitsPerSample == 32)
	{
		for (size_t i = 0; i < numberOfSamples; i++)
		{
			samples[i] = ConversionHelpers::thirtytwoBitToDouble(BinaryHelpers::readInt32(infile));
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

void WavFile::save(std::string path)
{
	outfile.open(path, std::ofstream::binary);

	if (outfile.good() != 1)
	{
		std::cout << "File " << path << " could not be opened for writing, check destination directory" << std::endl;
		exit(1);
	}

	BinaryHelpers::writeString(outfile, fileChunkId);
	BinaryHelpers::writeInt32(outfile, fileChunkSize);
	BinaryHelpers::writeString(outfile, format);

	BinaryHelpers::writeString(outfile, formatChunkId);
	BinaryHelpers::writeInt32(outfile, formatChunkSize);
	BinaryHelpers::writeInt16(outfile, audioFormat);
	BinaryHelpers::writeInt16(outfile, numberOfChannels);
	BinaryHelpers::writeInt32(outfile, sampleRate);
	BinaryHelpers::writeInt32(outfile, byteRate);
	BinaryHelpers::writeInt16(outfile, blockAlign);
	BinaryHelpers::writeInt16(outfile, bitsPerSample);

	//TODO: BWav stuff

	BinaryHelpers::writeString(outfile, dataChunkId);
	BinaryHelpers::writeInt32(outfile, dataChunkSize);

	if (bitsPerSample == 16)
	{
		for (size_t i = 0; i < numberOfSamples; i++)
		{
			BinaryHelpers::writeInt16(outfile, ConversionHelpers::doubleToSixteenBit(samples[i]));
		}
	}
	else if (bitsPerSample == 24)
	{
		for (size_t i = 0; i < numberOfSamples; i++)
		{
			BinaryHelpers::writeInt24(outfile, ConversionHelpers::doubleToThirtyTwoBit(samples[i]));
		}
	}
	else if (bitsPerSample == 32)
	{
		for (size_t i = 0; i < numberOfSamples; i++)
		{
			BinaryHelpers::writeInt32(outfile, ConversionHelpers::doubleToThirtyTwoBit(samples[i]));
		}
	}

	//TODO: Random text at end of file?
	outfile.close();
}

//Destructor
WavFile::~WavFile()
{
	//TODO: Implement destructor
}