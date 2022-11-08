#pragma once

#include <fstream>
#include <string>
#include <vector>

class WavFile
{
public:
	/// <summary>
	/// Constructor for loading an existing WAV file.
	/// </summary>
	/// <param name="path">The path of the file to open</param>
	/// <param name="loadSamples">Whether sample data should be loaded. This can take a while for large files, so can be set to false if only header information is required</param>
	WavFile(const std::string& path, bool loadSamples);

	/// <summary>
	/// Constructor for creating a new WAV file.
	/// </summary>
	/// <param name="numberOfChannels">Number of channels, ie. Mono = 1, Stereo = 2</param>
	/// <param name="sampleRate">The sample rate in Hz</param>
	/// <param name="bitsPerSample">The number of bits per sample: 8/16/24/32</param>
	WavFile(unsigned short numberOfChannels, uint32_t sampleRate, unsigned short bitsPerSample);
	
	///Destructor
	~WavFile() = default;

	/// <summary>
	/// Checks that the file is a valid WAV, exits if not.
	/// </summary>
	void validate();

	/// <summary>
	/// Prints full details of the WAV file to stdout, including things only useful to developers
	/// </summary>
    void printDetailsDebug();

	/// <summary>
	/// Prints the details of the WAV file to stdout.
	/// </summary>
	void printDetails();

	/// <summary>
	/// Writes the file to disk.
	/// </summary>
	/// <param name="path">The path to write the file to</param>
	void save(const std::string& path);

	/// <summary>
	/// Gets the sample data of the file
	/// </summary>
	/// <returns>Sample data as scaled doubles in the range -1.0 to 1.0</returns>
	std::vector<double> getSamples();

	/// <summary>
	/// Gets the number of audio channels ie. Mono = 1, Stereo = 2
	/// </summary>
	uint16_t getNumberOfChannels() const;

	/// <summary>
	/// Gets the sample rate in Hz
	/// </summary>
	uint32_t getSampleRate() const;

	/// <summary>
	/// Gets the bits per sample: 8/16/24/32
	/// </summary>
	uint16_t getBitsPerSample() const;

	/// <summary>
	/// Sets the number of channels
	/// </summary>
	/// <param name="number">value</param>
	void setNumberOfChannels(uint16_t number);

	/// <summary>
	/// Sets the samples of the wav file
	/// </summary>
	void setSamples(std::vector<double>& newSamples);

private:
	size_t lengthInBytes{};
	
	//File

	std::string fileChunkId;
	uint32_t fileChunkSize{};
	std::string format;

	//Format

	std::string formatChunkId;
	uint32_t formatChunkSize{};
	uint16_t audioFormat;
	uint16_t numberOfChannels;
	uint32_t sampleRate;
	uint32_t byteRate{};
	uint16_t blockAlign;
	uint16_t bitsPerSample;

	//BWAV Chunks - https://tech.ebu.ch/docs/tech/tech3285.pdf

	bool isBroadcastWav = false;
	void readBextFields(std::ifstream& file, bool isFooter);
	void setDefaultBextFields();

	//BWAV v0 fields

	std::string bextChunkId;
	uint32_t bextChunkSize{};
	std::string description; /* 256 bytes - ASCII : �Description of the sound sequence�*/
	std::string originator; /* 32 Bytes - ASCII : �Name of the originator� */
	std::string originatorReference; /* 32 Bytes - ASCII : �Reference of the originator� */
	std::string originationDate;  /* 10 Bytes - ASCII : �yyyy:mm:dd� */
	std::string originationTime; /* 8 Bytes - ASCII : �hh:mm:ss� */
	uint32_t timeReferenceLow{}; /* First sample count since midnight, low word*/
	uint32_t timeReferenceHigh{}; /* First sample count since midnight, high word*/
	uint16_t version{}; /* Version of the BWF; unsigned binary number */

	//Data

	std::string dataChunkId;
	uint32_t dataChunkSize{};
	std::streamoff dataChunkStart{};

	//Samples

	size_t numberOfSamples{};
	std::vector<double> samples;
};

