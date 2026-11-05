#include <wavlab/Delay.h>
#include <wavlab/FX.h>
#include <wavlab/WavFile.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <span>
#include <string_view>
#include <vector>

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

void checkSamples(std::span<const double> actual, std::span<const double> expected,
                  std::string_view message)
{
    if (actual.size() != expected.size())
    {
        check(false, message);
        return;
    }
    for (std::size_t index = 0; index < actual.size(); ++index)
    {
        if (std::abs(actual[index] - expected[index]) > tolerance)
        {
            check(false, message);
            return;
        }
    }
}

template <typename Action>
void checkEffectError(Action action, wavlab::EffectErrorCode expected, std::string_view message)
{
    try
    {
        action();
        check(false, message);
    }
    catch (const wavlab::EffectError &error)
    {
        check(error.code() == expected, message);
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

wavlab::WavFile file(std::uint16_t channels, std::uint32_t sampleRate, std::vector<double> samples)
{
    auto result = wavlab::WavFile::create(channels, sampleRate, 24);
    result.setSamples(std::move(samples));
    return result;
}

wavlab::WavFile monoImpulse(std::uint32_t sampleRate, std::size_t frames)
{
    std::vector<double> samples(frames, 0.0);
    if (!samples.empty())
    {
        samples.front() = 1.0;
    }
    return file(1, sampleRate, std::move(samples));
}

void testSaturation()
{
    using namespace wavlab;

    auto wave = file(2, 48'000, {-0.25, 0.25, -0.5, 0.5});
    FX::saturate(wave, 0.0);
    checkNear(wave.samples()[0], std::tanh(-0.625), "zero-dB saturation uses unity input gain");
    checkNear(wave.samples()[1], std::tanh(0.625), "saturation retains odd symmetry");

    auto reduced = file(1, 48'000, {0.25});
    auto driven = file(1, 48'000, {0.25});
    FX::saturate(reduced, -20.0);
    FX::saturate(driven, 12.0);
    check(reduced.samples()[0] > 0.0 && reduced.samples()[0] < wave.samples()[1],
          "negative dB drive reduces saturation without inverting polarity");
    check(driven.samples()[0] > wave.samples()[1], "positive dB drive increases saturation");

    const std::vector<double> original{0.25, -0.25};
    auto unchanged = file(1, 48'000, original);
    checkEffectError([&] { FX::saturate(unchanged, std::numeric_limits<double>::quiet_NaN()); },
                     EffectErrorCode::invalidDrive, "non-finite saturation drive is rejected");
    checkSamples(unchanged.samples(), original, "invalid saturation drive preserves samples");
}

void testDigitalDelay()
{
    using namespace wavlab;

    auto impulse = file(1, 1'000, {1.0, 0.0, 0.0});
    FX::digitalDelay(impulse, DelaySettings{{2.0, 0.5}, 1.0, 4});
    const std::array expected{0.0, 0.0, 1.0, 0.0, 0.5, 0.0, 0.25};
    checkSamples(impulse.samples(), expected,
                 "digital delay produces clean repeats and an additional tail");

    auto fractional = file(1, 1'000, {1.0});
    FX::digitalDelay(fractional, DelaySettings{{2.5, 0.0}, 1.0, 3});
    checkSamples(fractional.samples(), std::array{0.0, 0.0, 0.0, 1.0},
                 "effect delays support fractional milliseconds rounded to samples");

    auto negative = file(1, 1'000, {1.0});
    FX::digitalDelay(negative, DelaySettings{{1.0, -0.5}, 1.0, 2});
    checkSamples(negative.samples(), std::array{0.0, 1.0, -0.5},
                 "digital delay supports stable signed feedback");

    auto mixed = file(1, 1'000, {0.25, 0.0});
    FX::digitalDelay(mixed, DelaySettings{{1.0, 0.0}, 0.5, 0});
    checkSamples(mixed.samples(), std::array{0.125, 0.125},
                 "delay mix does not secretly normalise global level");

    auto promoted = file(1, 1'000, {1.0, 0.0});
    FX::digitalDelay(promoted, StereoDelaySettings{{1.0, 0.0}, {2.0, 0.0}, 1.0, 2});
    check(promoted.numberOfChannels() == 2, "stereo delay promotes mono input");
    checkSamples(promoted.samples(), std::array{0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0},
                 "promoted stereo delay uses independent channel settings");

    auto stereo = file(2, 1'000, {1.0, 0.0, 0.0, 1.0});
    FX::digitalDelay(stereo, DelaySettings{{1.0, 0.0}, 1.0, 1});
    checkSamples(stereo.samples(), std::array{0.0, 0.0, 1.0, 0.0, 0.0, 1.0},
                 "ordinary stereo delay keeps channel state independent");

    auto empty = file(1, 1'000, {});
    FX::digitalDelay(empty, DelaySettings{{1.0, 0.0}, 0.5, 0});
    check(empty.samples().empty(), "delay handles an empty input without indexing errors");
}

void testDelayValidation()
{
    using namespace wavlab;

    const std::vector<double> original{1.0, 0.0};
    auto wave = file(1, 1'000, original);
    checkEffectError([&] { FX::digitalDelay(wave, DelaySettings{{1.0, 1.0}, 0.5, 0}); },
                     EffectErrorCode::invalidFeedback,
                     "non-decaying feedback magnitude is rejected");
    checkSamples(wave.samples(), original, "invalid feedback preserves samples");
    checkEffectError([&] { FX::digitalDelay(wave, DelaySettings{{1.0, 0.0}, 1.1, 0}); },
                     EffectErrorCode::invalidMix, "out-of-range mix is rejected");
    checkSamples(wave.samples(), original, "invalid mix preserves samples");
    checkDelayError([&] { FX::digitalDelay(wave, DelaySettings{{0.0, 0.0}, 0.5, 0}); },
                    DelayErrorCode::invalidDelayTime, "zero delay is rejected");
    checkSamples(wave.samples(), original, "invalid delay time preserves samples");
    checkDelayError([&] { FX::digitalDelay(wave, DelaySettings{{0.1, 0.0}, 0.5, 0}); },
                    DelayErrorCode::delayTooShort, "a delay shorter than one sample is rejected");
    checkSamples(wave.samples(), original, "too-short delay preserves samples");
    checkEffectError(
        [&]
        {
            FX::digitalDelay(
                wave, DelaySettings{{1.0, 0.0}, 0.5, std::numeric_limits<std::uint64_t>::max()});
        },
        EffectErrorCode::sizeOverflow, "unrepresentable delay tail is rejected");
    checkSamples(wave.samples(), original, "oversized delay tail preserves samples");
}

void testAnalogueAndTapeDelay()
{
    using namespace wavlab;

    auto digital = monoImpulse(8'000, 10);
    auto analogue = monoImpulse(8'000, 10);
    auto tape = monoImpulse(8'000, 10);
    const DelaySettings settings{{1.0, 0.5}, 1.0, 2};
    FX::digitalDelay(digital, settings);
    FX::analogDelay(analogue, settings, 2'000.0);
    FX::tapeDelay(tape, settings, 0.0, 2'000.0);
    checkNear(digital.samples()[8], analogue.samples()[8],
              "analogue feedback processing leaves the first repeat clean");
    checkNear(analogue.samples()[8], tape.samples()[8],
              "tape feedback processing leaves the first repeat clean");
    check(std::abs(analogue.samples()[16] - 0.5) > tolerance,
          "analogue low-pass colours subsequent feedback repeats");
    check(std::abs(tape.samples()[16] - analogue.samples()[16]) > tolerance,
          "tape saturation changes subsequent feedback repeats");

    auto zeroDrive = monoImpulse(8'000, 10);
    auto driven = monoImpulse(8'000, 10);
    FX::tapeDelay(zeroDrive, settings, 0.0, 2'000.0);
    FX::tapeDelay(driven, settings, 12.0, 2'000.0);
    check(driven.samples()[16] > zeroDrive.samples()[16],
          "positive tape drive increases feedback saturation");

    auto stereo = file(2, 8'000, {1.0, 0.0});
    FX::analogDelay(stereo, DelaySettings{{1.0, 0.5}, 1.0, 2}, 2'000.0);
    bool rightSilent = true;
    for (std::size_t index = 1; index < stereo.samples().size(); index += 2)
    {
        rightSilent = rightSilent && stereo.samples()[index] == 0.0;
    }
    check(rightSilent, "analogue delay feedback state does not leak between stereo channels");

    const std::vector<double> original{1.0, 0.0};
    auto unchanged = file(1, 8'000, original);
    checkEffectError([&] { FX::analogDelay(unchanged, settings, 4'000.0); },
                     EffectErrorCode::invalidCutoffFrequency,
                     "delay cutoff at Nyquist is rejected");
    checkSamples(unchanged.samples(), original, "invalid cutoff preserves samples");
    checkEffectError(
        [&]
        { FX::tapeDelay(unchanged, settings, std::numeric_limits<double>::infinity(), 2'000.0); },
        EffectErrorCode::invalidDrive, "non-finite tape drive is rejected");
    checkSamples(unchanged.samples(), original, "invalid tape drive preserves samples");
}

void testReverb()
{
    using namespace wavlab;

    auto dry = file(1, 1'000, {1.0, 0.0});
    FX::reverb(dry, ReverbSettings{0.5, 1.0, 0.0, 2});
    checkSamples(dry.samples(), std::array{1.0, 0.0, 0.0, 0.0},
                 "zero-mix reverb returns input followed by an explicit silent tail");

    auto wet = monoImpulse(1'000, 1);
    FX::reverb(wet, ReverbSettings{0.5, 1.0, 1.0, 50});
    bool silentBeforeFirstComb = true;
    for (std::size_t index = 0; index < 30; ++index)
    {
        silentBeforeFirstComb = silentBeforeFirstComb && wet.samples()[index] == 0.0;
    }
    check(silentBeforeFirstComb, "reverb uses the published first comb delay");
    checkNear(wet.samples()[30], 0.1225,
              "series all-pass stages produce the expected first wet impulse");

    auto shortDecay = monoImpulse(1'000, 1);
    auto longDecay = monoImpulse(1'000, 1);
    FX::reverb(shortDecay, ReverbSettings{0.5, 0.25, 1.0, 150});
    FX::reverb(longDecay, ReverbSettings{0.5, 2.0, 1.0, 150});
    const auto tailEnergy = [](const WavFile &wave)
    {
        double energy{};
        for (std::size_t frame = 60; frame < wave.frameCount(); ++frame)
        {
            energy += std::abs(wave.samples()[frame]);
        }
        return energy;
    };
    check(tailEnergy(longDecay) > tailEnergy(shortDecay),
          "longer RT60 settings retain more late-tail energy");

    auto stereo = file(2, 1'000, {1.0, 0.0});
    FX::reverb(stereo, ReverbSettings{0.5, 1.0, 1.0, 50});
    bool rightSilent = true;
    for (std::size_t index = 1; index < stereo.samples().size(); index += 2)
    {
        rightSilent = rightSilent && stereo.samples()[index] == 0.0;
    }
    check(rightSilent, "reverb maintains independent per-channel state");
    check(stereo.numberOfChannels() == 2 && stereo.frameCount() == 51,
          "reverb preserves channels and appends the requested tail");

    const std::vector<double> original{1.0, 0.0};
    auto unchanged = file(1, 48'000, original);
    checkEffectError([&] { FX::reverb(unchanged, ReverbSettings{1.1, 1.0, 0.5, 0}); },
                     EffectErrorCode::invalidRoomSize, "invalid reverb room size is rejected");
    checkEffectError([&] { FX::reverb(unchanged, ReverbSettings{0.5, 0.0, 0.5, 0}); },
                     EffectErrorCode::invalidDecayTime, "zero RT60 is rejected");
    checkEffectError([&] { FX::reverb(unchanged, ReverbSettings{0.5, 1.0, -0.1, 0}); },
                     EffectErrorCode::invalidMix, "invalid reverb mix is rejected");
    checkSamples(unchanged.samples(), original, "invalid reverb settings preserve samples");
}

} // namespace

int main()
{
    testSaturation();
    testDigitalDelay();
    testDelayValidation();
    testAnalogueAndTapeDelay();
    testReverb();
    return failures == 0 ? 0 : 1;
}
