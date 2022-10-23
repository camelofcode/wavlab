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
	uint32_t fileChunkSize;
	std::string format;

	//Format
	std::string formatChunkId;
	uint32_t formatChunkSize;
	unsigned short audioFormat;
	unsigned short numberOfChannels;
	uint32_t sampleRate;
	uint32_t byteRate;
	unsigned short blockAlign;
	unsigned short bitsPerSample;

	//BWAV Chunks - https://tech.ebu.ch/docs/tech/tech3285.pdf
	bool isBroadcastWav = false;
	void readBextFields(std::ifstream& file, bool isFooter);
	//v0 fields
	std::string bextChunkId;
	uint32_t bextChunkSize;
	std::string description; /* 256 bytes - ASCII : «Description of the sound sequence»*/ 
	std::string originator; /* 32 Bytes - ASCII : «Name of the originator» */
	std::string originatorReference; /* 32 Bytes - ASCII : «Reference of the originator» */ 
	std::string originationDate;  /* 10 Bytes - ASCII : «yyyy:mm:dd» */ 
	std::string originationTime; /* 8 Bytes - ASCII : «hh:mm:ss» */
	uint32_t timeReferenceLow; /* First sample count since midnight, low word*/ 
	uint32_t timeReferenceHigh; /* First sample count since midnight, high word*/ 
	unsigned short version; /* Version of the BWF; unsigned binary number */

	//Data
	std::string dataChunkId;
	uint32_t dataChunkSize;
	std::streamoff dataChunkStart;

	//Samples
	int numberOfSamples;
	std::vector<double> samples;
};

