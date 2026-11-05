// Robert Bristow-Johnson's Audio EQ Cookbook biquad formulae.
// https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html

#include <wavlab/EQ.h>

#include "Constants.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace wavlab
{

EqError::EqError(EqErrorCode code, std::string message)
    : std::invalid_argument(std::move(message)), code_(code)
{
}

EqErrorCode EqError::code() const noexcept { return code_; }

namespace
{

struct RawCoefficients
{
    double a0{};
    double a1{};
    double a2{};
    double b0{};
    double b1{};
    double b2{};
};

void validateCommon(double frequency, std::uint32_t sampleRate)
{
    if (sampleRate == 0)
    {
        throw EqError(EqErrorCode::invalidSampleRate, "the EQ sample rate must be non-zero");
    }
    const auto nyquist = static_cast<double>(sampleRate) / 2.0;
    if (!std::isfinite(frequency) || frequency <= 0.0 || frequency >= nyquist)
    {
        throw EqError(EqErrorCode::invalidFrequency,
                      "EQ frequency must be finite, greater than zero, and below Nyquist");
    }
}

void validateQ(double q)
{
    if (!std::isfinite(q) || q <= 0.0)
    {
        throw EqError(EqErrorCode::invalidQ, "EQ Q must be finite and greater than zero");
    }
}

void validateBandwidth(double bandwidth)
{
    if (!std::isfinite(bandwidth) || bandwidth <= 0.0)
    {
        throw EqError(EqErrorCode::invalidBandwidth,
                      "EQ bandwidth must be finite and greater than zero");
    }
}

void validateGain(double gainDecibels)
{
    if (!std::isfinite(gainDecibels))
    {
        throw EqError(EqErrorCode::invalidGain, "EQ gain must be finite");
    }
}

[[nodiscard]] BiquadCoefficients validateCoefficients(BiquadCoefficients coefficients)
{
    for (const auto coefficient :
         {coefficients.b0, coefficients.b1, coefficients.b2, coefficients.a1, coefficients.a2})
    {
        if (!std::isfinite(coefficient))
        {
            throw EqError(EqErrorCode::invalidCoefficients,
                          "EQ settings produced invalid coefficients");
        }
    }
    return coefficients;
}

[[nodiscard]] BiquadCoefficients normaliseCoefficients(const RawCoefficients &raw)
{
    if (!std::isfinite(raw.a0) || raw.a0 == 0.0)
    {
        throw EqError(EqErrorCode::invalidCoefficients,
                      "EQ settings produced invalid coefficients");
    }
    return validateCoefficients(
        {raw.b0 / raw.a0, raw.b1 / raw.a0, raw.b2 / raw.a0, raw.a1 / raw.a0, raw.a2 / raw.a0});
}

struct FrequencyTerms
{
    double omega;
    double sine;
    double cosine;
};

[[nodiscard]] FrequencyTerms frequencyTerms(double frequency, std::uint32_t sampleRate)
{
    const auto omega = Constants::TAU * frequency / sampleRate;
    return {omega, std::sin(omega), std::cos(omega)};
}

[[nodiscard]] double alphaForBandwidth(const FrequencyTerms &terms, double bandwidth)
{
    return terms.sine * std::sinh(std::numbers::ln2 / 2.0 * bandwidth * terms.omega / terms.sine);
}

[[nodiscard]] RawCoefficients passDenominator(const FrequencyTerms &terms, double alpha)
{
    return {1.0 + alpha, -2.0 * terms.cosine, 1.0 - alpha, 0.0, 0.0, 0.0};
}

template <typename Factory> void apply(WavFile &wave, Factory factory)
{
    const auto channels = static_cast<std::size_t>(wave.numberOfChannels());
    std::vector<EQInstance> filters;
    filters.reserve(channels);
    for (std::size_t channel = 0; channel < channels; ++channel)
    {
        filters.push_back(factory(wave.sampleRate()));
    }

    auto samples = wave.samples();
    for (std::size_t index = 0; index < samples.size(); ++index)
    {
        samples[index] = filters[index % channels].processSample(samples[index]);
    }
}

} // namespace

EQInstance::EQInstance(BiquadCoefficients coefficients) : coefficients_(coefficients) {}

EQInstance EQInstance::peak(double centreFrequency, std::uint32_t sampleRate,
                            double bandwidthOctaves, double gainDecibels)
{
    validateCommon(centreFrequency, sampleRate);
    validateBandwidth(bandwidthOctaves);
    validateGain(gainDecibels);
    const auto terms = frequencyTerms(centreFrequency, sampleRate);
    const auto alpha = alphaForBandwidth(terms, bandwidthOctaves);
    const auto amplitude = std::pow(10.0, gainDecibels / 40.0);
    return EQInstance(normaliseCoefficients({1.0 + alpha / amplitude, -2.0 * terms.cosine,
                                             1.0 - alpha / amplitude, 1.0 + alpha * amplitude,
                                             -2.0 * terms.cosine, 1.0 - alpha * amplitude}));
}

EQInstance EQInstance::allPass(double centreFrequency, std::uint32_t sampleRate,
                               double bandwidthOctaves)
{
    validateCommon(centreFrequency, sampleRate);
    validateBandwidth(bandwidthOctaves);
    const auto terms = frequencyTerms(centreFrequency, sampleRate);
    const auto alpha = alphaForBandwidth(terms, bandwidthOctaves);
    // Evaluate the normalised ratios directly. The cookbook's intermediate
    // sinh can overflow very close to Nyquist even though the coefficient
    // ratios have a finite, well-defined limit.
    if (alpha > 1.0)
    {
        const auto inverseAlpha = 1.0 / alpha;
        const auto scale = 1.0 / (1.0 + inverseAlpha);
        const auto edge = (inverseAlpha - 1.0) * scale;
        const auto middle = -2.0 * terms.cosine * inverseAlpha * scale;
        return EQInstance(validateCoefficients({edge, middle, 1.0, middle, edge}));
    }
    const auto scale = 1.0 / (1.0 + alpha);
    const auto edge = (1.0 - alpha) * scale;
    const auto middle = -2.0 * terms.cosine * scale;
    return EQInstance(validateCoefficients({edge, middle, 1.0, middle, edge}));
}

EQInstance EQInstance::bandPass(double centreFrequency, std::uint32_t sampleRate,
                                double bandwidthOctaves)
{
    validateCommon(centreFrequency, sampleRate);
    validateBandwidth(bandwidthOctaves);
    const auto terms = frequencyTerms(centreFrequency, sampleRate);
    const auto alpha = alphaForBandwidth(terms, bandwidthOctaves);
    auto raw = passDenominator(terms, alpha);
    raw.b0 = terms.sine / 2.0;
    raw.b1 = 0.0;
    raw.b2 = -terms.sine / 2.0;
    return EQInstance(normaliseCoefficients(raw));
}

EQInstance EQInstance::lowPass(double cutoffFrequency, std::uint32_t sampleRate, double q)
{
    validateCommon(cutoffFrequency, sampleRate);
    validateQ(q);
    const auto terms = frequencyTerms(cutoffFrequency, sampleRate);
    const auto alpha = terms.sine / (2.0 * q);
    auto raw = passDenominator(terms, alpha);
    raw.b0 = (1.0 - terms.cosine) / 2.0;
    raw.b1 = 1.0 - terms.cosine;
    raw.b2 = raw.b0;
    return EQInstance(normaliseCoefficients(raw));
}

EQInstance EQInstance::highPass(double cutoffFrequency, std::uint32_t sampleRate, double q)
{
    validateCommon(cutoffFrequency, sampleRate);
    validateQ(q);
    const auto terms = frequencyTerms(cutoffFrequency, sampleRate);
    const auto alpha = terms.sine / (2.0 * q);
    auto raw = passDenominator(terms, alpha);
    raw.b0 = (1.0 + terms.cosine) / 2.0;
    raw.b1 = -(1.0 + terms.cosine);
    raw.b2 = raw.b0;
    return EQInstance(normaliseCoefficients(raw));
}

EQInstance EQInstance::lowShelf(double centreFrequency, std::uint32_t sampleRate, double q,
                                double gainDecibels)
{
    validateCommon(centreFrequency, sampleRate);
    validateQ(q);
    validateGain(gainDecibels);
    const auto terms = frequencyTerms(centreFrequency, sampleRate);
    const auto amplitude = std::pow(10.0, gainDecibels / 40.0);
    const auto betaSine = std::sqrt(amplitude) / q * terms.sine;
    return EQInstance(normaliseCoefficients(
        {(amplitude + 1.0) + (amplitude - 1.0) * terms.cosine + betaSine,
         -2.0 * ((amplitude - 1.0) + (amplitude + 1.0) * terms.cosine),
         (amplitude + 1.0) + (amplitude - 1.0) * terms.cosine - betaSine,
         amplitude * ((amplitude + 1.0) - (amplitude - 1.0) * terms.cosine + betaSine),
         2.0 * amplitude * ((amplitude - 1.0) - (amplitude + 1.0) * terms.cosine),
         amplitude * ((amplitude + 1.0) - (amplitude - 1.0) * terms.cosine - betaSine)}));
}

EQInstance EQInstance::highShelf(double centreFrequency, std::uint32_t sampleRate, double q,
                                 double gainDecibels)
{
    validateCommon(centreFrequency, sampleRate);
    validateQ(q);
    validateGain(gainDecibels);
    const auto terms = frequencyTerms(centreFrequency, sampleRate);
    const auto amplitude = std::pow(10.0, gainDecibels / 40.0);
    const auto betaSine = std::sqrt(amplitude) / q * terms.sine;
    return EQInstance(normaliseCoefficients(
        {(amplitude + 1.0) - (amplitude - 1.0) * terms.cosine + betaSine,
         2.0 * ((amplitude - 1.0) - (amplitude + 1.0) * terms.cosine),
         (amplitude + 1.0) - (amplitude - 1.0) * terms.cosine - betaSine,
         amplitude * ((amplitude + 1.0) + (amplitude - 1.0) * terms.cosine + betaSine),
         -2.0 * amplitude * ((amplitude - 1.0) + (amplitude + 1.0) * terms.cosine),
         amplitude * ((amplitude + 1.0) + (amplitude - 1.0) * terms.cosine - betaSine)}));
}

void EQInstance::setAllPass(double centreFrequency, std::uint32_t sampleRate,
                            double bandwidthOctaves)
{
    coefficients_ = allPass(centreFrequency, sampleRate, bandwidthOctaves).coefficients_;
}

void EQInstance::setLowPass(double cutoffFrequency, std::uint32_t sampleRate, double q)
{
    coefficients_ = lowPass(cutoffFrequency, sampleRate, q).coefficients_;
}

void EQInstance::setHighPass(double cutoffFrequency, std::uint32_t sampleRate, double q)
{
    coefficients_ = highPass(cutoffFrequency, sampleRate, q).coefficients_;
}

double EQInstance::processSample(double input) noexcept
{
    const auto output = coefficients_.b0 * input + coefficients_.b1 * input1_ +
                        coefficients_.b2 * input2_ - coefficients_.a1 * output1_ -
                        coefficients_.a2 * output2_;
    input2_ = input1_;
    input1_ = input;
    output2_ = output1_;
    output1_ = output;
    return output;
}

const BiquadCoefficients &EQInstance::coefficients() const noexcept { return coefficients_; }

namespace EQ
{

void peak(WavFile &wave, double centreFrequency, double bandwidthOctaves, double gainDecibels)
{
    apply(
        wave, [&](std::uint32_t sampleRate)
        { return EQInstance::peak(centreFrequency, sampleRate, bandwidthOctaves, gainDecibels); });
}

void allPass(WavFile &wave, double centreFrequency, double bandwidthOctaves)
{
    apply(wave, [&](std::uint32_t sampleRate)
          { return EQInstance::allPass(centreFrequency, sampleRate, bandwidthOctaves); });
}

void bandPass(WavFile &wave, double centreFrequency, double bandwidthOctaves)
{
    apply(wave, [&](std::uint32_t sampleRate)
          { return EQInstance::bandPass(centreFrequency, sampleRate, bandwidthOctaves); });
}

void lowPass(WavFile &wave, double cutoffFrequency, double q)
{
    apply(wave, [&](std::uint32_t sampleRate)
          { return EQInstance::lowPass(cutoffFrequency, sampleRate, q); });
}

void highPass(WavFile &wave, double cutoffFrequency, double q)
{
    apply(wave, [&](std::uint32_t sampleRate)
          { return EQInstance::highPass(cutoffFrequency, sampleRate, q); });
}

void lowShelf(WavFile &wave, double centreFrequency, double q, double gainDecibels)
{
    apply(wave, [&](std::uint32_t sampleRate)
          { return EQInstance::lowShelf(centreFrequency, sampleRate, q, gainDecibels); });
}

void highShelf(WavFile &wave, double centreFrequency, double q, double gainDecibels)
{
    apply(wave, [&](std::uint32_t sampleRate)
          { return EQInstance::highShelf(centreFrequency, sampleRate, q, gainDecibels); });
}

void process(WavFile &wave, FilterType type, double gainDecibels, double centreFrequency,
             double shape)
{
    switch (type)
    {
    case FilterType::Peak:
        peak(wave, centreFrequency, shape, gainDecibels);
        return;
    case FilterType::AllPass:
        allPass(wave, centreFrequency, shape);
        return;
    case FilterType::BandPass:
        bandPass(wave, centreFrequency, shape);
        return;
    case FilterType::LowPass:
        lowPass(wave, centreFrequency, shape);
        return;
    case FilterType::HighPass:
        highPass(wave, centreFrequency, shape);
        return;
    case FilterType::LowShelf:
        lowShelf(wave, centreFrequency, shape, gainDecibels);
        return;
    case FilterType::HighShelf:
        highShelf(wave, centreFrequency, shape, gainDecibels);
        return;
    case FilterType::None:
        throw EqError(EqErrorCode::invalidFilterType, "an EQ filter type is required");
    }
    throw EqError(EqErrorCode::invalidFilterType, "unknown EQ filter type");
}

} // namespace EQ

} // namespace wavlab
