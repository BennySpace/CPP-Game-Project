#include "Core/Game.h"

#include "Core/GameIds.h"
#include "Data/DataLoader.h"
#include "Entities/Item.h"
#include "Entities/Rival.h"

#include <algorithm>
#include <stdexcept>

namespace
{
bool hasLiveRival(const Location &location)
{
    return !location.rivalId.empty() && location.rivalHp > 0;
}

Location &currentLocationOrThrow(std::map<std::string, Location> &locations, const std::string &locationId)
{
    const auto currentIt = locations.find(locationId);
    if (currentIt == locations.end())
    {
        throw std::runtime_error("Current location is missing from loaded data: '" + locationId + "'.");
    }

    return currentIt->second;
}
} // namespace

GameCommandResult Game::handleMove(const std::string &direction)
{
    if (direction.empty())
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::MoveBlocked, player.getCurrentLocation(), 0, "msg.move_where");
        return result;
    }

    auto &locations = DataLoader::getLocations();
    Location &current = currentLocationOrThrow(locations, player.getCurrentLocation());
    const std::string previousLocationId = current.id;

    const auto exitIt = current.exits.find(direction);
    if (exitIt == current.exits.end())
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::MoveBlocked, previousLocationId, 0, "msg.no_safe_path");
        return result;
    }

    const std::string &nextId = exitIt->second;
    if (locations.find(nextId) == locations.end())
    {
        throw std::runtime_error("Exit points to a missing location: '" + previousLocationId + "' -> '" + nextId +
                                 "'.");
    }

    if (nextId == GameIds::kCommandBridgeLocation && !player.hasItem(GameIds::kBridgeKeycardItem))
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::MoveBlocked, previousLocationId, 0, "msg.bridge_locked");
        return result;
    }

    player.setCurrentLocation(nextId);
    return describeCurrentLocation();
}

GameCommandResult Game::handleTake(const std::string &itemId)
{
    if (itemId.empty())
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::ActionRejected, "", 0, "msg.take_what");
        return result;
    }

    auto &locations = DataLoader::getLocations();
    Location &current = currentLocationOrThrow(locations, player.getCurrentLocation());
    const std::string resolvedItemId = resolveItemId(itemId, current.itemIds);

    if (hasLiveRival(current))
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::ActionRejected, current.id, 0, "msg.threat_first");
        return result;
    }

    const auto itemIt = std::find(current.itemIds.begin(), current.itemIds.end(), resolvedItemId);
    if (itemIt == current.itemIds.end())
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::ActionRejected, itemId, 0, "msg.no_such_item_here");
        return result;
    }

    const auto &items = DataLoader::getItems();
    const auto dataIt = items.find(resolvedItemId);
    if (dataIt == items.end())
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::ActionRejected, resolvedItemId, 0, "msg.item_unreadable");
        return result;
    }

    player.addItem(resolvedItemId);
    current.itemIds.erase(itemIt);

    GameCommandResult result;
    appendEvent(result, GameEventType::ItemTaken, resolvedItemId);
    return result;
}

GameCommandResult Game::handleUse(const std::string &itemId)
{
    if (itemId.empty())
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::ItemUseRejected, itemId, 0, "msg.use_what");
        return result;
    }

    const std::string resolvedItemId = resolveItemId(itemId, player.getInventory());
    if (resolvedItemId.empty() || !player.hasItem(resolvedItemId))
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::ItemUseRejected, itemId, 0, "msg.item_not_in_inventory");
        return result;
    }

    const auto &items = DataLoader::getItems();
    const auto itemIt = items.find(resolvedItemId);
    if (itemIt == items.end())
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::ItemUseRejected, resolvedItemId, 0, "msg.item_uninterpretable");
        return result;
    }

    const Item &item = itemIt->second;
    if (resolvedItemId == GameIds::kLensItem)
    {
        return useLensItem(resolvedItemId, item);
    }

    const auto &handlers = itemUseHandlers();
    const auto handlerIt = handlers.find(item.type);
    if (handlerIt == handlers.end())
    {
        return useGenericItem(resolvedItemId, item);
    }

    return (this->*handlerIt->second)(resolvedItemId, item);
}

GameCommandResult Game::handleRead(const std::string &target)
{
    auto &locations = DataLoader::getLocations();
    Location &current = currentLocationOrThrow(locations, player.getCurrentLocation());

    if (hasLiveRival(current))
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::ActionRejected, current.id, 0, "msg.threat_first");
        return result;
    }

    if (target.empty())
    {
        GameCommandResult result;
        if (player.getKnownLogs().empty())
        {
            appendEvent(result, GameEventType::ActionRejected, "", 0, "msg.no_logs_known");
            return result;
        }

        appendResourceMessage(result, MessageTone::Info, "msg.log_archive_title");
        const auto &logs = DataLoader::getLogs();
        for (const std::string &logId : player.getKnownLogs())
        {
            const auto logIt = logs.find(logId);
            if (logIt != logs.end())
            {
                appendFormattedMessage(result, MessageTone::Muted, "msg.log_archive_entry",
                                       {{"name", logIt->second.name}, {"id", logIt->second.id}});
            }
        }
        return result;
    }

    std::vector<std::string> candidateLogIds = current.logIds;
    for (const std::string &logId : player.getKnownLogs())
    {
        if (std::find(candidateLogIds.begin(), candidateLogIds.end(), logId) == candidateLogIds.end())
        {
            candidateLogIds.push_back(logId);
        }
    }

    const std::string resolvedLogId = resolveLogId(target, candidateLogIds);
    if (resolvedLogId.empty())
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::ActionRejected, target, 0, "msg.no_such_log");
        return result;
    }

    const bool newlyDiscovered = !player.knowsLog(resolvedLogId);
    if (!player.knowsLog(resolvedLogId))
    {
        player.learnLog(resolvedLogId);
    }

    GameCommandResult result;
    if (newlyDiscovered)
    {
        appendEvent(result, GameEventType::LogDiscovered, resolvedLogId);
    }

    appendEvent(result, GameEventType::LogRead, resolvedLogId);
    return result;
}

GameCommandResult Game::handleAttack()
{
    auto &locations = DataLoader::getLocations();
    Location &current = currentLocationOrThrow(locations, player.getCurrentLocation());

    if (!hasLiveRival(current))
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::ActionRejected, current.id, 0, "msg.attack_nothing");
        return result;
    }

    const auto &rivals = DataLoader::getRivals();
    const auto rivalIt = rivals.find(current.rivalId);
    if (rivalIt == rivals.end())
    {
        current.rivalHp = 0;
        GameCommandResult result;
        appendEvent(result, GameEventType::ActionRejected, current.rivalId, 0, "msg.threat_malformed");
        appendEvent(result, GameEventType::ThreatDefeated, current.rivalId);
        return result;
    }

    const Rival &rival = rivalIt->second;
    const int attackDamage = player.getAttack();
    const std::string weaponId = player.getActiveWeaponId();
    current.rivalHp -= attackDamage;

    GameCommandResult result;
    appendEvent(result, GameEventType::AttackHit, current.rivalId, attackDamage);
    if (!weaponId.empty())
    {
        const int remainingCharges = player.consumeActiveWeaponCharge();
        if (remainingCharges <= 0)
        {
            appendEvent(result, GameEventType::WeaponDepleted, weaponId);
            player.removeItem(weaponId);
        }
    }

    if (current.rivalHp <= 0)
    {
        current.rivalHp = 0;
        appendEvent(result, GameEventType::ThreatDefeated, current.rivalId);
        if (current.rivalId == GameIds::kRootHeartRival)
        {
            player.addFlag(GameIds::kRootHeartDestroyedFlag);
        }
        return result;
    }

    player.takeDamage(rival.atk);
    appendEvent(result, GameEventType::PlayerDamaged, current.rivalId, rival.atk);
    return result;
}

GameCommandResult Game::useHealingItem(const std::string &itemId, const Item &item)
{
    player.heal(item.value);
    player.removeItem(itemId);

    GameCommandResult result;
    appendEvent(result, GameEventType::ItemUsed, itemId, item.value, item.type);
    return result;
}

GameCommandResult Game::useWeaponItem(const std::string &itemId, const Item &item)
{
    if (player.getActiveWeaponId() == itemId)
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::ItemUseRejected, itemId, 0, "msg.weapon_ready");
        return result;
    }

    player.armWeapon(itemId, item.value, item.charges);

    GameCommandResult result;
    appendEvent(result, GameEventType::ItemUsed, itemId, item.value, item.type);
    appendFormattedMessage(result, MessageTone::Info, "msg.weapon_armed",
                           {{"name", item.name}, {"charges", std::to_string(player.getWeaponCharges(itemId))}});
    return result;
}

GameCommandResult Game::useLensItem(const std::string &, const Item &item)
{
    if (player.getCurrentLocation() != GameIds::kCommandBridgeLocation)
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::ItemUseRejected, item.id, 0, "msg.lens_wrong_place");
        return result;
    }

    if (!player.hasItem(GameIds::kAccessKeyItem))
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::ItemUseRejected, item.id, 0, "msg.bridge_needs_access_key");
        return result;
    }

    if (player.hasFlag(GameIds::kBeaconOnlineFlag))
    {
        GameCommandResult result;
        appendEvent(result, GameEventType::ItemUseRejected, item.id, 0, "msg.beacon_already_on");
        return result;
    }

    player.addFlag(GameIds::kBeaconOnlineFlag);
    GameCommandResult result;
    appendEvent(result, GameEventType::ItemUsed, item.id, 0, item.type);
    return result;
}

GameCommandResult Game::useGenericItem(const std::string &, const Item &item)
{
    GameCommandResult result;
    appendEvent(result, GameEventType::ItemUsed, item.id, item.value, item.type);
    return result;
}

const std::unordered_map<std::string, Game::ItemUseHandler> &Game::itemUseHandlers()
{
    static const std::unordered_map<std::string, ItemUseHandler> handlers = {{"heal", &Game::useHealingItem},
                                                                             {"weapon", &Game::useWeaponItem}};

    return handlers;
}
