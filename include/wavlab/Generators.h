#pragma once
#include <wavlab/WavFile.h>

namespace wavlab
{

namespace Generators
{
	/// <summary>
	/// Generate a sine wave and write it to a wave file.
	/// </summary>
	/// <param name="wave">The wave file to write to</param>
	/// <param name="lengthMs">The length in ms of the generated wave</param>
	/// <param name="frequency">The frequency in Hz of the sine wav</param>
	void sineWave(WavFile& wave, int lengthMs, double frequency);

	/// <summary>
	/// Generate a pure square wave and write it to a wave file.
	/// </summary>
	/// <param name="wave">The wave file to write to</param>
	/// <param name="lengthMs">The length in ms of the generated wave</param>
	/// <param name="frequency">The frequency in Hz of the square wav</param>
	void squareWave(WavFile& wave, int lengthMs, double frequency);

	/// <summary>
	/// Generate a square wave by additive synthesis and write it to a wave file.
	/// </summary>
	/// <param name="wave">The wave file to write to</param>
	/// <param name="lengthMs">The length in ms of the generated wave</param>
	/// <param name="frequency">The frequency in Hz of the square wav</param>
	/// <param name="harmonics">The number of odd harmonics to use</param>
	void squareWaveAdditive(WavFile& wave, int lengthMs, double frequency, int harmonics);

	/// <summary>
	/// Generate a pure triangle wave and write it to a wave file.
	/// </summary>
	/// <param name="wave">The wave file to write to</param>
	/// <param name="lengthMs">The length in ms of the generated wave</param>
	/// <param name="frequency">The frequency in Hz of the triangle wav</param>
	void triangleWave(WavFile& wave, int lengthMs, double frequency);

	/// <summary>
	/// Generate a triangle wave by additive synthesis and write it to a wave file.
	/// </summary>
	/// <param name="wave">The wave file to write to</param>
	/// <param name="lengthMs">The length in ms of the generated wave</param>
	/// <param name="frequency">The frequency in Hz of the triangle wav</param>
	/// <param name="harmonics">The number of odd harmonics to use</param>
	void triangleWaveAdditive(WavFile& wave, int lengthMs, double frequency, int harmonics);

	/// <summary>
	/// Generate a pure sawtooth wave and write it to a wave file.
	/// </summary>
	/// <param name="wave">The wave file to write to</param>
	/// <param name="lengthMs">The length in ms of the generated wave</param>
	/// <param name="frequency">The frequency in Hz of the sawtooth wav</param>
	void sawtoothWave(WavFile& wave, int lengthMs, double frequency);

	/// <summary>
	/// Generate a sawtooth wave by additive synthesis and write it to a wave file.
	/// </summary>
	/// <param name="wave">The wave file to write to</param>
	/// <param name="lengthMs">The length in ms of the generated wave</param>
	/// <param name="frequency">The frequency in Hz of the sawtooth wav</param>
	/// <param name="harmonics">The number of harmonics to use</param>
	void sawtoothWaveAdditive(WavFile& wave, int lengthMs, double frequency, int harmonics);

	/// <summary>
	/// Generates white noise
	/// </summary>
	/// <param name="wave">The wave file to write to</param>
	/// <param name="lengthMs">The length in ms of the generated wave</param>
	void whiteNoise(WavFile& wave, int lengthMs);

	/// <summary>
	/// Generates pink noise
	/// </summary>
	/// <param name="wave">The wave file to write to</param>
	/// <param name="lengthMs">The length in ms of the generated wave</param>
	void pinkNoise(WavFile& wave, int lengthMs);
}

}
