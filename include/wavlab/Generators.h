#pragma once

#include <wavlab/WavFile.h>

#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>

namespace wavlab
{

enum class GeneratorErrorCode
{
    invalidFrequency,
    invalidHarmonicCount,
    sizeOverflow,
};

class GeneratorError : public std::invalid_argument
{
  public:
    GeneratorError(GeneratorErrorCode code, std::string message);

    [[nodiscard]] GeneratorErrorCode code() const noexcept;

  private:
    GeneratorErrorCode code_;
};

namespace Generators
{

/// Replaces the samples with a sine wave copied identically to every channel.
void sineWave(WavFile &wave, std::uint64_t lengthMilliseconds, double frequency);

/// Replaces the samples with a directly generated square wave.
void squareWave(WavFile &wave, std::uint64_t lengthMilliseconds, double frequency);

/// Generates a band-limited square approximation from odd harmonics.
void squareWaveAdditive(WavFile &wave, std::uint64_t lengthMilliseconds, double frequency,
                        std::uint32_t harmonics);

/// Replaces the samples with a directly generated triangle wave.
void triangleWave(WavFile &wave, std::uint64_t lengthMilliseconds, double frequency);

/// Generates a band-limited triangle approximation from odd harmonics.
void triangleWaveAdditive(WavFile &wave, std::uint64_t lengthMilliseconds, double frequency,
                          std::uint32_t harmonics);

/// Replaces the samples with a directly generated rising sawtooth wave.
void sawtoothWave(WavFile &wave, std::uint64_t lengthMilliseconds, double frequency);

/// Generates a band-limited sawtooth approximation from consecutive harmonics.
void sawtoothWaveAdditive(WavFile &wave, std::uint64_t lengthMilliseconds, double frequency,
                          std::uint32_t harmonics);

/// Generates uniformly distributed white noise using a nondeterministic seed.
void whiteNoise(WavFile &wave, std::uint64_t lengthMilliseconds);

/// Generates white noise using the supplied engine, allowing deterministic tests.
void whiteNoise(WavFile &wave, std::uint64_t lengthMilliseconds, std::mt19937_64 &engine);

/// Generates filtered, peak-normalised pink noise using a nondeterministic seed.
void pinkNoise(WavFile &wave, std::uint64_t lengthMilliseconds);

/// Generates pink noise using the supplied engine, allowing deterministic tests.
void pinkNoise(WavFile &wave, std::uint64_t lengthMilliseconds, std::mt19937_64 &engine);

} // namespace Generators

} // namespace wavlab
