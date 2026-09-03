#pragma once
#include <map>

#include "Entities/Item.h"
#include "Entities/LogEntry.h"
#include "Entities/Location.h"
#include "Entities/Rival.h"

class DataLoader
{
  public:
    static void loadAll();
    static const std::map<std::string, Item> &getItems();
    static const std::map<std::string, LogEntry> &getLogs();
    static const std::map<std::string, Rival> &getRivals();
    static std::map<std::string, Location> &getLocations();

  private:
    static std::map<std::string, Item> items;
    static std::map<std::string, LogEntry> logs;
    static std::map<std::string, Rival> rivals;
    static std::map<std::string, Location> locations;

    static void loadItems();
    static void loadLogs();
    static void loadRivals();
    static void loadLocations();
    static void validateGameData();
};
