#pragma once
#include <fstream>
#include <string>

namespace BinaryHelpers
{
	std::string readString(std::ifstream& file, int length);
	short readInt16(std::ifstream& file);
	int32_t readInt24(std::ifstream& file);
	int32_t readInt32(std::ifstream& file);

	void writeString(std::ofstream& file, std::string value);
	void writeString(std::ofstream& file, std::string value, size_t size);
	void writeInt16(std::ofstream& file, short value);
	void writeInt24(std::ofstream& file, int32_t value);
	void writeInt32(std::ofstream& file, int32_t value);
}
