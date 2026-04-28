#pragma once
#include <cstdint>

struct Position
{
    int x;
    int y;
};
struct Player
{
};
struct Enemy
{
};
struct Collider
{
};
struct Health
{
    int current;
    int max;
};
struct CombatStats
{
    int attack;
    int defense;
};

struct RenderColor
{
    uint8_t r, g, b, a;
};