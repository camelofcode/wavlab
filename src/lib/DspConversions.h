#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace wavlab::DspConversions
{

enum class TimeError
{
    none,
    invalidSampleRate,
    invalidMilliseconds,
    shorterThanOneSample,
    sizeOverflow,
};

struct SampleCount
{
    std::size_t value{};
    TimeError error{TimeError::none};

    [[nodiscard]] explicit operator bool() const noexcept { return error == TimeError::none; }
};

[[nodiscard]] inline SampleCount millisecondsToSamples(double milliseconds,
                                                       std::uint32_t sampleRate) noexcept
{
    if (sampleRate == 0)
    {
        return {0, TimeError::invalidSampleRate};
    }
    if (!std::isfinite(milliseconds) || milliseconds <= 0.0)
    {
        return {0, TimeError::invalidMilliseconds};
    }

    const auto samples = std::round(static_cast<long double>(milliseconds) * sampleRate / 1'000.0L);
    if (samples < 1.0L)
    {
        return {0, TimeError::shorterThanOneSample};
    }
    if (samples > static_cast<long double>(std::numeric_limits<std::size_t>::max()))
    {
        return {0, TimeError::sizeOverflow};
    }
    return {static_cast<std::size_t>(samples), TimeError::none};
}

[[nodiscard]] inline SampleCount millisecondsToFrames(std::uint64_t milliseconds,
                                                      std::uint32_t sampleRate) noexcept
{
    if (sampleRate == 0)
    {
        return {0, TimeError::invalidSampleRate};
    }
    if (milliseconds > std::numeric_limits<std::uint64_t>::max() / sampleRate)
    {
        return {0, TimeError::sizeOverflow};
    }
    const auto frames = milliseconds * sampleRate / 1'000;
    if (frames > std::numeric_limits<std::size_t>::max())
    {
        return {0, TimeError::sizeOverflow};
    }
    return {static_cast<std::size_t>(frames), TimeError::none};
}

[[nodiscard]] inline double samplesToMilliseconds(std::size_t samples,
                                                  std::uint32_t sampleRate) noexcept
{
    return static_cast<double>(samples) * 1'000.0 / sampleRate;
}

[[nodiscard]] inline double decibelsToAmplitude(double decibels) noexcept
{
    return std::pow(10.0, decibels / 20.0);
}

} // namespace wavlab::DspConversions
