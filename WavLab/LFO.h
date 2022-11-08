#pragma once
#include <cstdint>

class LFO
{
public:
	LFO(uint32_t sampleRate);
	double next(double input);

private:
	uint32_t count;
	uint32_t sampleRate;
};