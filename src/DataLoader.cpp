#include "../include/DataLoader.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

std::vector<EnemyDef> DataLoader::loadEnemyDefs(const std::string &filepath)
{
    std::vector<EnemyDef> defs;
    std::ifstream file(filepath);

    if (!file.is_open())
    {
        std::cerr << "ERROR: Failed to open " << filepath << std::endl;
        return defs;
    }

    try
    {
        json j;
        file >> j;

        if (!j.is_array()) {
            std::cerr << "FATAL ERROR: " << filepath << " is not a JSON Array!" << std::endl;
            return defs;
        }

        for (const auto& value : j)
        {
            EnemyDef def;

            def.id = value.value("id", "unknown_id");
            

            if (value.contains("components")) {
                auto& comps = value["components"];
                
                if (comps.contains("Health")) {
                    def.hp = comps["Health"].value("max", 10);
                } else {
                    def.hp = 10;
                }
                
                if (comps.contains("CombatStats")) {
                    def.attack = comps["CombatStats"].value("attack", 1);
                } else {
                    def.attack = 1;
                }
                
                def.defense = 0; 
            }

            if (value.contains("color") && value["color"].is_array() && value["color"].size() == 4)
            {
                def.r = value["color"][0];
                def.g = value["color"][1];
                def.b = value["color"][2];
                def.a = value["color"][3];
            }
            else
            {
                def.r = 255;
                def.g = 0;
                def.b = 255;
                def.a = 255;
            }

            defs.push_back(def);
        }
    }
    catch (json::parse_error &e)
    {
        std::cerr << "JSON parse error in " << filepath << ": " << e.what() << std::endl;
    }

    return defs;
}

std::vector<ItemDef> DataLoader::loadItemDefs(const std::string &filepath)
{
    std::vector<ItemDef> defs;
    std::ifstream file(filepath);

    if (!file.is_open())
    {
        std::cerr << "ERROR: Failed to open " << filepath << std::endl;
        return defs;
    }

    try
    {
        json j;
        file >> j;

        for (auto &[key, value] : j["items"].items())
        {
            ItemDef def;
            def.id = key;
            def.name = value.value("name", "Unknown Item");
            def.effectType = value.value("effectType", "none");
            def.magnitude = value.value("magnitude", 0);

            if (value.contains("color") && value["color"].is_array() && value["color"].size() == 4)
            {
                def.r = value["color"][0];
                def.g = value["color"][1];
                def.b = value["color"][2];
                def.a = value["color"][3];
            }
            else
            {
                def.r = 255;
                def.g = 255;
                def.b = 255;
                def.a = 255;
            }

            defs.push_back(def);
        }
    }
    catch (json::parse_error &e)
    {
        std::cerr << "JSON parse error in " << filepath << ": " << e.what() << std::endl;
    }

    return defs;
}

std::vector<SpellDef> DataLoader::loadSpellDefs(const std::string &filepath)
{
    std::vector<SpellDef> defs;
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "ERROR: Failed to open " << filepath << std::endl;
        return defs;
    }
    try
    {
        json j;
        file >> j;
        for (auto &[key, value] : j["spells"].items())
        {
            SpellDef def;
            def.name = value.value("name", "Unknown");
            def.baseCost = value.value("baseCost", 0);
            def.damage = value.value("damage", 0);
            def.element = value.value("element", "none");
            defs.push_back(def);
        }
    }
    catch (json::parse_error &e)
    {
        std::cerr << "JSON parse error in " << filepath << ": " << e.what() << std::endl;
    }
    return defs;
}

std::vector<VowDef> DataLoader::loadVowDefs(const std::string &filepath)
{
    std::vector<VowDef> defs;
    std::ifstream file(filepath);
    if (!file.is_open())
        return defs;
    try
    {
        json j;
        file >> j;
        for (auto &[key, value] : j["vows"].items())
        {
            VowDef def;
            def.id = key;
            def.name = value.value("name", "Unknown Vow");
            def.description = value.value("description", "???");
            def.type = value.value("type", "none");
            defs.push_back(def);
        }
    }
    catch (json::parse_error &e)
    {
        std::cerr << "JSON Error: " << e.what() << "\n";
    }
    return defs;
}