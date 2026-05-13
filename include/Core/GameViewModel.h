#pragma once

#include "Core/GameCommandResult.h"

#include <optional>
#include <string>
#include <vector>

struct GameItemView
{
    std::string name;
    std::string id;
    std::string detail;
};

struct GameLogView
{
    std::string name;
    std::string id;
    std::string detail;
};

struct GameExitView
{
    std::string directionId;
    std::string targetName;
};

struct GameLocationView
{
    std::string name;
    std::string description;
    std::vector<GameItemView> nearbyItems;
    std::vector<GameLogView> nearbyLogs;
    std::string rivalName;
    int rivalHp = 0;
    std::vector<GameExitView> exits;
};

struct GameHudView
{
    std::string currentLocationName;
    int health = 0;
    int maxHealth = 0;
    int baseAttack = 0;
    int attack = 0;
    std::string threatName;
    int threatHp = 0;
    std::vector<GameExitView> exits;
    std::vector<GameItemView> inventory;
    std::vector<GameLogView> knownLogs;
    std::string activeWeaponName;
    int activeWeaponCharges = 0;
    bool hasAccessKey = false;
    bool hasLens = false;
    bool hasKeycard = false;
    bool beaconOnline = false;
    bool rootHeartDestroyed = false;
};

struct GameViewModel
{
    GameHudView hud;
    GameCommandResult result;
    std::optional<GameLocationView> location;
    std::string mapCurrentLocationName;
};
