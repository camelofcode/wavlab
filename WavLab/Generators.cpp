#include "Generators.h"
#include "Commands.h"
#include "Constants.h"
#include "Tools.h"
#include <cmath>

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

int Commands::sineWaveCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 7)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string output = static_cast<std::string>(argv[2]);

    double frequency = 44100;
    if (!tryParse(argv[3], frequency))
    {
        std::cout << "Parameter 'frequency' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return EXIT_FAILURE;
    }

    uint32_t sampleRate = 44100;
    if (!tryParse(argv[4], sampleRate))
    {
        std::cout << "Parameter 'sampleRate' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
        return EXIT_FAILURE;
    }

    unsigned short bitDepth = 16;
    if (!tryParse(argv[5], bitDepth))
    {
        std::cout << "Parameter 'bitDepth' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
        return EXIT_FAILURE;
    }

    int lengthMs = 1000;
    if (!tryParse(argv[6], lengthMs))
    {
        std::cout << "Parameter 'lengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[6] << std::endl;
        return EXIT_FAILURE;
    }

    WavFile wav(1, sampleRate, bitDepth);
    Generators::sineWave(wav, lengthMs, frequency);
    wav.save(output);

    return EXIT_SUCCESS;
}

void Generators::sineWave(WavFile& wave, int lengthMs, double frequency)
{
	std::vector<double> samples;
	uint32_t numberOfSamples = wave.getSampleRate() * (lengthMs / 1000);
	for (uint32_t i = 0; i < numberOfSamples; i++)
	{
		double value = sin(Constants::TAU * i * (frequency / static_cast<double>(wave.getSampleRate())));
		for (size_t j = 0; j < wave.getNumberOfChannels(); j++)
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

int Commands::squareWaveCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 7)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string output = static_cast<std::string>(argv[2]);

    double frequency = 44100;
    if (!tryParse(argv[3], frequency))
    {
        std::cout << "Parameter 'frequency' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return EXIT_FAILURE;
    }

    uint32_t sampleRate = 44100;
    if (!tryParse(argv[4], sampleRate))
    {
        std::cout << "Parameter 'sampleRate' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
        return EXIT_FAILURE;
    }

    unsigned short bitDepth = 16;
    if (!tryParse(argv[5], bitDepth))
    {
        std::cout << "Parameter 'bitDepth' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
        return EXIT_FAILURE;
    }

    int lengthMs = 1000;
    if (!tryParse(argv[6], lengthMs))
    {
        std::cout << "Parameter 'lengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[6] << std::endl;
        return EXIT_FAILURE;
    }

    WavFile wav(1, sampleRate, bitDepth);
    Generators::squareWave(wav, lengthMs, frequency);
    wav.save(output);

    return EXIT_SUCCESS;
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
        for (size_t j = 0; j < wave.getNumberOfChannels(); j++)
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

/* square-wave-additive */

void Commands::squareWaveAdditiveHelp()
{
    std::cout << "square-wave-additive <output> <frequency> <sampleRate> <bitDepth> <length> [numberOfHarmonics]" << std::endl;
    std::cout << "\tdescription: writes a square wave generated by additive synthesis to the specified output file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\toutput: path to output wave file" << std::endl;
    std::cout << "\t\tfrequency: frequency in Hz of the square wave" << std::endl;
    std::cout << "\t\tsampleRate: sample rate of the output wave file" << std::endl;
    std::cout << "\t\tbitDepth: bit depth of the output wave file" << std::endl;
    std::cout << "\t\tlength: length of the output file in milliseconds" << std::endl;
    std::cout << "\t\tnumberOfHarmonics: the number of odd harmonics to use. Optional, defaults to 1000" << std::endl;
}

int Commands::squareWaveAdditiveCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 7 && argc != 8)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string output = static_cast<std::string>(argv[2]);

    double frequency = 44100;
    if (!tryParse(argv[3], frequency))
    {
        std::cout << "Parameter 'frequency' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return EXIT_FAILURE;
    }

    uint32_t sampleRate = 44100;
    if (!tryParse(argv[4], sampleRate))
    {
        std::cout << "Parameter 'sampleRate' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
        return EXIT_FAILURE;
    }

    unsigned short bitDepth = 16;
    if (!tryParse(argv[5], bitDepth))
    {
        std::cout << "Parameter 'bitDepth' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
        return EXIT_FAILURE;
    }

    int lengthMs = 1000;
    if (!tryParse(argv[6], lengthMs))
    {
        std::cout << "Parameter 'lengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[6] << std::endl;
        return EXIT_FAILURE;
    }

    int harmonics = 1000;
    if (argc == 8)
    {
        if (!tryParse(argv[7], harmonics))
        {
            std::cout << "Parameter 'harmonics' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[7] << std::endl;
            return EXIT_FAILURE;
        }
    }

    WavFile wav(1, sampleRate, bitDepth);
    Generators::squareWaveAdditive(wav, lengthMs, frequency, harmonics);
    wav.save(output);

    return EXIT_SUCCESS;
}

void Generators::squareWaveAdditive(WavFile& wave, int lengthMs, double frequency, int harmonics)
{
    std::vector<double> samples;
    uint32_t numberOfSamples = wave.getSampleRate() * (lengthMs / 1000);
    for (uint32_t i = 0; i < numberOfSamples; i++)
    {
        double sample = 0.0;
        for (int j = 1; j < (harmonics * 2 + 2); j = j + 2)
        {
            double harmonicFrequency = frequency * j;
            double harmonicAmplitude = 1.0 / j;
            sample += sin(Constants::TAU * i * (harmonicFrequency / static_cast<double>(wave.getSampleRate()))) * harmonicAmplitude;

        }
        for (uint16_t k = 0; k < wave.getNumberOfChannels(); k++)
        {
            samples.push_back(sample);
        }

    }
    wave.setSamples(samples);
    Tools::normalise(wave);
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

int Commands::triangleWaveCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 7)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string output = static_cast<std::string>(argv[2]);

    double frequency = 44100;
    if (!tryParse(argv[3], frequency))
    {
        std::cout << "Parameter 'frequency' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return EXIT_FAILURE;
    }

    uint32_t sampleRate = 44100;
    if (!tryParse(argv[4], sampleRate))
    {
        std::cout << "Parameter 'sampleRate' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
        return EXIT_FAILURE;
    }

    unsigned short bitDepth = 16;
    if (!tryParse(argv[5], bitDepth))
    {
        std::cout << "Parameter 'bitDepth' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
        return EXIT_FAILURE;
    }

    int lengthMs = 1000;
    if (!tryParse(argv[6], lengthMs))
    {
        std::cout << "Parameter 'lengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[6] << std::endl;
        return EXIT_FAILURE;
    }

    WavFile wav(1, sampleRate, bitDepth);
    Generators::triangleWave(wav, lengthMs, frequency);
    wav.save(output);

    return EXIT_SUCCESS;
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
        for (size_t j = 0; j < wave.getNumberOfChannels(); j++)
        {
            samples.push_back(sample);
        }

        if(positive)
        {
            sample += increment;
        } else
        {
            sample -= increment;
        }

        if (fabs(sample) >= 1.0) 
        {
            positive = !positive;
        }
    }
    wave.setSamples(samples);
}

/* triangle-wave-additive */

void Commands::triangleWaveAdditiveHelp()
{
    std::cout << "triangle-wave-additive <output> <frequency> <sampleRate> <bitDepth> <length> [numberOfHarmonics]" << std::endl;
    std::cout << "\tdescription: writes a triangle wave generated by additive synthesis to the specified output file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\toutput: path to output wave file" << std::endl;
    std::cout << "\t\tfrequency: frequency in Hz of the triangle wave" << std::endl;
    std::cout << "\t\tsampleRate: sample rate of the output wave file" << std::endl;
    std::cout << "\t\tbitDepth: bit depth of the output wave file" << std::endl;
    std::cout << "\t\tlength: length of the output file in milliseconds" << std::endl;
    std::cout << "\t\tnumberOfHarmonics: the number of odd harmonics to use. Optional, defaults to 1000" << std::endl;
}

int Commands::triangleWaveAdditiveCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 7 && argc != 8)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string output = static_cast<std::string>(argv[2]);

    double frequency = 44100;
    if (!tryParse(argv[3], frequency))
    {
        std::cout << "Parameter 'frequency' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return EXIT_FAILURE;
    }

    uint32_t sampleRate = 44100;
    if (!tryParse(argv[4], sampleRate))
    {
        std::cout << "Parameter 'sampleRate' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
        return EXIT_FAILURE;
    }

    unsigned short bitDepth = 16;
    if (!tryParse(argv[5], bitDepth))
    {
        std::cout << "Parameter 'bitDepth' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
        return EXIT_FAILURE;
    }

    int lengthMs = 1000;
    if (!tryParse(argv[6], lengthMs))
    {
        std::cout << "Parameter 'lengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[6] << std::endl;
        return EXIT_FAILURE;
    }

    int harmonics = 1000;
    if (argc == 8)
    {
        if (!tryParse(argv[7], harmonics))
        {
            std::cout << "Parameter 'harmonics' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[7] << std::endl;
            return EXIT_FAILURE;
        }
    }

    WavFile wav(1, sampleRate, bitDepth);
    Generators::triangleWaveAdditive(wav, lengthMs, frequency, harmonics);
    wav.save(output);

    return EXIT_SUCCESS;
}

void Generators::triangleWaveAdditive(WavFile& wave, int lengthMs, double frequency, int harmonics)
{
    std::vector<double> samples;
    uint32_t numberOfSamples = wave.getSampleRate() * (lengthMs / 1000);
    for (uint32_t i = 0; i < numberOfSamples; i++)
    {
        double sample = 0.0;
        for (int j = 1; j < (harmonics * 2 + 2); j = j + 2)
        {
            double harmonicFrequency = frequency * j;
            double harmonicAmplitude = 1.0 / (j * j);
            sample += sin(Constants::TAU * i * (harmonicFrequency / static_cast<double>(wave.getSampleRate()))) * harmonicAmplitude;

        }
        for (uint16_t k = 0; k < wave.getNumberOfChannels(); k++)
        {
            samples.push_back(sample);
        }

    }
    wave.setSamples(samples);
    Tools::normalise(wave);
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

int Commands::sawtoothWaveCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 7)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string output = static_cast<std::string>(argv[2]);

    double frequency = 44100;
    if (!tryParse(argv[3], frequency))
    {
        std::cout << "Parameter 'frequency' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return EXIT_FAILURE;
    }

    uint32_t sampleRate = 44100;
    if (!tryParse(argv[4], sampleRate))
    {
        std::cout << "Parameter 'sampleRate' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
        return EXIT_FAILURE;
    }

    unsigned short bitDepth = 16;
    if (!tryParse(argv[5], bitDepth))
    {
        std::cout << "Parameter 'bitDepth' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
        return EXIT_FAILURE;
    }

    int lengthMs = 1000;
    if (!tryParse(argv[6], lengthMs))
    {
        std::cout << "Parameter 'lengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[6] << std::endl;
        return EXIT_FAILURE;
    }

    WavFile wav(1, sampleRate, bitDepth);
    Generators::sawtoothWave(wav, lengthMs, frequency);
    wav.save(output);

    return EXIT_SUCCESS;
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
        for (size_t j = 0; j < wave.getNumberOfChannels(); j++)
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

/* sawtooth-wave-additive */

void Commands::sawtoothWaveAdditiveHelp()
{
    std::cout << "sawtooth-wave-additive <output> <frequency> <sampleRate> <bitDepth> <length> [numberOfHarmonics]" << std::endl;
    std::cout << "\tdescription: writes a sawtooth wave generated by additive synthesis to the specified output file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\toutput: path to output wave file" << std::endl;
    std::cout << "\t\tfrequency: frequency in Hz of the square wave" << std::endl;
    std::cout << "\t\tsampleRate: sample rate of the output wave file" << std::endl;
    std::cout << "\t\tbitDepth: bit depth of the output wave file" << std::endl;
    std::cout << "\t\tlength: length of the output file in milliseconds" << std::endl;
    std::cout << "\t\tnumberOfHarmonics: the number of harmonics to use. Optional, defaults to 1000" << std::endl;
}

int Commands::sawtoothWaveAdditiveCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 7 && argc != 8)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string output = static_cast<std::string>(argv[2]);

    double frequency = 44100;
    if (!tryParse(argv[3], frequency))
    {
        std::cout << "Parameter 'frequency' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return EXIT_FAILURE;
    }

    uint32_t sampleRate = 44100;
    if (!tryParse(argv[4], sampleRate))
    {
        std::cout << "Parameter 'sampleRate' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
        return EXIT_FAILURE;
    }

    unsigned short bitDepth = 16;
    if (!tryParse(argv[5], bitDepth))
    {
        std::cout << "Parameter 'bitDepth' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
        return EXIT_FAILURE;
    }

    int lengthMs = 1000;
    if (!tryParse(argv[6], lengthMs))
    {
        std::cout << "Parameter 'lengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[6] << std::endl;
        return EXIT_FAILURE;
    }

    int harmonics = 1000;
    if (argc == 8)
    {
        if (!tryParse(argv[7], harmonics))
        {
            std::cout << "Parameter 'harmonics' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[7] << std::endl;
            return EXIT_FAILURE;
        }
    }

    WavFile wav(1, sampleRate, bitDepth);
    Generators::sawtoothWaveAdditive(wav, lengthMs, frequency, harmonics);
    wav.save(output);

    return EXIT_SUCCESS;
}

void Generators::sawtoothWaveAdditive(WavFile& wave, int lengthMs, double frequency, int harmonics)
{
    std::vector<double> samples;
    uint32_t numberOfSamples = wave.getSampleRate() * (lengthMs / 1000);
    for (uint32_t i = 0; i < numberOfSamples; i++)
    {
        double sample = 0.0;
        for (int j = 1; j < harmonics + 1; j++)
        {
            double harmonicFrequency = frequency * j;
            double harmonicAmplitude = 1.0 / j;
            sample += sin(Constants::TAU * i * (harmonicFrequency / static_cast<double>(wave.getSampleRate()))) * harmonicAmplitude;

        }
        for (uint16_t k = 0; k < wave.getNumberOfChannels(); k++)
        {
            samples.push_back(sample);
        }

    }
    wave.setSamples(samples);
    Tools::normalise(wave);
}

/* white-noise command */

void Commands::whiteNoiseHelp()
{
    std::cout << "white-noise <output> <sampleRate> <bitDepth> <length>" << std::endl;
    std::cout << "\tdescription: writes generated white noise to specified output file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\toutput: path to output wave file" << std::endl;
    std::cout << "\t\tsampleRate: sample rate of the output wave file" << std::endl;
    std::cout << "\t\tbitDepth: bit depth of the output wave file" << std::endl;
    std::cout << "\t\tlength: length of the output file in milliseconds" << std::endl;
}

int Commands::whiteNoiseCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 6)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string output = static_cast<std::string>(argv[2]);


    uint32_t sampleRate = 44100;
    if (!tryParse(argv[3], sampleRate))
    {
        std::cout << "Parameter 'sampleRate' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return EXIT_FAILURE;
    }

    unsigned short bitDepth = 16;
    if (!tryParse(argv[4], bitDepth))
    {
        std::cout << "Parameter 'bitDepth' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
        return EXIT_FAILURE;
    }

    int lengthMs = 1000;
    if (!tryParse(argv[5], lengthMs))
    {
        std::cout << "Parameter 'lengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
        return EXIT_FAILURE;
    }

    WavFile wav(1, sampleRate, bitDepth);
    Generators::whiteNoise(wav, lengthMs);
    wav.save(output);

    return EXIT_SUCCESS;
}

void Generators::whiteNoise(WavFile& wave, int lengthMs)
{
    std::vector<double> samples;
    uint32_t numberOfSamples = wave.getSampleRate() * (lengthMs / 1000);
    for (size_t i = 0; i < numberOfSamples; i++)
    {
        double sample = (rand() / (RAND_MAX / 2.0)) - 1.0; // NOLINT(cert-msc50-cpp) - It's random enough for purpose
        for (size_t j = 0; j < wave.getNumberOfChannels(); j++)
        {
            samples.push_back(sample);
        }
    }
    wave.setSamples(samples);
}

/* pink-noise command */

void Commands::pinkNoiseHelp()
{
    std::cout << "pink-noise <output> <sampleRate> <bitDepth> <length>" << std::endl;
    std::cout << "\tdescription: writes generated pink noise to specified output file" << std::endl;
    std::cout << "\tparameters:" << std::endl;
    std::cout << "\t\toutput: path to output wave file" << std::endl;
    std::cout << "\t\tsampleRate: sample rate of the output wave file" << std::endl;
    std::cout << "\t\tbitDepth: bit depth of the output wave file" << std::endl;
    std::cout << "\t\tlength: length of the output file in milliseconds" << std::endl;
}

int Commands::pinkNoiseCommand(const std::string& command, int argc, char* argv[])
{
    if (argc != 6)
    {
        Commands::printIncorrectArgumentsWarning(command);
        return EXIT_FAILURE;
    }
    std::string output = static_cast<std::string>(argv[2]);


    uint32_t sampleRate = 44100;
    if (!tryParse(argv[3], sampleRate))
    {
        std::cout << "Parameter 'sampleRate' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[3] << std::endl;
        return EXIT_FAILURE;
    }

    unsigned short bitDepth = 16;
    if (!tryParse(argv[4], bitDepth))
    {
        std::cout << "Parameter 'bitDepth' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[4] << std::endl;
        return EXIT_FAILURE;
    }

    int lengthMs = 1000;
    if (!tryParse(argv[5], lengthMs))
    {
        std::cout << "Parameter 'lengthMs' could not be parsed. Please ensure it is a numeric value. Provided value: " << argv[5] << std::endl;
        return EXIT_FAILURE;
    }

    WavFile wav(1, sampleRate, bitDepth);
    Generators::pinkNoise(wav, lengthMs);
    wav.save(output);

    return EXIT_SUCCESS;
}

void Generators::pinkNoise(WavFile& wave, int lengthMs)
{
    //https://www.firstpr.com.au/dsp/pink-noise/

    double buf0, buf1, buf2, buf3, buf4, buf5, buf6;
    buf0 = buf1 = buf2 = buf3 = buf4 = buf5 = buf6 = 0.0;
    double amplitude = 0.129;
    std::vector<double> samples;
    uint32_t numberOfSamples = wave.getSampleRate() * (lengthMs / 1000);
    for (size_t i = 0; i < numberOfSamples; i++)
    {
        double white = (rand() / (RAND_MAX / 2.0)) - 1.0; // NOLINT(cert-msc50-cpp) - It's random enough for purpose
        buf0 = 0.99886f * buf0 + 0.0555179f * white;
        buf1 = 0.99332f * buf1 + 0.0750759f * white;
        buf2 = 0.96900f * buf2 + 0.1538520f * white;
        buf3 = 0.86650f * buf3 + 0.3104856f * white;
        buf4 = 0.55000f * buf4 + 0.5329522f * white;
        buf5 = -0.7616f * buf5 - 0.0168980f * white;
        double sample = amplitude * (buf0 + buf1 + buf2 + buf3 + buf4 + buf5 + buf6 + white * 0.5362);
        buf6 = white * 0.115926;

        for (size_t j = 0; j < wave.getNumberOfChannels(); j++)
        {
            samples.push_back(sample);
        }
    }
    wave.setSamples(samples);
    Tools::normalise(wave);
}