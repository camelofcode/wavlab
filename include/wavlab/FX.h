#pragma once
#include <wavlab/WavFile.h>
#include <cstdint>
#include <functional>

namespace wavlab
{

namespace FX
{
	/// <summary>
	/// Drives the audio input into a tape saturation algorithm
	/// </summary>
	/// <param name="wave">The wave file to apply saturation to</param>
	/// <param name="drive">The gain multiplier to apply</param>
	void saturate(WavFile& wave, double drive);

	/// <summary>
	/// Drives a single sample into a tape saturation algorithm
	/// </summary>
	/// <param name="wave">The sample to apply saturation to</param>
	/// <param name="drive">The gain multiplier to apply</param>
	double saturateSample(double in, double gainFactor);

	/// <summary>
	/// Applies a mono digital delay to the specified wave file
	/// </summary>
	/// <param name="wave">The wave file to add delay to</param>
	/// <param name="delayMs">Delay time in milliseconds</param>
	/// <param name="mix">Feedback of the delay effect, from 0-100%</param>
	/// <param name="mix">Mix of the delay effect, from 0-100%</param>
	void delay(WavFile& wave, uint32_t delayMs, double feedback, double mix, uint32_t maxFileLengthMs);

	/// <summary>
	/// Applies a stereo digital delay to the specified wave file
	/// </summary>
	void delay(WavFile& wave, uint32_t delayMsL, double feedbackL, uint32_t delayMsR, double feedbackR, double mix, uint32_t maxFileLengthMs);

	/// <summary>
	/// Applies a mono digital delay to the specified wave file with feedback processing
	/// </summary>
	void delay(WavFile& wave, uint32_t delayMs, double feedback, double mix, uint32_t maxFileLengthMs, const std::function<double(double input)>& feedbackProcessor);

	/// <summary>
	/// Applies a stereo digital delay to the specified wave file with feedback processing
	/// </summary>
	void delay(WavFile& wave, bool stereo, uint32_t delayMsL, double feedbackL, uint32_t delayMsR, double feedbackR, double mix, uint32_t maxFileLengthMs, const std::function<double(double input)>& feedbackProcessor);

	/// <summary>
	/// Applies a basic algorithmic reverb to the specified wave file
	/// </summary>
	void reverb(WavFile& wave, double size, double time, double mix, uint32_t maxFileLengthMs);
}

}
