#pragma once
#include "WavFile.h"

namespace Tools
{
	/// <summary>
	/// Normalises a wave file to maximum
	/// </summary>
	/// <param name="wave">The wave file to normalise</param>
	void Normalise(WavFile &wave);

	/// <summary>
	/// Applies a gain multiplier to the wave file
	/// </summary>
	/// <param name="wave">The wave file to apply gain to</param>
	/// <param name="gain">The gain multiplier to apply</param>
	void Gain(WavFile &wave, double gain);

	/// <summary>
	/// Applies a gain in dB to the wave file
	/// </summary>
	/// <param name="wave">The wave file to apply gain to</param>
	/// <param name="dB">The gain in dB to apply</param>
	void DecibelGain(WavFile &wave, double dB);

	/// <summary>
	/// Returns the peak dB value of the wave file
	/// </summary>
	double GetPeakDecibels(WavFile& wave);
}