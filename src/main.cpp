#include <clocale>
#include <exception>
#include <iostream>
#include <string>

#ifdef _WIN32
#include "resource.h"
#include <windows.h>
#endif

#include "Core/CommandLogger.h"
#include "Core/Game.h"

namespace
{
bool hasCommandLoggingFlag(int argc, char *argv[])
{
    for (int index = 1; index < argc; ++index)
    {
        if (std::string(argv[index]) == "--log-commands")
        {
            return true;
        }
    }

    return false;
}

void configureConsole()
{
    std::setlocale(LC_ALL, ".UTF-8");

#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
    if (inputHandle != INVALID_HANDLE_VALUE)
    {
        DWORD inputMode = 0;
        if (GetConsoleMode(inputHandle, &inputMode) != 0)
        {
            // Keep the console in a normal line-input state after focus changes.
            inputMode |= ENABLE_PROCESSED_INPUT;
            inputMode |= ENABLE_EXTENDED_FLAGS;
            inputMode &= ~ENABLE_QUICK_EDIT_MODE;
            inputMode &= ~ENABLE_MOUSE_INPUT;
            SetConsoleMode(inputHandle, inputMode);
        }
    }

    HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (outputHandle != INVALID_HANDLE_VALUE)
    {
        DWORD consoleMode = 0;
        if (GetConsoleMode(outputHandle, &consoleMode) != 0)
        {
            // ANSI colors are used across the whole TUI, so VT processing needs to
            // stay enabled even when the rest of the app talks to the console
            // through higher-level iostreams.
            consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(outputHandle, consoleMode);
        }
    }

#ifdef APP_HAS_ICON_RESOURCE
    HWND consoleWindow = GetConsoleWindow();
    if (consoleWindow != nullptr)
    {
        HINSTANCE instanceHandle = GetModuleHandleW(nullptr);
        HICON largeIcon =
            static_cast<HICON>(LoadImageW(instanceHandle, MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON,
                                          GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR));
        HICON smallIcon = static_cast<HICON>(LoadImageW(instanceHandle, MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON,
                                                        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                                                        LR_DEFAULTCOLOR));

        if (largeIcon != nullptr)
        {
            SendMessageW(consoleWindow, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(largeIcon));
        }

        if (smallIcon != nullptr)
        {
            SendMessageW(consoleWindow, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(smallIcon));
        }
    }
#endif
#endif
}
} // namespace

int main(int argc, char *argv[])
{
    configureConsole();
    CommandLogger::setEnabled(hasCommandLoggingFlag(argc, argv));

    try
    {
        Game game;
        game.run();
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << "Ошибка запуска: " << error.what() << '\n';
        return 1;
    }
}
