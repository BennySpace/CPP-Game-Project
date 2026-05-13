#pragma once

#include <map>
#include <string>
#include <vector>

struct Location
{
    std::string id;
    std::string name;
    std::string description;
    std::map<std::string, std::string> exits;
    std::vector<std::string> itemIds;
    std::vector<std::string> logIds;
    std::string rivalId;
    int rivalHp = 0;
};
