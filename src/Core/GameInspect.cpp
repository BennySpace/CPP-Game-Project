#include "Core/Game.h"

#include "Core/GameIds.h"
#include "Data/DataLoader.h"
#include "Data/TextResources.h"
#include "Entities/Item.h"
#include "Entities/Rival.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace
{
std::string locationHint(const std::string &locationId)
{
    static const std::unordered_map<std::string, std::string> hintKeys = {
        {GameIds::kCommandBridgeLocation, "hint.command_bridge"},
        {GameIds::kReactorSpineLocation, "hint.reactor_spine"},
        {GameIds::kShuttleBayLocation, "hint.shuttle_bay"}};

    const auto hintIt = hintKeys.find(locationId);
    if (hintIt != hintKeys.end())
    {
        return TextResources::get(hintIt->second);
    }

    return "";
}

bool isRoomInspectionTarget(const std::string &target)
{
    static const std::unordered_set<std::string> roomTargets = {"", "room", "комната"};
    return roomTargets.find(target) != roomTargets.end();
}

bool isRivalInspectionTarget(const std::string &target, const std::string &rivalId)
{
    static const std::unordered_set<std::string> rivalAliases = {"enemy", "враг"};
    return target == rivalId || rivalAliases.find(target) != rivalAliases.end();
}

const Location &currentLocationOrThrow(const std::map<std::string, Location> &locations, const std::string &locationId)
{
    const auto currentIt = locations.find(locationId);
    if (currentIt == locations.end())
    {
        throw std::runtime_error("Current location is missing from loaded data: '" + locationId + "'.");
    }

    return currentIt->second;
}
} // namespace

GameCommandResult Game::handleInspect(const std::string &target)
{
    const auto &locations = DataLoader::getLocations();
    const Location &current = currentLocationOrThrow(locations, player.getCurrentLocation());

    if (isRoomInspectionTarget(target))
    {
        return inspectCurrentLocation(current);
    }

    GameCommandResult result = inspectItemTarget(target, current);
    if (result.view != ViewKind::None || !result.messages.empty() || !result.events.empty())
    {
        return result;
    }

    result = inspectRivalTarget(target, current);
    if (result.view != ViewKind::None || !result.messages.empty() || !result.events.empty())
    {
        return result;
    }

    appendEvent(result, GameEventType::ActionRejected, target, 0, "msg.nothing_useful");
    return result;
}

GameCommandResult Game::inspectCurrentLocation(const Location &current) const
{
    GameCommandResult result = describeCurrentLocation();
    const std::string hint = locationHint(current.id);
    if (!hint.empty())
    {
        appendEvent(result, GameEventType::InspectHintShown, current.id, 0, hint);
    }
    return result;
}

GameCommandResult Game::inspectItemTarget(const std::string &target, const Location &current) const
{
    std::vector<std::string> visibleItemIds = current.itemIds;
    for (const std::string &itemId : player.getInventory())
    {
        if (std::find(visibleItemIds.begin(), visibleItemIds.end(), itemId) == visibleItemIds.end())
        {
            visibleItemIds.push_back(itemId);
        }
    }

    const std::string resolvedItemId = resolveItemId(target, visibleItemIds);
    if (resolvedItemId.empty())
    {
        return {};
    }

    GameCommandResult result;
    appendEvent(result, GameEventType::InspectItemFound, resolvedItemId);
    return result;
}

GameCommandResult Game::inspectRivalTarget(const std::string &target, const Location &current) const
{
    const std::string resolvedRivalId = resolveRivalId(target, current);
    if (resolvedRivalId.empty() && (current.rivalId.empty() || !isRivalInspectionTarget(target, current.rivalId)))
    {
        return {};
    }

    const auto &rivals = DataLoader::getRivals();
    const auto rivalIt = rivals.find(current.rivalId);
    if (rivalIt == rivals.end() || current.rivalHp <= 0)
    {
        return {};
    }

    GameCommandResult result;
    appendEvent(result, GameEventType::InspectRivalFound, current.rivalId, current.rivalHp);
    return result;
}
