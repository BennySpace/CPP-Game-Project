#include "Core/ConsoleInput.h"

#include <algorithm>
#include <iostream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{
#ifdef _WIN32
constexpr DWORD kNormalizedConsoleInputMode = ENABLE_PROCESSED_INPUT | ENABLE_EXTENDED_FLAGS;

std::vector<std::wstring> gCommandHistory;

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

bool writeWideText(HANDLE outputHandle, const wchar_t *text, DWORD length)
{
    DWORD charsWritten = 0;
    return WriteConsoleW(outputHandle, text, length, &charsWritten, nullptr) != 0;
}

void overwriteLineFrom(HANDLE outputHandle, const COORD &origin, const std::wstring &text, std::size_t previousLength)
{
    SetConsoleCursorPosition(outputHandle, origin);

    const std::size_t clearLength = (std::max)(previousLength, text.size());
    if (clearLength > 0)
    {
        std::wstring blank(clearLength, L' ');
        writeWideText(outputHandle, blank.c_str(), static_cast<DWORD>(blank.size()));
        SetConsoleCursorPosition(outputHandle, origin);
    }

    if (!text.empty())
    {
        writeWideText(outputHandle, text.c_str(), static_cast<DWORD>(text.size()));
    }
}

bool tryReadConsoleLine(std::string &line)
{
    HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
    if (inputHandle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (outputHandle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD inputMode = 0;
    if (GetConsoleMode(inputHandle, &inputMode) == 0)
    {
        return false;
    }

    // The default cooked console editor proved fragile after repeated focus
    // changes, so on Windows we read raw key events and rebuild the line
    // ourselves. If any of this setup fails, the caller falls back to getline.
    DWORD normalizedMode = kNormalizedConsoleInputMode;
    normalizedMode &= ~ENABLE_QUICK_EDIT_MODE;
    normalizedMode &= ~ENABLE_MOUSE_INPUT;
    SetConsoleMode(inputHandle, normalizedMode);

    std::wstring collected;
    std::wstring draft;
    std::size_t historyIndex = gCommandHistory.size();
    CONSOLE_SCREEN_BUFFER_INFO screenInfo{};
    if (GetConsoleScreenBufferInfo(outputHandle, &screenInfo) == 0)
    {
        return false;
    }
    const COORD inputOrigin = screenInfo.dwCursorPosition;

    while (true)
    {
        INPUT_RECORD inputRecord{};
        DWORD recordsRead = 0;
        if (ReadConsoleInputW(inputHandle, &inputRecord, 1, &recordsRead) == 0)
        {
            return false;
        }

        if (recordsRead == 0 || inputRecord.EventType != KEY_EVENT)
        {
            continue;
        }

        const KEY_EVENT_RECORD &keyEvent = inputRecord.Event.KeyEvent;
        if (!keyEvent.bKeyDown)
        {
            continue;
        }

        const wchar_t character = keyEvent.uChar.UnicodeChar;
        if (keyEvent.wVirtualKeyCode == VK_RETURN)
        {
            static constexpr wchar_t kNewline[] = L"\r\n";
            writeWideText(outputHandle, kNewline, 2);
            line = utf8FromWide(collected);
            if (!collected.empty() && (gCommandHistory.empty() || gCommandHistory.back() != collected))
            {
                gCommandHistory.push_back(collected);
            }
            return true;
        }

        if (keyEvent.wVirtualKeyCode == VK_BACK)
        {
            if (!collected.empty())
            {
                const std::size_t previousLength = collected.size();
                collected.pop_back();
                overwriteLineFrom(outputHandle, inputOrigin, collected, previousLength);
            }
            continue;
        }

        if (keyEvent.wVirtualKeyCode == VK_UP)
        {
            if (gCommandHistory.empty() || historyIndex == 0)
            {
                continue;
            }

            if (historyIndex == gCommandHistory.size())
            {
                draft = collected;
            }

            const std::size_t previousLength = collected.size();
            --historyIndex;
            collected = gCommandHistory[historyIndex];
            overwriteLineFrom(outputHandle, inputOrigin, collected, previousLength);
            continue;
        }

        if (keyEvent.wVirtualKeyCode == VK_DOWN)
        {
            if (historyIndex >= gCommandHistory.size())
            {
                continue;
            }

            const std::size_t previousLength = collected.size();
            ++historyIndex;
            collected = historyIndex < gCommandHistory.size() ? gCommandHistory[historyIndex] : draft;
            overwriteLineFrom(outputHandle, inputOrigin, collected, previousLength);
            continue;
        }

        if (character == L'\0' || character == L'\r' || character == L'\n')
        {
            continue;
        }

        if (historyIndex != gCommandHistory.size())
        {
            historyIndex = gCommandHistory.size();
            draft = collected;
        }

        collected.push_back(character);
        writeWideText(outputHandle, &character, 1);
    }
}
#endif
} // namespace

namespace ConsoleInput
{
bool readLine(std::string &line)
{
#ifdef _WIN32
    if (tryReadConsoleLine(line))
    {
        return true;
    }
#endif

    return static_cast<bool>(std::getline(std::cin, line));
}
} // namespace ConsoleInput
