#include "Data/DataUtils.h"

#include <sstream>

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
} // namespace DataUtils
