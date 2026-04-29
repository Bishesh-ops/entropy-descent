#pragma once
#include <string>
#include <cstdint>

struct EnemyDef
{
    std::string id; // Storing the JSON key
    int hp;
    int attack;
    int defense;
    uint8_t r, g, b, a; // The color channels
};

struct ItemDef
{
    std::string id;
    std::string name;
    std::string effectType;
    int magnitude;
    uint8_t r, g, b, a;
};
struct SpellDef
{
    std::string name;
    int baseCost;
    int damage;
    std::string element;
};