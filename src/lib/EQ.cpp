//This file is an implementation of the praiseworthy "Cookbook formulae for audio EQ biquad filter coefficients" by Robert Bristow-Johnson <rbj@audioimagination.com>
//See https://webaudio.github.io/Audio-EQ-Cookbook/Audio-EQ-Cookbook.txt , https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
//My implementation is however taken mostly from https://www.musicdsp.org/en/latest/Filters/128-c-class-implementation-of-rbj-filters.html

#include <wavlab/EQ.h>
#include "Constants.h"
#include <cmath>

namespace wavlab
{

EQInstance::EQInstance(FilterType type, double gainDecibels, double centreFrequency, double q, uint32_t sampleRate)
{
	double alpha, a0 = 0.0, a1 = 0.0, a2 = 0.0, b0 = 0.0, b1 = 0.0, b2 = 0.0;

	bool qIsBandwidth = type == FilterType::Peak || type == FilterType::BandPass || type == FilterType::AllPass;

	double const omega = Constants::TAU * centreFrequency / sampleRate;
	double const sinOmega = sin(omega);
	double const cosOmega = cos(omega);

	alpha = qIsBandwidth ? sinOmega * sinh(log(2.0) / 2.0 * q * omega / sinOmega) : sinOmega / (2.0 * q);

	if (type == FilterType::Peak || type == FilterType::LowShelf || type == FilterType::HighShelf)
	{
		double const amplitude = pow(10.0, (gainDecibels / 40.0));
		double const beta = sqrt(amplitude) / q;

		if (type == FilterType::Peak)
		{
			b0 = float(1.0 + alpha * amplitude);
			b1 = float(-2.0 * cosOmega);
			b2 = float(1.0 - alpha * amplitude);
			a0 = float(1.0 + alpha / amplitude);
			a1 = float(-2.0 * cosOmega);
			a2 = float(1.0 - alpha / amplitude);
		}		
		if (type == FilterType::LowShelf)
		{
			b0 = float(amplitude * ((amplitude + 1.0) - (amplitude - 1.0) * cosOmega + beta * sinOmega));
			b1 = float(2.0 * amplitude * ((amplitude - 1.0) - (amplitude + 1.0) * cosOmega));
			b2 = float(amplitude * ((amplitude + 1.0) - (amplitude - 1.0) * cosOmega - beta * sinOmega));
			a0 = float((amplitude + 1.0) + (amplitude - 1.0) * cosOmega + beta * sinOmega);
			a1 = float(-2.0 * ((amplitude - 1.0) + (amplitude + 1.0) * cosOmega));
			a2 = float((amplitude + 1.0) + (amplitude - 1.0) * cosOmega - beta * sinOmega);
		}
		else if (type == FilterType::HighShelf)
		{
			b0 = float(amplitude * ((amplitude + 1.0) + (amplitude - 1.0) * cosOmega + beta * sinOmega));
			b1 = float(-2.0 * amplitude * ((amplitude - 1.0) + (amplitude + 1.0) * cosOmega));
			b2 = float(amplitude * ((amplitude + 1.0) + (amplitude - 1.0) * cosOmega - beta * sinOmega));
			a0 = float((amplitude + 1.0) - (amplitude - 1.0) * cosOmega + beta * sinOmega);
			a1 = float(2.0 * ((amplitude - 1.0) - (amplitude + 1.0) * cosOmega));
			a2 = float((amplitude + 1.0) - (amplitude - 1.0) * cosOmega - beta * sinOmega);
		}
	}
	else 
	{
		if (type == FilterType::LowPass)
		{
			b0 = (1.0 - cosOmega) / 2.0;
			b1 = 1.0 - cosOmega;
			b2 = (1.0 - cosOmega) / 2.0;
			a0 = 1.0 + alpha;
			a1 = -2.0 * cosOmega;
			a2 = 1.0 - alpha;
		}
		if (type == FilterType::HighPass)
		{
			b0 = (1.0 + cosOmega) / 2.0;
			b1 = -(1.0 + cosOmega);
			b2 = (1.0 + cosOmega) / 2.0;
			a0 = 1.0 + alpha;
			a1 = -2.0 * cosOmega;
			a2 = 1.0 - alpha;
		}
		if (type == FilterType::BandPass)
		{
			b0 = sinOmega / 2.0;
			b1 = 0.0;
			b2 = -sinOmega / 2;
			a0 = 1.0 + alpha;
			a1 = -2.0 * cosOmega;
			a2 = 1.0 - alpha;
		}
		if (type == FilterType::AllPass)
		{
			b0 = 1.0 - alpha;
			b1 = -2.0 * cosOmega;
			b2 = 1.0 + alpha;
			a0 = 1.0 + alpha;
			a1 = -2.0 * cosOmega;
			a2 = 1.0 - alpha;
		}
	}

	b0a0 = double(b0 / a0);
	b1a0 = double(b1 / a0);
	b2a0 = double(b2 / a0);
	a1a0 = double(a1 / a0);
	a2a0 = double(a2 / a0);
}

double EQInstance::processSample(double in)
{
	double yn = b0a0 * in + b1a0 * in1 + b2a0 * in2 - a1a0 * out1 - a2a0 * out2;

	// push in/out buffers
	in2 = in1;
	in1 = in;
	out2 = out1;
	out1 = yn;

	// return output
	return yn;
}

void EQ::process(WavFile& wave, FilterType type, double gainDecibels, double centreFrequency, double q)
{
	if (wave.getNumberOfChannels() == 1)
	{
		EQInstance eqMono(type, gainDecibels, centreFrequency, q, wave.getSampleRate());

		std::vector<double> newSamples;

		for (auto& sample : wave.getSamples())
		{
			newSamples.push_back(eqMono.processSample(sample));
		}
		wave.setSamples(newSamples);
		return;
	}
	else if (wave.getNumberOfChannels() == 2)
	{
		EQInstance eqLeft(type, gainDecibels, centreFrequency, q, wave.getSampleRate());
		EQInstance eqRight(type, gainDecibels, centreFrequency, q, wave.getSampleRate());

		std::vector<double> original = wave.getSamples();
		std::vector<double> newSamples;

		for (size_t i = 0; i < original.size() - 1; i = i + 2)
		{
			double left = original[i];
			double right = original[i + 1];

			newSamples.push_back(eqLeft.processSample(left));
			newSamples.push_back(eqRight.processSample(right));
		}
		wave.setSamples(newSamples);
	}
}

/* Commands */

}
