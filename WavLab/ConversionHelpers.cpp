#include "ConversionHelpers.h"

double ConversionHelpers::sixteenBitToDouble(short input)
{
	double min = 32767.0;
	double max = 32768.0;
	double sample = 0.0;

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
	double max = 32767.0;
	double min = 32768.0;
	double sample = 0.0;

	if (input > 0)
	{
		sample = input * max;
	}
	else
	{
		sample = input * min;
	}
	return static_cast<short>(sample);
}

double ConversionHelpers::twentyfourBitToDouble(int input)
{
	double min = 8388608.0;
	double max = 8388607.0;
	double sample = 0.0;

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
	double sample = 0.0;

	if (input > 0)
	{
		sample = input * max;
	}
	else
	{
		sample = input * min;
	}
	return static_cast<int>(sample);
}


double ConversionHelpers::thirtytwoBitToDouble(int input)
{
	double min = 2147483648.0;
	double max = 2147483647.0;
	double sample = 0.0;

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
	double sample = 0.0;

	if (input > 0)
	{
		sample = input * max;
	}
	else
	{
		sample = input * min;
	}
	return static_cast<int>(sample);
}