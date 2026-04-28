#pragma once
#include <entt/entt.hpp>

struct MeleeCombact
{
    entt::entity atacker;
    entt::entity target;
};

struct EntityDeathEvent
{
    entt::entity deadEntity;
};