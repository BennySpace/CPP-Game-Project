#include "Core/ConsoleInput.h"
#include "UI/ConsoleUI.h"

#include "Data/TextResources.h"
#include "detail/ConsoleDecor.h"
#include "detail/StationMap.h"

#include <algorithm>
#include <iostream>

namespace
{
constexpr std::size_t kPanelWidth = 116;

std::string exitsLine(const std::vector<GameExitView> &exits)
{
    if (exits.empty())
    {
        return TextResources::get("hud.none");
    }

    std::string result;
    for (const auto &exit : exits)
    {
        if (!result.empty())
        {
            result += color("90", " | ");
        }
        result += localizeDirection(exit.directionId) + color("90", " -> ") + exit.targetName;
    }

    return result;
}

std::string inventoryLine(const GameHudView &hud)
{
    if (hud.inventory.empty())
    {
        return TextResources::get("inventory.empty");
    }

    std::string result;
    for (std::size_t index = 0; index < hud.inventory.size(); ++index)
    {
        if (index > 0)
        {
            result += color("90", " | ");
        }
        result += hud.inventory[index].name;
        if (!hud.inventory[index].detail.empty())
        {
            result += color("90", " (" + hud.inventory[index].detail + ")");
        }
    }

    return result;
}

std::string weaponLine(const GameHudView &hud)
{
    if (hud.activeWeaponName.empty())
    {
        return TextResources::get("hud.none");
    }

    return hud.activeWeaponName + color("90", " [" + std::to_string(hud.activeWeaponCharges) + "]");
}

std::string missionLine(const GameHudView &hud)
{
    return TextResources::get("hud.mission.access") + " " +
           (hud.hasAccessKey ? color("32", TextResources::get("state.yes"))
                             : color("31", TextResources::get("state.no"))) +
           color("90", " | ") + TextResources::get("hud.mission.lens") + " " +
           (hud.hasLens ? color("32", TextResources::get("state.yes")) : color("31", TextResources::get("state.no"))) +
           color("90", " | ") + TextResources::get("hud.mission.card") + " " +
           (hud.hasKeycard ? color("32", TextResources::get("state.yes"))
                           : color("31", TextResources::get("state.no"))) +
           color("90", " | ") + TextResources::get("hud.mission.beacon") + " " +
           (hud.beaconOnline ? color("32", TextResources::get("state.yes"))
                             : color("31", TextResources::get("state.no"))) +
           color("90", " | ") + TextResources::get("hud.mission.core") + " " +
           (hud.rootHeartDestroyed ? color("32", TextResources::get("state.yes"))
                                   : color("31", TextResources::get("state.no")));
}

std::string threatLine(const GameHudView &hud)
{
    if (hud.threatName.empty())
    {
        return TextResources::get("hud.none");
    }

    return hud.threatName + " [" + std::to_string(hud.threatHp) + " HP]";
}

std::string messageColorCode(MessageTone tone)
{
    switch (tone)
    {
    case MessageTone::Muted:
        return "90";
    case MessageTone::Info:
        return "36";
    case MessageTone::Success:
        return "32";
    case MessageTone::Warning:
        return "33";
    case MessageTone::Danger:
        return "31";
    case MessageTone::Normal:
    default:
        return "37";
    }
}

void printPanelTop(const std::string &title)
{
    const std::string decoratedTitle = " " + title + " ";
    const std::size_t titleWidth = utf8Length(decoratedTitle);
    const std::size_t fillWidth = kPanelWidth > titleWidth ? kPanelWidth - titleWidth : 0;
    std::cout << color("36", "┌" + decoratedTitle + repeat("─", fillWidth) + "┐") << "\n";
}

void printPanelBottom()
{
    std::cout << color("36", "└" + repeat("─", kPanelWidth) + "┘") << "\n";
}

void printPanelLine(const std::string &text, const std::string &tone = "37")
{
    std::cout << color("36", "│ ") << color(tone, padRightUtf8(text, kPanelWidth - 2)) << color("36", " │") << "\n";
}

void printSectionHeader(const std::string &title)
{
    std::cout << "\n" << color("1;36", title) << "\n";
    std::cout << color("90", repeat("─", 52)) << "\n";
}

bool isAffirmativeReplayAnswer(const std::string &answer)
{
    return answer == "y" || answer == "yes" || answer == "да" || answer == "д";
}

bool isNegativeReplayAnswer(const std::string &answer)
{
    return answer == "n" || answer == "no" || answer == "нет" || answer == "н";
}
} // namespace

void ConsoleUI::showWelcome()
{
    const std::string title = TextResources::get("title");
    const std::size_t innerWidth = std::max<std::size_t>(52, utf8Length(title) + 10);

    printHorizontalFrame("╔", "═", "╗", innerWidth);
    printFramedText(title, innerWidth, true);
    printHorizontalFrame("╚", "═", "╝", innerWidth);
    std::cout << "\n";

    std::cout << color("90", TextResources::get("intro.line1")) << "\n";
    std::cout << color("90", TextResources::get("intro.line2")) << "\n\n";

    std::cout << color("1;37", TextResources::get("objective.title")) << "\n";
    for (int index = 1; index <= 4; ++index)
    {
        std::cout << "  " << index << ". " << TextResources::get("objective.step" + std::to_string(index)) << "\n";
    }
    std::cout << "\n";
    std::cout << color("33", TextResources::get("hint.title")) << " " << TextResources::get("hint.body") << "\n";
}

void ConsoleUI::printLocation(const GameLocationView &location, const GameHudView &hud)
{
    printSectionHeader(location.name);
    std::cout << location.description << "\n";

    if (!location.nearbyItems.empty())
    {
        std::cout << "\n" << color("33", TextResources::get("nearby.label")) << "\n";
        for (const auto &item : location.nearbyItems)
        {
            std::cout << "• " << item.name << color("90", " [" + item.id + "]");
            if (!item.detail.empty())
            {
                std::cout << color("90", " (" + item.detail + ")");
            }
            std::cout << "\n";
        }
    }

    if (!location.nearbyLogs.empty())
    {
        std::cout << "\n" << color("33", TextResources::get("logs.label")) << "\n";
        for (const auto &log : location.nearbyLogs)
        {
            std::cout << "• " << log.name << color("90", " [" + log.id + "]");
            if (!log.detail.empty())
            {
                std::cout << color("90", " (" + log.detail + ")");
            }
            std::cout << "\n";
        }
    }

    if (!location.rivalName.empty() && location.rivalHp > 0)
    {
        std::cout << "\n"
                  << color("31", TextResources::get("threat.label") + " " + location.rivalName + " [" +
                                     std::to_string(location.rivalHp) + " HP]")
                  << "\n";
    }

    std::cout << "\n" << color("36", TextResources::get("exits.label")) << "\n";
    for (const auto &exit : location.exits)
    {
        std::cout << "• " << localizeDirection(exit.directionId) << color("90", " -> ") << exit.targetName << "\n";
    }

    std::cout << "\n"
              << TextResources::get("status.health") << " " << hud.health << "/" << hud.maxHealth << color("90", " | ")
              << TextResources::get("status.attack") << " " << hud.attack << "\n";
}

void ConsoleUI::printMap(const std::string &currentLocationName)
{
    printSectionHeader(TextResources::get("map.title"));
    for (const auto &line : buildHighlightedStationMapLines(currentLocationName))
    {
        std::cout << line << "\n";
    }
    std::cout << "\n" << color("36", TextResources::get("map.position")) << " " << currentLocationName << "\n";
    std::cout << color("90", TextResources::get("map.footer")) << "\n";
}

void ConsoleUI::showHelp()
{
    printSectionHeader(TextResources::get("help.title"));
    for (int index = 1; index <= 12; ++index)
    {
        std::cout << TextResources::get("help.line" + std::to_string(index)) << "\n";
    }
    std::cout << "\n" << color("90", TextResources::get("help.footer")) << "\n";
}

void ConsoleUI::showGameOver()
{
    printSectionHeader(TextResources::get("gameover.title"));
    std::cout << color("31", TextResources::get("gameover.body")) << "\n";
}

void ConsoleUI::showVictory()
{
    printSectionHeader(TextResources::get("victory.title"));
    std::cout << color("32", TextResources::get("victory.line1")) << "\n";
    std::cout << color("32", TextResources::get("victory.line2")) << "\n";
}

void ConsoleUI::showCredits(bool victory)
{
    std::cout << "\x1b[2J\x1b[H";

    printSectionHeader(TextResources::get(victory ? "credits.victory_title" : "credits.gameover_title"));
    std::cout << color(victory ? "32" : "31",
                       TextResources::get(victory ? "credits.victory_body" : "credits.gameover_body"))
              << "\n";

    printSectionHeader(TextResources::get("credits.title"));
    for (int index = 1; index <= 5; ++index)
    {
        const std::string line = TextResources::get("credits.line" + std::to_string(index));
        if (!line.empty())
        {
            std::cout << color("90", line) << "\n";
        }
    }
}

bool ConsoleUI::waitForContinue()
{
    std::cout << "\n" << color("1;36", TextResources::get("credits.continue_prompt")) << " ";

    std::string answer;
    return ConsoleInput::readLine(answer);
}

bool ConsoleUI::showCreditsAndReplay(bool victory)
{
    while (true)
    {
        showCredits(victory);
        std::cout << "\n" << color("1;36", TextResources::get("credits.replay_question")) << " ";

        std::string answer;
        if (!ConsoleInput::readLine(answer))
        {
            return false;
        }

        std::transform(answer.begin(), answer.end(), answer.begin(), [](unsigned char ch) {
            if (ch >= 'A' && ch <= 'Z')
            {
                return static_cast<char>(ch - 'A' + 'a');
            }
            return static_cast<char>(ch);
        });

        if (isAffirmativeReplayAnswer(answer))
        {
            return true;
        }

        if (isNegativeReplayAnswer(answer))
        {
            return false;
        }

        std::cout << color("33", TextResources::get("credits.replay_invalid")) << "\n";
    }
}

void ConsoleUI::showStatus(const GameHudView &hud)
{
    printSectionHeader(TextResources::get("status.title"));
    std::cout << TextResources::get("status.health") << " " << hud.health << "/" << hud.maxHealth << color("90", " | ")
              << TextResources::get("status.attack") << " " << hud.attack << "\n";
    std::cout << TextResources::get("status.weapon") << " " << weaponLine(hud) << "\n";
}

void ConsoleUI::showObjective(const GameHudView &hud)
{
    printSectionHeader(TextResources::get("mission.title"));
    std::cout << TextResources::get("mission.key_access")
              << (hud.hasAccessKey ? color("32", TextResources::get("state.found"))
                                   : color("31", TextResources::get("state.missing")))
              << "\n";
    std::cout << TextResources::get("mission.beacon_lens")
              << (hud.hasLens ? color("32", TextResources::get("state.found_f"))
                              : color("31", TextResources::get("state.missing_f")))
              << "\n";
    std::cout << TextResources::get("mission.bridge_card")
              << (hud.hasKeycard ? color("32", TextResources::get("state.found_f"))
                                 : color("31", TextResources::get("state.missing_f")))
              << "\n";
    std::cout << TextResources::get("mission.beacon_on")
              << (hud.beaconOnline ? color("32", TextResources::get("state.yes"))
                                   : color("31", TextResources::get("state.no")))
              << "\n";
    std::cout << TextResources::get("mission.core_dead")
              << (hud.rootHeartDestroyed ? color("32", TextResources::get("state.yes"))
                                         : color("31", TextResources::get("state.no")))
              << "\n";
    std::cout << "\n" << color("90", TextResources::get("mission.footer")) << "\n";
}

void ConsoleUI::showInventory(const GameHudView &hud)
{
    printSectionHeader(TextResources::get("inventory.title"));
    if (hud.inventory.empty() && hud.knownLogs.empty())
    {
        std::cout << color("90", TextResources::get("inventory.empty")) << "\n";
        return;
    }

    for (const auto &item : hud.inventory)
    {
        std::cout << "• " << item.name << color("90", " [" + item.id + "]");
        if (!item.detail.empty())
        {
            std::cout << color("90", " (" + item.detail + ")");
        }
        std::cout << "\n";
    }

    if (!hud.knownLogs.empty())
    {
        std::cout << "\n" << color("33", TextResources::get("logs.archive_title")) << "\n";
        for (const auto &log : hud.knownLogs)
        {
            std::cout << "• " << log.name << color("90", " [" + log.id + "]") << "\n";
        }
    }
}

void ConsoleUI::renderView(const GameViewModel &viewModel)
{
    switch (viewModel.result.view)
    {
    case ViewKind::Welcome:
        showWelcome();
        break;
    case ViewKind::Location:
        if (viewModel.location.has_value())
        {
            printLocation(*viewModel.location, viewModel.hud);
        }
        break;
    case ViewKind::Map:
        printMap(viewModel.mapCurrentLocationName);
        break;
    case ViewKind::Help:
        showHelp();
        break;
    case ViewKind::Status:
        showStatus(viewModel.hud);
        break;
    case ViewKind::Objective:
        showObjective(viewModel.hud);
        break;
    case ViewKind::Inventory:
        showInventory(viewModel.hud);
        break;
    case ViewKind::Victory:
        showVictory();
        break;
    case ViewKind::GameOver:
        showGameOver();
        break;
    case ViewKind::None:
    default:
        break;
    }
}

void ConsoleUI::renderMessages(const GameCommandResult &result)
{
    for (const auto &message : result.messages)
    {
        std::cout << color(messageColorCode(message.tone), message.text) << "\n";
    }
}

void ConsoleUI::renderFrame(const std::string &lastCommand, const GameViewModel &viewModel)
{
    std::cout << "\x1b[2J\x1b[H";

    printPanelTop(TextResources::get("hud.title"));
    printPanelLine(TextResources::get("location.label") + " " + viewModel.hud.currentLocationName + color("90", " | ") +
                       color("32", "HP ") + std::to_string(viewModel.hud.health) + "/" +
                       std::to_string(viewModel.hud.maxHealth) + color("90", " | ") + color("33", "ATK ") +
                       std::to_string(viewModel.hud.attack),
                   "1;37");
    printPanelLine(color("31", TextResources::get("threat.label") + " ") + threatLine(viewModel.hud) + color("90", " | ") +
                       color("36", TextResources::get("exits.label") + " ") + exitsLine(viewModel.hud.exits),
                   "37");
    printPanelLine(color("33", TextResources::get("hud.weapon") + " ") + weaponLine(viewModel.hud), "37");
    printPanelLine(color("33", TextResources::get("hud.inventory") + " ") + inventoryLine(viewModel.hud), "37");
    printPanelLine(color("35", TextResources::get("hud.objective") + " ") + missionLine(viewModel.hud), "37");
    printPanelBottom();

    printSectionHeader(TextResources::get("hud.last_result"));
    std::cout << color("36", TextResources::get("hud.last_command") + " ")
              << color("1;37", (lastCommand.empty() ? TextResources::get("hud.none") : lastCommand)) << "\n";

    if (viewModel.result.view != ViewKind::None)
    {
        renderView(viewModel);
    }

    if (viewModel.result.view != ViewKind::None || !viewModel.result.messages.empty())
    {
        if (viewModel.result.view != ViewKind::None)
        {
            std::cout << "\n";
        }
        renderMessages(viewModel.result);
    }

    std::cout << "\n" << color("1;36", TextResources::get("hud.prompt")) << " ";
}
