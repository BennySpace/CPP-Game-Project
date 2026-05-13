#include "Data/DataLoader.h"

#include "Data/DataUtils.h"
#include "Data/EmbeddedData.h"

#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string_view>

std::map<std::string, Item> DataLoader::items;
std::map<std::string, LogEntry> DataLoader::logs;
std::map<std::string, Rival> DataLoader::rivals;
std::map<std::string, Location> DataLoader::locations;

namespace
{
std::unique_ptr<std::istream> openDataStream(const char *filePath, std::string_view embeddedText)
{
    auto file = std::make_unique<std::ifstream>(filePath);
    if (file->is_open())
    {
        return file;
    }

    return std::make_unique<std::istringstream>(std::string(embeddedText));
}

int parseIntOrThrow(const std::string &value, const char *fileName, int lineNumber, const char *fieldName,
                    const std::string &recordId)
{
    try
    {
        std::size_t consumed = 0;
        const int parsed = std::stoi(value, &consumed);
        if (consumed != value.size())
        {
            throw std::invalid_argument("trailing characters");
        }
        return parsed;
    }
    catch (const std::exception &)
    {
        throw std::runtime_error(std::string("Invalid numeric value in ") + fileName + " at line " +
                                 std::to_string(lineNumber) + " for record '" + recordId + "', field '" + fieldName +
                                 "': '" + value + "'.");
    }
}
} // namespace

void DataLoader::loadAll()
{
    items.clear();
    logs.clear();
    rivals.clear();
    locations.clear();
    loadItems();
    loadLogs();
    loadRivals();
    loadLocations();
}

void DataLoader::loadItems()
{
    std::unique_ptr<std::istream> stream = openDataStream("data/items.txt", EmbeddedData::kItemsText);

    std::string line;
    int lineNumber = 0;
    while (std::getline(*stream, line))
    {
        ++lineNumber;
        DataUtils::stripUtf8Bom(line);
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        const auto tokens = DataUtils::split(line, '|');
        if (tokens.size() < 4)
        {
            continue;
        }

        Item item;
        item.id = tokens[0];
        item.name = tokens[1];
        item.description = tokens[2];
        item.type = tokens[3];
        item.value = tokens.size() > 4 ? parseIntOrThrow(tokens[4], "data/items.txt", lineNumber, "value", item.id) : 0;
        item.charges =
            tokens.size() > 5 ? parseIntOrThrow(tokens[5], "data/items.txt", lineNumber, "charges", item.id) : 0;
        item.useText = tokens.size() > 6 ? tokens[6] : "";
        items[item.id] = item;
    }
}

void DataLoader::loadRivals()
{
    std::unique_ptr<std::istream> stream = openDataStream("data/rivals.txt", EmbeddedData::kRivalsText);

    std::string line;
    int lineNumber = 0;
    while (std::getline(*stream, line))
    {
        ++lineNumber;
        DataUtils::stripUtf8Bom(line);
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        const auto tokens = DataUtils::split(line, '|');
        if (tokens.size() < 5)
        {
            continue;
        }

        Rival rival;
        rival.id = tokens[0];
        rival.name = tokens[1];
        rival.description = tokens[2];
        rival.maxHp = parseIntOrThrow(tokens[3], "data/rivals.txt", lineNumber, "maxHp", rival.id);
        rival.atk = parseIntOrThrow(tokens[4], "data/rivals.txt", lineNumber, "atk", rival.id);
        rival.attackText = tokens.size() > 5 ? tokens[5] : "";
        rival.defeatText = tokens.size() > 6 ? tokens[6] : "";
        rivals[rival.id] = rival;
    }
}

void DataLoader::loadLogs()
{
    std::unique_ptr<std::istream> stream = openDataStream("data/logs.txt", EmbeddedData::kLogsText);

    std::string line;
    while (std::getline(*stream, line))
    {
        DataUtils::stripUtf8Bom(line);
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        const auto tokens = DataUtils::split(line, '|');
        if (tokens.size() < 3)
        {
            continue;
        }

        LogEntry log;
        log.id = tokens[0];
        log.name = tokens[1];
        log.body = DataUtils::decodeEscapes(tokens[2]);
        logs[log.id] = log;
    }
}

void DataLoader::loadLocations()
{
    std::unique_ptr<std::istream> stream = openDataStream("data/locations.txt", EmbeddedData::kLocationsText);

    std::string line;
    int lineNumber = 0;
    while (std::getline(*stream, line))
    {
        ++lineNumber;
        DataUtils::stripUtf8Bom(line);
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        const auto tokens = DataUtils::split(line, '|');
        if (tokens.size() < 4)
        {
            continue;
        }

        Location location;
        location.id = tokens[0];
        location.name = tokens[1];
        location.description = tokens[2];

        for (const auto &exitToken : DataUtils::split(tokens[3], ','))
        {
            const auto separator = exitToken.find(':');
            if (separator == std::string::npos)
            {
                continue;
            }

            const auto direction = exitToken.substr(0, separator);
            const auto target = exitToken.substr(separator + 1);
            location.exits[direction] = target;
        }

        if (tokens.size() > 4 && !tokens[4].empty())
        {
            location.itemIds = DataUtils::split(tokens[4], ',');
        }

        if (tokens.size() > 5)
        {
            // Support both generations of the content format:
            // - legacy: exits|items|rival
            // - current: exits|items|logs|rival
            if (tokens.size() == 6)
            {
                const auto rivalIt = rivals.find(tokens[5]);
                if (rivalIt != rivals.end())
                {
                    location.rivalId = tokens[5];
                    location.rivalHp = rivalIt->second.maxHp;
                }
                else if (!tokens[5].empty())
                {
                    location.logIds = DataUtils::split(tokens[5], ',');
                }
            }
            else
            {
                if (!tokens[5].empty())
                {
                    location.logIds = DataUtils::split(tokens[5], ',');
                }

                if (!tokens[6].empty())
                {
                    location.rivalId = tokens[6];
                    const auto rivalIt = rivals.find(location.rivalId);
                    if (rivalIt != rivals.end())
                    {
                        location.rivalHp = rivalIt->second.maxHp;
                    }
                    else
                    {
                        throw std::runtime_error("Unknown rival id in data/locations.txt at line " +
                                                 std::to_string(lineNumber) + " for location '" + location.id + "': '" +
                                                 location.rivalId + "'.");
                    }
                }
            }
        }

        locations[location.id] = location;
    }
}

const std::map<std::string, Item> &DataLoader::getItems()
{
    return items;
}

const std::map<std::string, LogEntry> &DataLoader::getLogs()
{
    return logs;
}

const std::map<std::string, Rival> &DataLoader::getRivals()
{
    return rivals;
}

std::map<std::string, Location> &DataLoader::getLocations()
{
    return locations;
}
