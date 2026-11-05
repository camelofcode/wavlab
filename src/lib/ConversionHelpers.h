#pragma once
#include <string>

namespace wavlab
{

namespace ConversionHelpers
{
	double eightBitToDouble (char input);
	char doubleToEightBit(double input);
	double sixteenBitToDouble(short input);
	short doubleToSixteenBit(double input);
	double twentyFourBitToDouble(int input);
	int doubleToTwentyFourBit(double input);
	double thirtyTwoBitToDouble(int input);
	int doubleToThirtyTwoBit(double input);

	double safeScaleDouble(double input, double max, double min);
}

}
