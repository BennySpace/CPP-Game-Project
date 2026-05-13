#pragma once

#include <string>

namespace CommandLogger
{
void setEnabled(bool enabled);
void beginSession();
void logCommand(const std::string &command);
void endSession(const std::string &outcome);
} // namespace CommandLogger
