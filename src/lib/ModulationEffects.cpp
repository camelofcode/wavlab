#include <wavlab/FX.h>

#include "DspConversions.h"

#include <wavlab/Delay.h>
#include <wavlab/LFO.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace wavlab::FX
{
namespace
{

constexpr double chorusCentreDelayMilliseconds = 14.0;
constexpr double chorusMaximumDepthMilliseconds = 10.0;
constexpr double flangerMaximumDelayMilliseconds = 10.0;
constexpr double flangerMaximumFeedback = 0.95;
constexpr double flangerMinimumFrequencyHz = 0.1;
constexpr double flangerMaximumFrequencyHz = 10.0;

void validateMix(double mix)
{
    if (!std::isfinite(mix) || mix < 0.0 || mix > 1.0)
    {
        throw EffectError(EffectErrorCode::invalidMix,
                          "effect mix must be finite and between zero and one");
    }
}

void validateLfoFrequency(double frequencyHz)
{
    if (!std::isfinite(frequencyHz) || frequencyHz < 0.0)
    {
        throw EffectError(EffectErrorCode::invalidLfoFrequency,
                          "LFO frequency must be finite and non-negative");
    }
}

void validateChorus(const ChorusChannelSettings &settings)
{
    if (!std::isfinite(settings.depth) || settings.depth < 0.0 || settings.depth > 1.0)
    {
        throw EffectError(EffectErrorCode::invalidChorusDepth,
                          "chorus depth must be finite and between zero and one");
    }
    validateLfoFrequency(settings.frequencyHz);
}

void validateFlanger(const FlangerChannelSettings &settings)
{
    if (!std::isfinite(settings.depth) || settings.depth < 0.0 || settings.depth > 1.0)
    {
        throw EffectError(EffectErrorCode::invalidFlangerDepth,
                          "flanger depth must be finite and between zero and one");
    }
    if (!std::isfinite(settings.frequencyHz) || settings.frequencyHz < flangerMinimumFrequencyHz ||
        settings.frequencyHz > flangerMaximumFrequencyHz)
    {
        throw EffectError(EffectErrorCode::invalidFlangerFrequency,
                          "flanger frequency must be finite and between 0.1 and 10 Hz");
    }
    if (!std::isfinite(settings.feedback) || std::abs(settings.feedback) > flangerMaximumFeedback)
    {
        throw EffectError(EffectErrorCode::invalidFlangerFeedback,
                          "flanger feedback must be finite and between -0.95 and 0.95");
    }
}

[[noreturn]] void throwDelayTimeError(DspConversions::TimeError error, std::uint32_t sampleRate)
{
    switch (error)
    {
    case DspConversions::TimeError::invalidSampleRate:
        throw DelayError(DelayErrorCode::invalidSampleRate,
                         "delay sample rate must be greater than zero");
    case DspConversions::TimeError::invalidMilliseconds:
        throw DelayError(DelayErrorCode::invalidDelayTime,
                         "delay time must be finite and greater than zero");
    case DspConversions::TimeError::shorterThanOneSample:
        throw DelayError(DelayErrorCode::delayTooShort,
                         "delay time is shorter than one sample at " + std::to_string(sampleRate) +
                             " Hz");
    case DspConversions::TimeError::sizeOverflow:
        throw DelayError(DelayErrorCode::sizeOverflow,
                         "delay buffer is too large for this platform");
    case DspConversions::TimeError::none:
        break;
    }
    throw DelayError(DelayErrorCode::invalidDelayTime, "invalid delay time");
}

[[nodiscard]] std::size_t delaySamples(double milliseconds, std::uint32_t sampleRate)
{
    const auto result = DspConversions::millisecondsToSamples(milliseconds, sampleRate);
    if (!result)
    {
        throwDelayTimeError(result.error, sampleRate);
    }
    return result.value;
}

void commitOutput(WavFile &wave, std::vector<double> output, std::size_t channels)
{
    const auto oldChannels = static_cast<std::size_t>(wave.numberOfChannels());
    wave.setSamples(std::move(output));
    if (oldChannels != channels)
    {
        wave.setNumberOfChannels(static_cast<std::uint16_t>(channels));
    }
}

[[nodiscard]] std::size_t checkedOutputSamples(std::size_t frames, std::size_t channels,
                                               std::string_view effect)
{
    if (channels != 0 && frames > std::vector<double>{}.max_size() / channels)
    {
        throw EffectError(EffectErrorCode::sizeOverflow,
                          std::string(effect) + " output sample count is too large");
    }
    return frames * channels;
}

class ChorusChannel
{
  public:
    ChorusChannel(const ChorusChannelSettings &settings, std::uint32_t sampleRate, double mix)
        : delay_(maximumDelay(settings), maximumDelay(settings), sampleRate),
          lfo_(sampleRate, settings.frequencyHz, 0.0,
               static_cast<double>(maximumOffset(settings, sampleRate)), LfoWaveform::sine),
          mix_(mix)
    {
    }

    [[nodiscard]] double nextSample(double sample) noexcept
    {
        return delay_.nextSampleWithOffset(sample, roundedOffset(lfo_.nextSample()), mix_);
    }

  private:
    [[nodiscard]] static double maximumDelay(const ChorusChannelSettings &settings)
    {
        return chorusCentreDelayMilliseconds + settings.depth * chorusMaximumDepthMilliseconds;
    }

    [[nodiscard]] static std::size_t maximumOffset(const ChorusChannelSettings &settings,
                                                   std::uint32_t sampleRate)
    {
        const auto depth = settings.depth * chorusMaximumDepthMilliseconds;
        const auto minimumSamples = delaySamples(chorusCentreDelayMilliseconds - depth, sampleRate);
        const auto maximumSamples = delaySamples(chorusCentreDelayMilliseconds + depth, sampleRate);
        return maximumSamples - minimumSamples;
    }

    [[nodiscard]] static std::size_t roundedOffset(double offset) noexcept
    {
        return static_cast<std::size_t>(std::llround(offset));
    }

    Delay delay_;
    LFO lfo_;
    double mix_;
};

class FlangerChannel
{
  public:
    FlangerChannel(const FlangerChannelSettings &settings, std::uint32_t sampleRate, double mix)
        : delay_(maximumDelay(settings, sampleRate), maximumDelay(settings, sampleRate),
                 sampleRate),
          lfo_(sampleRate, settings.frequencyHz, 0.0,
               static_cast<double>(maximumOffset(settings, sampleRate)), LfoWaveform::triangle),
          feedback_(settings.feedback), mix_(mix)
    {
    }

    [[nodiscard]] double nextSample(double sample) noexcept
    {
        return delay_.nextSampleWithOffsetAndFeedback(
            sample, static_cast<std::size_t>(std::llround(lfo_.nextSample())), feedback_, mix_);
    }

  private:
    [[nodiscard]] static double minimumDelay(std::uint32_t sampleRate)
    {
        return DspConversions::samplesToMilliseconds(1, sampleRate);
    }

    [[nodiscard]] static double maximumDelay(const FlangerChannelSettings &settings,
                                             std::uint32_t sampleRate)
    {
        return std::max(settings.depth * flangerMaximumDelayMilliseconds, minimumDelay(sampleRate));
    }

    [[nodiscard]] static std::size_t maximumOffset(const FlangerChannelSettings &settings,
                                                   std::uint32_t sampleRate)
    {
        return delaySamples(maximumDelay(settings, sampleRate), sampleRate) - 1;
    }

    Delay delay_;
    LFO lfo_;
    double feedback_;
    double mix_;
};

template <typename Processor, typename SettingsContainer>
void applyChannels(WavFile &wave, const SettingsContainer &settings, double mix)
{
    const auto inputChannels = static_cast<std::size_t>(wave.numberOfChannels());
    const auto outputChannels = settings.size();
    const auto inputSpan = wave.samples();
    const std::vector<double> input(inputSpan.begin(), inputSpan.end());

    std::vector<Processor> processors;
    processors.reserve(outputChannels);
    for (const auto &channel : settings)
    {
        processors.emplace_back(channel, wave.sampleRate(), mix);
    }

    std::vector<double> output;
    output.reserve(checkedOutputSamples(wave.frameCount(), outputChannels, "modulation effect"));
    for (std::size_t frame = 0; frame < wave.frameCount(); ++frame)
    {
        for (std::size_t channel = 0; channel < outputChannels; ++channel)
        {
            const auto inputChannel = inputChannels == 1 ? 0 : channel;
            output.push_back(
                processors[channel].nextSample(input[frame * inputChannels + inputChannel]));
        }
    }
    commitOutput(wave, std::move(output), outputChannels);
}

} // namespace

void chorus(WavFile &wave, const ChorusSettings &settings)
{
    validateChorus(settings.channel);
    validateMix(settings.mix);
    const std::vector<ChorusChannelSettings> channels(wave.numberOfChannels(), settings.channel);
    applyChannels<ChorusChannel>(wave, channels, settings.mix);
}

void chorus(WavFile &wave, const StereoChorusSettings &settings)
{
    validateChorus(settings.left);
    validateChorus(settings.right);
    validateMix(settings.mix);
    const std::array channels{settings.left, settings.right};
    applyChannels<ChorusChannel>(wave, channels, settings.mix);
}

void vibrato(WavFile &wave, const VibratoSettings &settings)
{
    chorus(wave, ChorusSettings{settings.channel, 1.0});
}

void vibrato(WavFile &wave, const StereoVibratoSettings &settings)
{
    chorus(wave, StereoChorusSettings{settings.left, settings.right, 1.0});
}

void flanger(WavFile &wave, const FlangerSettings &settings)
{
    validateFlanger(settings.channel);
    validateMix(settings.mix);
    const std::vector<FlangerChannelSettings> channels(wave.numberOfChannels(), settings.channel);
    applyChannels<FlangerChannel>(wave, channels, settings.mix);
}

void flanger(WavFile &wave, const StereoFlangerSettings &settings)
{
    validateFlanger(settings.left);
    validateFlanger(settings.right);
    validateMix(settings.mix);
    const std::array channels{settings.left, settings.right};
    applyChannels<FlangerChannel>(wave, channels, settings.mix);
}

void tremolo(WavFile &wave, const TremoloSettings &settings)
{
    if (!std::isfinite(settings.depth) || settings.depth < 0.0 || settings.depth > 1.0)
    {
        throw EffectError(EffectErrorCode::invalidTremoloDepth,
                          "tremolo depth must be finite and between zero and one");
    }
    validateLfoFrequency(settings.frequencyHz);
    LFO lfo(wave.sampleRate(), settings.frequencyHz, 1.0 - settings.depth, 1.0, settings.waveform);

    const auto input = wave.samples();
    std::vector<double> output(input.begin(), input.end());
    const auto channels = static_cast<std::size_t>(wave.numberOfChannels());
    for (std::size_t frame = 0; frame < wave.frameCount(); ++frame)
    {
        const auto gain = lfo.nextSample();
        for (std::size_t channel = 0; channel < channels; ++channel)
        {
            output[frame * channels + channel] *= gain;
        }
    }
    wave.setSamples(std::move(output));
}

void autoPan(WavFile &wave, const AutoPanSettings &settings)
{
    if (!std::isfinite(settings.depth) || settings.depth < 0.0 || settings.depth > 1.0)
    {
        throw EffectError(EffectErrorCode::invalidAutoPanDepth,
                          "autopan depth must be finite and between zero and one");
    }
    validateLfoFrequency(settings.frequencyHz);
    LFO lfo(wave.sampleRate(), settings.frequencyHz, 0.0, 1.0, settings.waveform);

    const auto inputChannels = static_cast<std::size_t>(wave.numberOfChannels());
    const auto inputSpan = wave.samples();
    const std::vector<double> input(inputSpan.begin(), inputSpan.end());
    std::vector<double> output;
    output.reserve(checkedOutputSamples(wave.frameCount(), 2, "autopan"));
    for (std::size_t frame = 0; frame < wave.frameCount(); ++frame)
    {
        const auto position = lfo.nextSample();
        const auto leftGain = 1.0 - settings.depth * position;
        const auto rightGain = 1.0 - settings.depth * (1.0 - position);
        const auto left = input[frame * inputChannels];
        const auto right = input[frame * inputChannels + (inputChannels == 1 ? 0 : 1)];
        output.push_back(left * leftGain);
        output.push_back(right * rightGain);
    }
    commitOutput(wave, std::move(output), 2);
}

} // namespace wavlab::FX
