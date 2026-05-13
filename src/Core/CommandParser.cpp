#include "Core/CommandParser.h"

#include "Core/TextUtils.h"

#include <sstream>
#include <unordered_map>

namespace
{
std::string normalizeToken(const std::string &text)
{
    return TextUtils::normalizeLookupToken(text);
}

std::string normalizeDirection(const std::string &direction)
{
    const std::string normalized = normalizeToken(direction);

    static const std::unordered_map<std::string, std::string> directionAliases = {
        {"north", "north"}, {"n", "north"}, {"север", "north"}, {"с", "north"},

        {"south", "south"}, {"s", "south"}, {"юг", "south"},    {"ю", "south"},

        {"east", "east"},   {"e", "east"},  {"восток", "east"}, {"в", "east"},

        {"west", "west"},   {"w", "west"},  {"запад", "west"},  {"з", "west"}};

    const auto aliasIt = directionAliases.find(normalized);
    if (aliasIt != directionAliases.end())
    {
        return aliasIt->second;
    }

    return normalized;
}

std::string normalizeArgument(const std::string &canonicalVerb, const std::string &argument)
{
    if (canonicalVerb == "move")
    {
        return normalizeDirection(argument);
    }

    return normalizeToken(argument);
}

const std::unordered_map<std::string, std::string> &commandAliases()
{
    static const std::unordered_map<std::string, std::string> aliases = {{"go", "move"},
                                                                         {"move", "move"},
                                                                         {"идти", "move"},
                                                                         {"иди", "move"},
                                                                         {"go_to", "move"},

                                                                         {"take", "take"},
                                                                         {"get", "take"},
                                                                         {"взять", "take"},
                                                                         {"поднять", "take"},
                                                                         {"забрать", "take"},

                                                                         {"use", "use"},
                                                                         {"использовать", "use"},
                                                                         {"применить", "use"},

                                                                         {"read", "read"},
                                                                         {"читать", "read"},
                                                                         {"лог", "read"},

                                                                         {"attack", "attack"},
                                                                         {"fight", "attack"},
                                                                         {"атаковать", "attack"},
                                                                         {"ударить", "attack"},
                                                                         {"бой", "attack"},

                                                                         {"inspect", "inspect"},
                                                                         {"examine", "inspect"},
                                                                         {"осмотреть", "inspect"},
                                                                         {"изучить", "inspect"},

                                                                         {"look", "look"},
                                                                         {"l", "look"},
                                                                         {"смотреть", "look"},
                                                                         {"осмотр", "look"},

                                                                         {"inventory", "inventory"},
                                                                         {"inv", "inventory"},
                                                                         {"инвентарь", "inventory"},
                                                                         {"рюкзак", "inventory"},

                                                                         {"status", "status"},
                                                                         {"статус", "status"},
                                                                         {"состояние", "status"},

                                                                         {"objective", "objective"},
                                                                         {"mission", "objective"},
                                                                         {"цель", "objective"},
                                                                         {"задача", "objective"},

                                                                         {"map", "map"},
                                                                         {"карта", "map"},

                                                                         {"help", "help"},
                                                                         {"помощь", "help"},

                                                                         {"exit", "exit"},
                                                                         {"quit", "exit"},
                                                                         {"выход", "exit"}};

    return aliases;
}
} // namespace

ParsedCommand CommandParser::parse(const std::string &rawCommand)
{
    std::stringstream stream(rawCommand);
    std::string verb;
    stream >> verb;

    std::string argument;
    std::getline(stream, argument);

    const std::string normalizedVerb = normalizeToken(verb);
    const auto aliasIt = commandAliases().find(normalizedVerb);
    if (aliasIt == commandAliases().end())
    {
        return {"", normalizeToken(argument), false};
    }

    return {aliasIt->second, normalizeArgument(aliasIt->second, argument), true};
}
