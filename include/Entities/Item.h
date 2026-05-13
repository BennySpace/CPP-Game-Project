#pragma once

#include <string>

struct Item
{
    std::string id;
    std::string name;
    std::string description;
    std::string type;
    int value = 0;
    int charges = 0;
    std::string useText;
};
