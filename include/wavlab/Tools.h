#pragma once

#include <wavlab/WavFile.h>

#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace wavlab
{

enum class ProcessingErrorCode
{
    expectedMono,
    expectedStereo,
    sampleRateMismatch,
    channelCountMismatch,
    tooFewInputs,
    invalidSplitTime,
    sizeOverflow,
};

class ProcessingError : public std::invalid_argument
{
  public:
    ProcessingError(ProcessingErrorCode code, std::string message);

    [[nodiscard]] ProcessingErrorCode code() const noexcept;

  private:
    ProcessingErrorCode code_;
};

namespace Tools
{

/// Scales loaded samples to a peak amplitude of 1.0. Silence is unchanged.
void normalise(WavFile &wave);

/// US-English compatibility alias for normalise().
void normalize(WavFile &wave);

/// Scales loaded samples to the requested peak amplitude. Silence is unchanged.
void normaliseToAmplitude(WavFile &wave, double targetAmplitude);

/// Compatibility overload whose second argument is a linear target amplitude.
void normalise(WavFile &wave, double targetAmplitude);

/// US-English compatibility alias for normaliseToAmplitude().
void normalizeToAmplitude(WavFile &wave, double targetAmplitude);

/// Scales loaded samples to the requested peak level in dBFS.
void normaliseToDecibels(WavFile &wave, double targetDecibels);

/// US-English compatibility alias for normaliseToDecibels().
void normalizeToDecibels(WavFile &wave, double targetDecibels);

/// Reverses sample frames while retaining the channel order within each frame.
void reverse(WavFile &wave);

/// Inverts the polarity of every loaded sample.
void flipPhase(WavFile &wave);

/// Swaps the left and right samples of every stereo frame. Mono is unchanged.
void swapChannels(WavFile &wave);

/// Splits a stereo file into left and right mono files.
[[nodiscard]] std::pair<WavFile, WavFile> splitStereo(const WavFile &stereo);

/// Combines mono left and right files, silence-padding the shorter input.
[[nodiscard]] WavFile combineLeftRight(const WavFile &left, const WavFile &right);

/// Splits a stereo file into mono mid and side files.
[[nodiscard]] std::pair<WavFile, WavFile> splitStereoMidSide(const WavFile &stereo);

/// Decodes mono mid and side files, silence-padding the shorter input.
[[nodiscard]] WavFile combineMidSide(const WavFile &mid, const WavFile &side);

/// Multiplies every loaded sample by a linear gain factor.
void gain(WavFile &wave, double gainFactor);

/// Applies an amplitude gain measured in decibels.
void decibelGain(WavFile &wave, double decibels);

/// Returns the greatest absolute sample level in dBFS.
[[nodiscard]] double getPeakDecibels(const WavFile &wave);

/// Returns the RMS level across all interleaved samples in dBFS.
[[nodiscard]] double getRMSDecibels(const WavFile &wave);

/// Mixes at least two compatible files, with mono promotion and silence padding.
[[nodiscard]] WavFile mix(std::span<const WavFile> inputs);

/// Splits a file at increasing millisecond positions on frame boundaries.
[[nodiscard]] std::vector<WavFile> split(const WavFile &input,
                                         std::span<const std::uint64_t> splitTimesMs);

/// Joins at least two files with matching sample rates and channel counts.
[[nodiscard]] WavFile join(std::span<const WavFile> inputs);

} // namespace Tools

} // namespace wavlab
