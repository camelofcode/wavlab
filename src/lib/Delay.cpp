#include <wavlab/Delay.h>

#include "DspConversions.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

namespace wavlab
{

DelayError::DelayError(DelayErrorCode code, std::string message)
    : std::invalid_argument(std::move(message)), code_(code)
{
}

DelayErrorCode DelayError::code() const noexcept { return code_; }

namespace
{

[[noreturn]] void throwTimeError(DspConversions::TimeError error, double milliseconds,
                                 std::uint32_t sampleRate)
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
    throw DelayError(DelayErrorCode::invalidDelayTime,
                     "invalid delay time " + std::to_string(milliseconds));
}

[[nodiscard]] std::size_t checkedSamples(double milliseconds, std::uint32_t sampleRate)
{
    const auto result = DspConversions::millisecondsToSamples(milliseconds, sampleRate);
    if (!result)
    {
        throwTimeError(result.error, milliseconds, sampleRate);
    }
    return result.value;
}

[[nodiscard]] std::size_t checkedBufferSamples(double milliseconds, std::uint32_t sampleRate)
{
    const auto samples = checkedSamples(milliseconds, sampleRate);
    if (samples > std::vector<double>{}.max_size())
    {
        throw DelayError(DelayErrorCode::sizeOverflow,
                         "delay buffer is too large for this platform");
    }
    return samples;
}

[[nodiscard]] double mixSamples(double input, double delayed, double mix) noexcept
{
    return delayed * mix + input * (1.0 - mix);
}

} // namespace

Delay::Delay(double maximumMilliseconds, double initialMilliseconds, std::uint32_t sampleRate)
    : sampleRate_(sampleRate), maximumMilliseconds_(maximumMilliseconds),
      delaySamples_(checkedSamples(initialMilliseconds, sampleRate)),
      buffer_(checkedBufferSamples(maximumMilliseconds, sampleRate), 0.0)
{
    if (initialMilliseconds > maximumMilliseconds || delaySamples_ > buffer_.size())
    {
        throw DelayError(DelayErrorCode::delayExceedsMaximum,
                         "initial delay exceeds the configured maximum");
    }
}

void Delay::setDelayTime(double milliseconds)
{
    const auto newDelaySamples = checkedSamples(milliseconds, sampleRate_);
    if (milliseconds > maximumMilliseconds_ || newDelaySamples > buffer_.size())
    {
        throw DelayError(DelayErrorCode::delayExceedsMaximum,
                         "delay time exceeds the configured maximum");
    }
    delaySamples_ = newDelaySamples;
}

double Delay::nextSample(double sample, double feedback, double mix) noexcept
{
    const auto output = delayedSample(0);
    buffer_[writePosition_] = sample + feedback * output;
    advance();
    return mixSamples(sample, output, mix);
}

double Delay::nextSampleWith(double sample, double feedback, double mix,
                             const FeedbackProcessor &feedbackProcessor)
{
    const auto output = delayedSample(0);
    buffer_[writePosition_] = sample + feedbackProcessor(feedback * output);
    advance();
    return mixSamples(sample, output, mix);
}

double Delay::nextSampleWithOffset(double sample, std::size_t offsetSamples, double mix) noexcept
{
    return nextSampleWithOffsetAndFeedback(sample, offsetSamples, 0.0, mix);
}

double Delay::nextSampleWithOffsetAndFeedback(double sample, std::size_t offsetSamples,
                                              double feedback, double mix) noexcept
{
    const auto output = delayedSample(offsetSamples);
    buffer_[writePosition_] = sample + feedback * output;
    advance();
    return mixSamples(sample, output, mix);
}

std::size_t Delay::delaySamples() const noexcept { return delaySamples_; }

std::size_t Delay::capacitySamples() const noexcept { return buffer_.size(); }

double Delay::delayedSample(std::size_t offsetSamples) const noexcept
{
    const auto readPosition = (writePosition_ + buffer_.size() - delaySamples_) % buffer_.size();
    const auto offset = offsetSamples % buffer_.size();
    return buffer_[(readPosition + offset) % buffer_.size()];
}

void Delay::advance() noexcept { writePosition_ = (writePosition_ + 1) % buffer_.size(); }

} // namespace wavlab
