#include "Core/Player.h"

#include <algorithm>

Player::Player() : currentLocation("dock"), health(85), maxHealth(85), baseAttack(5)
{
}

const std::string &Player::getCurrentLocation() const
{
    return currentLocation;
}

void Player::setCurrentLocation(const std::string &locationId)
{
    currentLocation = locationId;
}

int Player::getHealth() const
{
    return health;
}

int Player::getMaxHealth() const
{
    return maxHealth;
}

void Player::takeDamage(int damage)
{
    health -= damage;
    if (health < 0)
    {
        health = 0;
    }
}

void Player::heal(int amount)
{
    health += amount;
    if (health > maxHealth)
    {
        health = maxHealth;
    }
}

int Player::getBaseAttack() const
{
    return baseAttack;
}

int Player::getAttack() const
{
    return baseAttack + activeWeaponBonus;
}

const std::vector<std::string> &Player::getInventory() const
{
    return inventory;
}

bool Player::hasItem(const std::string &itemId) const
{
    return std::find(inventory.begin(), inventory.end(), itemId) != inventory.end();
}

void Player::addItem(const std::string &itemId)
{
    inventory.push_back(itemId);
}

void Player::removeItem(const std::string &itemId)
{
    const auto it = std::find(inventory.begin(), inventory.end(), itemId);
    if (it != inventory.end())
    {
        inventory.erase(it);
    }

    weaponCharges.erase(itemId);
    if (activeWeaponId == itemId)
    {
        activeWeaponId.clear();
        activeWeaponBonus = 0;
    }
}

bool Player::knowsLog(const std::string &logId) const
{
    return std::find(knownLogs.begin(), knownLogs.end(), logId) != knownLogs.end();
}

void Player::learnLog(const std::string &logId)
{
    if (!knowsLog(logId))
    {
        knownLogs.push_back(logId);
    }
}

const std::vector<std::string> &Player::getKnownLogs() const
{
    return knownLogs;
}

const std::string &Player::getActiveWeaponId() const
{
    return activeWeaponId;
}

int Player::getActiveWeaponCharges() const
{
    if (activeWeaponId.empty())
    {
        return 0;
    }

    return getWeaponCharges(activeWeaponId);
}

void Player::armWeapon(const std::string &itemId, int attackBonus, int maxCharges)
{
    auto [it, inserted] = weaponCharges.emplace(itemId, maxCharges);
    if (!inserted && it->second <= 0)
    {
        it->second = maxCharges;
    }

    activeWeaponId = itemId;
    activeWeaponBonus = attackBonus;
}

int Player::getWeaponCharges(const std::string &itemId) const
{
    const auto it = weaponCharges.find(itemId);
    if (it == weaponCharges.end())
    {
        return 0;
    }

    return it->second;
}

int Player::consumeActiveWeaponCharge()
{
    if (activeWeaponId.empty())
    {
        return 0;
    }

    auto it = weaponCharges.find(activeWeaponId);
    if (it == weaponCharges.end() || it->second <= 0)
    {
        activeWeaponId.clear();
        activeWeaponBonus = 0;
        return 0;
    }

    --it->second;
    const int remainingCharges = it->second;
    if (remainingCharges <= 0)
    {
        activeWeaponId.clear();
        activeWeaponBonus = 0;
        weaponCharges.erase(it);
        return 0;
    }

    return remainingCharges;
}

bool Player::hasFlag(const std::string &flag) const
{
    return flags.find(flag) != flags.end();
}

void Player::addFlag(const std::string &flag)
{
    flags.insert(flag);
}
