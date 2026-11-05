#include "StringHelpers.h"

std::filesystem::path StringHelpers::pathWithStemSuffix(const std::filesystem::path &path,
                                                        const std::string &suffix)
{
    auto fileName = path.stem().string() + suffix;
    fileName += path.extension().string();
    return path.parent_path() / fileName;
}
