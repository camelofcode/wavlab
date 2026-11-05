#pragma once

#include <wavlab/WavFile.h>

#include <cstdint>
#include <stdexcept>
#include <string>

namespace wavlab
{

enum class FilterType
{
    None = 0,
    Peak,
    AllPass,
    BandPass,
    LowPass,
    HighPass,
    LowShelf,
    HighShelf,
};

enum class EqErrorCode
{
    invalidFilterType,
    invalidSampleRate,
    invalidFrequency,
    invalidQ,
    invalidBandwidth,
    invalidGain,
    invalidCoefficients,
};

class EqError : public std::invalid_argument
{
  public:
    EqError(EqErrorCode code, std::string message);

    [[nodiscard]] EqErrorCode code() const noexcept;

  private:
    EqErrorCode code_;
};

struct BiquadCoefficients
{
    double b0{};
    double b1{};
    double b2{};
    double a1{};
    double a2{};
};

/// Stateful Direct Form I biquad. Use one instance per independently processed channel.
class EQInstance
{
  public:
    [[nodiscard]] static EQInstance peak(double centreFrequency, std::uint32_t sampleRate,
                                         double bandwidthOctaves, double gainDecibels);
    [[nodiscard]] static EQInstance allPass(double centreFrequency, std::uint32_t sampleRate,
                                            double bandwidthOctaves);
    [[nodiscard]] static EQInstance bandPass(double centreFrequency, std::uint32_t sampleRate,
                                             double bandwidthOctaves);
    [[nodiscard]] static EQInstance lowPass(double cutoffFrequency, std::uint32_t sampleRate,
                                            double q);
    [[nodiscard]] static EQInstance highPass(double cutoffFrequency, std::uint32_t sampleRate,
                                             double q);
    [[nodiscard]] static EQInstance lowShelf(double centreFrequency, std::uint32_t sampleRate,
                                             double q, double gainDecibels);
    [[nodiscard]] static EQInstance highShelf(double centreFrequency, std::uint32_t sampleRate,
                                              double q, double gainDecibels);

    [[nodiscard]] double processSample(double input) noexcept;
    [[nodiscard]] const BiquadCoefficients &coefficients() const noexcept;

  private:
    explicit EQInstance(BiquadCoefficients coefficients);

    BiquadCoefficients coefficients_;
    double input1_{};
    double input2_{};
    double output1_{};
    double output2_{};
};

namespace EQ
{

void peak(WavFile &wave, double centreFrequency, double bandwidthOctaves, double gainDecibels);
void allPass(WavFile &wave, double centreFrequency, double bandwidthOctaves);
void bandPass(WavFile &wave, double centreFrequency, double bandwidthOctaves);
void lowPass(WavFile &wave, double cutoffFrequency, double q);
void highPass(WavFile &wave, double cutoffFrequency, double q);
void lowShelf(WavFile &wave, double centreFrequency, double q, double gainDecibels);
void highShelf(WavFile &wave, double centreFrequency, double q, double gainDecibels);

/// Compatibility dispatcher. `shape` is bandwidth for peak/band/all-pass and Q otherwise.
void process(WavFile &wave, FilterType type, double gainDecibels, double centreFrequency,
             double shape);

} // namespace EQ

} // namespace wavlab
