#pragma once
#include <entt/entt.hpp>
#include <sol/sol.hpp>
#include <vector>
#include "../Events.hpp"
#include "../Definitions.hpp"

class SpellSystem
{
public:
    SpellSystem(entt::registry &reg, entt::dispatcher &disp, sol::state &luaState);
    void onSpellCast(const SpellCastEvent &event);

private:
    entt::registry &registry;
    entt::dispatcher &dispatcher;
    sol::state &lua;
};