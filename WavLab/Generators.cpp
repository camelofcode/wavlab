#define _USE_MATH_DEFINES
#include "Generators.h"
#include <cmath>

const double tau = M_PI * 2.0;

void Generators::sineWave(WavFile& wave, int lengthMs, double frequency)
{
	std::vector<double> samples;
	uint32_t numberOfSamples = wave.getSampleRate() * (lengthMs / 1000);
	for (size_t i = 0; i < numberOfSamples; i++)
	{
		double value = sin(tau * i * (frequency / static_cast<double>(wave.getSampleRate())));
		for (size_t i = 0; i < wave.getNumberOfChannels(); i++)
		{
			samples.push_back(value);
		}
	}
	wave.setSamples(samples);
}