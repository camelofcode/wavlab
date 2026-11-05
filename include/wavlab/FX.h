#pragma once

#include <wavlab/LFO.h>
#include <wavlab/WavFile.h>

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace wavlab
{

enum class EffectErrorCode
{
    invalidFeedback,
    invalidMix,
    invalidCutoffFrequency,
    invalidDrive,
    invalidRoomSize,
    invalidDecayTime,
    invalidChorusDepth,
    invalidLfoFrequency,
    invalidFlangerDepth,
    invalidFlangerFrequency,
    invalidFlangerFeedback,
    invalidTremoloDepth,
    invalidAutoPanDepth,
    invalidPhaserDepth,
    invalidPhaserCentreFrequency,
    invalidPhaserStages,
    invalidModulatedFilterFrequencyRange,
    invalidModulatedFilterQ,
    invalidModulatedFilterType,
    sizeOverflow,
};

class EffectError : public std::invalid_argument
{
  public:
    EffectError(EffectErrorCode code, std::string message);

    [[nodiscard]] EffectErrorCode code() const noexcept;

  private:
    EffectErrorCode code_;
};

/// Delay time and feedback for one independently processed channel.
struct DelayChannelSettings
{
    /// Fractional milliseconds are supported and round to the nearest sample.
    double delayMilliseconds;
    /// Signed feedback with magnitude strictly below one.
    double feedback;
};

/// Settings shared by every channel of a mono/common delay operation.
struct DelaySettings
{
    DelayChannelSettings channel;
    /// Wet proportion in the inclusive range zero to one.
    double mix;
    /// Additional output duration, rather than an absolute output length.
    std::uint64_t tailMilliseconds{};
};

/// Independent left/right settings. Mono input is promoted to stereo.
struct StereoDelaySettings
{
    DelayChannelSettings left;
    DelayChannelSettings right;
    /// Wet proportion in the inclusive range zero to one.
    double mix;
    /// Additional output duration, rather than an absolute output length.
    std::uint64_t tailMilliseconds{};
};

struct ReverbSettings
{
    /// Normalised room size in the inclusive range zero to one.
    double roomSize;
    /// RT60 decay time in seconds.
    double decaySeconds;
    /// Wet proportion in the inclusive range zero to one.
    double mix;
    /// Additional output duration for the reverb tail.
    std::uint64_t tailMilliseconds{};
};

struct ChorusChannelSettings
{
    /// Normalised modulation depth in the inclusive range zero to one.
    double depth;
    /// LFO frequency in hertz. Zero selects a stationary maximum delay.
    double frequencyHz;
};

struct ChorusSettings
{
    ChorusChannelSettings channel;
    /// Wet proportion in the inclusive range zero to one.
    double mix;
};

/// Independent left/right settings. Mono input is promoted to stereo.
struct StereoChorusSettings
{
    ChorusChannelSettings left;
    ChorusChannelSettings right;
    /// Wet proportion in the inclusive range zero to one.
    double mix;
};

using VibratoChannelSettings = ChorusChannelSettings;

struct VibratoSettings
{
    /// Depth and LFO frequency for every existing channel.
    VibratoChannelSettings channel;
};

struct StereoVibratoSettings
{
    /// Independent settings; mono input is promoted to stereo.
    VibratoChannelSettings left;
    VibratoChannelSettings right;
};

struct FlangerChannelSettings
{
    /// Normalised modulation depth in the inclusive range zero to one.
    double depth;
    /// LFO frequency in hertz, from 0.1 to 10 inclusive.
    double frequencyHz;
    /// Signed feedback in the inclusive range -0.95 to 0.95.
    double feedback;
};

struct FlangerSettings
{
    FlangerChannelSettings channel;
    /// Wet proportion in the inclusive range zero to one.
    double mix;
};

/// Independent left/right settings. Mono input is promoted to stereo.
struct StereoFlangerSettings
{
    FlangerChannelSettings left;
    FlangerChannelSettings right;
    /// Wet proportion in the inclusive range zero to one.
    double mix;
};

struct TremoloSettings
{
    /// Normalised modulation depth in the inclusive range zero to one.
    double depth;
    /// Non-negative LFO frequency in hertz.
    double frequencyHz;
    /// Gain-modulation waveform.
    LfoWaveform waveform{LfoWaveform::sine};
};

struct AutoPanSettings
{
    /// Normalised modulation depth in the inclusive range zero to one.
    double depth;
    /// Non-negative LFO frequency in hertz.
    double frequencyHz;
    /// Panning waveform.
    LfoWaveform waveform{LfoWaveform::sine};
};

struct PhaserChannelSettings
{
    /// Normalised sweep depth in the inclusive range zero to one.
    double depth;
    /// Non-negative LFO frequency in hertz.
    double frequencyHz;
    /// Geometric sweep centre in hertz, greater than zero and below Nyquist.
    double centreFrequencyHz;
    /// Frequency-sweep waveform.
    LfoWaveform waveform{LfoWaveform::sine};
};

struct PhaserSettings
{
    PhaserChannelSettings channel;
    /// Number of second-order all-pass stages, from one to sixteen.
    std::size_t stages{3};
    /// Wet proportion in the inclusive range zero to one.
    double mix;
};

/// Independent left/right settings. Mono input is promoted to stereo.
struct StereoPhaserSettings
{
    PhaserChannelSettings left;
    PhaserChannelSettings right;
    /// Common number of second-order all-pass stages, from one to sixteen.
    std::size_t stages{3};
    /// Common wet proportion in the inclusive range zero to one.
    double mix;
};

enum class ModulatedFilterType
{
    lowPass,
    highPass,
};

struct ModulatedFilterSettings
{
    /// Lowest cutoff in hertz; must be greater than zero.
    double minimumFrequencyHz;
    /// Highest cutoff in hertz; must be at least the minimum and below Nyquist.
    double maximumFrequencyHz;
    /// Non-negative LFO frequency in hertz.
    double frequencyHz;
    /// Low-pass or high-pass response.
    ModulatedFilterType filterType{ModulatedFilterType::lowPass};
    /// Cutoff-modulation waveform.
    LfoWaveform waveform{LfoWaveform::sine};
    /// Resonance Q, finite and greater than zero.
    double q;
};

namespace FX
{

inline constexpr double defaultDelayCutoffHz = 2'000.0;
inline constexpr std::size_t defaultPhaserStages = 3;
inline constexpr std::size_t maximumPhaserStages = 16;

/// Applies sigmoid tape saturation with pre-saturation input gain in dB.
void saturate(WavFile &wave, double driveDecibels);

/// Applies the saturation curve using a precomputed linear input gain.
[[nodiscard]] double saturateSample(double sample, double inputGain) noexcept;

/// Applies a clean delay independently to every existing channel.
void digitalDelay(WavFile &wave, const DelaySettings &settings);

/// Applies a clean stereo delay, promoting mono input when necessary.
void digitalDelay(WavFile &wave, const StereoDelaySettings &settings);

/// Applies a delay with an independent low-pass feedback path per channel.
void analogDelay(WavFile &wave, const DelaySettings &settings,
                 double cutoffHz = defaultDelayCutoffHz);

/// Applies a stereo analog delay with independent channel state.
void analogDelay(WavFile &wave, const StereoDelaySettings &settings,
                 double cutoffHz = defaultDelayCutoffHz);

/// Applies saturation then low-pass filtering in each feedback path.
void tapeDelay(WavFile &wave, const DelaySettings &settings, double driveDecibels = 0.0,
               double cutoffHz = defaultDelayCutoffHz);

/// Applies a stereo tape delay with independent channel state.
void tapeDelay(WavFile &wave, const StereoDelaySettings &settings, double driveDecibels = 0.0,
               double cutoffHz = defaultDelayCutoffHz);

/// Compatibility name for the clean digital delay.
void delay(WavFile &wave, const DelaySettings &settings);
void delay(WavFile &wave, const StereoDelaySettings &settings);

/// Applies a four-comb, two-all-pass Schroeder reverb independently per channel.
void reverb(WavFile &wave, const ReverbSettings &settings);

/// Applies a modulated short delay independently to every existing channel.
void chorus(WavFile &wave, const ChorusSettings &settings);
/// Applies independent left/right chorus, promoting mono input when necessary.
void chorus(WavFile &wave, const StereoChorusSettings &settings);

/// Fully wet chorus, producing pitch modulation without the dry signal.
void vibrato(WavFile &wave, const VibratoSettings &settings);
void vibrato(WavFile &wave, const StereoVibratoSettings &settings);

/// Applies a triangle-modulated short delay with stable signed feedback.
void flanger(WavFile &wave, const FlangerSettings &settings);
void flanger(WavFile &wave, const StereoFlangerSettings &settings);

/// Applies one shared gain LFO to every existing channel.
void tremolo(WavFile &wave, const TremoloSettings &settings);

/// Pans mono or stereo input across an always-stereo output.
void autoPan(WavFile &wave, const AutoPanSettings &settings);

/// Applies a cascade of LFO-modulated all-pass filters.
void phaser(WavFile &wave, const PhaserSettings &settings);
void phaser(WavFile &wave, const StereoPhaserSettings &settings);

/// Applies an independently modulated low- or high-pass filter to each channel.
void modulatedFilter(WavFile &wave, const ModulatedFilterSettings &settings);

} // namespace FX

} // namespace wavlab
