#include <vector>
#include "BinaryHelpers.h"

std::string BinaryHelpers::readString(std::ifstream& file, int16_t length)
{
	std::vector<char> buffer;
	buffer.resize(length);
	file.read(&buffer[0], length);
	std::string header(buffer.begin(), buffer.end());
	return header;
}

char BinaryHelpers::readInt8(std::ifstream& file)
{
	char b0;
	file.read((char*)&b0, 1);
	return b0;
}

short BinaryHelpers::readInt16(std::ifstream& file)
{
	union uni
	{
		short s;
		struct bytes
		{
			char b0;
			char b1;
		} b;
	} conversionUnion{};

	file.read((char*)&conversionUnion.b.b0, 1);
	file.read((char*)&conversionUnion.b.b1, 1);

	return conversionUnion.s;
}

int32_t BinaryHelpers::readInt24(std::ifstream& file)
{
	char b0, b1, b2;
	file.read((char*)&b0, 1);
	file.read((char*)&b1, 1);
	file.read((char*)&b2, 1);

    int32_t sampleAsInt = ((b2 & 0xFF) << 16) | ((b1 & 0xFF) << 8) | (b0 & 0xFF);

	//Convert sign of 24-bit number to sign of 32-bit number;
	if (sampleAsInt & 0x800000)
	{
		sampleAsInt = sampleAsInt | ~0xFFFFFF;
	}

	return sampleAsInt;
}

int32_t BinaryHelpers::readInt32(std::ifstream& file)
{
	union uni
	{
		int32_t i;
		struct bytes
		{
			char b0;
			char b1;
			char b2;
			char b3;
		} b;
	} conversionUnion{};

	file.read((char*)&conversionUnion.b.b0, 1);
	file.read((char*)&conversionUnion.b.b1, 1);
	file.read((char*)&conversionUnion.b.b2, 1);
	file.read((char*)&conversionUnion.b.b3, 1);

	return conversionUnion.i;
}

void BinaryHelpers::writeString(std::ofstream& file, const std::string& value)
{
	file.write(value.c_str(), value.size()); // NOLINT(cppcoreguidelines-narrowing-conversions)
}

void BinaryHelpers::writeString(std::ofstream& file, const std::string& value, size_t size)
{
	file.write(value.c_str(), size); // NOLINT(cppcoreguidelines-narrowing-conversions)
}

void BinaryHelpers::writeInt8(std::ofstream& file, int8_t value)
{
	file.write((char*)&value, 1);
}

void BinaryHelpers::writeInt16(std::ofstream& file, int16_t value)
{
	file.write((char*)&value, 2);
}

void BinaryHelpers::writeInt24(std::ofstream& file, int32_t value)
{
	uint8_t bytes[3];
	bytes[2] = (uint8_t)(value >> 16) & 0xFF;
	bytes[1] = (uint8_t)(value >> 8) & 0xFF;
	bytes[0] = (uint8_t)value & 0xFF;

	file.write((char*)&bytes[0], 1);
	file.write((char*)&bytes[1], 1);
	file.write((char*)&bytes[2], 1);
}

void BinaryHelpers::writeInt32(std::ofstream& file, int32_t value)
{
	file.write((char*)&value, 4);
}