#pragma once

#include <cstddef>
#include <string>
#include <vector>

std::size_t utf8Length(const std::string &text);
std::string repeat(const std::string &token, std::size_t count);
std::string color(const std::string &code, const std::string &text);
std::string padRightUtf8(const std::string &text, std::size_t width);
void printHorizontalFrame(const std::string &left, const std::string &fill, const std::string &right,
                          std::size_t innerWidth);
void printFramedText(const std::string &text, std::size_t innerWidth, bool centered);
std::string localizeDirection(const std::string &direction);
const std::vector<std::string> &orderedDirections();
