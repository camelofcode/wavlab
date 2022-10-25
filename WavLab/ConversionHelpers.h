#pragma once

namespace ConversionHelpers
{
	double sixteenBitToDouble(short input);
	short doubleToSixteenBit(double input);
	double twentyfourBitToDouble(int input);
	int doubleToTwentyFourBit(double input);
	double thirtytwoBitToDouble(int input);
	int doubleToThirtyTwoBit(double input);

	double safeScaleDouble(double input, double max, double min);
}