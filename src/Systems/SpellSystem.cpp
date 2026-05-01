#include "../../include/Systems/SpellSystem.hpp"
#include "../../include/Components.hpp"
#include <iostream>

SpellSystem::SpellSystem(entt::registry &reg, entt::dispatcher &disp, sol::state &luaState)
    : registry(reg), dispatcher(disp), lua(luaState)
{
    dispatcher.sink<SpellCastEvent>().connect<&SpellSystem::onSpellCast>(this);
}

void SpellSystem::onSpellCast(const SpellCastEvent &event)
{
    if (!registry.valid(event.caster) || !registry.all_of<Transform>(event.caster))
        return;

    auto &t = registry.get<Transform>(event.caster);
    int entropyBonus = registry.all_of<EntropyStats>(event.caster) ? registry.get<EntropyStats>(event.caster).bonusAoE : 0;
    int baseAttack = registry.all_of<CombatStats>(event.caster) ? registry.get<CombatStats>(event.caster).attack : 5;

    std::string spellStr = "MELEE";
    if (event.type == SpellCastEvent::SpellType::FIRE)
        spellStr = "FIRE";
    if (event.type == SpellCastEvent::SpellType::CRYO)
        spellStr = "CRYO";

    sol::table spellsTable = lua["Spells"];
    if (spellsTable.valid())
    {
        sol::function castFunc = spellsTable["Cast"];
        if (castFunc.valid())
        {
            auto result = castFunc(spellStr, t.x, t.y, event.targetX, event.targetY, entropyBonus, baseAttack);
            if (!result.valid())
            {
                sol::error err = result;
                std::cerr << "Lua Spell Cast Error: " << err.what() << "\n";
            }
        }
        else
        {
            std::cerr << "C++ Warning: 'Cast' function not found in 'Spells' Lua table.\n";
        }
    }
    else
    {
        std::cerr << "C++ Warning: 'Spells' table not found in Lua environment. Did spells.lua fail to load?\n";
    }

    if (event.type != SpellCastEvent::SpellType::MELEE && registry.all_of<EntropyStats>(event.caster))
    {
        registry.get<EntropyStats>(event.caster).entropy += 15;
    }
}