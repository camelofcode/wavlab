#pragma once
#include <wavlab/WavFile.h>

namespace wavlab
{

enum class FilterType
{
	None = 0,
	Peak = 1,
	AllPass = 2,
	BandPass = 3,
	LowPass = 4,
	HighPass = 5,
	LowShelf = 6,
	HighShelf = 7
};

namespace EQ
{
	void process(WavFile& wave, FilterType type, double gainDecibels, double centreFrequency, double q);
}

class EQInstance
{
public:
	EQInstance(FilterType type, double gainDecibels, double centreFrequency, double q, uint32_t sampleRate);

	double processSample(double in);

private:
	double b0a0, b1a0, b2a0, a1a0, a2a0;
	double in1 = 0.0, in2 = 0.0, out1 = 0.0, out2 = 0.0;
};

}
