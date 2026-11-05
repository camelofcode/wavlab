#include <wavlab/FX.h>

#include <wavlab/EQ.h>
#include <wavlab/LFO.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace wavlab::FX
{
namespace
{

constexpr double phaserMaximumSweepOctaves = 2.0;
constexpr double phaserBandwidthOctaves = 1.0;
constexpr double nyquistMargin = 0.999999;

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

void validatePhaserChannel(const PhaserChannelSettings &settings, std::uint32_t sampleRate)
{
    if (!std::isfinite(settings.depth) || settings.depth < 0.0 || settings.depth > 1.0)
    {
        throw EffectError(EffectErrorCode::invalidPhaserDepth,
                          "phaser depth must be finite and between zero and one");
    }
    validateLfoFrequency(settings.frequencyHz);
    const auto nyquist = static_cast<double>(sampleRate) / 2.0;
    if (!std::isfinite(settings.centreFrequencyHz) || settings.centreFrequencyHz <= 0.0 ||
        settings.centreFrequencyHz >= nyquist)
    {
        throw EffectError(EffectErrorCode::invalidPhaserCentreFrequency,
                          "phaser centre frequency must be greater than zero and below Nyquist");
    }
}

void validatePhaser(std::size_t stages, double mix)
{
    if (stages == 0 || stages > maximumPhaserStages)
    {
        throw EffectError(EffectErrorCode::invalidPhaserStages,
                          "phaser stages must be between 1 and 16");
    }
    validateMix(mix);
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

class PhaserChannel
{
  public:
    PhaserChannel(const PhaserChannelSettings &settings, std::size_t stages,
                  std::uint32_t sampleRate, double mix)
        : lfo_(sampleRate, settings.frequencyHz, -1.0, 1.0, settings.waveform),
          sampleRate_(sampleRate), centreFrequencyHz_(settings.centreFrequencyHz),
          depth_(settings.depth), mix_(mix)
    {
        stages_.reserve(stages);
        for (std::size_t stage = 0; stage < stages; ++stage)
        {
            stages_.push_back(EQInstance::allPass(settings.centreFrequencyHz, sampleRate,
                                                  phaserBandwidthOctaves));
        }
    }

    [[nodiscard]] double nextSample(double input)
    {
        const auto sweepOctaves = lfo_.nextSample() * depth_ * phaserMaximumSweepOctaves;
        const auto nyquistLimit = static_cast<double>(sampleRate_) / 2.0 * nyquistMargin;
        const auto frequency = std::min(centreFrequencyHz_ * std::exp2(sweepOctaves), nyquistLimit);
        auto wet = input;
        for (auto &stage : stages_)
        {
            stage.setAllPass(frequency, sampleRate_, phaserBandwidthOctaves);
            wet = stage.processSample(wet);
        }
        return input * (1.0 - mix_) + wet * mix_;
    }

  private:
    std::vector<EQInstance> stages_;
    LFO lfo_;
    std::uint32_t sampleRate_;
    double centreFrequencyHz_;
    double depth_;
    double mix_;
};

template <typename SettingsContainer>
void applyPhaser(WavFile &wave, const SettingsContainer &settings, std::size_t stages, double mix)
{
    const auto inputChannels = static_cast<std::size_t>(wave.numberOfChannels());
    const auto outputChannels = settings.size();
    const auto inputSpan = wave.samples();
    const std::vector<double> input(inputSpan.begin(), inputSpan.end());

    std::vector<PhaserChannel> processors;
    processors.reserve(outputChannels);
    for (const auto &channel : settings)
    {
        processors.emplace_back(channel, stages, wave.sampleRate(), mix);
    }

    std::vector<double> output;
    output.reserve(checkedOutputSamples(wave.frameCount(), outputChannels, "phaser"));
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

void phaser(WavFile &wave, const PhaserSettings &settings)
{
    validatePhaserChannel(settings.channel, wave.sampleRate());
    validatePhaser(settings.stages, settings.mix);
    const std::vector<PhaserChannelSettings> channels(wave.numberOfChannels(), settings.channel);
    applyPhaser(wave, channels, settings.stages, settings.mix);
}

void phaser(WavFile &wave, const StereoPhaserSettings &settings)
{
    validatePhaserChannel(settings.left, wave.sampleRate());
    validatePhaserChannel(settings.right, wave.sampleRate());
    validatePhaser(settings.stages, settings.mix);
    const std::array channels{settings.left, settings.right};
    applyPhaser(wave, channels, settings.stages, settings.mix);
}

void modulatedFilter(WavFile &wave, const ModulatedFilterSettings &settings)
{
    validateLfoFrequency(settings.frequencyHz);
    const auto nyquist = static_cast<double>(wave.sampleRate()) / 2.0;
    if (!std::isfinite(settings.minimumFrequencyHz) ||
        !std::isfinite(settings.maximumFrequencyHz) || settings.minimumFrequencyHz <= 0.0 ||
        settings.minimumFrequencyHz > settings.maximumFrequencyHz ||
        settings.maximumFrequencyHz >= nyquist)
    {
        throw EffectError(EffectErrorCode::invalidModulatedFilterFrequencyRange,
                          "modulated filter frequencies must be ordered, greater than zero, and "
                          "below Nyquist");
    }
    if (!std::isfinite(settings.q) || settings.q <= 0.0)
    {
        throw EffectError(EffectErrorCode::invalidModulatedFilterQ,
                          "modulated filter Q must be finite and greater than zero");
    }
    if (settings.filterType != ModulatedFilterType::lowPass &&
        settings.filterType != ModulatedFilterType::highPass)
    {
        throw EffectError(EffectErrorCode::invalidModulatedFilterType,
                          "unknown modulated filter type");
    }

    const auto channels = static_cast<std::size_t>(wave.numberOfChannels());
    if (channels == 0)
    {
        throw std::logic_error("a WAV file must have at least one channel");
    }
    std::vector<LFO> lfos;
    std::vector<EQInstance> filters;
    lfos.reserve(channels);
    filters.reserve(channels);
    for (std::size_t channel = 0; channel < channels; ++channel)
    {
        lfos.emplace_back(wave.sampleRate(), settings.frequencyHz, settings.minimumFrequencyHz,
                          settings.maximumFrequencyHz, settings.waveform);
        if (settings.filterType == ModulatedFilterType::lowPass)
        {
            filters.push_back(
                EQInstance::lowPass(settings.minimumFrequencyHz, wave.sampleRate(), settings.q));
        }
        else
        {
            filters.push_back(
                EQInstance::highPass(settings.minimumFrequencyHz, wave.sampleRate(), settings.q));
        }
    }

    const auto input = wave.samples();
    std::vector<double> output(input.begin(), input.end());
    for (std::size_t index = 0; index < output.size(); ++index)
    {
        const auto channel = index % channels;
        const auto frequency = lfos[channel].nextSample();
        if (settings.filterType == ModulatedFilterType::lowPass)
        {
            filters[channel].setLowPass(frequency, wave.sampleRate(), settings.q);
        }
        else
        {
            filters[channel].setHighPass(frequency, wave.sampleRate(), settings.q);
        }
        output[index] = filters[channel].processSample(output[index]);
    }
    wave.setSamples(std::move(output));
}

} // namespace wavlab::FX
