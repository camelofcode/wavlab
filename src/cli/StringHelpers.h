#pragma once
#include <filesystem>
#include <string>

namespace StringHelpers
{
std::filesystem::path pathWithStemSuffix(const std::filesystem::path &path,
                                         const std::string &suffix);
}
