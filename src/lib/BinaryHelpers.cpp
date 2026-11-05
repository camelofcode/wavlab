#include "BinaryHelpers.h"

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>
#include <wavlab/WavFile.h>

namespace wavlab::BinaryHelpers
{

namespace
{

template <std::size_t Size> std::array<std::uint8_t, Size> readBytes(std::istream &stream)
{
    std::array<std::uint8_t, Size> bytes{};
    stream.read(reinterpret_cast<char *>(bytes.data()), static_cast<std::streamsize>(Size));
    if (!stream)
    {
        throw WavError(WavErrorCode::io, "unexpected end of WAV input");
    }
    return bytes;
}

void writeBytes(std::ostream &stream, const std::uint8_t *bytes, std::size_t size)
{
    stream.write(reinterpret_cast<const char *>(bytes), static_cast<std::streamsize>(size));
    if (!stream)
    {
        throw WavError(WavErrorCode::io, "failed while writing WAV output");
    }
}

} // namespace

std::string readString(std::istream &stream, std::size_t length)
{
    std::string value(length, '\0');
    stream.read(value.data(), static_cast<std::streamsize>(length));
    if (!stream)
    {
        throw WavError(WavErrorCode::io, "unexpected end of WAV input");
    }
    return value;
}

std::vector<std::uint8_t> readBytes(std::istream &stream, std::size_t length)
{
    std::vector<std::uint8_t> value(length);
    stream.read(reinterpret_cast<char *>(value.data()), static_cast<std::streamsize>(length));
    if (!stream)
    {
        throw WavError(WavErrorCode::io, "unexpected end of WAV input");
    }
    return value;
}

std::uint8_t readUInt8(std::istream &stream) { return readBytes<1>(stream)[0]; }

std::uint16_t readUInt16(std::istream &stream)
{
    const auto bytes = readBytes<2>(stream);
    return static_cast<std::uint16_t>(bytes[0]) |
           static_cast<std::uint16_t>(static_cast<std::uint16_t>(bytes[1]) << 8U);
}

std::int16_t readInt16(std::istream &stream)
{
    return std::bit_cast<std::int16_t>(readUInt16(stream));
}

std::uint32_t readUInt32(std::istream &stream)
{
    const auto bytes = readBytes<4>(stream);
    return static_cast<std::uint32_t>(bytes[0]) | (static_cast<std::uint32_t>(bytes[1]) << 8U) |
           (static_cast<std::uint32_t>(bytes[2]) << 16U) |
           (static_cast<std::uint32_t>(bytes[3]) << 24U);
}

std::int32_t readInt24(std::istream &stream)
{
    const auto bytes = readBytes<3>(stream);
    std::uint32_t value = static_cast<std::uint32_t>(bytes[0]) |
                          (static_cast<std::uint32_t>(bytes[1]) << 8U) |
                          (static_cast<std::uint32_t>(bytes[2]) << 16U);
    if ((value & 0x0080'0000U) != 0)
    {
        value |= 0xFF00'0000U;
    }
    return std::bit_cast<std::int32_t>(value);
}

std::int32_t readInt32(std::istream &stream)
{
    return std::bit_cast<std::int32_t>(readUInt32(stream));
}

void writeString(std::ostream &stream, std::string_view value)
{
    stream.write(value.data(), static_cast<std::streamsize>(value.size()));
    if (!stream)
    {
        throw WavError(WavErrorCode::io, "failed while writing WAV output");
    }
}

void writeBytes(std::ostream &stream, const std::vector<std::uint8_t> &value)
{
    writeBytes(stream, value.data(), value.size());
}

void writeFixedString(std::ostream &stream, std::string_view value, std::size_t size)
{
    if (value.size() > size)
    {
        throw WavError(WavErrorCode::invalidFile, "fixed WAV text field is " +
                                                      std::to_string(value.size()) +
                                                      " bytes; maximum is " + std::to_string(size));
    }
    std::vector<std::uint8_t> bytes(size, 0);
    for (std::size_t index = 0; index < value.size(); ++index)
    {
        bytes[index] = static_cast<std::uint8_t>(value[index]);
    }
    writeBytes(stream, bytes.data(), bytes.size());
}

void writeUInt8(std::ostream &stream, std::uint8_t value) { writeBytes(stream, &value, 1); }

void writeUInt16(std::ostream &stream, std::uint16_t value)
{
    const std::array<std::uint8_t, 2> bytes{static_cast<std::uint8_t>(value),
                                            static_cast<std::uint8_t>(value >> 8U)};
    writeBytes(stream, bytes.data(), bytes.size());
}

void writeInt16(std::ostream &stream, std::int16_t value)
{
    writeUInt16(stream, std::bit_cast<std::uint16_t>(value));
}

void writeUInt32(std::ostream &stream, std::uint32_t value)
{
    const std::array<std::uint8_t, 4> bytes{
        static_cast<std::uint8_t>(value), static_cast<std::uint8_t>(value >> 8U),
        static_cast<std::uint8_t>(value >> 16U), static_cast<std::uint8_t>(value >> 24U)};
    writeBytes(stream, bytes.data(), bytes.size());
}

void writeInt24(std::ostream &stream, std::int32_t value)
{
    if (value < -8'388'608 || value > 8'388'607)
    {
        throw WavError(WavErrorCode::invalidFile,
                       std::to_string(value) + " does not fit in a signed 24-bit integer");
    }
    const auto unsignedValue = std::bit_cast<std::uint32_t>(value);
    const std::array<std::uint8_t, 3> bytes{static_cast<std::uint8_t>(unsignedValue),
                                            static_cast<std::uint8_t>(unsignedValue >> 8U),
                                            static_cast<std::uint8_t>(unsignedValue >> 16U)};
    writeBytes(stream, bytes.data(), bytes.size());
}

void writeInt32(std::ostream &stream, std::int32_t value)
{
    writeUInt32(stream, std::bit_cast<std::uint32_t>(value));
}

} // namespace wavlab::BinaryHelpers
