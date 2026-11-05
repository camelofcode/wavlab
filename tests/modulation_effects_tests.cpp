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

wavlab::WavFile impulse(std::uint32_t sampleRate, std::size_t frames)
{
    std::vector<double> samples(frames, 0.0);
    samples.front() = 1.0;
    return file(1, sampleRate, std::move(samples));
}

void testChorusAndVibrato()
{
    using namespace wavlab;

    auto centred = impulse(1'000, 30);
    FX::chorus(centred, ChorusSettings{{0.0, 1.0}, 1.0});
    check(centred.samples()[14] == 1.0,
          "zero-depth chorus retains the documented 14 ms centre delay");

    auto swept = impulse(1'000, 30);
    FX::chorus(swept, ChorusSettings{{1.0, 125.0}, 1.0});
    check(swept.samples()[4] == 1.0, "full-depth chorus reaches its 4 ms minimum delay");

    auto stationary = impulse(1'000, 30);
    FX::chorus(stationary, ChorusSettings{{1.0, 0.0}, 1.0});
    check(stationary.samples()[24] == 1.0,
          "zero-frequency full-depth chorus remains at its 24 ms maximum delay");

    auto commonStereo = file(2, 1'000, {1.0, 0.0, 0.0, 1.0});
    FX::chorus(commonStereo, ChorusSettings{{0.0, 1.0}, 0.0});
    checkSamples(commonStereo.samples(), std::array{1.0, 0.0, 0.0, 1.0},
                 "dry common chorus preserves a stereo signal");

    std::vector<double> stereoImpulse(60, 0.0);
    stereoImpulse.front() = 1.0;
    auto independentStereo = file(2, 1'000, stereoImpulse);
    FX::chorus(independentStereo, ChorusSettings{{0.0, 1.0}, 1.0});
    bool rightSilent = true;
    for (std::size_t index = 1; index < independentStereo.samples().size(); index += 2)
    {
        rightSilent = rightSilent && independentStereo.samples()[index] == 0.0;
    }
    check(rightSilent, "common chorus delay state does not leak between stereo channels");

    auto promoted = impulse(1'000, 26);
    FX::chorus(promoted, StereoChorusSettings{{0.0, 0.0}, {1.0, 0.0}, 1.0});
    check(promoted.numberOfChannels() == 2, "stereo chorus promotes mono input");
    check(promoted.samples()[14 * 2] == 1.0 && promoted.samples()[24 * 2 + 1] == 1.0,
          "stereo chorus uses independent channel modulation settings");

    auto chorusWave = impulse(1'000, 30);
    auto vibratoWave = impulse(1'000, 30);
    FX::chorus(chorusWave, ChorusSettings{{0.5, 4.0}, 1.0});
    FX::vibrato(vibratoWave, VibratoSettings{{0.5, 4.0}});
    checkSamples(vibratoWave.samples(), chorusWave.samples(),
                 "vibrato is exactly a fully wet chorus");

    const std::vector<double> original{1.0, 0.0};
    auto unchanged = file(1, 1'000, original);
    checkEffectError([&] { FX::chorus(unchanged, ChorusSettings{{1.1, 1.0}, 0.5}); },
                     EffectErrorCode::invalidChorusDepth, "out-of-range chorus depth is rejected");
    checkSamples(unchanged.samples(), original, "invalid chorus settings preserve input");
}

void testFlanger()
{
    using namespace wavlab;

    auto minimum = impulse(1'000, 4);
    FX::flanger(minimum, FlangerSettings{{0.0, 1.0, 0.0}, 1.0});
    checkSamples(minimum.samples(), std::array{0.0, 1.0, 0.0, 0.0},
                 "zero-depth flanger uses the safe one-sample delay");

    auto positive = impulse(1'000, 50);
    FX::flanger(positive, FlangerSettings{{0.0, 1.0, 0.95}, 1.0});
    check(positive.samples()[1] == 1.0 && positive.samples()[2] == 0.95 &&
              positive.samples()[3] == 0.95 * 0.95,
          "maximum positive flanger feedback remains decaying");

    auto dry = file(2, 1'000, {0.25, -0.5, 0.5, -0.25});
    const std::vector<double> dryOriginal(dry.samples().begin(), dry.samples().end());
    FX::flanger(dry, FlangerSettings{{1.0, 10.0, -0.95}, 0.0});
    checkSamples(dry.samples(), dryOriginal, "zero-mix flanger is an exact stereo identity");

    std::vector<double> stereoImpulse(20, 0.0);
    stereoImpulse.front() = 1.0;
    auto independentStereo = file(2, 1'000, stereoImpulse);
    FX::flanger(independentStereo, FlangerSettings{{0.0, 1.0, 0.95}, 1.0});
    bool rightSilent = true;
    for (std::size_t index = 1; index < independentStereo.samples().size(); index += 2)
    {
        rightSilent = rightSilent && independentStereo.samples()[index] == 0.0;
    }
    check(rightSilent, "common flanger feedback state does not leak between stereo channels");

    auto promoted = impulse(1'000, 20);
    FX::flanger(promoted, StereoFlangerSettings{{0.0, 1.0, 0.0}, {1.0, 0.1, 0.0}, 1.0});
    check(promoted.numberOfChannels() == 2 && promoted.samples()[2] == 1.0 &&
              promoted.samples()[20 + 1] == 1.0,
          "stereo flanger promotes mono and applies independent depths");

    const std::vector<double> original{1.0, 0.0};
    auto unchanged = file(1, 1'000, original);
    checkEffectError([&] { FX::flanger(unchanged, FlangerSettings{{0.5, 0.09, 0.0}, 0.5}); },
                     EffectErrorCode::invalidFlangerFrequency,
                     "flanger rejects frequencies below its stable range");
    checkEffectError([&] { FX::flanger(unchanged, FlangerSettings{{0.5, 1.0, 0.951}, 0.5}); },
                     EffectErrorCode::invalidFlangerFeedback, "flanger rejects unstable feedback");
    checkSamples(unchanged.samples(), original, "invalid flanger settings preserve input");
}

void testTremoloAndAutoPan()
{
    using namespace wavlab;

    auto tremolo = file(1, 4, {1.0, 1.0, 1.0, 1.0});
    FX::tremolo(tremolo, TremoloSettings{1.0, 1.0, LfoWaveform::triangle});
    checkSamples(tremolo.samples(), std::array{0.0, 0.5, 1.0, 0.5},
                 "triangle tremolo follows the selected LFO waveform");

    auto square = file(1, 4, {1.0, 1.0, 1.0, 1.0});
    FX::tremolo(square, TremoloSettings{1.0, 1.0, LfoWaveform::square});
    checkSamples(square.samples(), std::array{0.0, 0.0, 1.0, 1.0},
                 "square tremolo switches between its depth endpoints");

    auto stereo = file(2, 4, {0.25, -0.5, 0.5, -0.25});
    const std::vector<double> stereoOriginal(stereo.samples().begin(), stereo.samples().end());
    FX::tremolo(stereo, TremoloSettings{0.0, 1.0, LfoWaveform::sine});
    checkSamples(stereo.samples(), stereoOriginal,
                 "zero-depth tremolo preserves both channels and their stereo image");

    auto pan = file(1, 4, {1.0, 1.0, 1.0, 1.0});
    FX::autoPan(pan, AutoPanSettings{1.0, 1.0, LfoWaveform::triangle});
    check(pan.numberOfChannels() == 2, "autopan always produces stereo output");
    checkSamples(pan.samples(), std::array{1.0, 0.0, 0.5, 0.5, 0.0, 1.0, 0.5, 0.5},
                 "full-depth autopan traverses the stereo field");

    auto dryPan = file(2, 4, {0.25, -0.5, 0.5, -0.25});
    FX::autoPan(dryPan, AutoPanSettings{0.0, 1.0, LfoWaveform::square});
    checkSamples(dryPan.samples(), stereoOriginal,
                 "zero-depth autopan preserves existing stereo samples");

    auto unchanged = file(1, 4, {1.0, 0.0});
    const std::vector<double> original(unchanged.samples().begin(), unchanged.samples().end());
    checkEffectError(
        [&]
        {
            FX::autoPan(unchanged, AutoPanSettings{0.5, std::numeric_limits<double>::infinity(),
                                                   LfoWaveform::sine});
        },
        EffectErrorCode::invalidLfoFrequency, "autopan rejects non-finite LFO frequency");
    check(unchanged.numberOfChannels() == 1, "invalid autopan preserves the channel layout");
    checkSamples(unchanged.samples(), original, "invalid autopan preserves input samples");
}

} // namespace

int main()
{
    testChorusAndVibrato();
    testFlanger();
    testTremoloAndAutoPan();
    return failures == 0 ? 0 : 1;
}
