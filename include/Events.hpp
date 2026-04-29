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

struct SpellCastEvent
{
    entt::entity caster;
    enum class SpellType
    {
        CRYO,
        FIRE
    } type;
};
struct ItemUseEvent
{
    entt::entity user;
    entt::entity item;
};