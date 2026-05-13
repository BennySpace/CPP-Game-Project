#include "Data/TextResources.h"

#include "Data/DataUtils.h"
#include "Data/EmbeddedData.h"

#include <fstream>
#include <memory>
#include <sstream>
#include <string_view>

std::map<std::string, std::string> TextResources::strings;

namespace
{
std::unique_ptr<std::istream> openStringsStream()
{
    auto file = std::make_unique<std::ifstream>("data/strings.txt");
    if (file->is_open())
    {
        return file;
    }

    return std::make_unique<std::istringstream>(std::string(EmbeddedData::kStringsText));
}
} // namespace

void TextResources::loadAll()
{
    strings.clear();
    loadStrings();
}

std::string TextResources::get(const std::string &key)
{
    const auto it = strings.find(key);
    if (it != strings.end())
    {
        return it->second;
    }

    return "[missing text: " + key + "]";
}

std::string TextResources::format(const std::string &key, const std::map<std::string, std::string> &replacements)
{
    std::string text = get(key);
    for (const auto &[name, value] : replacements)
    {
        DataUtils::replaceAll(text, "{" + name + "}", value);
    }
    return text;
}

void TextResources::loadStrings()
{
    std::unique_ptr<std::istream> stream = openStringsStream();

    std::string line;
    while (std::getline(*stream, line))
    {
        DataUtils::stripUtf8Bom(line);
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        const auto separator = line.find('=');
        if (separator == std::string::npos)
        {
            continue;
        }

        const std::string key = DataUtils::trim(line.substr(0, separator));
        const std::string value = DataUtils::decodeEscapes(DataUtils::trim(line.substr(separator + 1)));
        strings[key] = value;
    }
}
