#include "Core/Game.h"

#include "Core/GameIds.h"
#include "Data/DataLoader.h"
#include "Data/TextResources.h"

namespace
{
std::string localizeLocationId(const std::string &locationId)
{
    const auto &locations = DataLoader::getLocations();
    const auto it = locations.find(locationId);
    if (it != locations.end())
    {
        return it->second.name;
    }
    return locationId;
}

std::string currentLocationDescription(const Player &player, const Location &location)
{
    if (location.id == GameIds::kCommandBridgeLocation && player.hasFlag(GameIds::kBeaconOnlineFlag))
    {
        return TextResources::get("location.command_bridge.beacon_online");
    }

    if (location.id == GameIds::kReactorSpineLocation && player.hasFlag(GameIds::kRootHeartDestroyedFlag))
    {
        return TextResources::get("location.reactor_spine.cleared");
    }

    return location.description;
}

void fillMissionState(GameHudView &hud, const Player &player)
{
    hud.hasAccessKey = player.hasItem(GameIds::kAccessKeyItem);
    hud.hasLens = player.hasItem(GameIds::kLensItem);
    hud.hasKeycard = player.hasItem(GameIds::kBridgeKeycardItem);
    hud.beaconOnline = player.hasFlag(GameIds::kBeaconOnlineFlag);
    hud.rootHeartDestroyed = player.hasFlag(GameIds::kRootHeartDestroyedFlag);
}

void fillWeaponState(GameHudView &hud, const Player &player, const std::map<std::string, Item> &items)
{
    if (player.getActiveWeaponId().empty())
    {
        return;
    }

    const auto activeIt = items.find(player.getActiveWeaponId());
    hud.activeWeaponName = activeIt != items.end() ? activeIt->second.name : player.getActiveWeaponId();
    hud.activeWeaponCharges = player.getActiveWeaponCharges();
}

void fillInventoryView(GameHudView &hud, const Player &player, const std::map<std::string, Item> &items)
{
    const auto &inventory = player.getInventory();
    hud.inventory.reserve(inventory.size());

    for (const auto &itemId : inventory)
    {
        const auto itemIt = items.find(itemId);
        std::string detail;
        if (itemIt != items.end() && itemIt->second.type == "weapon")
        {
            const int currentCharges = player.getWeaponCharges(itemId);
            const int visibleCharges = currentCharges > 0 ? currentCharges : itemIt->second.charges;
            detail = std::to_string(visibleCharges) + " зар.";
            detail += player.getActiveWeaponId() == itemId ? ", активно" : ", наготове";
        }

        hud.inventory.push_back({itemIt != items.end() ? itemIt->second.name : itemId, itemId, detail});
    }
}

void fillKnownLogsView(GameHudView &hud, const Player &player, const std::map<std::string, LogEntry> &logs)
{
    const auto &knownLogs = player.getKnownLogs();
    hud.knownLogs.reserve(knownLogs.size());

    for (const auto &logId : knownLogs)
    {
        const auto logIt = logs.find(logId);
        if (logIt != logs.end())
        {
            hud.knownLogs.push_back({logIt->second.name, logId, "архив"});
        }
    }
}

void fillLocationState(GameHudView &hud, const Location &location)
{
    hud.exits.reserve(location.exits.size());
    for (const auto &[directionId, targetId] : location.exits)
    {
        hud.exits.push_back({directionId, localizeLocationId(targetId)});
    }

    if (!location.rivalId.empty() && location.rivalHp > 0)
    {
        const auto &rivals = DataLoader::getRivals();
        const auto rivalIt = rivals.find(location.rivalId);
        if (rivalIt != rivals.end())
        {
            hud.threatName = rivalIt->second.name;
            hud.threatHp = location.rivalHp;
        }
    }
}
} // namespace

GameViewModel Game::buildViewModel(const GameCommandResult &result) const
{
    GameViewModel viewModel;
    viewModel.result = result;

    GameHudView hud;
    hud.currentLocationName = localizeLocationId(player.getCurrentLocation());
    hud.health = player.getHealth();
    hud.maxHealth = player.getMaxHealth();
    hud.attack = player.getAttack();
    fillMissionState(hud, player);

    const auto &items = DataLoader::getItems();
    fillWeaponState(hud, player, items);
    fillInventoryView(hud, player, items);

    const auto &logs = DataLoader::getLogs();
    fillKnownLogsView(hud, player, logs);

    const auto &locations = DataLoader::getLocations();
    const auto locationIt = locations.find(player.getCurrentLocation());
    if (locationIt != locations.end())
    {
        fillLocationState(hud, locationIt->second);
    }

    viewModel.hud = std::move(hud);
    viewModel.mapCurrentLocationName = viewModel.hud.currentLocationName;

    if (result.view == ViewKind::Location)
    {
        viewModel.location = buildLocationView();
    }

    return viewModel;
}

GameLocationView Game::buildLocationView() const
{
    GameLocationView locationView;

    const auto &locations = DataLoader::getLocations();
    const auto locationIt = locations.find(player.getCurrentLocation());
    if (locationIt == locations.end())
    {
        return locationView;
    }

    const Location &location = locationIt->second;
    locationView.name = location.name;
    locationView.description = currentLocationDescription(player, location);
    locationView.nearbyItems.reserve(location.itemIds.size());
    locationView.nearbyLogs.reserve(location.logIds.size());
    locationView.exits.reserve(location.exits.size());

    const auto &items = DataLoader::getItems();
    for (const auto &itemId : location.itemIds)
    {
        const auto itemIt = items.find(itemId);
        if (itemIt != items.end())
        {
            locationView.nearbyItems.push_back({itemIt->second.name, itemId});
        }
    }

    const auto &logs = DataLoader::getLogs();
    for (const auto &logId : location.logIds)
    {
        const auto logIt = logs.find(logId);
        if (logIt != logs.end())
        {
            locationView.nearbyLogs.push_back({logIt->second.name, logId, "читать"});
        }
    }

    const auto &rivals = DataLoader::getRivals();
    if (!location.rivalId.empty() && location.rivalHp > 0)
    {
        const auto rivalIt = rivals.find(location.rivalId);
        if (rivalIt != rivals.end())
        {
            locationView.rivalName = rivalIt->second.name;
            locationView.rivalHp = location.rivalHp;
        }
    }

    for (const auto &[directionId, targetId] : location.exits)
    {
        locationView.exits.push_back({directionId, localizeLocationId(targetId)});
    }

    return locationView;
}
