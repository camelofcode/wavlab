#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

namespace wavlab
{

enum class LfoWaveform
{
    sine,
    triangle,
    square,
};

enum class LfoErrorCode
{
    invalidSampleRate,
    invalidFrequency,
    invalidRange,
};

class LfoError : public std::invalid_argument
{
  public:
    LfoError(LfoErrorCode code, std::string message);

    [[nodiscard]] LfoErrorCode code() const noexcept;

  private:
    LfoErrorCode code_;
};

/// A stateful unipolar oscillator scaled to an inclusive output range.
class LFO
{
  public:
    LFO(std::uint32_t sampleRate, double frequencyHz, double minimum, double maximum,
        LfoWaveform waveform);

    /// Returns the current value and advances the phase by one audio sample.
    [[nodiscard]] double nextSample() noexcept;

  private:
    double phase_{};
    double radiansPerSample_{};
    double minimum_{};
    double range_{};
    LfoWaveform waveform_{};
};

} // namespace wavlab
