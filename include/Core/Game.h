#pragma once

#include "Core/CommandParser.h"
#include "Core/GameCommandResult.h"
#include "Core/GameViewModel.h"
#include "Core/Player.h"

#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>

struct Item;
struct Location;

class Game
{
  public:
    void run();
    GameCommandResult executeCommand(const std::string &rawCommand);

  private:
    using CommandHandler = GameCommandResult (Game::*)(const std::string &);
    using ItemUseHandler = GameCommandResult (Game::*)(const std::string &, const Item &);

    GameCommandResult processCommand(const ParsedCommand &command);
    GameCommandResult handleMove(const std::string &direction);
    GameCommandResult handleTake(const std::string &itemId);
    GameCommandResult handleUse(const std::string &itemId);
    GameCommandResult handleRead(const std::string &target);
    GameCommandResult handleAttack();
    GameCommandResult handleInspect(const std::string &target);
    GameCommandResult useHealingItem(const std::string &itemId, const Item &item);
    GameCommandResult useWeaponItem(const std::string &itemId, const Item &item);
    GameCommandResult useLensItem(const std::string &itemId, const Item &item);
    GameCommandResult useGenericItem(const std::string &itemId, const Item &item);
    GameCommandResult inspectCurrentLocation(const Location &current) const;
    GameCommandResult inspectItemTarget(const std::string &target, const Location &current) const;
    GameCommandResult inspectRivalTarget(const std::string &target, const Location &current) const;
    GameCommandResult describeCurrentLocation() const;
    void applyEndState(GameCommandResult &result);
    static std::string resolveItemId(const std::string &target, const std::vector<std::string> &candidateItemIds);
    static std::string resolveLogId(const std::string &target, const std::vector<std::string> &candidateLogIds);
    static std::string resolveRivalId(const std::string &target, const Location &current);
    static const std::unordered_map<std::string, CommandHandler> &commandHandlers();
    static const std::unordered_map<std::string, ViewKind> &commandViews();
    static const std::unordered_map<std::string, ItemUseHandler> &itemUseHandlers();
    static GameCommandResult makeViewResult(ViewKind view);
    static void appendMessage(GameCommandResult &result, MessageTone tone, const std::string &text);
    static void appendResourceMessage(GameCommandResult &result, MessageTone tone, const std::string &key);
    static void appendFormattedMessage(GameCommandResult &result, MessageTone tone, const std::string &key,
                                       std::initializer_list<std::pair<const std::string, std::string>> replacements);
    static void appendEvent(GameCommandResult &result, GameEventType type, const std::string &primaryId = "",
                            int amount = 0, const std::string &detail = "");
    void resetSession();
    GameViewModel buildViewModel(const GameCommandResult &result) const;
    GameLocationView buildLocationView() const;

    Player player;
    bool isGameOver = false;
};
