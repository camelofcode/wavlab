#include "ConversionHelpers.h"

double ConversionHelpers::eightBitToDouble(char input)
{
	double min = 128.0;
	double max = 127.0;
	double sample;

	if (input > 0)
	{
		sample = static_cast<double>(input) / max;
	}
	else
	{
		sample = static_cast<double>(input) / min;
	}

	return sample;
}

char ConversionHelpers::doubleToEightBit(double input)
{
	double min = 128.0;
	double max = 127.0;

	double sample = safeScaleDouble(input, max, min);
	return static_cast<char>(sample);
}

double ConversionHelpers::sixteenBitToDouble(short input)
{
	double min = 32768.0;
	double max = 32767.0;
	double sample;

	if (input > 0)
	{
		sample = static_cast<double>(input) / max;
	}
	else
	{
		sample = static_cast<double>(input) / min;
	}

	return sample;
}

short ConversionHelpers::doubleToSixteenBit(double input)
{
	double min = 32768.0;
	double max = 32767.0;

	double sample = safeScaleDouble(input, max, min);
	return static_cast<short>(sample);
}

double ConversionHelpers::twentyFourBitToDouble(int input)
{
	double min = 8388608.0;
	double max = 8388607.0;
	double sample;

	if (input > 0)
	{
		sample = static_cast<double>(input) / max;
	}
	else
	{
		sample = static_cast<double>(input) / min;
	}

	return sample;
}

int ConversionHelpers::doubleToTwentyFourBit(double input)
{
	double max = 8388607.0;
	double min = 8388608.0;

	double sample = safeScaleDouble(input, max, min);
	return static_cast<int>(sample);
}


double ConversionHelpers::thirtyTwoBitToDouble(int input)
{
	double min = 2147483648.0;
	double max = 2147483647.0;
	double sample;

	if (input > 0)
	{
		sample = static_cast<double>(input) / max;
	}
	else
	{
		sample = static_cast<double>(input) / min;
	}

	return sample;
}

int ConversionHelpers::doubleToThirtyTwoBit(double input)
{
	double max = 2147483647.0;
	double min = 2147483648.0;
	
	double sample = safeScaleDouble(input, max, min);
	return static_cast<int>(sample);
}

double ConversionHelpers::safeScaleDouble(double input, double max, double min)
{
	double sample;

	if (input > 0)
	{
		sample = input * max;
		if (sample > max)
		{
			sample = max;
		}
	}
	else
	{
		sample = input * min;
		if (sample < -min)
		{
			sample = -min;
		}
	}

	return sample;
}