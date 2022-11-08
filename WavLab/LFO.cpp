#include "LFO.h"
#include <cmath>
#include "Constants.h"

LFO::LFO(uint32_t sampleRate) :
	sampleRate(sampleRate),
	count(0)
{

}

double LFO::next(double input)
{
	double frequency = input / (sampleRate / 2);
	double output = fabs(sin(Constants::TAU * count * frequency));

	count++;
	if (count > sampleRate)
	{
		count = 0;
	}

	return output;
}