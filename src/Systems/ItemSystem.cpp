#include "../../include/Systems/ItemSystem.hpp"
#include "../../include/Components.hpp"
#include <iostream>

ItemSystem::ItemSystem(entt::registry &reg, entt::dispatcher &disp) : registry(reg), dispatcher(disp)
{
    dispatcher.sink<ItemUseEvent>().connect<&ItemSystem::onItemUse>(this);
    dispatcher.sink<ItemPickupEvent>().connect<&ItemSystem::onItemPickup>(this);
}

void ItemSystem::onItemUse(const ItemUseEvent &event)
{
    if (!registry.valid(event.item) || !registry.all_of<ItemEffect>(event.item))
        return;

    auto &effect = registry.get<ItemEffect>(event.item);

    if (effect.effectType == "heal")
    {
        dispatcher.trigger(HealEvent{event.user, effect.magnitude});
    }
    else if (effect.effectType == "damage_boost")
    {
        if (registry.all_of<CombatStats>(event.user))
        {
            auto &stats = registry.get<CombatStats>(event.user);
            stats.attack += effect.magnitude;
            std::cout << "Consumed item. Attack permanently boosted by " << effect.magnitude << "!\n";
        }
    }

    if (registry.all_of<Inventory>(event.user))
    {
        auto &inv = registry.get<Inventory>(event.user);
        auto it = std::find(inv.items.begin(), inv.items.end(), event.item);
        if (it != inv.items.end())
            inv.items.erase(it);
    }
    registry.destroy(event.item);
}

void ItemSystem::onItemPickup(const ItemPickupEvent &event)
{
    if (!registry.all_of<Position>(event.user) || !registry.all_of<Inventory>(event.user))
        return;

    auto &userPos = registry.get<Position>(event.user);
    auto &inv = registry.get<Inventory>(event.user);

    entt::entity pickedUp = entt::null;
    auto view = registry.view<Position, Item>();

    for (auto entity : view)
    {
        auto &itemPos = view.get<Position>(entity);
        if (itemPos.x == userPos.x && itemPos.y == userPos.y)
        {
            pickedUp = entity;
            break;
        }
    }

    bool pickedSomething = false;
    if (pickedUp != entt::null)
    {
        if (static_cast<int>(inv.items.size()) < inv.maxCapacity)
        {
            inv.items.push_back(pickedUp);
            registry.remove<Position>(pickedUp);
            std::cout << "Picked up item!\n";
            pickedSomething = true;
        }
        else
        {
            std::cout << "Inventory is full!\n";
        }
    }
    if (event.successFlag)
    {
        *(event.successFlag) = pickedSomething;
    }
}