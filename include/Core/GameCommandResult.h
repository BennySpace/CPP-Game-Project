#pragma once

#include <string>
#include <vector>

enum class MessageTone
{
    Normal,
    Muted,
    Info,
    Success,
    Warning,
    Danger
};

enum class GameEventType
{
    ActionRejected,
    MoveBlocked,
    ItemTaken,
    ItemUseRejected,
    ItemUsed,
    AttackHit,
    WeaponDepleted,
    ThreatDefeated,
    PlayerDamaged,
    InspectHintShown,
    InspectItemFound,
    LogDiscovered,
    LogRead,
    InspectRivalFound
};

struct GameEvent
{
    GameEventType type = GameEventType::ActionRejected;
    std::string primaryId;
    int amount = 0;
    std::string detail;
};

struct OutputMessage
{
    MessageTone tone = MessageTone::Normal;
    std::string text;
};

enum class ViewKind
{
    None,
    Welcome,
    Location,
    Map,
    Help,
    Status,
    Objective,
    Inventory,
    Victory,
    GameOver
};

struct GameCommandResult
{
    ViewKind view = ViewKind::None;
    std::vector<GameEvent> events;
    std::vector<OutputMessage> messages;
};
