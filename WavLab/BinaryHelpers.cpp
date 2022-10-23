#include <vector>
#include "BinaryHelpers.h"

std::string BinaryHelpers::readString(std::ifstream& file, int length)
{
	std::vector<char> buffer;
	buffer.resize(length);
	file.read(&buffer[0], length);
	std::string header(buffer.begin(), buffer.end());
	return header;
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
	} conversionUnion;

	file.read((char*)&conversionUnion.b.b0, 1);
	file.read((char*)&conversionUnion.b.b1, 1);

	return conversionUnion.s;
}

int32_t BinaryHelpers::readInt24(std::ifstream& file)
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
	} conversionUnion;

	file.read((char*)&conversionUnion.b.b0, 1);
	file.read((char*)&conversionUnion.b.b1, 1);
	file.read((char*)&conversionUnion.b.b2, 1);
	conversionUnion.b.b3 = 0;

	return conversionUnion.i;
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
	} conversionUnion;

	file.read((char*)&conversionUnion.b.b0, 1);
	file.read((char*)&conversionUnion.b.b1, 1);
	file.read((char*)&conversionUnion.b.b2, 1);
	file.read((char*)&conversionUnion.b.b3, 1);

	return conversionUnion.i;
}

void BinaryHelpers::writeString(std::ofstream& file, std::string value)
{
	file.write(value.c_str(), value.size());
}

void BinaryHelpers::writeString(std::ofstream& file, std::string value, size_t size)
{
	file.write(value.c_str(), size);
}

void BinaryHelpers::writeInt16(std::ofstream& file, short value)
{
	file.write((char*)&value, 2);
}

void BinaryHelpers::writeInt24(std::ofstream& file, int32_t value)
{
	file.write((char*)&value, 3);
}

void BinaryHelpers::writeInt32(std::ofstream& file, int32_t value)
{
	file.write((char*)&value, 4);
}