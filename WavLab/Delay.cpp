#include "Delay.h"

Delay::Delay(uint32_t maxMilliseconds, uint32_t initialMilliseconds, uint32_t sampleRate) :
	sampleRate(sampleRate),
	inputPosition(1),
	outputPosition(0)
{
	numberOfSamples = static_cast<uint32_t>(sampleRate * (maxMilliseconds / 1000.0));
	delayBuffer.resize(numberOfSamples);
	setDelayTime(initialMilliseconds);
}

void Delay::setDelayTime(uint32_t milliseconds)
{
	delaySamples = static_cast<uint32_t>(sampleRate * (milliseconds / 1000.0));

	if (inputPosition > outputPosition)
	{
		outputPosition = inputPosition - delaySamples;
	}

	if (inputPosition < outputPosition)
	{
		outputPosition = (numberOfSamples + inputPosition) - delaySamples;
	}

	if (inputPosition == delaySamples)
	{
		outputPosition = numberOfSamples;
	}

	if (inputPosition <= (delaySamples - 1))
	{
		outputPosition = numberOfSamples - (delaySamples - inputPosition);
	}

	if (inputPosition == 0)
	{
		outputPosition = numberOfSamples - delaySamples;
	}

	if (outputPosition == numberOfSamples)
	{
		inputPosition = delaySamples;
	}
}

double Delay::next(double sample, double feedback, const std::function<double(double input)>& feedbackProcessor, double mix)
{
	double output = delayBuffer[outputPosition];
	delayBuffer[inputPosition] = sample + feedbackProcessor((feedback * output));
	movePositions();

	double inverseMix = 1.0 - mix;
	return (output * mix) + (sample * inverseMix);
}

double Delay::next(double sample, uint32_t lfo, double mix)
{
    uint32_t outputPositionLfo = outputPosition + lfo;
	if (outputPositionLfo >= numberOfSamples)
	{
		outputPositionLfo = outputPositionLfo - numberOfSamples;
	}

	double output = delayBuffer[outputPositionLfo];
	delayBuffer[inputPosition] = sample;
	movePositions();

	double inverseMix = 1.0 - mix;
	return (output * mix) + (sample * inverseMix);
}

void Delay::movePositions()
{
	inputPosition++;
	outputPosition++;

	if (inputPosition == numberOfSamples)
	{
		inputPosition = 0;
	}
	if (outputPosition == numberOfSamples)
	{
		outputPosition = 0;
	}
}