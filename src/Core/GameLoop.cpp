#include "Core/ConsoleInput.h"
#include "Core/CommandLogger.h"
#include "Core/Game.h"

#include "Data/TextResources.h"
#include "UI/ConsoleUI.h"

#include <iostream>

void Game::run()
{
    TextResources::loadAll();
    bool shouldReplay = true;
    while (shouldReplay)
    {
        resetSession();
        CommandLogger::beginSession();
        std::string sessionOutcome = "interrupted";

        std::string lastCommand;
        GameCommandResult lastResult = makeViewResult(ViewKind::Welcome);
        ConsoleUI::renderFrame(lastCommand, buildViewModel(lastResult));

        std::string command;
        bool inputClosed = false;
        while (!isGameOver)
        {
            if (!ConsoleInput::readLine(command))
            {
                inputClosed = true;
                break;
            }

            if (command.empty())
            {
                ConsoleUI::renderFrame(lastCommand, buildViewModel(lastResult));
                continue;
            }

            lastCommand = command;
            CommandLogger::logCommand(command);
            lastResult = executeCommand(command);
            ConsoleUI::renderFrame(lastCommand, buildViewModel(lastResult));
        }

        if (inputClosed)
        {
            sessionOutcome = "input_closed";
            CommandLogger::endSession(sessionOutcome);
            break;
        }

        const bool endedWithFinale = lastResult.view == ViewKind::Victory || lastResult.view == ViewKind::GameOver;
        if (!endedWithFinale)
        {
            sessionOutcome = "exit";
            CommandLogger::endSession(sessionOutcome);
            break;
        }

        sessionOutcome = lastResult.view == ViewKind::Victory ? "victory" : "gameover";
        CommandLogger::endSession(sessionOutcome);

        if (!ConsoleUI::waitForContinue())
        {
            break;
        }

        shouldReplay = ConsoleUI::showCreditsAndReplay(lastResult.view == ViewKind::Victory);
    }
}
