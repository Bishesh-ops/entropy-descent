#pragma once
#include <entt/entt.hpp>
#include "../Events.hpp"

class ItemSystem
{
public:
    ItemSystem(entt::registry &reg, entt::dispatcher &disp);
    void onItemUse(const ItemUseEvent &event);
    void onItemPickup(const ItemPickupEvent &event);

private:
    entt::registry &registry;
    entt::dispatcher &dispatcher;
};