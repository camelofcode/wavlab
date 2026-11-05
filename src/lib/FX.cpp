#include <wavlab/FX.h>

#include "DspConversions.h"

#include <wavlab/Delay.h>
#include <wavlab/EQ.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <numbers>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace wavlab
{

EffectError::EffectError(EffectErrorCode code, std::string message)
    : std::invalid_argument(std::move(message)), code_(code)
{
}

EffectErrorCode EffectError::code() const noexcept { return code_; }

namespace
{

using FeedbackProcessor = std::function<double(std::size_t, double)>;

void validateFeedback(double feedback)
{
    if (!std::isfinite(feedback) || std::abs(feedback) >= 1.0)
    {
        throw EffectError(EffectErrorCode::invalidFeedback,
                          "delay feedback must be finite with magnitude below one");
    }
}

void validateMix(double mix)
{
    if (!std::isfinite(mix) || mix < 0.0 || mix > 1.0)
    {
        throw EffectError(EffectErrorCode::invalidMix,
                          "effect mix must be finite and between zero and one");
    }
}

void validateCutoff(double cutoffHz, std::uint32_t sampleRate)
{
    const auto nyquist = static_cast<double>(sampleRate) / 2.0;
    if (!std::isfinite(cutoffHz) || cutoffHz <= 0.0 || cutoffHz >= nyquist)
    {
        throw EffectError(EffectErrorCode::invalidCutoffFrequency,
                          "delay cutoff must be finite, greater than zero, and below Nyquist");
    }
}

[[nodiscard]] double inputGainForDrive(double driveDecibels)
{
    if (!std::isfinite(driveDecibels))
    {
        throw EffectError(EffectErrorCode::invalidDrive, "tape saturation drive must be finite");
    }
    const auto inputGain = DspConversions::decibelsToAmplitude(driveDecibels);
    if (!std::isfinite(inputGain))
    {
        throw EffectError(EffectErrorCode::invalidDrive,
                          "tape saturation drive produces an unrepresentable gain");
    }
    return inputGain;
}

[[nodiscard]] std::size_t tailFrames(std::uint64_t milliseconds, std::uint32_t sampleRate)
{
    const auto result = DspConversions::millisecondsToFrames(milliseconds, sampleRate);
    if (!result)
    {
        throw EffectError(EffectErrorCode::sizeOverflow,
                          "effect tail is too large for this platform");
    }
    return result.value;
}

[[nodiscard]] std::size_t checkedOutputSamples(std::size_t inputFrames, std::size_t tail,
                                               std::size_t channels)
{
    if (inputFrames > std::numeric_limits<std::size_t>::max() - tail)
    {
        throw EffectError(EffectErrorCode::sizeOverflow, "effect output frame count is too large");
    }
    const auto frames = inputFrames + tail;
    if (channels != 0 && frames > std::vector<double>{}.max_size() / channels)
    {
        throw EffectError(EffectErrorCode::sizeOverflow, "effect output sample count is too large");
    }
    return frames * channels;
}

void commitOutput(WavFile &wave, std::vector<double> output, std::size_t outputChannels)
{
    const auto inputChannels = static_cast<std::size_t>(wave.numberOfChannels());
    wave.setSamples(std::move(output));
    if (inputChannels != outputChannels)
    {
        wave.setNumberOfChannels(static_cast<std::uint16_t>(outputChannels));
    }
}

void applyDelayChannels(WavFile &wave, std::span<const DelayChannelSettings> channelSettings,
                        double mix, std::uint64_t tailMilliseconds,
                        const FeedbackProcessor &processFeedback)
{
    for (const auto &channel : channelSettings)
    {
        validateFeedback(channel.feedback);
    }
    validateMix(mix);

    const auto inputChannels = static_cast<std::size_t>(wave.numberOfChannels());
    const auto outputChannels = channelSettings.size();
    const auto inputSpan = wave.samples();
    const std::vector<double> input(inputSpan.begin(), inputSpan.end());
    const auto inputFrames = input.size() / inputChannels;
    const auto tail = tailFrames(tailMilliseconds, wave.sampleRate());
    const auto outputSamples = checkedOutputSamples(inputFrames, tail, outputChannels);
    const auto outputFrames = outputSamples / outputChannels;

    std::vector<Delay> delays;
    delays.reserve(outputChannels);
    for (const auto &channel : channelSettings)
    {
        delays.emplace_back(channel.delayMilliseconds, channel.delayMilliseconds,
                            wave.sampleRate());
    }

    std::vector<double> output;
    output.reserve(outputSamples);
    for (std::size_t frame = 0; frame < outputFrames; ++frame)
    {
        for (std::size_t channel = 0; channel < outputChannels; ++channel)
        {
            double inputSample{};
            if (frame < inputFrames)
            {
                const auto inputChannel = inputChannels == 1 ? 0 : channel;
                inputSample = input[frame * inputChannels + inputChannel];
            }
            output.push_back(delays[channel].nextSampleWith(
                inputSample, channelSettings[channel].feedback, mix,
                [&](double feedbackSample) { return processFeedback(channel, feedbackSample); }));
        }
    }

    commitOutput(wave, std::move(output), outputChannels);
}

void applyCommonDelay(WavFile &wave, const DelaySettings &settings,
                      const FeedbackProcessor &processFeedback)
{
    const std::vector<DelayChannelSettings> channels(wave.numberOfChannels(), settings.channel);
    applyDelayChannels(wave, channels, settings.mix, settings.tailMilliseconds, processFeedback);
}

void applyStereoDelay(WavFile &wave, const StereoDelaySettings &settings,
                      const FeedbackProcessor &processFeedback)
{
    const std::array channels{settings.left, settings.right};
    applyDelayChannels(wave, channels, settings.mix, settings.tailMilliseconds, processFeedback);
}

constexpr std::array<double, 4> combDelayMilliseconds{29.7, 37.1, 41.1, 43.7};
constexpr std::array<double, 2> allPassDelayMilliseconds{5.0, 1.7};
constexpr double allPassGain = 0.7;

class ReverbComb
{
  public:
    ReverbComb(double delayMilliseconds, std::uint32_t sampleRate, double feedback)
        : delay_(delayMilliseconds, delayMilliseconds, sampleRate), feedback_(feedback)
    {
    }

    [[nodiscard]] double nextSample(double input) noexcept
    {
        return delay_.nextSample(input, feedback_);
    }

  private:
    Delay delay_;
    double feedback_;
};

class SchroederAllPass
{
  public:
    SchroederAllPass(double delayMilliseconds, std::uint32_t sampleRate, double gain)
        : delay_(delayMilliseconds, delayMilliseconds, sampleRate), gain_(gain)
    {
    }

    [[nodiscard]] double nextSample(double input) noexcept
    {
        const auto delayed = delay_.nextSample(input, gain_);
        return delayed * (1.0 - gain_ * gain_) - gain_ * input;
    }

  private:
    Delay delay_;
    double gain_;
};

class ReverbChannel
{
  public:
    ReverbChannel(std::uint32_t sampleRate, double roomScale, double decaySeconds)
    {
        combs_.reserve(combDelayMilliseconds.size());
        for (const auto baseDelay : combDelayMilliseconds)
        {
            const auto delayMilliseconds = baseDelay * roomScale;
            const auto feedback = std::pow(0.001, delayMilliseconds / (decaySeconds * 1'000.0));
            combs_.emplace_back(delayMilliseconds, sampleRate, feedback);
        }

        allPasses_.reserve(allPassDelayMilliseconds.size());
        for (const auto baseDelay : allPassDelayMilliseconds)
        {
            const auto delayMilliseconds = baseDelay * roomScale;
            allPasses_.emplace_back(delayMilliseconds, sampleRate, allPassGain);
        }
    }

    [[nodiscard]] double nextSample(double input) noexcept
    {
        double output{};
        for (auto &comb : combs_)
        {
            output += comb.nextSample(input);
        }
        output /= static_cast<double>(combs_.size());
        for (auto &allPass : allPasses_)
        {
            output = allPass.nextSample(output);
        }
        return output;
    }

  private:
    std::vector<ReverbComb> combs_;
    std::vector<SchroederAllPass> allPasses_;
};

} // namespace

namespace FX
{

double saturateSample(double sample, double inputGain) noexcept
{
    return std::tanh(2.5 * sample * inputGain);
}

void saturate(WavFile &wave, double driveDecibels)
{
    const auto inputGain = inputGainForDrive(driveDecibels);
    for (auto &sample : wave.samples())
    {
        sample = saturateSample(sample, inputGain);
    }
}

void digitalDelay(WavFile &wave, const DelaySettings &settings)
{
    applyCommonDelay(wave, settings,
                     [](std::size_t, double feedbackSample) { return feedbackSample; });
}

void digitalDelay(WavFile &wave, const StereoDelaySettings &settings)
{
    applyStereoDelay(wave, settings,
                     [](std::size_t, double feedbackSample) { return feedbackSample; });
}

void analogDelay(WavFile &wave, const DelaySettings &settings, double cutoffHz)
{
    validateCutoff(cutoffHz, wave.sampleRate());
    std::vector<EQInstance> filters;
    filters.reserve(wave.numberOfChannels());
    for (std::size_t channel = 0; channel < wave.numberOfChannels(); ++channel)
    {
        filters.push_back(
            EQInstance::lowPass(cutoffHz, wave.sampleRate(), std::numbers::sqrt2 / 2));
    }
    applyCommonDelay(wave, settings, [&](std::size_t channel, double feedbackSample)
                     { return filters[channel].processSample(feedbackSample); });
}

void analogDelay(WavFile &wave, const StereoDelaySettings &settings, double cutoffHz)
{
    validateCutoff(cutoffHz, wave.sampleRate());
    std::array filters{EQInstance::lowPass(cutoffHz, wave.sampleRate(), std::numbers::sqrt2 / 2),
                       EQInstance::lowPass(cutoffHz, wave.sampleRate(), std::numbers::sqrt2 / 2)};
    applyStereoDelay(wave, settings, [&](std::size_t channel, double feedbackSample)
                     { return filters[channel].processSample(feedbackSample); });
}

void tapeDelay(WavFile &wave, const DelaySettings &settings, double driveDecibels, double cutoffHz)
{
    validateCutoff(cutoffHz, wave.sampleRate());
    const auto inputGain = inputGainForDrive(driveDecibels);
    std::vector<EQInstance> filters;
    filters.reserve(wave.numberOfChannels());
    for (std::size_t channel = 0; channel < wave.numberOfChannels(); ++channel)
    {
        filters.push_back(
            EQInstance::lowPass(cutoffHz, wave.sampleRate(), std::numbers::sqrt2 / 2));
    }
    applyCommonDelay(
        wave, settings, [&](std::size_t channel, double feedbackSample)
        { return filters[channel].processSample(saturateSample(feedbackSample, inputGain)); });
}

void tapeDelay(WavFile &wave, const StereoDelaySettings &settings, double driveDecibels,
               double cutoffHz)
{
    validateCutoff(cutoffHz, wave.sampleRate());
    const auto inputGain = inputGainForDrive(driveDecibels);
    std::array filters{EQInstance::lowPass(cutoffHz, wave.sampleRate(), std::numbers::sqrt2 / 2),
                       EQInstance::lowPass(cutoffHz, wave.sampleRate(), std::numbers::sqrt2 / 2)};
    applyStereoDelay(
        wave, settings, [&](std::size_t channel, double feedbackSample)
        { return filters[channel].processSample(saturateSample(feedbackSample, inputGain)); });
}

void delay(WavFile &wave, const DelaySettings &settings) { digitalDelay(wave, settings); }

void delay(WavFile &wave, const StereoDelaySettings &settings) { digitalDelay(wave, settings); }

void reverb(WavFile &wave, const ReverbSettings &settings)
{
    if (!std::isfinite(settings.roomSize) || settings.roomSize < 0.0 || settings.roomSize > 1.0)
    {
        throw EffectError(EffectErrorCode::invalidRoomSize,
                          "reverb room size must be finite and between zero and one");
    }
    if (!std::isfinite(settings.decaySeconds) || settings.decaySeconds <= 0.0)
    {
        throw EffectError(EffectErrorCode::invalidDecayTime,
                          "reverb RT60 decay must be finite and greater than zero");
    }
    validateMix(settings.mix);

    const auto channels = static_cast<std::size_t>(wave.numberOfChannels());
    const auto inputSpan = wave.samples();
    const std::vector<double> input(inputSpan.begin(), inputSpan.end());
    const auto inputFrames = input.size() / channels;
    const auto tail = tailFrames(settings.tailMilliseconds, wave.sampleRate());
    const auto outputSamples = checkedOutputSamples(inputFrames, tail, channels);
    const auto outputFrames = outputSamples / channels;

    std::vector<ReverbChannel> reverbs;
    reverbs.reserve(channels);
    const auto roomScale = 0.5 + settings.roomSize;
    for (std::size_t channel = 0; channel < channels; ++channel)
    {
        reverbs.emplace_back(wave.sampleRate(), roomScale, settings.decaySeconds);
    }

    std::vector<double> output;
    output.reserve(outputSamples);
    for (std::size_t frame = 0; frame < outputFrames; ++frame)
    {
        for (std::size_t channel = 0; channel < channels; ++channel)
        {
            const auto inputSample = frame < inputFrames ? input[frame * channels + channel] : 0.0;
            const auto wet = reverbs[channel].nextSample(inputSample);
            output.push_back(inputSample * (1.0 - settings.mix) + wet * settings.mix);
        }
    }
    wave.setSamples(std::move(output));
}

} // namespace FX

} // namespace wavlab
