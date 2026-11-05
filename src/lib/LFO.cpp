#include <wavlab/LFO.h>
#include <cmath>
#include "Constants.h"

namespace wavlab
{

LFO::LFO(uint32_t sampleRate) :
    count(0),
    sampleRate(sampleRate)
{

}

double LFO::next(double input)
{
	double frequency = input / (sampleRate / 2.0);
	double output = fabs(sin(Constants::TAU * count * frequency));

	count++;
	if (count > sampleRate)
	{
		count = 0;
	}

	return output;
}

}
