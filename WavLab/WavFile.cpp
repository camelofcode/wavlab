#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "BinaryHelpers.h"
#include "ConversionHelpers.h"
#include "WavFile.h"

WavFile::WavFile(const std::string& path, bool loadSamples)
{
	std::ifstream infile;
	infile.open(path, std::ofstream::binary);

	if (infile.good() != 1)
	{
		std::cerr << "File " << path << " does not exist" << std::endl;
		exit(EXIT_FAILURE);
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

	//Correct block align if it is incorrect.
	switch (bitsPerSample)
	{
		case 8: blockAlign = 1;
			break;
		case 16: blockAlign = 2;
			break;
		case 24: blockAlign = 3;
			break;
		case 32: blockAlign = 4;
			break;
		default: blockAlign = 0;
	}

	auto nextChunk = BinaryHelpers::readString(infile, 4);

	if(nextChunk == "bext")
	{
		isBroadcastWav = true;
		readBextFields(infile, false);

		nextChunk = BinaryHelpers::readString(infile, 4);
	}

	if (nextChunk == "JUNK" || nextChunk == "junk") //I've seen both........
	{
		auto junkSize = BinaryHelpers::readInt32(infile);
		if (junkSize % 2 != 0)
		{
			junkSize += 1;
		}
		infile.ignore(junkSize);

		nextChunk = BinaryHelpers::readString(infile, 4); //Should be "data" if we have read the bext chunk and junk chunks properly.
	}

	if(nextChunk == "data")
	{
		dataChunkId = nextChunk;
		dataChunkSize = BinaryHelpers::readInt32(infile);
		dataChunkStart = infile.tellg();
	}

	if ((dataChunkStart + dataChunkSize) != static_cast<int64_t>(lengthInBytes))
	{
		//There is a footer...
		infile.seekg(dataChunkStart + dataChunkSize + 1);
		nextChunk = BinaryHelpers::readString(infile, 4);
		if (nextChunk == "bext")
		{
			//Unsurprisingly some geniuses think it makes sense to put BWAV ***HEADER*** info as a footer. 
			//Also, unsurprisingly those fine folks at AVID seem to think this is a great idea. So I'm supporting it I guess.
			isBroadcastWav = true;
			readBextFields(infile, true);
		}
		else
		{
			//It's a non-standard footer. Let's just ignore it. This branch exists to note I'm doing it on purpose, I'm sure it will be optimised out.
		}
	}

	validate();

	if (loadSamples)
	{
		infile.seekg(dataChunkStart);

		numberOfSamples = dataChunkSize / blockAlign;
		samples.resize(numberOfSamples);

		if (bitsPerSample == 8)
		{
			for (size_t i = 0; i < numberOfSamples; i++)
			{
				samples[i] = ConversionHelpers::eightBitToDouble(BinaryHelpers::readInt8(infile));
			}
		}
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
				samples[i] = ConversionHelpers::twentyFourBitToDouble(BinaryHelpers::readInt24(infile));
			}
		}
		else if (bitsPerSample == 32)
		{
			for (size_t i = 0; i < numberOfSamples; i++)
			{
				samples[i] = ConversionHelpers::thirtyTwoBitToDouble(BinaryHelpers::readInt32(infile));
			}
		}
	}
}

WavFile::WavFile(unsigned short numberOfChannels, uint32_t sampleRate, unsigned short bitsPerSample) : 
	fileChunkId("RIFF"),
	format("WAVE"),
	formatChunkId("fmt "),
	audioFormat(1),
	numberOfChannels(numberOfChannels),
	sampleRate(sampleRate),
	bitsPerSample(bitsPerSample),
	dataChunkId("data")
{
	setDefaultBextFields();

	switch (bitsPerSample) 
	{
		case 8: blockAlign = 1;
			break;
		case 16: blockAlign = 2;
			break;
		case 24: blockAlign = 3;
			break;
		case 32: blockAlign = 4;
			break;
		default: blockAlign = 0;
	}
}

//Public functions
void WavFile::validate()
{
	if (fileChunkId != "RIFF")
	{
		std::cerr << "File chunk is not valid for Wav file. Expected: RIFF, Actual: " << fileChunkId << std::endl;
		exit(EXIT_FAILURE);
	}

	if (fileChunkSize != lengthInBytes - 8)
	{
		std::cerr << "File chunk size is not valid. Expected:" << lengthInBytes - 8 << ", Actual: " << fileChunkSize << std::endl;
		exit(EXIT_FAILURE);
	}

	if (format != "WAVE")
	{
		std::cerr << "Format is not valid for Wav file. Expected: WAVE, Actual: " << format << std::endl;
		exit(EXIT_FAILURE);
	}

	if (formatChunkId != "fmt ")
	{
		std::cerr << "Format chunk is not valid for Wav file. Expected: fmt , Actual: " << formatChunkId << std::endl;
		exit(EXIT_FAILURE);
	}

	if (audioFormat != 1)
	{
		std::cerr << "File is not PCM format and is not currently supported" << std::endl;
		exit(EXIT_FAILURE);
	}

	if (numberOfChannels != 1 && numberOfChannels != 2)
	{
		std::cerr << "File is not mono or stereo and is not currently supported" << std::endl;
		exit(EXIT_FAILURE);
	}

	if (bitsPerSample != 8 && bitsPerSample != 16 && bitsPerSample != 24 && bitsPerSample != 32)
	{
		std::cerr << "File is not 8, 16, 24 or 32 bit and is not supported" << std::endl;
		exit(EXIT_FAILURE);
	}
}

void WavFile::printDetailsDebug()
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

	if (isBroadcastWav)
	{
		std::cout << "bextChunkId: " << bextChunkId << std::endl;
		std::cout << "bextChunkSize: " << bextChunkSize << std::endl;
		std::cout << "description: " << description << std::endl;
		std::cout << "originator: " << originator << std::endl;
		std::cout << "originatorReference: " << originatorReference << std::endl;
		std::cout << "originationDate: " << originationDate << std::endl;
		std::cout << "originationTime: " << originationTime << std::endl;
		std::cout << "timeReferenceLow: " << timeReferenceLow << std::endl;
		std::cout << "timeReferenceHigh: " << timeReferenceHigh << std::endl;
		std::cout << "version: " << version << std::endl;
	}
}

void WavFile::printDetails()
{
	std::cout << "Number Of Channels: " << numberOfChannels << std::endl;
	std::cout << "Sample Rate: " << sampleRate << std::endl;
	std::cout << "Bit Depth: " << bitsPerSample << std::endl;

	if (isBroadcastWav)
	{
		std::cout << "Description: " << description << std::endl;
		std::cout << "Originator: " << originator << std::endl;
		std::cout << "Originator Reference: " << originatorReference << std::endl;
		std::cout << "Origination Date/Time: " << originationDate << " " << originationTime << std::endl;

		//TODO: These need to be combined into a presumably 64 bit number to get a useful value
		//std::cout << "timeReferenceLow: " << timeReferenceLow << std::endl;
		//std::cout << "timeReferenceHigh: " << timeReferenceHigh << std::endl;

		std::cout << "Broadcast Wave Version: " << version << std::endl;
	}
}

void WavFile::save(const std::string& path)
{
	//Set chunk sizes. I *think* this is correct.
	formatChunkSize = 16; //Could calculate this doing sizeof, but I already did that manually so why bother, it's always the same!
	dataChunkSize = static_cast<uint32_t>(samples.size() * blockAlign);
	bextChunkSize = isBroadcastWav ? 602 : 0; //Could calculate this doing sizeof, but I already did that manually so why bother, it's always the same!
	fileChunkSize = formatChunkSize
		+ 4 //format chunk id - should be counted?
		+ 4 //data chunk id - should be counted?
		+ 12 //Because my sizes always seem to be 12 short, I'm not quite sure why at the moment
		+ dataChunkSize
		+ bextChunkSize;
	//I guess if I work out *why* I need a junk chunk then that will make up the difference for the 2048 file boundary or whatever it is. 
	//I'm gonna burn a CD of wavs generated by this class sometime and see what happens.
	lengthInBytes = fileChunkSize
		+ 8; //Format chunk not included in file chunk 

	validate();

	std::ofstream outfile;
	outfile.open(path, std::ofstream::binary);

	if (outfile.good() != 1)
	{
		std::cerr << "File " << path << " could not be opened for writing, check destination directory" << std::endl;
		exit(EXIT_FAILURE);
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

	if (isBroadcastWav)
	{
		BinaryHelpers::writeString(outfile, bextChunkId);
		BinaryHelpers::writeInt32(outfile, bextChunkSize);

		BinaryHelpers::writeString(outfile, description, 256);
		BinaryHelpers::writeString(outfile, originator, 32);
		BinaryHelpers::writeString(outfile, originatorReference, 32);
		BinaryHelpers::writeString(outfile, originationDate, 10);
		BinaryHelpers::writeString(outfile, originationTime, 8);

		BinaryHelpers::writeInt32(outfile, timeReferenceLow);
		BinaryHelpers::writeInt32(outfile, timeReferenceHigh);
		BinaryHelpers::writeInt16(outfile, version);

		//Write reserved bytes. There should be 254 because we are not yet supporting v1 and v2 fields.
		for (size_t i = 0; i < 254; i++)
		{
			outfile.write("\0", 1);
		}
	}

	//TODO: Write Junk chunk?! Not sure why we need it
	//"To align RIFF chunks to certain boundaries (i.e. 2048bytes for CD-ROMs) the RIFF specification includes a JUNK chunk. Its contents are to be skipped when reading. When writing RIFFs, JUNK chunks should not have odd number as Size."

	BinaryHelpers::writeString(outfile, dataChunkId);
	BinaryHelpers::writeInt32(outfile, dataChunkSize);

	if (bitsPerSample == 8)
	{
		for (size_t i = 0; i < numberOfSamples; i++)
		{
			BinaryHelpers::writeInt8(outfile, ConversionHelpers::doubleToEightBit(samples[i]));
		}
	}
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
			BinaryHelpers::writeInt24(outfile, ConversionHelpers::doubleToTwentyFourBit(samples[i]));
		}
	}
	else if (bitsPerSample == 32)
	{
		for (size_t i = 0; i < numberOfSamples; i++)
		{
			BinaryHelpers::writeInt32(outfile, ConversionHelpers::doubleToThirtyTwoBit(samples[i]));
		}
	}

	outfile.close();
}

//Getters
std::vector<double> WavFile::getSamples()
{
	return samples;
}

uint16_t WavFile::getNumberOfChannels() const
{
	return numberOfChannels;
}

uint32_t WavFile::getSampleRate() const
{
	return sampleRate;
}

uint16_t WavFile::getBitsPerSample() const
{
	return bitsPerSample;
}

//Setters
void WavFile::setNumberOfChannels(uint16_t number)
{
	if (number > 2)
	{
		std::cerr << "Attempted to set number of channels to a value greater than 2. Aborting." << std::endl;
		exit(EXIT_FAILURE);
	}

	numberOfChannels = number;
}

void WavFile::setSamples(std::vector<double>& newSamples)
{
	samples = newSamples;
	numberOfSamples = samples.size();
}

//Private functions
void WavFile::readBextFields(std::ifstream& file, bool isFooter)
{
	bextChunkId = "bext";
	bextChunkSize = BinaryHelpers::readInt32(file);

	description = BinaryHelpers::readString(file, 256);
	originator = BinaryHelpers::readString(file, 32);
	originatorReference = BinaryHelpers::readString(file, 32);
	originationDate = BinaryHelpers::readString(file, 10);
	originationTime = BinaryHelpers::readString(file, 8);
	timeReferenceLow = BinaryHelpers::readInt32(file);
	timeReferenceHigh = BinaryHelpers::readInt32(file);
	version = BinaryHelpers::readInt16(file);

	if (version == 1)
	{
		std::cout << "BWAV v1 fields are not currently supported" << std::endl;
	}
	else if (version == 2)
	{
		std::cout << "BWAV v1 and v2 fields are not currently supported" << std::endl;
	}

	if (isFooter)
	{
		//Might be a JUNK Chunk, but we are pretty much done caring at this point.
	}
	else 
	{
		//Need to get to end of this chunk. We are ignoring v1 and v2 fields for now so need to skip the 254 reserved bytes
		std::streamoff position = file.tellg();
		file.seekg(position + 254);
	}
}

void WavFile::setDefaultBextFields()
{
	isBroadcastWav = false;
	bextChunkId = "bext";
	description = "";
	originator = "";
	originatorReference = "";
	originationDate = "";
	originationTime = "";
	timeReferenceLow = 0;
	timeReferenceHigh = 0;
	version = 0;
}