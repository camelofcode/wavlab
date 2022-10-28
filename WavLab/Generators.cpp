#include "Generators.h"
#include "Commands.h"
#include "Constants.h"

/* sine-wave command */

void Commands::sineWaveHelp()
{
	std::cout << "sine-wave <output> <frequency> <sampleRate> <bitDepth> <length>" << std::endl;
	std::cout << "\tdescription: writes a sine wave to the specified output file" << std::endl;
	std::cout << "\tparameters:" << std::endl;
	std::cout << "\t\toutput: path to output wave file" << std::endl;
	std::cout << "\t\tfrequency: frequency in Hz of the sine wave" << std::endl;
	std::cout << "\t\tsampleRate: sample rate of the output wave file" << std::endl;
	std::cout << "\t\tbitDepth: bit depth of the output wave file" << std::endl;
	std::cout << "\t\tlength: length of the output file in milliseconds" << std::endl;
}

int Commands::sineWaveCommand(std::string command, int argc, char* argv[])
{
    if (argc != 7)
    {
        std::cout << "Incorrect number of arguments for command 'saturate'. To see available commands run 'wavlab help'" << std::endl;
        return 1;
    }
    std::string output = static_cast<std::string>(argv[2]);

    double frequency = 44100;
    if (!tryParse(argv[3], frequency))
    {
        std::cout << "Parameter 'frequency' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return 1;
    }

    uint32_t sampleRate = 44100;
    if (!tryParse(argv[4], sampleRate))
    {
        std::cout << "Parameter 'sampleRate' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
        return 1;
    }

    unsigned short bitDepth = 16;
    if (!tryParse(argv[5], bitDepth))
    {
        std::cout << "Parameter 'bitDepth' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
        return 1;
    }

    int lengthMs = 1000;
    if (!tryParse(argv[6], lengthMs))
    {
        std::cout << "Parameter 'lengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[6] << std::endl;
        return 1;
    }

    WavFile wav(1, sampleRate, bitDepth);
    Generators::sineWave(wav, lengthMs, frequency);
    wav.save(output);

    return 0;
}

void Generators::sineWave(WavFile& wave, int lengthMs, double frequency)
{
	std::vector<double> samples;
	uint32_t numberOfSamples = wave.getSampleRate() * (lengthMs / 1000);
	for (size_t i = 0; i < numberOfSamples; i++)
	{
		double value = sin(Constants::TAU * i * (frequency / static_cast<double>(wave.getSampleRate())));
		for (size_t i = 0; i < wave.getNumberOfChannels(); i++)
		{
			samples.push_back(value);
		}
	}
	wave.setSamples(samples);
}