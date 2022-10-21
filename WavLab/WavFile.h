#pragma once

#include <fstream>
#include <string>
#include <vector>

class WavFile
{
public:
	WavFile(std::string path);
	
	~WavFile();

	void validate();
	void printDetails();
	void loadSamplesFromFile();

	std::vector<double> getSamples();

private:
	std::ifstream infile;
	size_t lengthInBytes;
	
	//File
	std::string fileChunkId;
	int fileChunkSize;
	std::string format;

	//Format
	std::string formatChunkId;
	int formatChunkSize;
	short audioFormat;
	short numberOfChannels;
	int sampleRate;
	int byteRate;
	short blockAlign;
	short bitsPerSample;

	//TODO: BWAV Chunks

	//Data
	std::string dataChunkId;
	int dataChunkSize;

	//Samples
	int numberOfSamples;
	std::vector<double> samples;

	//IO functions
	std::string readString(std::ifstream& file, int length);
	short readInt16(std::ifstream& file);
	int readInt24(std::ifstream& file);
	int readInt32(std::ifstream& file);
	void writeString(std::ofstream& file, std::string value);
	void writeInt16(std::ofstream& file, short value);
	void writeInt24(std::ofstream& file, int value);
	void writeInt32(std::ofstream& file, int value);

	//Normalisation functions
	double sixteenBitToDouble(short input);
	short doubleToSixteenBit(double input);
	double twentyfourBitToDouble(int input);
	int doubleToTwentyFourBit(double input);
	double thirtytwoBitToDouble(int input);
	int doubleToThirtyTwoBit(double input);
};

