#include "Core/Game.h"

#include "Core/CommandParser.h"
#include "detail/GameEventMessageMapper.h"
#include "Core/GameIds.h"
#include "Core/TextUtils.h"
#include "Data/DataLoader.h"
#include "Data/TextResources.h"

void Game::resetSession()
{
    DataLoader::loadAll();
    player = Player{};
    isGameOver = false;
}

GameCommandResult Game::executeCommand(const std::string &rawCommand)
{
    GameCommandResult result = processCommand(CommandParser::parse(rawCommand));
    if (!isGameOver)
    {
        applyEndState(result);
    }
    GameEventMessageMapper::appendMessages(result);
    return result;
}

GameCommandResult Game::processCommand(const ParsedCommand &command)
{
    if (!command.isValid)
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::ActionRejected, "", "", 0, "msg.unknown_command");
        return result;
    }

    if (command.canonicalVerb == "exit")
    {
        isGameOver = true;
        return {};
    }

    if (command.canonicalVerb == "look")
    {
        return describeCurrentLocation();
    }

    if (command.canonicalVerb == "attack")
    {
        return handleAttack();
    }

    const auto viewIt = commandViews().find(command.canonicalVerb);
    if (viewIt != commandViews().end())
    {
        return makeViewResult(viewIt->second);
    }

    const auto handlerIt = commandHandlers().find(command.canonicalVerb);
    if (handlerIt != commandHandlers().end())
    {
        return (this->*handlerIt->second)(command.argument);
    }

    GameCommandResult result;
    appendEvent(result, GameEventType::ActionRejected, "", "", 0, "msg.unknown_command");
    return result;
}

GameCommandResult Game::describeCurrentLocation() const
{
    const auto &locations = DataLoader::getLocations();
    const auto it = locations.find(player.getCurrentLocation());
    if (it == locations.end())
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::ActionRejected, "", "", 0, "msg.unknown_location");
        return result;
    }

    return makeViewResult(ViewKind::Location);
}

void Game::applyEndState(GameCommandResult &result)
{
    if (player.getCurrentLocation() == GameIds::kShuttleBayLocation && player.hasFlag(GameIds::kBeaconOnlineFlag) &&
        player.hasFlag(GameIds::kRootHeartDestroyedFlag))
    {
        result.view = ViewKind::Victory;
        appendEvent(result, GameEventType::GameWon, player.getCurrentLocation());
        isGameOver = true;
        return;
    }

    if (player.getHealth() <= 0)
    {
        result.view = ViewKind::GameOver;
        appendEvent(result, GameEventType::GameLost, player.getCurrentLocation());
        isGameOver = true;
    }
}

std::string Game::resolveItemId(const std::string &target, const std::vector<std::string> &candidateItemIds)
{
    const std::string normalizedTarget = TextUtils::normalizeLookupToken(target);
    if (normalizedTarget.empty())
    {
        return "";
    }

    const auto &items = DataLoader::getItems();
    for (const std::string &itemId : candidateItemIds)
    {
        const auto itemIt = items.find(itemId);
        if (itemIt == items.end())
        {
            continue;
        }

        if (normalizedTarget == TextUtils::normalizeLookupToken(itemIt->second.id) ||
            normalizedTarget == TextUtils::normalizeLookupToken(itemIt->second.name))
        {
            return itemIt->second.id;
        }
    }

    return "";
}

std::string Game::resolveLogId(const std::string &target, const std::vector<std::string> &candidateLogIds)
{
    const std::string normalizedTarget = TextUtils::normalizeLookupToken(target);
    if (normalizedTarget.empty())
    {
        return "";
    }

    const auto &logs = DataLoader::getLogs();
    for (const std::string &logId : candidateLogIds)
    {
        const auto logIt = logs.find(logId);
        if (logIt == logs.end())
        {
            continue;
        }

        if (normalizedTarget == TextUtils::normalizeLookupToken(logIt->second.id) ||
            normalizedTarget == TextUtils::normalizeLookupToken(logIt->second.name))
        {
            return logIt->second.id;
        }
    }

    return "";
}

std::string Game::resolveRivalId(const std::string &target, const Location &current)
{
    if (current.rivalId.empty() || current.rivalHp <= 0)
    {
        return "";
    }

    const std::string normalizedTarget = TextUtils::normalizeLookupToken(target);
    if (normalizedTarget.empty())
    {
        return "";
    }

    const auto &rivals = DataLoader::getRivals();
    const auto rivalIt = rivals.find(current.rivalId);
    if (rivalIt == rivals.end())
    {
        return "";
    }

    if (normalizedTarget == TextUtils::normalizeLookupToken(rivalIt->second.id) ||
        normalizedTarget == TextUtils::normalizeLookupToken(rivalIt->second.name))
    {
        return rivalIt->second.id;
    }

    return "";
}

const std::unordered_map<std::string, Game::CommandHandler> &Game::commandHandlers()
{
    static const std::unordered_map<std::string, CommandHandler> handlers = {{"move", &Game::handleMove},
                                                                             {"take", &Game::handleTake},
                                                                             {"use", &Game::handleUse},
                                                                             {"read", &Game::handleRead},
                                                                             {"inspect", &Game::handleInspect}};

    return handlers;
}

const std::unordered_map<std::string, ViewKind> &Game::commandViews()
{
    static const std::unordered_map<std::string, ViewKind> views = {{"inventory", ViewKind::Inventory},
                                                                    {"status", ViewKind::Status},
                                                                    {"objective", ViewKind::Objective},
                                                                    {"map", ViewKind::Map},
                                                                    {"help", ViewKind::Help}};

    return views;
}

GameCommandResult Game::makeViewResult(ViewKind view)
{
    GameCommandResult result;
    result.view = view;
    return result;
}

void Game::appendMessage(GameCommandResult &result, MessageTone tone, const std::string &text)
{
    result.messages.push_back({tone, text});
}

void Game::appendResourceMessage(GameCommandResult &result, MessageTone tone, const std::string &key)
{
    appendMessage(result, tone, TextResources::get(key));
}

void Game::appendFormattedMessage(GameCommandResult &result, MessageTone tone, const std::string &key,
                                  std::initializer_list<std::pair<const std::string, std::string>> replacements)
{
    appendMessage(result, tone, TextResources::format(key, replacements));
}

void Game::appendEvent(GameCommandResult &result, GameEventType type, const std::string &primaryId,
                       const std::string &secondaryId, int amount, const std::string &detail)
{
    result.events.push_back({type, primaryId, secondaryId, amount, detail});
}
