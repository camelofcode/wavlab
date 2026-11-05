#include "ConversionHelpers.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace wavlab::ConversionHelpers
{

namespace
{

double clampNormalized(double input) noexcept
{
    return std::isnan(input) ? 0.0 : std::clamp(input, -1.0, 1.0);
}

double signedToDouble(std::int64_t input, double negativeScale, double positiveScale) noexcept
{
    return static_cast<double>(input) / (input > 0 ? positiveScale : negativeScale);
}

std::int64_t doubleToSigned(double input, double negativeScale, double positiveScale) noexcept
{
    const double sample = clampNormalized(input);
    return static_cast<std::int64_t>(
        std::round(sample * (sample >= 0.0 ? positiveScale : negativeScale)));
}

} // namespace

double eightBitToDouble(std::uint8_t input) noexcept
{
    constexpr std::int16_t silence = 128;
    const auto centered = static_cast<std::int16_t>(input) - silence;
    return static_cast<double>(centered) / (centered > 0 ? 127.0 : 128.0);
}

std::uint8_t doubleToEightBit(double input) noexcept
{
    constexpr std::int16_t silence = 128;
    const double sample = clampNormalized(input);
    const double scale = sample >= 0.0 ? 127.0 : 128.0;
    return static_cast<std::uint8_t>(static_cast<std::int16_t>(std::round(sample * scale)) +
                                     silence);
}

double sixteenBitToDouble(std::int16_t input) noexcept
{
    return signedToDouble(input, 32'768.0, 32'767.0);
}

std::int16_t doubleToSixteenBit(double input) noexcept
{
    return static_cast<std::int16_t>(doubleToSigned(input, 32'768.0, 32'767.0));
}

double twentyFourBitToDouble(std::int32_t input) noexcept
{
    return signedToDouble(input, 8'388'608.0, 8'388'607.0);
}

std::int32_t doubleToTwentyFourBit(double input) noexcept
{
    return static_cast<std::int32_t>(doubleToSigned(input, 8'388'608.0, 8'388'607.0));
}

double thirtyTwoBitToDouble(std::int32_t input) noexcept
{
    return signedToDouble(input, 2'147'483'648.0, 2'147'483'647.0);
}

std::int32_t doubleToThirtyTwoBit(double input) noexcept
{
    return static_cast<std::int32_t>(doubleToSigned(input, 2'147'483'648.0, 2'147'483'647.0));
}

} // namespace wavlab::ConversionHelpers
