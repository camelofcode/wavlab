#pragma once

#include <cstddef>
#include <cstdint>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace wavlab::BinaryHelpers
{

std::string readString(std::istream &stream, std::size_t length);
std::vector<std::uint8_t> readBytes(std::istream &stream, std::size_t length);
std::uint8_t readUInt8(std::istream &stream);
std::uint16_t readUInt16(std::istream &stream);
std::int16_t readInt16(std::istream &stream);
std::uint32_t readUInt32(std::istream &stream);
std::int32_t readInt24(std::istream &stream);
std::int32_t readInt32(std::istream &stream);

void writeString(std::ostream &stream, std::string_view value);
void writeBytes(std::ostream &stream, const std::vector<std::uint8_t> &value);
void writeFixedString(std::ostream &stream, std::string_view value, std::size_t size);
void writeUInt8(std::ostream &stream, std::uint8_t value);
void writeUInt16(std::ostream &stream, std::uint16_t value);
void writeInt16(std::ostream &stream, std::int16_t value);
void writeUInt32(std::ostream &stream, std::uint32_t value);
void writeInt24(std::ostream &stream, std::int32_t value);
void writeInt32(std::ostream &stream, std::int32_t value);

} // namespace wavlab::BinaryHelpers
