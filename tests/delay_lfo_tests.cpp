#include <wavlab/Delay.h>
#include <wavlab/LFO.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numbers>
#include <span>
#include <string_view>

namespace
{

constexpr double tolerance = 1e-12;
int failures = 0;

void check(bool condition, std::string_view message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

void checkNear(double actual, double expected, std::string_view message)
{
    check(std::abs(actual - expected) <= tolerance, message);
}

template <std::size_t Size>
void checkSamples(const std::array<double, Size> &actual, const std::array<double, Size> &expected,
                  std::string_view message)
{
    for (std::size_t index = 0; index < Size; ++index)
    {
        if (std::abs(actual[index] - expected[index]) > tolerance)
        {
            check(false, message);
            return;
        }
    }
}

template <typename Action>
void checkDelayError(Action action, wavlab::DelayErrorCode expected, std::string_view message)
{
    try
    {
        action();
        check(false, message);
    }
    catch (const wavlab::DelayError &error)
    {
        check(error.code() == expected, message);
    }
}

template <typename Action>
void checkLfoError(Action action, wavlab::LfoErrorCode expected, std::string_view message)
{
    try
    {
        action();
        check(false, message);
    }
    catch (const wavlab::LfoError &error)
    {
        check(error.code() == expected, message);
    }
}

void testDelayImpulseAndFeedback()
{
    using wavlab::Delay;

    Delay impulse(10.0, 3.0, 1'000);
    std::array<double, 5> input{1.0, 0.0, 0.0, 0.0, 0.0};
    auto output = input;
    output.fill(0.0);
    for (std::size_t index = 0; index < input.size(); ++index)
    {
        output[index] = impulse.nextSample(input[index], 0.0);
    }
    checkSamples(output, std::array{0.0, 0.0, 0.0, 1.0, 0.0},
                 "delay places an impulse at the requested sample");

    Delay feedback(10.0, 2.0, 1'000);
    const std::array<double, 7> feedbackInput{1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    auto feedbackOutput = feedbackInput;
    feedbackOutput.fill(0.0);
    for (std::size_t index = 0; index < feedbackInput.size(); ++index)
    {
        feedbackOutput[index] = feedback.nextSample(feedbackInput[index], 0.5);
    }
    checkSamples(feedbackOutput, std::array{0.0, 0.0, 1.0, 0.0, 0.5, 0.0, 0.25},
                 "delay feedback repeats decay at the delay interval");

    Delay mixed(10.0, 2.0, 1'000);
    checkNear(mixed.nextSample(1.0, 0.0, 0.25), 0.75,
              "delay mix retains the requested dry proportion");
    static_cast<void>(mixed.nextSample(0.0, 0.0, 0.25));
    checkNear(mixed.nextSample(0.0, 0.0, 0.25), 0.25,
              "delay mix applies the requested wet proportion");

    Delay processed(10.0, 2.0, 1'000);
    std::array<double, 5> processedOutput{};
    for (std::size_t index = 0; index < processedOutput.size(); ++index)
    {
        processedOutput[index] = processed.nextSampleWith(index == 0 ? 1.0 : 0.0, 0.5, 1.0,
                                                          [](double value) { return -value; });
    }
    checkSamples(processedOutput, std::array{0.0, 0.0, 1.0, 0.0, -0.5},
                 "delay applies feedback processing before storage");
}

void testDelayOffsetsAndValidation()
{
    using namespace wavlab;

    Delay offset(10.0, 4.0, 1'000);
    const std::array<double, 3> input{1.0, 0.0, 0.0};
    auto output = input;
    output.fill(0.0);
    for (std::size_t index = 0; index < input.size(); ++index)
    {
        output[index] = offset.nextSampleWithOffset(input[index], 2, 1.0);
    }
    checkSamples(output, std::array{0.0, 0.0, 1.0},
                 "a modulation offset shortens the effective delay");

    Delay fractional(4.0, 2.5, 1'000);
    check(fractional.delaySamples() == 3,
          "fractional milliseconds round to the nearest complete sample");
    static_cast<void>(fractional.nextSample(1.0, 0.0));
    fractional.setDelayTime(1.5);
    check(fractional.delaySamples() == 2, "setDelayTime supports fractional milliseconds");
    static_cast<void>(fractional.nextSample(0.0, 0.0));
    checkNear(fractional.nextSample(0.0, 0.0), 1.0, "changing delay time retains buffered samples");

    checkDelayError([] { static_cast<void>(Delay(10.0, 2.0, 0)); },
                    DelayErrorCode::invalidSampleRate, "zero delay sample rate is rejected");
    checkDelayError([] { static_cast<void>(Delay(0.0, 1.0, 1'000)); },
                    DelayErrorCode::invalidDelayTime, "zero maximum delay is rejected");
    checkDelayError([] { static_cast<void>(Delay(10.0, 0.1, 1'000)); },
                    DelayErrorCode::delayTooShort,
                    "a delay that rounds below one sample is rejected");
    checkDelayError([] { static_cast<void>(Delay(10.0, 11.0, 1'000)); },
                    DelayErrorCode::delayExceedsMaximum, "initial delay cannot exceed its maximum");
    checkDelayError([]
                    { static_cast<void>(Delay(std::numeric_limits<double>::max(), 1.0, 48'000)); },
                    DelayErrorCode::sizeOverflow, "unrepresentable delay storage is rejected");

    Delay unchanged(10.0, 2.0, 1'000);
    checkDelayError([&] { unchanged.setDelayTime(11.0); }, DelayErrorCode::delayExceedsMaximum,
                    "invalid delay changes are reported");
    static_cast<void>(unchanged.nextSample(1.0, 0.0));
    static_cast<void>(unchanged.nextSample(0.0, 0.0));
    checkNear(unchanged.nextSample(0.0, 0.0), 1.0,
              "an invalid delay change preserves the previous delay");
}

void testLfoWaveformsAndValidation()
{
    using namespace wavlab;

    const auto collect = [](LfoWaveform waveform)
    {
        LFO lfo(4, 1.0, 0.0, 1.0, waveform);
        std::array<double, 4> values{};
        for (auto &value : values)
        {
            value = lfo.nextSample();
        }
        return values;
    };

    checkSamples(collect(LfoWaveform::sine),
                 std::array{0.0, std::numbers::sqrt2 / 2.0, 1.0, std::numbers::sqrt2 / 2.0},
                 "sine LFO produces one stable unipolar cycle");
    checkSamples(collect(LfoWaveform::triangle), std::array{0.0, 0.5, 1.0, 0.5},
                 "triangle LFO produces a linear unipolar cycle");
    checkSamples(collect(LfoWaveform::square), std::array{0.0, 0.0, 1.0, 1.0},
                 "square LFO alternates between its endpoints");

    LFO ranged(8, 1.0, 10.0, 20.0, LfoWaveform::sine);
    checkNear(ranged.nextSample(), 10.0, "LFO begins at the configured minimum");
    for (int sample = 0; sample < 7; ++sample)
    {
        static_cast<void>(ranged.nextSample());
    }
    checkNear(ranged.nextSample(), 10.0, "LFO phase wraps without a one-second reset");

    LFO stationary(48'000, 0.0, -1.0, 1.0, LfoWaveform::sine);
    check(stationary.nextSample() == -1.0 && stationary.nextSample() == -1.0,
          "zero-frequency LFO remains at its initial value");

    checkLfoError([] { static_cast<void>(LFO(0, 1.0, 0.0, 1.0, LfoWaveform::sine)); },
                  LfoErrorCode::invalidSampleRate, "zero LFO sample rate is rejected");
    checkLfoError(
        []
        {
            static_cast<void>(
                LFO(48'000, std::numeric_limits<double>::quiet_NaN(), 0.0, 1.0, LfoWaveform::sine));
        },
        LfoErrorCode::invalidFrequency, "non-finite LFO frequency is rejected");
    checkLfoError(
        []
        {
            static_cast<void>(
                LFO(1, std::numeric_limits<double>::max(), 0.0, 1.0, LfoWaveform::sine));
        },
        LfoErrorCode::invalidFrequency, "an unrepresentable LFO phase increment is rejected");
    checkLfoError([] { static_cast<void>(LFO(48'000, 1.0, 2.0, 1.0, LfoWaveform::sine)); },
                  LfoErrorCode::invalidRange, "reversed LFO range is rejected");
}

} // namespace

int main()
{
    testDelayImpulseAndFeedback();
    testDelayOffsetsAndValidation();
    testLfoWaveformsAndValidation();
    return failures == 0 ? 0 : 1;
}
