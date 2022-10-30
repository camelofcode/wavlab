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
        std::cout << "Incorrect number of arguments for command 'sine-wave'. To see available commands run 'wavlab help'" << std::endl;
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

/* square-wave command */

void Commands::squareWaveHelp()
{
    std::cout << "square-wave <output> <frequency> <sampleRate> <bitDepth> <length>" << std::endl;
    std::cout << "\tdescription: writes a square wave to the specified output file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\toutput: path to output wave file" << std::endl;
    std::cout << "\t\tfrequency: frequency in Hz of the square wave" << std::endl;
    std::cout << "\t\tsampleRate: sample rate of the output wave file" << std::endl;
    std::cout << "\t\tbitDepth: bit depth of the output wave file" << std::endl;
    std::cout << "\t\tlength: length of the output file in milliseconds" << std::endl;
}

int Commands::squareWaveCommand(std::string command, int argc, char* argv[])
{
    if (argc != 7)
    {
        std::cout << "Incorrect number of arguments for command 'square-wave'. To see available commands run 'wavlab help'" << std::endl;
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
    Generators::squareWave(wav, lengthMs, frequency);
    wav.save(output);

    return 0;
}

void Generators::squareWave(WavFile& wave, int lengthMs, double frequency)
{
    std::vector<double> samples;
    uint32_t numberOfSamples = wave.getSampleRate() * (lengthMs / 1000);
    auto periodSamples = static_cast<int>(wave.getSampleRate() / frequency);
    int periodSamplesCounter = 0;
    bool positive = true;
    for (size_t i = 0; i < numberOfSamples; i++)
    {
        for (size_t i = 0; i < wave.getNumberOfChannels(); i++)
        {
            samples.push_back(positive ? 1.0 : -1.0);
        }
        periodSamplesCounter++;
        if (periodSamplesCounter > periodSamples / 2)
        {
            positive = !positive;
            periodSamplesCounter = 0;
        }
    }
    wave.setSamples(samples);
}

/* triangle-wave command */

void Commands::triangleWaveHelp()
{
    std::cout << "triangle-wave <output> <frequency> <sampleRate> <bitDepth> <length>" << std::endl;
    std::cout << "\tdescription: writes a triangle wave to the specified output file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\toutput: path to output wave file" << std::endl;
    std::cout << "\t\tfrequency: frequency in Hz of the triangle wave" << std::endl;
    std::cout << "\t\tsampleRate: sample rate of the output wave file" << std::endl;
    std::cout << "\t\tbitDepth: bit depth of the output wave file" << std::endl;
    std::cout << "\t\tlength: length of the output file in milliseconds" << std::endl;
}

int Commands::triangleWaveCommand(std::string command, int argc, char* argv[])
{
    if (argc != 7)
    {
        std::cout << "Incorrect number of arguments for command 'triangle-wave'. To see available commands run 'wavlab help'" << std::endl;
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
    Generators::triangleWave(wav, lengthMs, frequency);
    wav.save(output);

    return 0;
}

void Generators::triangleWave(WavFile& wave, int lengthMs, double frequency)
{
    std::vector<double> samples;
    uint32_t numberOfSamples = wave.getSampleRate() * (lengthMs / 1000);
    int periodSamples = static_cast<int>(wave.getSampleRate() / frequency);
    int rampSamples = periodSamples / 4;
    double increment = 1.0 / rampSamples;
    bool positive = true;
    double sample = 0.0;
    for (size_t i = 0; i < numberOfSamples; i++)
    {
        for (size_t i = 0; i < wave.getNumberOfChannels(); i++)
        {
            samples.push_back(sample);
        }

        sample = positive ? sample += increment : sample -= increment;

        if (fabs(sample) >= 1.0) 
        {
            positive = !positive;
        }
    }
    wave.setSamples(samples);
}

/* sawtooth-wave command */

void Commands::sawtoothWaveHelp()
{
    std::cout << "sawtooth-wave <output> <frequency> <sampleRate> <bitDepth> <length>" << std::endl;
    std::cout << "\tdescription: writes a sawtooth wave to the specified output file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\toutput: path to output wave file" << std::endl;
    std::cout << "\t\tfrequency: frequency in Hz of the sawtooth wave" << std::endl;
    std::cout << "\t\tsampleRate: sample rate of the output wave file" << std::endl;
    std::cout << "\t\tbitDepth: bit depth of the output wave file" << std::endl;
    std::cout << "\t\tlength: length of the output file in milliseconds" << std::endl;
}

int Commands::sawtoothWaveCommand(std::string command, int argc, char* argv[])
{
    if (argc != 7)
    {
        std::cout << "Incorrect number of arguments for command 'sawtooth-wave'. To see available commands run 'wavlab help'" << std::endl;
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
    Generators::sawtoothWave(wav, lengthMs, frequency);
    wav.save(output);

    return 0;
}

void Generators::sawtoothWave(WavFile& wave, int lengthMs, double frequency)
{
    std::vector<double> samples;
    uint32_t numberOfSamples = wave.getSampleRate() * (lengthMs / 1000);
    int periodSamples = static_cast<int>(wave.getSampleRate() / frequency);
    int rampSamples = periodSamples / 2;
    double increment = 1.0 / rampSamples;
    double sample = 0.0;
    for (size_t i = 0; i < numberOfSamples; i++)
    {
        for (size_t i = 0; i < wave.getNumberOfChannels(); i++)
        {
            samples.push_back(sample);
        }

        sample += increment;

        if (sample > 1.0)
        {
            sample = -1.0;
        }
    }
    wave.setSamples(samples);
}