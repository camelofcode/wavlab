#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace wavlab
{

enum class DelayErrorCode
{
    invalidSampleRate,
    invalidDelayTime,
    delayTooShort,
    delayExceedsMaximum,
    sizeOverflow,
};

class DelayError : public std::invalid_argument
{
  public:
    DelayError(DelayErrorCode code, std::string message);

    [[nodiscard]] DelayErrorCode code() const noexcept;

  private:
    DelayErrorCode code_;
};

/// A stateful single-channel delay line backed by a checked ring buffer.
class Delay
{
  public:
    using FeedbackProcessor = std::function<double(double)>;

    /// Both times are in milliseconds and are rounded to the nearest sample.
    Delay(double maximumMilliseconds, double initialMilliseconds, std::uint32_t sampleRate);

    /// Changes the delay without clearing buffered samples.
    void setDelayTime(double milliseconds);

    /// Processes one sample with feedback and wet/dry mix.
    [[nodiscard]] double nextSample(double sample, double feedback, double mix = 1.0) noexcept;

    /// Processes one sample and transforms the scaled feedback before storing it.
    [[nodiscard]] double nextSampleWith(double sample, double feedback, double mix,
                                        const FeedbackProcessor &feedbackProcessor);

    /// Processes one sample with a temporary non-negative offset that shortens the delay.
    [[nodiscard]] double nextSampleWithOffset(double sample, std::size_t offsetSamples,
                                              double mix) noexcept;

    /// Processes one sample with both a modulation offset and feedback.
    [[nodiscard]] double nextSampleWithOffsetAndFeedback(double sample, std::size_t offsetSamples,
                                                         double feedback, double mix) noexcept;

    [[nodiscard]] std::size_t delaySamples() const noexcept;
    [[nodiscard]] std::size_t capacitySamples() const noexcept;

  private:
    [[nodiscard]] double delayedSample(std::size_t offsetSamples) const noexcept;
    void advance() noexcept;

    std::uint32_t sampleRate_;
    double maximumMilliseconds_;
    std::size_t delaySamples_;
    std::size_t writePosition_{};
    std::vector<double> buffer_;
};

} // namespace wavlab
