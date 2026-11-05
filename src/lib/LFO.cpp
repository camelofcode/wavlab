#include <wavlab/LFO.h>

#include <cmath>
#include <cstdint>
#include <numbers>
#include <string>
#include <utility>

namespace wavlab
{

LfoError::LfoError(LfoErrorCode code, std::string message)
    : std::invalid_argument(std::move(message)), code_(code)
{
}

LfoErrorCode LfoError::code() const noexcept { return code_; }

LFO::LFO(std::uint32_t sampleRate, double frequencyHz, double minimum, double maximum,
         LfoWaveform waveform)
    : minimum_(minimum), range_(maximum - minimum), waveform_(waveform)
{
    if (sampleRate == 0)
    {
        throw LfoError(LfoErrorCode::invalidSampleRate,
                       "LFO sample rate must be greater than zero");
    }
    if (!std::isfinite(frequencyHz) || frequencyHz < 0.0)
    {
        throw LfoError(LfoErrorCode::invalidFrequency,
                       "LFO frequency must be finite and non-negative");
    }
    if (!std::isfinite(minimum) || !std::isfinite(maximum) || minimum > maximum ||
        !std::isfinite(range_))
    {
        throw LfoError(LfoErrorCode::invalidRange, "LFO range must have finite ordered bounds");
    }
    radiansPerSample_ = std::numbers::pi * frequencyHz / sampleRate;
    if (!std::isfinite(radiansPerSample_))
    {
        throw LfoError(LfoErrorCode::invalidFrequency,
                       "LFO frequency is too large to represent its phase increment");
    }
}

double LFO::nextSample() noexcept
{
    double normalised{};
    switch (waveform_)
    {
    case LfoWaveform::sine:
        normalised = std::abs(std::sin(phase_));
        break;
    case LfoWaveform::triangle:
        normalised = 1.0 - std::abs(2.0 * phase_ / std::numbers::pi - 1.0);
        break;
    case LfoWaveform::square:
        normalised = phase_ >= std::numbers::pi / 2.0 ? 1.0 : 0.0;
        break;
    }
    phase_ = std::fmod(phase_ + radiansPerSample_, std::numbers::pi);
    return minimum_ + normalised * range_;
}

} // namespace wavlab
