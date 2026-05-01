#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <entt/entt.hpp>

struct Position
{
    int x;
    int y;
};
struct Transform
{
    float x;
    float y;
};
struct Velocity
{
    float dx = 0.0f;
    float dy = 0.0f;
    float speed = 100.0f; // Pixels per second
};
struct Player
{
};

struct Hitbox
{
    float width = 16.0f;
    float height = 16.0f;
    float offsetX = 2.0f;
    float offsetY = 2.0f;
};
struct Enemy
{
    float attackCooldown = 0.0f;
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

struct Item
{
};

struct ItemEffect
{
    std::string effectType;
    int magnitude;
};

struct Inventory
{
    std::vector<entt::entity> items;
    int maxCapacity = 10;
};
struct Phantom
{
};

struct EntropyStats
{
    int entropy = 0;
    int fovRadius = 15;

    // --- Vow Mutations ---
    int bonusAoE = 0;            // FOV Vow
    bool hasPassiveAura = false; // Combat Vow
    bool healthLocked = false;   // Healing Vow
};
struct PendingDestroy
{
};

struct Stairs
{
};

struct Projectile
{
    float lifeTime;
    int damage;
    std::string element;
    entt::entity caster;
};

struct Particle
{
    float lifeTime;
    float maxLife;
    float vx, vy;
    uint8_t r, g, b;
};