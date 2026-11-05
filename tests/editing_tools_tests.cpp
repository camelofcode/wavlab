#include <wavlab/Tools.h>
#include <wavlab/WavFile.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
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

void checkNear(double actual, double expected, std::string_view message)
{
    check(std::abs(actual - expected) <= epsilon, message);
}

void checkSamples(const wavlab::WavFile &file, std::span<const double> expected,
                  std::string_view message)
{
    const auto actual = file.samples();
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
void checkSamples(const wavlab::WavFile &file, const std::array<double, Size> &expected,
                  std::string_view message)
{
    checkSamples(file, std::span<const double>(expected), message);
}

wavlab::WavFile file(std::uint16_t channels, std::uint32_t sampleRate, std::uint16_t bits,
                     std::vector<double> samples)
{
    auto result = wavlab::WavFile::create(channels, sampleRate, bits);
    result.setSamples(std::move(samples));
    return result;
}

template <typename Action>
void checkProcessingError(Action action, wavlab::ProcessingErrorCode expected,
                          std::string_view message)
{
    try
    {
        action();
        check(false, message);
    }
    catch (const wavlab::ProcessingError &error)
    {
        check(error.code() == expected, message);
    }
}

void testLevelsGainAndNormalise()
{
    using namespace wavlab;

    auto measured = file(1, 48'000, 24, {0.25, -0.5, 0.0, 0.0});
    checkNear(Tools::getPeakDecibels(measured), 20.0 * std::log10(0.5),
              "peak uses the greatest absolute sample");
    checkNear(Tools::getRMSDecibels(measured), 20.0 * std::log10(std::sqrt(0.078125)),
              "RMS covers every interleaved sample");

    const auto silence = file(2, 48'000, 24, {0.0, 0.0});
    const auto empty = file(1, 48'000, 24, {});
    check(std::isinf(Tools::getPeakDecibels(silence)) && Tools::getPeakDecibels(silence) < 0.0,
          "silent peak is negative infinity");
    check(std::isinf(Tools::getRMSDecibels(empty)) && Tools::getRMSDecibels(empty) < 0.0,
          "empty RMS is negative infinity");

    auto normalised = file(1, 48'000, 24, {0.25, -0.5, 0.125});
    Tools::normalise(normalised);
    checkSamples(normalised, std::array{0.5, -1.0, 0.25}, "normalise scales by the absolute peak");

    auto silentNormalisation = file(2, 48'000, 24, {0.0, 0.0});
    Tools::normalise(silentNormalisation);
    checkSamples(silentNormalisation, std::array{0.0, 0.0}, "normalising silence leaves it silent");
    auto emptyNormalisation = file(1, 48'000, 24, {});
    Tools::normalise(emptyNormalisation);
    check(emptyNormalisation.samples().empty(), "normalising an empty file is safe");

    auto target = file(1, 48'000, 24, {0.25, -0.5, 0.125});
    Tools::normaliseToDecibels(target, -20.0);
    checkSamples(target, std::array{0.05, -0.1, 0.025},
                 "normalise-to-dB reaches the requested peak");

    auto gained = file(1, 48'000, 24, {0.25, -0.5, 0.0});
    Tools::gain(gained, -2.0);
    checkSamples(gained, std::array{-0.5, 1.0, 0.0},
                 "linear gain permits amplification and polarity inversion");
    Tools::decibelGain(gained, -20.0);
    checkSamples(gained, std::array{-0.05, 0.1, 0.0}, "decibel gain uses an amplitude ratio");
}

void testFrameTransforms()
{
    using namespace wavlab;

    auto mono = file(1, 48'000, 24, {0.25, -0.5, 0.75});
    Tools::reverse(mono);
    checkSamples(mono, std::array{0.75, -0.5, 0.25}, "mono reversal reverses samples");

    auto stereo = file(2, 48'000, 24, {0.1, 0.2, 0.3, 0.4, 0.5, 0.6});
    Tools::reverse(stereo);
    checkSamples(stereo, std::array{0.5, 0.6, 0.3, 0.4, 0.1, 0.2},
                 "stereo reversal retains all frames and channel order");

    auto empty = file(2, 48'000, 24, {});
    Tools::reverse(empty);
    Tools::swapChannels(empty);
    check(empty.samples().empty(), "empty stereo transforms do not underflow");

    Tools::flipPhase(stereo);
    checkSamples(stereo, std::array{-0.5, -0.6, -0.3, -0.4, -0.1, -0.2},
                 "polarity inversion covers every interleaved sample");
    Tools::swapChannels(stereo);
    checkSamples(stereo, std::array{-0.6, -0.5, -0.4, -0.3, -0.2, -0.1},
                 "channel swap covers every stereo frame");

    const auto before = mono.samples()[0];
    Tools::swapChannels(mono);
    check(mono.samples()[0] == before, "channel swap is a no-op for mono");
}

void testStereoSplitAndCombine()
{
    using namespace wavlab;

    const auto stereo = file(2, 96'000, 32, {0.1, -0.2, 0.3, -0.4, 0.5, -0.6});
    const auto [left, right] = Tools::splitStereo(stereo);
    check(left.numberOfChannels() == 1 && right.numberOfChannels() == 1,
          "stereo split produces mono files");
    check(left.sampleRate() == 96'000 && right.bitsPerSample() == 32,
          "stereo split preserves PCM settings");
    checkSamples(left, std::array{0.1, 0.3, 0.5}, "stereo split extracts left samples");
    checkSamples(right, std::array{-0.2, -0.4, -0.6}, "stereo split extracts right samples");

    const auto emptyStereo = file(2, 48'000, 24, {});
    const auto [emptyLeft, emptyRight] = Tools::splitStereo(emptyStereo);
    check(emptyLeft.samples().empty() && emptyRight.samples().empty(),
          "empty stereo files split safely");
    const auto mono = file(1, 48'000, 24, {0.1});
    checkProcessingError([&mono] { static_cast<void>(Tools::splitStereo(mono)); },
                         ProcessingErrorCode::expectedStereo, "stereo split rejects a mono input");

    const auto shortLeft = file(1, 48'000, 16, {0.1, 0.2, 0.3});
    const auto shortRight = file(1, 48'000, 24, {-0.1, -0.2});
    const auto combined = Tools::combineLeftRight(shortLeft, shortRight);
    check(combined.numberOfChannels() == 2 && combined.bitsPerSample() == 24,
          "left/right combination chooses stereo and the maximum bit depth");
    checkSamples(combined, std::array{0.1, -0.1, 0.2, -0.2, 0.3, 0.0},
                 "left/right combination silence-pads the shorter input");

    const auto [mid, side] = Tools::splitStereoMidSide(stereo);
    const auto decoded = Tools::combineMidSide(mid, side);
    checkSamples(decoded, stereo.samples(), "mid/side split and decode round-trip");

    const auto noSamples =
        Tools::combineLeftRight(file(1, 48'000, 16, {}), file(1, 48'000, 24, {}));
    check(noSamples.samples().empty(), "empty mono pairs combine safely");
    const auto wrongRate = file(1, 44'100, 24, {0.1});
    checkProcessingError([&] { static_cast<void>(Tools::combineLeftRight(stereo, mono)); },
                         ProcessingErrorCode::expectedMono,
                         "pair combination rejects stereo inputs");
    checkProcessingError([&] { static_cast<void>(Tools::combineLeftRight(mono, wrongRate)); },
                         ProcessingErrorCode::sampleRateMismatch,
                         "pair combination rejects mismatched rates");
}

void testTimeSplit()
{
    using namespace wavlab;

    const auto mono = file(1, 1'000, 16, {0.0, 0.1, 0.2, 0.3, 0.4, 0.5});
    const std::array<std::uint64_t, 2> positions{2, 5};
    const auto outputs = Tools::split(mono, positions);
    check(outputs.size() == 3, "multiple split points create one additional segment each");
    checkSamples(outputs[0], std::array{0.0, 0.1}, "split boundary excludes the next frame");
    checkSamples(outputs[1], std::array{0.2, 0.3, 0.4}, "middle split segment is exact");
    checkSamples(outputs[2], std::array{0.5}, "final split segment retains the last sample");

    const auto stereo = file(2, 1'000, 24, {0.1, -0.1, 0.2, -0.2, 0.3, -0.3});
    const std::array<std::uint64_t, 1> stereoPosition{2};
    const auto stereoOutputs = Tools::split(stereo, stereoPosition);
    checkSamples(stereoOutputs[0], std::array{0.1, -0.1, 0.2, -0.2},
                 "stereo split occurs only between complete frames");
    checkSamples(stereoOutputs[1], std::array{0.3, -0.3},
                 "stereo split retains the following frame");

    std::vector<double> preciseSamples(100, 0.0);
    const auto precise = file(1, 44'100, 16, std::move(preciseSamples));
    const std::array<std::uint64_t, 1> oneMillisecond{1};
    const auto preciseOutputs = Tools::split(precise, oneMillisecond);
    check(preciseOutputs[0].frameCount() == 44,
          "millisecond conversion retains non-integer sample-rate precision");

    const auto unsplit = Tools::split(mono, std::span<const std::uint64_t>{});
    check(unsplit.size() == 1 && unsplit[0].samples().size() == mono.samples().size(),
          "no split points return one complete file");

    const std::array<std::uint64_t, 1> zero{0};
    const std::array<std::uint64_t, 2> duplicate{2, 2};
    const std::array<std::uint64_t, 2> descending{3, 2};
    const std::array<std::uint64_t, 1> atEnd{6};
    checkProcessingError([&] { static_cast<void>(Tools::split(mono, zero)); },
                         ProcessingErrorCode::invalidSplitTime, "zero split time is rejected");
    checkProcessingError([&] { static_cast<void>(Tools::split(mono, duplicate)); },
                         ProcessingErrorCode::invalidSplitTime,
                         "duplicate split positions are rejected");
    checkProcessingError([&] { static_cast<void>(Tools::split(mono, descending)); },
                         ProcessingErrorCode::invalidSplitTime,
                         "descending split positions are rejected");
    checkProcessingError([&] { static_cast<void>(Tools::split(mono, atEnd)); },
                         ProcessingErrorCode::invalidSplitTime,
                         "split positions at the end are rejected");
}

void testMixAndJoin()
{
    using namespace wavlab;

    const auto first = file(1, 48'000, 16, {0.5, 0.25});
    const auto second = file(1, 48'000, 24, {0.5});
    const std::array monoInputs{first, second};
    const auto mixedMono = Tools::mix(monoInputs);
    check(mixedMono.numberOfChannels() == 1 && mixedMono.bitsPerSample() == 24,
          "mono mix retains mono and maximum bit depth");
    checkSamples(mixedMono, std::array{1.0, 0.25}, "mix silence-pads, averages, and normalises");

    const auto mono = file(1, 48'000, 16, {0.5});
    const auto stereo = file(2, 48'000, 32, {0.5, -0.5, 0.25, -0.25});
    const std::array promotedInputs{mono, stereo};
    const auto mixedStereo = Tools::mix(promotedInputs);
    check(mixedStereo.numberOfChannels() == 2 && mixedStereo.bitsPerSample() == 32,
          "mix promotes mono when any input is stereo");
    checkSamples(mixedStereo, std::array{1.0, 0.0, 0.25, -0.25},
                 "mono mix input is promoted and shorter inputs are silence-padded");

    const auto silent = file(1, 48'000, 16, {0.0, 0.0});
    const auto empty = file(1, 48'000, 24, {});
    const std::array silentInputs{silent, empty};
    const auto silentMix = Tools::mix(silentInputs);
    checkSamples(silentMix, std::array{0.0, 0.0}, "silent mixes remain finite silence");

    const std::array oneInput{first};
    const auto wrongRate = file(1, 44'100, 24, {0.1});
    const std::array mismatchedRates{first, wrongRate};
    checkProcessingError([&] { static_cast<void>(Tools::mix(oneInput)); },
                         ProcessingErrorCode::tooFewInputs, "mix requires at least two inputs");
    checkProcessingError([&] { static_cast<void>(Tools::mix(mismatchedRates)); },
                         ProcessingErrorCode::sampleRateMismatch,
                         "mix rejects mismatched sample rates");

    const auto joinedFirst = file(2, 48'000, 16, {0.1, -0.1, 0.2, -0.2});
    const auto joinedSecond = file(2, 48'000, 24, {0.3, -0.3});
    const std::array joinedInputs{joinedFirst, joinedSecond};
    const auto joined = Tools::join(joinedInputs);
    check(joined.numberOfChannels() == 2 && joined.sampleRate() == 48'000 &&
              joined.bitsPerSample() == 24,
          "join preserves layout and chooses maximum bit depth");
    checkSamples(joined, std::array{0.1, -0.1, 0.2, -0.2, 0.3, -0.3},
                 "join concatenates every frame in order");

    const std::array joinedWithEmpty{file(1, 48'000, 16, {}), first};
    const auto nonemptyJoin = Tools::join(joinedWithEmpty);
    checkSamples(nonemptyJoin, first.samples(), "join handles empty compatible inputs");

    const std::array channelMismatch{first, stereo};
    checkProcessingError([&] { static_cast<void>(Tools::join(oneInput)); },
                         ProcessingErrorCode::tooFewInputs, "join requires at least two inputs");
    checkProcessingError([&] { static_cast<void>(Tools::join(mismatchedRates)); },
                         ProcessingErrorCode::sampleRateMismatch,
                         "join rejects mismatched sample rates");
    checkProcessingError([&] { static_cast<void>(Tools::join(channelMismatch)); },
                         ProcessingErrorCode::channelCountMismatch,
                         "join rejects mismatched channel counts");
}

} // namespace

int main()
{
    testLevelsGainAndNormalise();
    testFrameTransforms();
    testStereoSplitAndCombine();
    testTimeSplit();
    testMixAndJoin();
    return failures == 0 ? 0 : 1;
}
