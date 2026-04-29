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
};
struct ItemUseEvent
{
    entt::entity user;
    entt::entity item;
};