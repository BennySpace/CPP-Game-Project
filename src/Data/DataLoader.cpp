#include "Data/DataLoader.h"

#include "Data/DataUtils.h"
#include "Data/EmbeddedData.h"

#include <fstream>
#include <memory>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

std::map<std::string, Item> DataLoader::items;
std::map<std::string, LogEntry> DataLoader::logs;
std::map<std::string, Rival> DataLoader::rivals;
std::map<std::string, Location> DataLoader::locations;

namespace
{
std::unique_ptr<std::istream> openDataStream(const char *filePath, std::string_view embeddedText)
{
    auto file = std::make_unique<std::ifstream>(DataUtils::executableDirectory() / filePath);
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

[[noreturn]] void throwInvalidRecord(const char *fileName, int lineNumber, const std::string &reason)
{
    throw std::runtime_error(std::string("Invalid record in ") + fileName + " at line " +
                             std::to_string(lineNumber) + ": " + reason + ".");
}

template <typename Record>
void insertRecordOrThrow(std::map<std::string, Record> &records, Record record, const char *fileName, int lineNumber)
{
    if (record.id.empty())
    {
        throwInvalidRecord(fileName, lineNumber, "id must not be empty");
    }

    const auto [it, inserted] = records.emplace(record.id, std::move(record));
    if (!inserted)
    {
        throwInvalidRecord(fileName, lineNumber, "duplicate id '" + it->first + "'");
    }
}

void requireNonNegative(int value, const char *fileName, int lineNumber, const char *fieldName,
                        const std::string &recordId)
{
    if (value < 0)
    {
        throwInvalidRecord(fileName, lineNumber,
                           "record '" + recordId + "' has negative " + fieldName + " '" +
                               std::to_string(value) + "'");
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
    validateGameData();
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
            throwInvalidRecord("data/items.txt", lineNumber, "expected at least 4 fields");
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
        requireNonNegative(item.value, "data/items.txt", lineNumber, "value", item.id);
        requireNonNegative(item.charges, "data/items.txt", lineNumber, "charges", item.id);
        insertRecordOrThrow(items, std::move(item), "data/items.txt", lineNumber);
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
            throwInvalidRecord("data/rivals.txt", lineNumber, "expected at least 5 fields");
        }

        Rival rival;
        rival.id = tokens[0];
        rival.name = tokens[1];
        rival.description = tokens[2];
        rival.maxHp = parseIntOrThrow(tokens[3], "data/rivals.txt", lineNumber, "maxHp", rival.id);
        rival.atk = parseIntOrThrow(tokens[4], "data/rivals.txt", lineNumber, "atk", rival.id);
        rival.attackText = tokens.size() > 5 ? tokens[5] : "";
        rival.defeatText = tokens.size() > 6 ? tokens[6] : "";
        if (rival.maxHp <= 0)
        {
            throwInvalidRecord("data/rivals.txt", lineNumber,
                               "record '" + rival.id + "' must have positive maxHp");
        }
        requireNonNegative(rival.atk, "data/rivals.txt", lineNumber, "atk", rival.id);
        insertRecordOrThrow(rivals, std::move(rival), "data/rivals.txt", lineNumber);
    }
}

void DataLoader::loadLogs()
{
    std::unique_ptr<std::istream> stream = openDataStream("data/logs.txt", EmbeddedData::kLogsText);

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
        if (tokens.size() < 3)
        {
            throwInvalidRecord("data/logs.txt", lineNumber, "expected at least 3 fields");
        }

        LogEntry log;
        log.id = tokens[0];
        log.name = tokens[1];
        log.body = DataUtils::decodeEscapes(tokens[2]);
        insertRecordOrThrow(logs, std::move(log), "data/logs.txt", lineNumber);
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
            throwInvalidRecord("data/locations.txt", lineNumber, "expected at least 4 fields");
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
                throwInvalidRecord("data/locations.txt", lineNumber, "exit must use direction:target format");
            }

            const auto direction = exitToken.substr(0, separator);
            const auto target = exitToken.substr(separator + 1);
            if (direction.empty() || target.empty())
            {
                throwInvalidRecord("data/locations.txt", lineNumber, "exit direction and target must not be empty");
            }

            if (!location.exits.emplace(direction, target).second)
            {
                throwInvalidRecord("data/locations.txt", lineNumber, "duplicate exit direction '" + direction + "'");
            }
        }

        if (tokens.size() > 4 && !tokens[4].empty())
        {
            location.itemIds = DataUtils::split(tokens[4], ',');
            for (const std::string &itemId : location.itemIds)
            {
                if (items.find(itemId) == items.end())
                {
                    throwInvalidRecord("data/locations.txt", lineNumber,
                                       "location '" + location.id + "' references unknown item '" + itemId + "'");
                }
            }
        }

        if (tokens.size() > 5 && !tokens[5].empty())
        {
            location.logIds = DataUtils::split(tokens[5], ',');
            for (const std::string &logId : location.logIds)
            {
                if (logs.find(logId) == logs.end())
                {
                    throwInvalidRecord("data/locations.txt", lineNumber,
                                       "location '" + location.id + "' references unknown log '" + logId + "'");
                }
            }
        }

        if (tokens.size() > 6 && !tokens[6].empty())
        {
            location.rivalId = tokens[6];
            const auto rivalIt = rivals.find(location.rivalId);
            if (rivalIt != rivals.end())
            {
                location.rivalHp = rivalIt->second.maxHp;
            }
            else
            {
                throwInvalidRecord("data/locations.txt", lineNumber,
                                   "location '" + location.id + "' references unknown rival '" + location.rivalId +
                                       "'");
            }
        }

        insertRecordOrThrow(locations, std::move(location), "data/locations.txt", lineNumber);
    }
}

void DataLoader::validateGameData()
{
    if (locations.find("dock") == locations.end())
    {
        throw std::runtime_error("Missing required location 'dock' in data/locations.txt.");
    }

    for (const auto &[locationId, location] : locations)
    {
        for (const auto &[direction, targetId] : location.exits)
        {
            if (locations.find(targetId) == locations.end())
            {
                throw std::runtime_error("Unknown exit target in data/locations.txt for location '" + locationId +
                                         "', direction '" + direction + "': '" + targetId + "'.");
            }
        }
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
