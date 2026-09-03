#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace DataUtils
{
std::vector<std::string> split(const std::string &line, char delimiter);
std::string trim(const std::string &value);
void replaceAll(std::string &text, const std::string &from, const std::string &to);
std::string decodeEscapes(std::string text);
void stripUtf8Bom(std::string &text);
std::filesystem::path executableDirectory();
} // namespace DataUtils
