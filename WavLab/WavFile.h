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

	void save(std::string path);

	std::vector<double> getSamples();

private:
	std::ifstream infile;
	std::ofstream outfile;
	size_t lengthInBytes;
	
	//File
	std::string fileChunkId;
	int32_t fileChunkSize;
	std::string format;

	//Format
	std::string formatChunkId;
	int32_t formatChunkSize;
	short audioFormat;
	short numberOfChannels;
	int32_t sampleRate;
	int32_t byteRate;
	short blockAlign;
	short bitsPerSample;

	//TODO: BWAV Chunks
	bool isBroadcastWav;

	//Data
	std::string dataChunkId;
	int32_t dataChunkSize;

	//Samples
	int numberOfSamples;
	std::vector<double> samples;
};

