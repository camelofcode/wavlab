#include "BinaryHelpers.h"
#include "ConversionHelpers.h"

#include <wavlab/WavFile.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <string_view>

namespace
{

int failures = 0;

void check(bool condition, std::string_view message)
{
    if (!condition)
    {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

} // namespace

int main()
{
    using namespace wavlab;

    std::stringstream bytes(std::ios::in | std::ios::out | std::ios::binary);
    BinaryHelpers::writeUInt8(bytes, 0xAB);
    BinaryHelpers::writeUInt16(bytes, 0x1234);
    BinaryHelpers::writeInt16(bytes, -2);
    BinaryHelpers::writeUInt32(bytes, 0x1234'5678);
    BinaryHelpers::writeInt32(bytes, -0x1234'5678);
    const auto encoded = bytes.str();
    constexpr std::array<std::uint8_t, 13> expectedBytes{0xAB, 0x34, 0x12, 0xFE, 0xFF, 0x78, 0x56,
                                                         0x34, 0x12, 0x88, 0xA9, 0xCB, 0xED};
    check(encoded.size() == expectedBytes.size(), "writes the expected number of bytes");
    for (std::size_t index = 0; index < expectedBytes.size(); ++index)
    {
        check(static_cast<std::uint8_t>(encoded[index]) == expectedBytes[index],
              "writes explicit little-endian bytes");
    }
    bytes.seekg(0);
    check(BinaryHelpers::readUInt8(bytes) == 0xAB, "reads and writes an unsigned byte");
    check(BinaryHelpers::readUInt16(bytes) == 0x1234, "reads and writes little-endian u16");
    check(BinaryHelpers::readInt16(bytes) == -2, "reads and writes little-endian i16");
    check(BinaryHelpers::readUInt32(bytes) == 0x1234'5678, "reads and writes little-endian u32");
    check(BinaryHelpers::readInt32(bytes) == -0x1234'5678, "reads and writes little-endian i32");

    constexpr std::array<std::int32_t, 5> int24Values{-8'388'608, -1, 0, 1, 8'388'607};
    for (const auto value : int24Values)
    {
        std::stringstream sample(std::ios::in | std::ios::out | std::ios::binary);
        BinaryHelpers::writeInt24(sample, value);
        sample.seekg(0);
        check(BinaryHelpers::readInt24(sample) == value, "round trips a signed 24-bit value");
    }

    try
    {
        std::istringstream truncated(std::string{"\x01\x02", 2}, std::ios::binary);
        static_cast<void>(BinaryHelpers::readUInt32(truncated));
        check(false, "truncated input throws a typed error");
    }
    catch (const WavError &error)
    {
        check(error.code() == WavErrorCode::io, "truncated input throws a typed I/O error");
    }

    check(ConversionHelpers::eightBitToDouble(0) == -1.0, "8-bit zero is negative full scale");
    check(ConversionHelpers::eightBitToDouble(128) == 0.0, "8-bit 128 is silence");
    check(ConversionHelpers::eightBitToDouble(255) == 1.0, "8-bit 255 is positive full scale");
    check(ConversionHelpers::doubleToEightBit(-1.0) == 0, "encodes 8-bit negative full scale");
    check(ConversionHelpers::doubleToEightBit(0.0) == 128, "encodes 8-bit silence");
    check(ConversionHelpers::doubleToEightBit(1.0) == 255, "encodes 8-bit positive full scale");

    check(ConversionHelpers::sixteenBitToDouble(std::numeric_limits<std::int16_t>::min()) == -1.0,
          "decodes 16-bit negative full scale");
    check(ConversionHelpers::sixteenBitToDouble(std::numeric_limits<std::int16_t>::max()) == 1.0,
          "decodes 16-bit positive full scale");
    check(ConversionHelpers::twentyFourBitToDouble(-8'388'608) == -1.0,
          "decodes 24-bit negative full scale");
    check(ConversionHelpers::twentyFourBitToDouble(8'388'607) == 1.0,
          "decodes 24-bit positive full scale");
    check(ConversionHelpers::thirtyTwoBitToDouble(std::numeric_limits<std::int32_t>::min()) == -1.0,
          "decodes 32-bit negative full scale");
    check(ConversionHelpers::thirtyTwoBitToDouble(std::numeric_limits<std::int32_t>::max()) == 1.0,
          "decodes 32-bit positive full scale");

    const double nan = std::numeric_limits<double>::quiet_NaN();
    check(ConversionHelpers::doubleToEightBit(nan) == 128, "converts NaN to 8-bit silence");
    check(ConversionHelpers::doubleToSixteenBit(nan) == 0, "converts NaN to 16-bit silence");
    check(ConversionHelpers::doubleToTwentyFourBit(-2.0) == -8'388'608, "clamps negative samples");
    check(ConversionHelpers::doubleToThirtyTwoBit(2.0) == std::numeric_limits<std::int32_t>::max(),
          "clamps positive samples");

    return failures == 0 ? 0 : 1;
}
