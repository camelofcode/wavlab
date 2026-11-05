#include <wavlab/Generators.h>
#include "Constants.h"
#include <wavlab/Tools.h>
#include <cmath>

namespace wavlab
{

/* sine-wave command */

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

}
