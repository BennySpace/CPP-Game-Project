#include "Core/TextUtils.h"

#include <algorithm>

#ifdef _WIN32
#include <cwctype>
#include <windows.h>
#endif

namespace
{
#ifdef _WIN32
std::wstring wideFromUtf8(const std::string &text)
{
    if (text.empty())
    {
        return L"";
    }

    const int charCount = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (charCount <= 0)
    {
        return L"";
    }

    std::wstring result(static_cast<std::size_t>(charCount), L'\0');
    if (MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, result.data(), charCount) <= 0)
    {
        return L"";
    }

    result.pop_back();
    return result;
}

std::string utf8FromWide(const std::wstring &text)
{
    if (text.empty())
    {
        return "";
    }

    const int byteCount =
        WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    if (byteCount <= 0)
    {
        return "";
    }

    std::string result(static_cast<std::size_t>(byteCount), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), result.data(), byteCount, nullptr,
                        nullptr);
    return result;
}
#endif
} // namespace

namespace TextUtils
{
std::string trimAsciiWhitespace(const std::string &text)
{
    const std::size_t first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
    {
        return "";
    }

    const std::size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

std::string toLowerForLookup(std::string text)
{
#ifdef _WIN32
    if (text.empty())
    {
        return "";
    }

    std::wstring wide = wideFromUtf8(text);
    if (wide.empty())
    {
        return text;
    }

    std::transform(wide.begin(), wide.end(), wide.begin(),
                   [](wchar_t ch) { return static_cast<wchar_t>(towlower(ch)); });
    const std::string lowered = utf8FromWide(wide);
    return lowered.empty() ? text : lowered;
#else
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        if (ch >= 'A' && ch <= 'Z')
        {
            return static_cast<char>(ch - 'A' + 'a');
        }

        return static_cast<char>(ch);
    });
    return text;
#endif
}

std::string normalizeLookupToken(const std::string &text, bool replaceSpacesWithUnderscores)
{
    std::string normalized = toLowerForLookup(trimAsciiWhitespace(text));
    if (replaceSpacesWithUnderscores)
    {
        std::replace(normalized.begin(), normalized.end(), ' ', '_');
    }

    return normalized;
}
} // namespace TextUtils
