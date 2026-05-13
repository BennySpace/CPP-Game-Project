#pragma once

#include <string>

struct ParsedCommand
{
    std::string canonicalVerb;
    std::string argument;
    bool isValid = false;
};

class CommandParser
{
  public:
    static ParsedCommand parse(const std::string &rawCommand);
};
