#pragma once
#include <cstdint>
#include <functional>
#include <wavlab/WavFile.h>

namespace wavlab
{

struct DelayChannelSettings
{
    std::uint32_t delayMilliseconds;
    double feedbackPercent;
};

struct DelaySettings
{
    DelayChannelSettings channel;
    double mixPercent;
    std::uint32_t maximumFileLengthMilliseconds;
};

struct StereoDelaySettings
{
    DelayChannelSettings left;
    DelayChannelSettings right;
    double mixPercent;
    std::uint32_t maximumFileLengthMilliseconds;
};

struct ReverbSettings
{
    double sizePercent;
    double timePercent;
    double mixPercent;
    std::uint32_t maximumFileLengthMilliseconds;
};

namespace FX
{
using FeedbackProcessor = std::function<double(double input)>;

/// <summary>
/// Drives the audio input into a tape saturation algorithm
/// </summary>
/// <param name="wave">The wave file to apply saturation to</param>
/// <param name="drive">The gain multiplier to apply</param>
void saturate(WavFile &wave, double drive);

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
void delay(
    WavFile &wave, const DelaySettings &settings,
    const FeedbackProcessor &feedbackProcessor = [](double input) { return input; });

/// <summary>
/// Applies a stereo digital delay to the specified wave file
/// </summary>
void delay(
    WavFile &wave, const StereoDelaySettings &settings,
    const FeedbackProcessor &feedbackProcessor = [](double input) { return input; });

/// <summary>
/// Applies a basic algorithmic reverb to the specified wave file
/// </summary>
void reverb(WavFile &wave, const ReverbSettings &settings);
} // namespace FX

} // namespace wavlab
