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

	/// <summary>
	/// Applies a mono digital delay to the specified wave file
	/// </summary>
	/// <param name="wave">The wave file to add delay to</param>
	/// <param name="delayMs">Delay time in milliseconds</param>
	/// <param name="mix">Feedback of the delay effect, from 0-100%</param>
	/// <param name="mix">Mix of the delay effect, from 0-100%</param>
	void digitalDelayMono(WavFile& wave, uint32_t delayMs, double feedback, double mix, uint32_t maxFileLength);
}