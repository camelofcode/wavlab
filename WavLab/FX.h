#pragma once
#include "WavFile.h"

namespace FX
{
	/// <summary>
	/// Drives the audio input into a tape saturation algorithm
	/// </summary>
	/// <param name="wave">The wave file to apply saturation to</param>
	/// <param name="drive">The gain multiplier to apply</param>
	void saturate(WavFile& wave, double drive);
}