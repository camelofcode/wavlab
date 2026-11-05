#pragma once

#include <cstdint>

namespace wavlab::ConversionHelpers
{

double eightBitToDouble(std::uint8_t input) noexcept;
std::uint8_t doubleToEightBit(double input) noexcept;
double sixteenBitToDouble(std::int16_t input) noexcept;
std::int16_t doubleToSixteenBit(double input) noexcept;
double twentyFourBitToDouble(std::int32_t input) noexcept;
std::int32_t doubleToTwentyFourBit(double input) noexcept;
double thirtyTwoBitToDouble(std::int32_t input) noexcept;
std::int32_t doubleToThirtyTwoBit(double input) noexcept;

} // namespace wavlab::ConversionHelpers
