#pragma once

#include <string>

struct Rival
{
    std::string id;
    std::string name;
    std::string description;
    int maxHp = 0;
    int atk = 0;
    std::string attackText;
    std::string defeatText;
};
