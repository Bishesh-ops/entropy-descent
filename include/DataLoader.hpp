#pragma once
#include "Definitions.hpp"
#include <string>
#include <vector>

class DataLoader
{
public:
    static std::vector<EnemyDef> loadEnemyDefs(const std::string &filepath);
    static std::vector<ItemDef> loadItemDefs(const std::string &filepath);
    static std::vector<SpellDef> loadSpellDefs(const std::string &filepath);
};