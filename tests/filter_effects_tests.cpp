#include <wavlab/EQ.h>
#include <wavlab/FX.h>
#include <wavlab/LFO.h>
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

wavlab::WavFile file(std::uint16_t channels, std::uint32_t sampleRate, std::vector<double> samples)
{
    auto result = wavlab::WavFile::create(channels, sampleRate, 24);
    result.setSamples(std::move(samples));
    return result;
}

void testPhaser()
{
    using namespace wavlab;

    const std::vector<double> original{1.0, 0.0, -0.25, 0.5};
    auto dry = file(1, 8'000, original);
    FX::phaser(dry, PhaserSettings{{1.0, 2.0, 1'000.0, LfoWaveform::triangle}, 4, 0.0});
    checkSamples(dry.samples(), original, "zero-mix phaser is an exact dry identity");

    auto wet = file(1, 8'000, original);
    FX::phaser(wet, PhaserSettings{{1.0, 2.0, 1'000.0, LfoWaveform::sine}, 3, 1.0});
    check(!std::equal(wet.samples().begin(), wet.samples().end(), original.begin()),
          "wet phaser changes the input signal");
    check(std::all_of(wet.samples().begin(), wet.samples().end(),
                      [](double sample) { return std::isfinite(sample); }),
          "wet phaser output remains finite");

    std::vector<double> sweepInput(8, 0.0);
    sweepInput.front() = 1.0;
    auto fullSweep = file(1, 8'000, sweepInput);
    FX::phaser(fullSweep, PhaserSettings{{1.0, 2'000.0, 1'500.0, LfoWaveform::sine}, 3, 1.0});
    check(std::all_of(fullSweep.samples().begin(), fullSweep.samples().end(),
                      [](double sample) { return std::isfinite(sample); }),
          "full phaser sweep remains finite when clamped close to Nyquist");

    auto independentStereo = file(2, 8'000, {1.0, 0.0, 0.0, 0.0, 0.0, 0.0});
    FX::phaser(independentStereo, PhaserSettings{{0.5, 1.0, 1'000.0, LfoWaveform::sine}, 3, 1.0});
    bool rightSilent = true;
    for (std::size_t index = 1; index < independentStereo.samples().size(); index += 2)
    {
        rightSilent = rightSilent && independentStereo.samples()[index] == 0.0;
    }
    check(rightSilent, "common phaser filter state does not leak between stereo channels");

    auto promoted = file(1, 8'000, original);
    FX::phaser(promoted, StereoPhaserSettings{{0.5, 1.0, 500.0, LfoWaveform::sine},
                                              {0.5, 1.0, 1'500.0, LfoWaveform::triangle},
                                              2,
                                              1.0});
    check(promoted.numberOfChannels() == 2, "stereo phaser promotes mono input");
    bool differs = false;
    for (std::size_t frame = 0; frame < promoted.frameCount(); ++frame)
    {
        differs = differs || std::abs(promoted.samples()[frame * 2] -
                                      promoted.samples()[frame * 2 + 1]) > tolerance;
    }
    check(differs, "stereo phaser channels retain independent settings and state");

    auto unchanged = file(1, 8'000, original);
    checkEffectError(
        [&]
        { FX::phaser(unchanged, PhaserSettings{{0.5, 1.0, 4'000.0, LfoWaveform::sine}, 3, 0.5}); },
        EffectErrorCode::invalidPhaserCentreFrequency,
        "phaser rejects a centre frequency at Nyquist");
    checkEffectError(
        [&]
        { FX::phaser(unchanged, PhaserSettings{{0.5, 1.0, 1'000.0, LfoWaveform::sine}, 17, 0.5}); },
        EffectErrorCode::invalidPhaserStages, "phaser rejects excessive stage counts");
    checkSamples(unchanged.samples(), original, "invalid phaser settings preserve input");
}

void testModulatedFilter()
{
    using namespace wavlab;

    const std::vector<double> input{1.0, 0.5, 0.0, -0.5, -1.0, 0.25, 0.0, 0.0};
    const ModulatedFilterSettings settings{
        200.0, 1'000.0, 1.0, ModulatedFilterType::lowPass, LfoWaveform::triangle, 0.707};
    auto actual = file(1, 8'000, input);
    FX::modulatedFilter(actual, settings);

    LFO lfo(8'000, settings.frequencyHz, settings.minimumFrequencyHz, settings.maximumFrequencyHz,
            settings.waveform);
    auto filter = EQInstance::lowPass(settings.minimumFrequencyHz, 8'000, settings.q);
    std::vector<double> expected;
    expected.reserve(input.size());
    for (const auto sample : input)
    {
        filter.setLowPass(lfo.nextSample(), 8'000, settings.q);
        expected.push_back(filter.processSample(sample));
    }
    checkSamples(actual.samples(), expected,
                 "modulated low-pass matches explicit state-preserving coefficient updates");

    auto high = file(1, 8'000, input);
    FX::modulatedFilter(high,
                        ModulatedFilterSettings{200.0, 1'000.0, 2.0, ModulatedFilterType::highPass,
                                                LfoWaveform::square, 1.0});
    check(!std::equal(high.samples().begin(), high.samples().end(), actual.samples().begin()),
          "modulated filter honours high-pass type and waveform selection");

    auto stereo = file(2, 8'000, {1.0, 0.0, 0.0, 0.0, 0.0, 0.0});
    FX::modulatedFilter(stereo, settings);
    bool rightSilent = true;
    for (std::size_t index = 1; index < stereo.samples().size(); index += 2)
    {
        rightSilent = rightSilent && stereo.samples()[index] == 0.0;
    }
    check(rightSilent, "modulated filter state does not leak between channels");

    auto unchanged = file(1, 8'000, input);
    checkEffectError(
        [&]
        {
            FX::modulatedFilter(unchanged, ModulatedFilterSettings{1'000.0, 500.0, 1.0,
                                                                   ModulatedFilterType::lowPass,
                                                                   LfoWaveform::sine, 0.707});
        },
        EffectErrorCode::invalidModulatedFilterFrequencyRange,
        "modulated filter rejects a reversed frequency range");
    checkEffectError(
        [&]
        {
            FX::modulatedFilter(unchanged, ModulatedFilterSettings{200.0, 1'000.0, 1.0,
                                                                   ModulatedFilterType::lowPass,
                                                                   LfoWaveform::sine, 0.0});
        },
        EffectErrorCode::invalidModulatedFilterQ, "modulated filter rejects non-positive Q");
    checkEffectError(
        [&]
        {
            FX::modulatedFilter(unchanged,
                                ModulatedFilterSettings{200.0, 1'000.0, 1.0,
                                                        static_cast<ModulatedFilterType>(99),
                                                        LfoWaveform::sine, 0.707});
        },
        EffectErrorCode::invalidModulatedFilterType,
        "modulated filter rejects unknown C++ enum values");
    checkSamples(unchanged.samples(), input, "invalid modulated filter settings preserve input");
}

} // namespace

int main()
{
    testPhaser();
    testModulatedFilter();
    return failures == 0 ? 0 : 1;
}
