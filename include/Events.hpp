#pragma once
#include <entt/entt.hpp>

struct MeleeAttackEvent
{
    entt::entity attacker;
    entt::entity target;
};
struct EntityDeathEvent
{
    entt::entity deadEntity;
};

struct DamageEvent
{
    entt::entity target;
    int damage;
};
struct HealEvent
{
    entt::entity target;
    int amount;
};

struct SpellCastEvent
{
    entt::entity caster;
    enum class SpellType
    {
        CRYO,
        FIRE,
        MELEE
    } type;
    float targetX;
    float targetY;
    bool *successFlag = nullptr;
};

struct ItemUseEvent
{
    entt::entity user;
    entt::entity item;
};
struct ItemPickupEvent
{
    entt::entity user;
    bool *successFlag = nullptr; // Added for turn-gating
};

struct DescendEvent
{
};