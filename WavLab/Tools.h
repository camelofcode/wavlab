#pragma once
#include "WavFile.h"

namespace Tools
{
	/// <summary>
	/// Normalises a wave file to maximum
	/// </summary>
	/// <param name="wave">The wave file to normalise</param>
	void normalise(WavFile& wave);

	/// <summary>
	/// Reverses a wave file
	/// </summary>
	/// <param name="wave">The wave file to reverse</param>
	void reverse(WavFile& wave);

	/// <summary>
	/// Flips the phase of a wave file
	/// </summary>
	/// <param name="wave">The wave file to phase flip</param>
	void flipPhase(WavFile& wave);

	/// <summary>
	/// Swaps the left and right channels of a (stereo) wave file
	/// </summary>
	/// <param name="wave">The wave file to swap channels of</param>
	void swapChannels(WavFile& wave);

	/// <summary>
	/// Splits a stero wav into left and right mono files
	/// </summary>
	/// <param name="wave">The wave file to split</param>
	void splitStereo(WavFile& stereo, WavFile& left, WavFile& right);

	/// <summary>
	/// Applies a gain multiplier to the wave file
	/// </summary>
	/// <param name="wave">The wave file to apply gain to</param>
	/// <param name="gain">The gain multiplier to apply</param>
	void gain(WavFile& wave, double gain);

	/// <summary>
	/// Applies a gain in dB to the wave file
	/// </summary>
	/// <param name="wave">The wave file to apply gain to</param>
	/// <param name="dB">The gain in dB to apply</param>
	void decibelGain(WavFile& wave, double dB);

	/// <summary>
	/// Returns the peak dB value of the wave file
	/// </summary>
	double getPeakDecibels(WavFile& wave);

	/// <summary>
	/// Returns the RMS dB value of the wave file
	/// </summary>
	double getRMSDecibels(WavFile& wave);
}