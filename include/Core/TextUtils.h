#pragma once

#include <string>

namespace TextUtils
{
std::string trimAsciiWhitespace(const std::string &text);
std::string toLowerForLookup(std::string text);
std::string normalizeLookupToken(const std::string &text, bool replaceSpacesWithUnderscores = true);
} // namespace TextUtils
