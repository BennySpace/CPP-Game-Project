#include "Data/DataUtils.h"

#include <array>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace DataUtils
{
std::vector<std::string> split(const std::string &line, char delimiter)
{
    std::vector<std::string> parts;
    std::stringstream stream(line);
    std::string token;
    while (std::getline(stream, token, delimiter))
    {
        parts.push_back(token);
    }
    return parts;
}

std::string trim(const std::string &value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
    {
        return "";
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

void replaceAll(std::string &text, const std::string &from, const std::string &to)
{
    std::size_t position = 0;
    while ((position = text.find(from, position)) != std::string::npos)
    {
        text.replace(position, from.size(), to);
        position += to.size();
    }
}

std::string decodeEscapes(std::string text)
{
    replaceAll(text, "\\n", "\n");
    replaceAll(text, "\\t", "\t");
    return text;
}

void stripUtf8Bom(std::string &text)
{
    if (text.size() >= 3 && static_cast<unsigned char>(text[0]) == 0xEF &&
        static_cast<unsigned char>(text[1]) == 0xBB && static_cast<unsigned char>(text[2]) == 0xBF)
    {
        text.erase(0, 3);
    }
}

std::filesystem::path executableDirectory()
{
#ifdef _WIN32
    std::array<wchar_t, MAX_PATH> buffer{};
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length > 0 && length < buffer.size())
    {
        return std::filesystem::path(buffer.data()).parent_path();
    }
#endif

    return std::filesystem::current_path();
}
} // namespace DataUtils
