#pragma once

#include "Core/GameViewModel.h"

class ConsoleUI
{
  public:
    static void renderFrame(const std::string &lastCommand, const GameViewModel &viewModel);
    static bool waitForContinue();
    static bool showCreditsAndReplay(bool victory);

  private:
    static void showWelcome();
    static void printLocation(const GameLocationView &location, const GameHudView &hud);
    static void printMap(const std::string &currentLocationName);
    static void showHelp();
    static void showGameOver();
    static void showVictory();
    static void showCredits(bool victory);
    static void showStatus(const GameHudView &hud);
    static void showObjective(const GameHudView &hud);
    static void showInventory(const GameHudView &hud);
    static void renderView(const GameViewModel &viewModel);
    static void renderMessages(const GameCommandResult &result);
};
