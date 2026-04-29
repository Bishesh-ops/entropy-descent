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

        // Iterate through the dictionary keys instead of an array
        for (auto &[key, value] : j["enemies"].items())
        {
            EnemyDef def;
            def.id = key;
            def.hp = value.value("hp", 10);
            def.attack = value.value("attack", 1);
            def.defense = value.value("defense", 0);

            // Parse the color array securely
            if (value.contains("color") && value["color"].is_array() && value["color"].size() == 4)
            {
                def.r = value["color"][0];
                def.g = value["color"][1];
                def.b = value["color"][2];
                def.a = value["color"][3];
            }
            else
            {
                // Fallback magenta if the JSON color array is missing or broken
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
std::vector<SpellDef> DataLoader::loadSpellDefs(const std::string &filepath) { return {}; }