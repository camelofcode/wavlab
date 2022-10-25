#pragma once
#include "WavFile.h"

namespace Generators
{
	/// <summary>
	/// Constructor for generating a sine wave and writing it to a wave file.
	/// </summary>
	/// <param name="wave">The wave file to write to</param>
	/// <param name="lengthMs">The length in ms of the generated wave</param>
	/// <param name="frequency">The frequency in Hz of the sine wav</param>
	void sineWave(WavFile& wave, int lengthMs, double frequency);
};