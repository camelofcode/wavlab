#pragma once
#include <vector>
#include <functional>

class Delay
{
public:
	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="maxMilliseconds">Max delay time in milliseconds</param>
	/// <param name="initialMilliseconds">Initial delay time in milliseconds</param>
	/// <param name="sampleRate">Sample rate</param>
	Delay(uint32_t maxMilliseconds, uint32_t initialMilliseconds, uint32_t sampleRate);

	///Destructor
	~Delay() = default;

	/// <summary>
	/// Set delay time in milliseconds
	/// </summary>
	/// <param name="milliseconds">Delay time in milliseconds</param>
	void setDelayTime(uint32_t milliseconds);

	/// <summary>
	/// Get delayed sample, with feedback
	/// </summary>
	/// <param name="sample">Input sample</param>
	/// <param name="feedback">Feedback</param>
    /// <param name="mix">Mix</param>
	/// <returns>Output sample</returns>
	double next(double sample, double feedback, double mix);

	/// <summary>
	/// Get delayed sample, with feedback that is run through a separate processor
	/// </summary>
	/// <param name="sample">Input sample</param>
	/// <param name="feedback">Feedback</param>
	/// <param name="feedbackProcessor">Feedback processing function</param>
	/// <param name="mix">Mix</param>
	/// <returns></returns>
	double next(double sample, double feedback, const std::function<double(double input)>& feedbackProcessor, double mix);

	/// <summary>
	/// Get delayed sample, with an lfo applied - useful for modulation FX such as chorus
	/// </summary>
	/// <param name="sample">Input sample</param>
	/// <param name="lfo">Lfo offset</param>
	/// <param name="mix">Mix</param>
	/// <returns></returns>
	double next(double sample, int lfo, double mix);

private:
	uint32_t sampleRate;
	uint32_t inputPosition;
	uint32_t outputPosition;
	uint32_t numberOfSamples;
	uint32_t delaySamples;
	std::vector<double> delayBuffer;

	void movePositions();
};