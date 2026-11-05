#include "StringHelpers.h"

std::string StringHelpers::stringReplace(std::string str, std::string const& old, std::string const& replacement)
{
	std::size_t pos = str.find(old);
	if (pos == std::string::npos)
	{
		return str;
	}
	str.replace(pos, old.length(), replacement);
	return str;
}