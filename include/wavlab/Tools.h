#pragma once
#include <wavlab/WavFile.h>

namespace wavlab
{

namespace Tools
{
	/// <summary>
	/// Normalises a wave file to maximum
	/// </summary>
	/// <param name="wave">The wave file to normalise</param>
	void normalise(WavFile& wave);

	/// <summary>
	/// Normalises a wave file to maximum
	/// </summary>
	/// <param name="wave">The wave file to normalise</param>
	/// <param name="normaliseTo">Level to normalise to</param>
	void normalise(WavFile& wave, double normaliseTo);

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
	/// Splits a stereo wav into left and right mono files
	/// </summary>
	/// <param name="stereo">The wave file to split</param>
	/// <param name="left">The wave file to write left channel to</param>
	/// <param name="right">The wave file to write right channel to</param>
	void splitStereo(WavFile& stereo, WavFile& left, WavFile& right);

	/// <summary>
	/// Splits a stereo wav into left and right mono files
	/// </summary>
	/// <param name="left">The left channel wave file</param>
	/// <param name="right">The right channel wave file</param>
	/// <param name="out">The wave file to write combined samples to</param>
	void combineLeftRight(WavFile& left, WavFile& right, WavFile& out);

	/// <summary>
	/// Splits a stereo wav into mid and side mono files
	/// </summary>
	/// <param name="wave">The wave file to split</param>
	/// <param name="mid">The wave file to write mid channel to</param>
	/// <param name="side">The wave file to write side channel to</param>
	void splitStereoMidSide(WavFile& stereo, WavFile& mid, WavFile& side);

	/// <summary>
	/// Joins mid and side mono wavs into a single stereo file
	/// </summary>
	/// <param name="mid">The mid channel wave file</param>
	/// <param name="side">The side channel wave file</param>
	/// <param name="out">The wave file to write combined samples to</param>
	void combineMidSide(WavFile& mid, WavFile& side, WavFile& out);

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

	/// <summary>
	/// Mixes an arbitrary number of wave files to a single wave file. Will attenuate and re-normalise to ensure lack of clipping. Currently will start all files at zero. In future could use BWAV chunk to determine position
	/// </summary>
	/// <param name="inputs">The wave files to mix</param>
	/// <param name="output">The output wave</param>
	void mix(std::vector<WavFile>& inputs, WavFile& output);

	/// <summary>
	/// Splits a wave file at specified times in milliseconds
	/// </summary>
	/// <param name="input">The wave file to split</param>
	/// <param name="outputs">The separate output wave files</param>
	/// <param name="splitTimesMs">The times to split at in milliseconds</param>
	void split(WavFile& input, std::vector<WavFile>& outputs, const std::vector<uint32_t>& splitTimesMs);

	/// <summary>
	/// Joins an arbitrary number of wave files to a single wave file.
	/// </summary>
	/// <param name="inputs">The wave files to join</param>
	/// <param name="output">The output wave</param>
	void join(std::vector<WavFile>& inputs, WavFile& output);
}

}
