#include "UI/ConsoleDecor.h"

#include "Data/TextResources.h"

#include <iostream>

namespace
{
std::size_t visibleUtf8Length(const std::string &text)
{
    std::size_t length = 0;
    bool inAnsi = false;

    for (std::size_t index = 0; index < text.size(); ++index)
    {
        const unsigned char ch = static_cast<unsigned char>(text[index]);

        if (!inAnsi && ch == 0x1B && index + 1 < text.size() && text[index + 1] == '[')
        {
            inAnsi = true;
            ++index;
            continue;
        }

        if (inAnsi)
        {
            if (ch == 'm')
            {
                inAnsi = false;
            }
            continue;
        }

        if ((ch & 0xC0) != 0x80)
        {
            ++length;
        }
    }

    return length;
}
} // namespace

std::size_t utf8Length(const std::string &text)
{
    return visibleUtf8Length(text);
}

std::string repeat(const std::string &token, std::size_t count)
{
    std::string result;
    result.reserve(token.size() * count);
    for (std::size_t index = 0; index < count; ++index)
    {
        result += token;
    }
    return result;
}

std::string color(const std::string &code, const std::string &text)
{
    return "\x1b[" + code + "m" + text + "\x1b[0m";
}

std::string padRightUtf8(const std::string &text, std::size_t width)
{
    const std::size_t currentWidth = utf8Length(text);
    if (currentWidth >= width)
    {
        return text;
    }

    return text + std::string(width - currentWidth, ' ');
}

void printHorizontalFrame(const std::string &left, const std::string &fill, const std::string &right,
                          std::size_t innerWidth)
{
    std::cout << color("36", left + repeat(fill.empty() ? " " : fill, innerWidth) + right) << "\n";
}

void printFramedText(const std::string &text, std::size_t innerWidth, bool centered)
{
    const std::size_t textWidth = utf8Length(text);
    const std::size_t safeWidth = innerWidth > textWidth ? innerWidth : textWidth;
    std::size_t leftPad = 0;
    std::size_t rightPad = 0;

    if (centered)
    {
        leftPad = (safeWidth - textWidth) / 2;
        rightPad = safeWidth - textWidth - leftPad;
    }
    else
    {
        leftPad = 1;
        rightPad = safeWidth > textWidth + leftPad ? safeWidth - textWidth - leftPad : 0;
    }

    std::cout << color("36", "║") << std::string(leftPad, ' ') << color("1;37", text) << std::string(rightPad, ' ')
              << color("36", "║") << "\n";
}

std::string localizeDirection(const std::string &direction)
{
    if (direction == "north")
    {
        return TextResources::get("direction.north");
    }
    if (direction == "south")
    {
        return TextResources::get("direction.south");
    }
    if (direction == "east")
    {
        return TextResources::get("direction.east");
    }
    if (direction == "west")
    {
        return TextResources::get("direction.west");
    }
    return direction;
}

const std::vector<std::string> &orderedDirections()
{
    static const std::vector<std::string> directions = {"north", "east", "south", "west"};
    return directions;
}
