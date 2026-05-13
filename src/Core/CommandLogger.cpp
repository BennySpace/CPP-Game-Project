#include "Core/CommandLogger.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{
std::mutex gLogMutex;
int gCommandIndex = 0;
#if defined(DEBUG) || defined(_DEBUG)
bool gLogEnabled = true;
#else
bool gLogEnabled = false;
#endif

std::string timestampNow()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);

    std::tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &nowTime);
#else
    localtime_r(&nowTime, &localTime);
#endif

    std::ostringstream stream;
    stream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return stream.str();
}

std::filesystem::path executableDirectory()
{
#ifdef _WIN32
    char buffer[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (length > 0 && length < MAX_PATH)
    {
        return std::filesystem::path(buffer).parent_path();
    }
#endif
    return std::filesystem::current_path();
}

std::filesystem::path logFilePath()
{
    return executableDirectory() / "majestic_station_commands.log";
}

void appendLogLine(const std::string &line)
{
    if (!gLogEnabled)
    {
        return;
    }

    // The file is reopened on every append on purpose: route logging is tiny,
    // and this keeps the log usable even if the process terminates unexpectedly.
    std::ofstream logFile(logFilePath(), std::ios::app);
    if (!logFile.is_open())
    {
        return;
    }

    logFile << line << '\n';
    logFile.flush();
}
} // namespace

void CommandLogger::setEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(gLogMutex);
#if defined(DEBUG) || defined(_DEBUG)
    (void)enabled;
    gLogEnabled = true;
#else
    // Release builds keep the logger dormant unless the explicit CLI flag is
    // present, so distributed builds stay silent by default.
    gLogEnabled = enabled;
#endif
}

void CommandLogger::beginSession()
{
    std::lock_guard<std::mutex> lock(gLogMutex);
    if (!gLogEnabled)
    {
        return;
    }

    gCommandIndex = 0;
    appendLogLine("=== Session started: " + timestampNow() + " ===");
}

void CommandLogger::logCommand(const std::string &command)
{
    std::lock_guard<std::mutex> lock(gLogMutex);
    if (!gLogEnabled)
    {
        return;
    }

    ++gCommandIndex;
    appendLogLine(std::to_string(gCommandIndex) + " | " + command);
}

void CommandLogger::endSession(const std::string &outcome)
{
    std::lock_guard<std::mutex> lock(gLogMutex);
    if (!gLogEnabled)
    {
        return;
    }

    appendLogLine("=== Session ended: " + outcome + " | " + timestampNow() + " ===");
    appendLogLine("");
}
