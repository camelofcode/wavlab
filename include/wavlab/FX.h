#pragma once

#include <wavlab/WavFile.h>

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

namespace FX
{

inline constexpr double defaultDelayCutoffHz = 2'000.0;

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

} // namespace FX

} // namespace wavlab
