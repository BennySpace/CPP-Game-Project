#include "Core/GameEventMessageMapper.h"

#include "Data/DataLoader.h"
#include "Data/TextResources.h"
#include "Entities/Item.h"
#include "Entities/LogEntry.h"
#include "Entities/Rival.h"

#include <map>

namespace
{
void appendMessage(GameCommandResult &result, MessageTone tone, const std::string &text)
{
    if (text.empty())
    {
        return;
    }

    result.messages.push_back({tone, text});
}

void appendResourceMessage(GameCommandResult &result, MessageTone tone, const std::string &key)
{
    appendMessage(result, tone, TextResources::get(key));
}

void appendFormattedMessage(GameCommandResult &result, MessageTone tone, const std::string &key,
                            const std::map<std::string, std::string> &replacements)
{
    appendMessage(result, tone, TextResources::format(key, replacements));
}

const Item *findItem(const std::string &itemId)
{
    const auto &items = DataLoader::getItems();
    const auto itemIt = items.find(itemId);
    if (itemIt == items.end())
    {
        return nullptr;
    }

    return &itemIt->second;
}

const Rival *findRival(const std::string &rivalId)
{
    const auto &rivals = DataLoader::getRivals();
    const auto rivalIt = rivals.find(rivalId);
    if (rivalIt == rivals.end())
    {
        return nullptr;
    }

    return &rivalIt->second;
}

const LogEntry *findLog(const std::string &logId)
{
    const auto &logs = DataLoader::getLogs();
    const auto logIt = logs.find(logId);
    if (logIt == logs.end())
    {
        return nullptr;
    }

    return &logIt->second;
}

MessageTone itemUseTone(const std::string &itemType)
{
    if (itemType == "heal")
    {
        return MessageTone::Success;
    }
    if (itemType == "weapon")
    {
        return MessageTone::Warning;
    }
    return MessageTone::Normal;
}

void appendWarningFromResource(GameCommandResult &result, const std::string &key)
{
    if (!key.empty())
    {
        appendResourceMessage(result, MessageTone::Warning, key);
    }
}
} // namespace

void GameEventMessageMapper::appendMessages(GameCommandResult &result)
{
    for (const GameEvent &event : result.events)
    {
        switch (event.type)
        {
        case GameEventType::ActionRejected:
        case GameEventType::MoveBlocked:
            appendWarningFromResource(result, event.detail);
            break;
        case GameEventType::ItemTaken: {
            const Item *item = findItem(event.primaryId);
            if (item != nullptr)
            {
                appendFormattedMessage(result, MessageTone::Success, "msg.received_item", {{"name", item->name}});
            }
            break;
        }
        case GameEventType::ItemUseRejected: {
            if (event.detail == "msg.weapon_ready")
            {
                const Item *item = findItem(event.primaryId);
                if (item != nullptr)
                {
                    appendFormattedMessage(result, MessageTone::Warning, event.detail, {{"name", item->name}});
                }
                break;
            }

            if (!event.detail.empty())
            {
                appendResourceMessage(result, MessageTone::Warning, event.detail);
            }
            break;
        }
        case GameEventType::ItemUsed: {
            const Item *item = findItem(event.primaryId);
            if (item != nullptr)
            {
                appendMessage(result, itemUseTone(event.detail), item->useText);
            }
            break;
        }
        case GameEventType::AttackHit: {
            const Rival *rival = findRival(event.primaryId);
            if (rival != nullptr)
            {
                appendFormattedMessage(result, MessageTone::Danger, "msg.attack_hit",
                                       {{"name", rival->name}, {"damage", std::to_string(event.amount)}});
            }
            break;
        }
        case GameEventType::WeaponDepleted: {
            const Item *item = findItem(event.primaryId);
            if (item != nullptr)
            {
                appendFormattedMessage(result, MessageTone::Warning, "msg.weapon_spent", {{"name", item->name}});
            }
            break;
        }
        case GameEventType::ThreatDefeated: {
            const Rival *rival = findRival(event.primaryId);
            if (rival != nullptr)
            {
                appendMessage(result, MessageTone::Success, rival->defeatText);
            }
            break;
        }
        case GameEventType::PlayerDamaged: {
            const Rival *rival = findRival(event.primaryId);
            if (rival != nullptr)
            {
                appendMessage(result, MessageTone::Info, rival->attackText);
            }
            appendFormattedMessage(result, MessageTone::Info, "msg.health_loss",
                                   {{"damage", std::to_string(event.amount)}});
            break;
        }
        case GameEventType::InspectHintShown:
            if (!event.detail.empty())
            {
                appendFormattedMessage(result, MessageTone::Info, "msg.hint_line", {{"text", event.detail}});
            }
            break;
        case GameEventType::InspectItemFound: {
            const Item *item = findItem(event.primaryId);
            if (item != nullptr)
            {
                appendFormattedMessage(result, MessageTone::Warning, "msg.inspect_item",
                                       {{"name", item->name}, {"description", item->description}});
            }
            break;
        }
        case GameEventType::LogDiscovered: {
            const LogEntry *log = findLog(event.primaryId);
            if (log != nullptr)
            {
                appendFormattedMessage(result, MessageTone::Info, "msg.log_discovered", {{"name", log->name}});
            }
            break;
        }
        case GameEventType::LogRead: {
            const LogEntry *log = findLog(event.primaryId);
            if (log != nullptr)
            {
                appendFormattedMessage(result, MessageTone::Info, "msg.log_header", {{"name", log->name}});
                appendMessage(result, MessageTone::Muted, log->body);
            }
            break;
        }
        case GameEventType::InspectRivalFound: {
            const Rival *rival = findRival(event.primaryId);
            if (rival != nullptr)
            {
                appendFormattedMessage(result, MessageTone::Danger, "msg.inspect_enemy",
                                       {{"name", rival->name}, {"description", rival->description}});
                appendFormattedMessage(result, MessageTone::Info, "msg.enemy_health_remaining",
                                       {{"health", std::to_string(event.amount)}});
            }
            break;
        }
        default:
            break;
        }
    }
}
