#pragma once

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

class Player
{
  public:
    Player();

    const std::string &getCurrentLocation() const;
    void setCurrentLocation(const std::string &locationId);

    int getHealth() const;
    int getMaxHealth() const;
    void takeDamage(int damage);
    void heal(int amount);

    int getBaseAttack() const;
    int getAttack() const;

    const std::vector<std::string> &getInventory() const;
    bool hasItem(const std::string &itemId) const;
    void addItem(const std::string &itemId);
    void removeItem(const std::string &itemId);
    bool knowsLog(const std::string &logId) const;
    void learnLog(const std::string &logId);
    const std::vector<std::string> &getKnownLogs() const;

    const std::string &getActiveWeaponId() const;
    int getActiveWeaponCharges() const;
    void armWeapon(const std::string &itemId, int attackBonus, int maxCharges);
    int getWeaponCharges(const std::string &itemId) const;
    int consumeActiveWeaponCharge();

    bool hasFlag(const std::string &flag) const;
    void addFlag(const std::string &flag);

  private:
    std::string currentLocation;
    int health;
    int maxHealth;
    int baseAttack;
    std::vector<std::string> inventory;
    std::vector<std::string> knownLogs;
    std::unordered_map<std::string, int> weaponCharges;
    std::string activeWeaponId;
    int activeWeaponBonus = 0;
    std::set<std::string> flags;
};
