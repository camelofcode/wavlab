#include <wavlab/EQ.h>
#include <wavlab/WavFile.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <numbers>
#include <span>
#include <string_view>
#include <vector>

namespace
{

constexpr std::uint32_t sampleRate = 48'000;
constexpr double centreFrequency = 6'000.0;
constexpr double tolerance = 1e-10;
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
void checkEqError(Action action, wavlab::EqErrorCode expected, std::string_view message)
{
    try
    {
        action();
        check(false, message);
    }
    catch (const wavlab::EqError &error)
    {
        check(error.code() == expected, message);
    }
}

wavlab::WavFile file(std::uint16_t channels, std::vector<double> samples)
{
    auto result = wavlab::WavFile::create(channels, sampleRate, 24);
    result.setSamples(std::move(samples));
    return result;
}

double magnitudeAt(const wavlab::EQInstance &filter, double frequency)
{
    const auto &coefficients = filter.coefficients();
    const auto omega = 2.0 * std::numbers::pi * frequency / sampleRate;
    const auto cosine = std::cos(omega);
    const auto sine = std::sin(omega);
    const auto cosine2 = std::cos(2.0 * omega);
    const auto sine2 = std::sin(2.0 * omega);
    const auto numeratorReal =
        coefficients.b0 + coefficients.b1 * cosine + coefficients.b2 * cosine2;
    const auto numeratorImaginary = -coefficients.b1 * sine - coefficients.b2 * sine2;
    const auto denominatorReal = 1.0 + coefficients.a1 * cosine + coefficients.a2 * cosine2;
    const auto denominatorImaginary = -coefficients.a1 * sine - coefficients.a2 * sine2;
    return std::hypot(numeratorReal, numeratorImaginary) /
           std::hypot(denominatorReal, denominatorImaginary);
}

void testResponses()
{
    using wavlab::EQInstance;

    const auto gain = std::pow(10.0, 6.0 / 20.0);
    const auto midpointGain = std::pow(10.0, 6.0 / 40.0);
    const auto peak = EQInstance::peak(centreFrequency, sampleRate, 1.0, 6.0);
    checkNear(magnitudeAt(peak, 0.0), 1.0, "peak EQ retains DC gain");
    checkNear(magnitudeAt(peak, centreFrequency), gain,
              "peak EQ reaches the requested centre gain");
    checkNear(magnitudeAt(peak, sampleRate / 2.0), 1.0, "peak EQ retains Nyquist gain");

    const auto lowShelf = EQInstance::lowShelf(centreFrequency, sampleRate, 1.0, 6.0);
    const auto highShelf = EQInstance::highShelf(centreFrequency, sampleRate, 1.0, 6.0);
    checkNear(magnitudeAt(lowShelf, 0.0), gain, "low shelf applies gain at DC");
    checkNear(magnitudeAt(lowShelf, centreFrequency), midpointGain,
              "low shelf reaches midpoint gain at its centre");
    checkNear(magnitudeAt(lowShelf, sampleRate / 2.0), 1.0, "low shelf retains Nyquist gain");
    checkNear(magnitudeAt(highShelf, 0.0), 1.0, "high shelf retains DC gain");
    checkNear(magnitudeAt(highShelf, centreFrequency), midpointGain,
              "high shelf reaches midpoint gain at its centre");
    checkNear(magnitudeAt(highShelf, sampleRate / 2.0), gain, "high shelf applies gain at Nyquist");

    const auto lowPass = EQInstance::lowPass(centreFrequency, sampleRate, 1.0);
    const auto highPass = EQInstance::highPass(centreFrequency, sampleRate, 1.0);
    checkNear(magnitudeAt(lowPass, 0.0), 1.0, "low-pass retains DC");
    checkNear(magnitudeAt(lowPass, sampleRate / 2.0), 0.0, "low-pass rejects Nyquist");
    checkNear(magnitudeAt(highPass, 0.0), 0.0, "high-pass rejects DC");
    checkNear(magnitudeAt(highPass, sampleRate / 2.0), 1.0, "high-pass retains Nyquist");

    const auto bandPass = EQInstance::bandPass(centreFrequency, sampleRate, 1.0);
    checkNear(magnitudeAt(bandPass, 0.0), 0.0, "band-pass rejects DC");
    check(magnitudeAt(bandPass, centreFrequency) > 0.0, "band-pass retains its centre frequency");
    checkNear(magnitudeAt(bandPass, sampleRate / 2.0), 0.0, "band-pass rejects Nyquist");

    const auto allPass = EQInstance::allPass(centreFrequency, sampleRate, 1.0);
    for (const auto frequency : {0.0, 1'000.0, centreFrequency, 12'000.0, 24'000.0})
    {
        checkNear(magnitudeAt(allPass, frequency), 1.0,
                  "all-pass has unity magnitude across the spectrum");
    }
}

void testStateAndNamedOperations()
{
    using namespace wavlab;

    auto filter = EQInstance::lowPass(centreFrequency, sampleRate, 1.0);
    const auto coefficients = filter.coefficients();
    const auto first = filter.processSample(1.0);
    const auto second = filter.processSample(0.0);
    const auto third = filter.processSample(0.0);
    checkNear(first, coefficients.b0, "first impulse sample equals b0");
    checkNear(second, coefficients.b1 - coefficients.a1 * first,
              "second impulse sample advances biquad state");
    checkNear(third, coefficients.b2 - coefficients.a1 * second - coefficients.a2 * first,
              "third impulse sample advances both delays");

    const std::array<double, 4> input{1.0, -0.5, 0.25, 0.0};
    const auto checkOperation = [&](EQInstance expectedFilter,
                                    const std::function<void(WavFile &)> &operation,
                                    std::string_view message)
    {
        auto expected = input;
        for (auto &sample : expected)
        {
            sample = expectedFilter.processSample(sample);
        }
        auto wave = file(1, std::vector<double>(input.begin(), input.end()));
        operation(wave);
        checkSamples(wave.samples(), expected, message);
    };

    checkOperation(
        EQInstance::peak(centreFrequency, sampleRate, 1.0, 6.0),
        [](WavFile &wave) { EQ::peak(wave, centreFrequency, 1.0, 6.0); },
        "named peak operation uses peak coefficients");
    checkOperation(
        EQInstance::lowShelf(centreFrequency, sampleRate, 1.0, 6.0),
        [](WavFile &wave) { EQ::lowShelf(wave, centreFrequency, 1.0, 6.0); },
        "named low-shelf operation uses shelf coefficients");
    checkOperation(
        EQInstance::highShelf(centreFrequency, sampleRate, 1.0, 6.0),
        [](WavFile &wave) { EQ::highShelf(wave, centreFrequency, 1.0, 6.0); },
        "named high-shelf operation uses shelf coefficients");
    checkOperation(
        EQInstance::lowPass(centreFrequency, sampleRate, 1.0),
        [](WavFile &wave) { EQ::lowPass(wave, centreFrequency, 1.0); },
        "named low-pass operation uses Q coefficients");
    checkOperation(
        EQInstance::highPass(centreFrequency, sampleRate, 1.0),
        [](WavFile &wave) { EQ::highPass(wave, centreFrequency, 1.0); },
        "named high-pass operation uses Q coefficients");
    checkOperation(
        EQInstance::bandPass(centreFrequency, sampleRate, 1.0),
        [](WavFile &wave) { EQ::bandPass(wave, centreFrequency, 1.0); },
        "named band-pass operation uses octave bandwidth");
    checkOperation(
        EQInstance::allPass(centreFrequency, sampleRate, 1.0),
        [](WavFile &wave) { EQ::allPass(wave, centreFrequency, 1.0); },
        "named all-pass operation uses octave bandwidth");
}

void testIndependentChannelState()
{
    using namespace wavlab;

    const std::array<double, 6> input{1.0, -0.25, 0.5, 0.0, 0.0, 0.75};
    std::array<double, input.size()> expected = input;
    std::array filters{EQInstance::lowPass(centreFrequency, sampleRate, 1.0),
                       EQInstance::lowPass(centreFrequency, sampleRate, 1.0)};
    for (std::size_t index = 0; index < expected.size(); ++index)
    {
        expected[index] = filters[index % filters.size()].processSample(expected[index]);
    }

    auto stereo = file(2, std::vector<double>(input.begin(), input.end()));
    EQ::lowPass(stereo, centreFrequency, 1.0);
    checkSamples(stereo.samples(), expected, "stereo EQ gives each channel independent state");

    auto empty = file(2, {});
    EQ::lowPass(empty, centreFrequency, 1.0);
    check(empty.samples().empty(), "EQ handles an empty stereo file");
}

void testValidationAndStrongGuarantee()
{
    using namespace wavlab;

    checkEqError([] { static_cast<void>(EQInstance::lowPass(0.0, sampleRate, 1.0)); },
                 EqErrorCode::invalidFrequency, "zero EQ frequency is rejected");
    checkEqError(
        []
        {
            static_cast<void>(
                EQInstance::lowPass(std::numeric_limits<double>::quiet_NaN(), sampleRate, 1.0));
        },
        EqErrorCode::invalidFrequency, "NaN EQ frequency is rejected");
    checkEqError([] { static_cast<void>(EQInstance::lowPass(sampleRate / 2.0, sampleRate, 1.0)); },
                 EqErrorCode::invalidFrequency, "Nyquist EQ frequency is rejected");
    checkEqError([] { static_cast<void>(EQInstance::lowPass(1'000.0, sampleRate, 0.0)); },
                 EqErrorCode::invalidQ, "zero Q is rejected");
    checkEqError([] { static_cast<void>(EQInstance::allPass(1'000.0, sampleRate, 0.0)); },
                 EqErrorCode::invalidBandwidth, "zero bandwidth is rejected");
    checkEqError(
        []
        {
            static_cast<void>(EQInstance::peak(1'000.0, sampleRate, 1.0,
                                               std::numeric_limits<double>::infinity()));
        },
        EqErrorCode::invalidGain, "non-finite EQ gain is rejected");
    checkEqError([] { static_cast<void>(EQInstance::lowPass(1'000.0, 0, 1.0)); },
                 EqErrorCode::invalidSampleRate, "zero EQ sample rate is rejected");

    const std::vector<double> original{0.25, -0.5};
    auto wave = file(1, original);
    checkEqError([&] { EQ::lowPass(wave, 0.0, 1.0); }, EqErrorCode::invalidFrequency,
                 "invalid in-place EQ settings are reported");
    checkSamples(wave.samples(), original, "invalid EQ settings leave samples unchanged");
    checkEqError([&] { EQ::process(wave, FilterType::None, 0.0, 1'000.0, 1.0); },
                 EqErrorCode::invalidFilterType, "the compatibility dispatcher rejects None");
    checkSamples(wave.samples(), original, "invalid filter types leave samples unchanged");
}

} // namespace

int main()
{
    testResponses();
    testStateAndNamedOperations();
    testIndependentChannelState();
    testValidationAndStrongGuarantee();
    return failures == 0 ? 0 : 1;
}
