#include <wavlab/Generators.h>

#include "Constants.h"

#include <wavlab/Tools.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numbers>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace wavlab
{

GeneratorError::GeneratorError(GeneratorErrorCode code, std::string message)
    : std::invalid_argument(std::move(message)), code_(code)
{
}

GeneratorErrorCode GeneratorError::code() const noexcept { return code_; }

namespace
{

void validateFrequency(const WavFile &wave, double frequency)
{
    const auto nyquist = static_cast<double>(wave.sampleRate()) / 2.0;
    if (!std::isfinite(frequency) || frequency <= 0.0 || frequency >= nyquist)
    {
        throw GeneratorError(
            GeneratorErrorCode::invalidFrequency,
            "invalid frequency " + std::to_string(frequency) +
                "; it must be finite, greater than zero, and below the Nyquist frequency");
    }
}

void validateHarmonics(std::uint32_t harmonics)
{
    if (harmonics == 0)
    {
        throw GeneratorError(GeneratorErrorCode::invalidHarmonicCount,
                             "the harmonic count must be greater than zero");
    }
}

[[nodiscard]] std::size_t frameCount(const WavFile &wave, std::uint64_t lengthMilliseconds)
{
    if (lengthMilliseconds > std::numeric_limits<std::uint64_t>::max() / wave.sampleRate())
    {
        throw GeneratorError(GeneratorErrorCode::sizeOverflow,
                             "generated frame count is too large");
    }

    const auto frames = lengthMilliseconds * wave.sampleRate() / 1'000;
    if (frames > std::numeric_limits<std::size_t>::max())
    {
        throw GeneratorError(GeneratorErrorCode::sizeOverflow,
                             "generated frame count is too large");
    }
    return static_cast<std::size_t>(frames);
}

template <typename GenerateFrame>
void generateSamples(WavFile &wave, std::uint64_t lengthMilliseconds, GenerateFrame generateFrame)
{
    const auto frames = frameCount(wave, lengthMilliseconds);
    const auto channels = static_cast<std::size_t>(wave.numberOfChannels());
    std::vector<double> samples;
    if (frames > samples.max_size() / channels)
    {
        throw GeneratorError(GeneratorErrorCode::sizeOverflow,
                             "generated sample count is too large");
    }
    samples.reserve(frames * channels);

    for (std::size_t frame = 0; frame < frames; ++frame)
    {
        const auto sample = generateFrame(frame);
        for (std::size_t channel = 0; channel < channels; ++channel)
        {
            samples.push_back(sample);
        }
    }

    wave.setSamples(std::move(samples));
}

using Harmonic = std::pair<double, double>;

[[nodiscard]] std::vector<Harmonic> oddHarmonics(double frequency, double nyquist,
                                                 std::uint32_t count, bool alternating,
                                                 bool squaredAmplitude)
{
    std::vector<Harmonic> coefficients;
    for (std::uint32_t index = 0; index < count; ++index)
    {
        const auto harmonic = 2.0 * static_cast<double>(index) + 1.0;
        if (frequency * harmonic >= nyquist)
        {
            break;
        }
        auto amplitude = 1.0 / harmonic;
        if (squaredAmplitude)
        {
            amplitude /= harmonic;
        }
        if (alternating && index % 2 != 0)
        {
            amplitude = -amplitude;
        }
        coefficients.emplace_back(harmonic, amplitude);
    }
    return coefficients;
}

template <typename Engine> [[nodiscard]] double randomSample(Engine &engine)
{
    std::uniform_real_distribution<double> distribution(-1.0, 1.0);
    return distribution(engine);
}

} // namespace

namespace Generators
{

void sineWave(WavFile &wave, std::uint64_t lengthMilliseconds, double frequency)
{
    validateFrequency(wave, frequency);
    const auto radiansPerFrame = Constants::TAU * frequency / wave.sampleRate();
    generateSamples(wave, lengthMilliseconds, [radiansPerFrame](std::size_t frame)
                    { return std::sin(static_cast<double>(frame) * radiansPerFrame); });
}

void squareWave(WavFile &wave, std::uint64_t lengthMilliseconds, double frequency)
{
    validateFrequency(wave, frequency);
    const auto cyclesPerFrame = frequency / wave.sampleRate();
    generateSamples(
        wave, lengthMilliseconds, [cyclesPerFrame](std::size_t frame)
        { return std::fmod(static_cast<double>(frame) * cyclesPerFrame, 1.0) < 0.5 ? 1.0 : -1.0; });
}

void squareWaveAdditive(WavFile &wave, std::uint64_t lengthMilliseconds, double frequency,
                        std::uint32_t harmonics)
{
    validateFrequency(wave, frequency);
    validateHarmonics(harmonics);
    const auto radiansPerFrame = Constants::TAU * frequency / wave.sampleRate();
    const auto coefficients =
        oddHarmonics(frequency, wave.sampleRate() / 2.0, harmonics, false, false);
    generateSamples(wave, lengthMilliseconds,
                    [&](std::size_t frame)
                    {
                        const auto phase = static_cast<double>(frame) * radiansPerFrame;
                        double sample = 0.0;
                        for (const auto &[harmonic, amplitude] : coefficients)
                        {
                            sample += std::sin(phase * harmonic) * amplitude;
                        }
                        return sample;
                    });
    Tools::normalise(wave);
}

void triangleWave(WavFile &wave, std::uint64_t lengthMilliseconds, double frequency)
{
    validateFrequency(wave, frequency);
    const auto radiansPerFrame = Constants::TAU * frequency / wave.sampleRate();
    generateSamples(wave, lengthMilliseconds,
                    [radiansPerFrame](std::size_t frame)
                    {
                        return (2.0 / std::numbers::pi) *
                               std::asin(std::sin(static_cast<double>(frame) * radiansPerFrame));
                    });
}

void triangleWaveAdditive(WavFile &wave, std::uint64_t lengthMilliseconds, double frequency,
                          std::uint32_t harmonics)
{
    validateFrequency(wave, frequency);
    validateHarmonics(harmonics);
    const auto radiansPerFrame = Constants::TAU * frequency / wave.sampleRate();
    const auto coefficients =
        oddHarmonics(frequency, wave.sampleRate() / 2.0, harmonics, true, true);
    generateSamples(wave, lengthMilliseconds,
                    [&](std::size_t frame)
                    {
                        const auto phase = static_cast<double>(frame) * radiansPerFrame;
                        double sample = 0.0;
                        for (const auto &[harmonic, amplitude] : coefficients)
                        {
                            sample += std::sin(phase * harmonic) * amplitude;
                        }
                        return sample;
                    });
    Tools::normalise(wave);
}

void sawtoothWave(WavFile &wave, std::uint64_t lengthMilliseconds, double frequency)
{
    validateFrequency(wave, frequency);
    const auto cyclesPerFrame = frequency / wave.sampleRate();
    generateSamples(wave, lengthMilliseconds,
                    [cyclesPerFrame](std::size_t frame)
                    {
                        const auto cycles = static_cast<double>(frame) * cyclesPerFrame;
                        return 2.0 * (cycles - std::floor(cycles + 0.5));
                    });
}

void sawtoothWaveAdditive(WavFile &wave, std::uint64_t lengthMilliseconds, double frequency,
                          std::uint32_t harmonics)
{
    validateFrequency(wave, frequency);
    validateHarmonics(harmonics);
    const auto radiansPerFrame = Constants::TAU * frequency / wave.sampleRate();
    const auto nyquist = wave.sampleRate() / 2.0;
    std::vector<Harmonic> coefficients;
    for (std::uint32_t harmonic = 1; harmonic <= harmonics; ++harmonic)
    {
        const auto harmonicValue = static_cast<double>(harmonic);
        if (frequency * harmonicValue >= nyquist)
        {
            break;
        }
        coefficients.emplace_back(harmonicValue, 1.0 / harmonicValue);
    }
    generateSamples(wave, lengthMilliseconds,
                    [&](std::size_t frame)
                    {
                        const auto phase = static_cast<double>(frame) * radiansPerFrame;
                        double sample = 0.0;
                        for (const auto &[harmonic, amplitude] : coefficients)
                        {
                            sample += std::sin(phase * harmonic) * amplitude;
                        }
                        return sample;
                    });
    Tools::normalise(wave);
}

void whiteNoise(WavFile &wave, std::uint64_t lengthMilliseconds)
{
    std::mt19937_64 engine(std::random_device{}());
    whiteNoise(wave, lengthMilliseconds, engine);
}

void whiteNoise(WavFile &wave, std::uint64_t lengthMilliseconds, std::mt19937_64 &engine)
{
    generateSamples(wave, lengthMilliseconds, [&](std::size_t) { return randomSample(engine); });
}

void pinkNoise(WavFile &wave, std::uint64_t lengthMilliseconds)
{
    std::mt19937_64 engine(std::random_device{}());
    pinkNoise(wave, lengthMilliseconds, engine);
}

void pinkNoise(WavFile &wave, std::uint64_t lengthMilliseconds, std::mt19937_64 &engine)
{
    std::array<double, 7> buffers{};
    generateSamples(wave, lengthMilliseconds,
                    [&](std::size_t)
                    {
                        const auto white = randomSample(engine);
                        buffers[0] = 0.998860 * buffers[0] + 0.0555179 * white;
                        buffers[1] = 0.993320 * buffers[1] + 0.0750759 * white;
                        buffers[2] = 0.969000 * buffers[2] + 0.1538520 * white;
                        buffers[3] = 0.866500 * buffers[3] + 0.3104856 * white;
                        buffers[4] = 0.550000 * buffers[4] + 0.5329522 * white;
                        buffers[5] = -0.76160 * buffers[5] - 0.0168980 * white;
                        const auto sample =
                            0.129 * (buffers[0] + buffers[1] + buffers[2] + buffers[3] +
                                     buffers[4] + buffers[5] + buffers[6] + 0.5362 * white);
                        buffers[6] = 0.115926 * white;
                        return sample;
                    });
    Tools::normalise(wave);
}

} // namespace Generators

} // namespace wavlab
