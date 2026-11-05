#pragma once
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>

namespace wavlab
{

namespace BinaryHelpers
{
	std::string readString(std::ifstream& file, int16_t length);
    int8_t readInt8(std::ifstream& file);
    int16_t readInt16(std::ifstream& file);
	int32_t readInt24(std::ifstream& file);
	int32_t readInt32(std::ifstream& file);

	void writeString(std::ofstream& file, const std::string& value);
	void writeString(std::ofstream& file, const std::string& value, size_t size);
	void writeInt8(std::ofstream& file, int8_t value);
	void writeInt16(std::ofstream& file, int16_t value);
	void writeInt24(std::ofstream& file, int32_t value);
	void writeInt32(std::ofstream& file, int32_t value);
}

}
