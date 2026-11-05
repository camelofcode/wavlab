#include <wavlab/Generators.h>
#include <wavlab/WavFile.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <span>
#include <string_view>
#include <vector>

namespace
{

constexpr double epsilon = 1e-12;
int failures = 0;

void check(bool condition, std::string_view message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
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
        if (std::abs(actual[index] - expected[index]) > epsilon)
        {
            check(false, message);
            return;
        }
    }
}

template <std::size_t Size>
void checkSamples(std::span<const double> actual, const std::array<double, Size> &expected,
                  std::string_view message)
{
    checkSamples(actual, std::span<const double>(expected), message);
}

template <typename Action>
void checkGeneratorError(Action action, wavlab::GeneratorErrorCode expected,
                         std::string_view message)
{
    try
    {
        action();
        check(false, message);
    }
    catch (const wavlab::GeneratorError &error)
    {
        check(error.code() == expected, message);
    }
}

wavlab::WavFile file(std::uint16_t channels, std::uint32_t sampleRate,
                     std::vector<double> samples = {})
{
    auto result = wavlab::WavFile::create(channels, sampleRate, 24);
    result.setSamples(std::move(samples));
    return result;
}

void checkMatchingChannels(const wavlab::WavFile &wave, std::string_view message)
{
    const auto samples = wave.samples();
    bool matches = true;
    for (std::size_t frame = 0; frame < samples.size(); frame += wave.numberOfChannels())
    {
        for (std::size_t channel = 1; channel < wave.numberOfChannels(); ++channel)
        {
            matches = matches && samples[frame] == samples[frame + channel];
        }
    }
    check(matches, message);
}

void testDurationsAndDirectWaveforms()
{
    using namespace wavlab;

    auto subsecond = file(2, 8'000);
    Generators::sineWave(subsecond, 250, 1'000.0);
    check(subsecond.frameCount() == 2'000 && subsecond.sampleCount() == 4'000,
          "sub-second generation calculates the complete frame count");
    checkMatchingChannels(subsecond, "tonal generators copy each frame to every channel");

    auto wave = file(1, 8);
    Generators::squareWave(wave, 1'000, 1.0);
    checkSamples(wave.samples(), std::array{1.0, 1.0, 1.0, 1.0, -1.0, -1.0, -1.0, -1.0},
                 "direct square produces one exact cycle");

    Generators::triangleWave(wave, 1'000, 1.0);
    checkSamples(wave.samples(), std::array{0.0, 0.5, 1.0, 0.5, 0.0, -0.5, -1.0, -0.5},
                 "direct triangle produces one exact cycle");

    Generators::sawtoothWave(wave, 1'000, 1.0);
    checkSamples(wave.samples(), std::array{0.0, 0.25, 0.5, 0.75, -1.0, -0.75, -0.5, -0.25},
                 "direct sawtooth produces one exact cycle");

    Generators::sineWave(wave, 0, 1.0);
    check(wave.samples().empty(), "zero-duration generation produces an empty valid file");
}

void testAdditiveGeneration()
{
    using namespace wavlab;

    auto limited = file(1, 8);
    auto excessive = file(1, 8);
    Generators::squareWaveAdditive(limited, 1'000, 1.0, 2);
    Generators::squareWaveAdditive(excessive, 1'000, 1.0, 100);
    checkSamples(excessive.samples(), limited.samples(),
                 "square synthesis omits harmonics at and above Nyquist");

    Generators::triangleWaveAdditive(limited, 1'000, 1.0, 2);
    Generators::triangleWaveAdditive(excessive, 1'000, 1.0, 100);
    checkSamples(excessive.samples(), limited.samples(),
                 "triangle synthesis omits harmonics at and above Nyquist");

    Generators::sawtoothWaveAdditive(limited, 1'000, 1.0, 3);
    Generators::sawtoothWaveAdditive(excessive, 1'000, 1.0, 100);
    checkSamples(excessive.samples(), limited.samples(),
                 "sawtooth synthesis omits harmonics at and above Nyquist");

    auto stereo = file(2, 48'000);
    Generators::triangleWaveAdditive(stereo, 10, 1'000.0, 20);
    checkMatchingChannels(stereo, "additive generation copies samples to every channel");
}

void testNoise()
{
    using namespace wavlab;

    std::mt19937_64 whiteEngine(1234);
    auto white = file(2, 8'000);
    Generators::whiteNoise(white, 250, whiteEngine);
    check(white.frameCount() == 2'000, "seeded white noise has the requested duration");
    checkMatchingChannels(white, "white-noise channels match within each frame");
    bool whiteValid = true;
    for (const auto sample : white.samples())
    {
        whiteValid = whiteValid && std::isfinite(sample) && sample >= -1.0 && sample <= 1.0;
    }
    check(whiteValid, "white-noise samples stay in the normalised range");

    std::mt19937_64 pinkEngine(5678);
    auto pink = file(2, 8'000);
    Generators::pinkNoise(pink, 250, pinkEngine);
    check(pink.frameCount() == 2'000, "seeded pink noise has the requested duration");
    checkMatchingChannels(pink, "pink-noise channels match within each frame");
    double peak = 0.0;
    for (const auto sample : pink.samples())
    {
        peak = std::max(peak, std::abs(sample));
    }
    check(std::abs(peak - 1.0) <= epsilon, "pink noise is peak-normalised");
}

void testValidationAndStrongGuarantee()
{
    using namespace wavlab;

    const std::vector<double> original{0.25, -0.5};
    auto wave = file(1, 8, original);
    const auto unchanged = [&]
    {
        checkSamples(wave.samples(), original,
                     "invalid generator settings leave samples unchanged");
    };

    checkGeneratorError([&] { Generators::sineWave(wave, 1'000, 0.0); },
                        GeneratorErrorCode::invalidFrequency, "zero frequency is rejected");
    unchanged();
    checkGeneratorError(
        [&] { Generators::sineWave(wave, 1'000, std::numeric_limits<double>::quiet_NaN()); },
        GeneratorErrorCode::invalidFrequency, "NaN frequency is rejected");
    unchanged();
    checkGeneratorError([&] { Generators::sineWave(wave, 1'000, 4.0); },
                        GeneratorErrorCode::invalidFrequency, "the Nyquist frequency is rejected");
    unchanged();
    checkGeneratorError([&] { Generators::squareWaveAdditive(wave, 1'000, 1.0, 0); },
                        GeneratorErrorCode::invalidHarmonicCount,
                        "zero additive harmonics are rejected");
    unchanged();
    checkGeneratorError(
        [&] { Generators::whiteNoise(wave, std::numeric_limits<std::uint64_t>::max()); },
        GeneratorErrorCode::sizeOverflow, "unrepresentable output duration is rejected");
    unchanged();
}

} // namespace

int main()
{
    testDurationsAndDirectWaveforms();
    testAdditiveGeneration();
    testNoise();
    testValidationAndStrongGuarantee();
    return failures == 0 ? 0 : 1;
}
