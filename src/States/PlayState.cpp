#include "../../include/States/PlayState.hpp"
#include "../../include/Events.hpp"
#include "../../include/Components.hpp"
#include "../../include/DataLoader.hpp"
#include "../../include/WorldBuilder.hpp"
#include "../../include/States/VowState.hpp"
#include <iostream>
#include <random>

constexpr int MAP_WIDTH = 200;
constexpr int MAP_HEIGHT = 150;
constexpr const char *ENEMY_DATA_PATH = "../data/enemies.json";
constexpr const char *ITEM_DATA_PATH = "../data/items.json";
constexpr const char *SPELL_DATA_PATH = "../data/spells.json";
constexpr const char *COMBAT_SCRIPT_PATH = "../scripts/combat.lua";
constexpr const char *AI_SCRIPT_PATH = "../scripts/ai.lua";

PlayState::PlayState(Game &gameRef)
    : game(gameRef),
      gameMap(MAP_WIDTH, MAP_HEIGHT, 20),
      cameraX(0),
      cameraY(0),
      currentTurnState(TurnState::WAITING_FOR_PLAYER),
      needsFOVUpdate(true),
      spatialGrid(MAP_WIDTH * MAP_HEIGHT, static_cast<entt::entity>(entt::null))
{

    lua.open_libraries(sol::lib::base, sol::lib::math);
    try
    {
        lua.script_file(COMBAT_SCRIPT_PATH);
        lua.script_file(AI_SCRIPT_PATH);
        lua.script_file("../scripts/spells.lua");
    }
    catch (const sol::error &e)
    {
        std::cerr << "Lua Error: " << e.what() << "\n";
    }

    lua.set_function("SpawnProjectile", [&](float x, float y, float dx, float dy, float speed, int damage, float lifeTime, const std::string &element, int r, int g, int b)
                     {
        auto proj = registry.create();
        registry.emplace<Transform>(proj, x, y);
        registry.emplace<Velocity>(proj, dx, dy, speed);
        registry.emplace<Hitbox>(proj, 8.0f, 8.0f, 6.0f, 6.0f);
        registry.emplace<RenderColor>(proj, static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), static_cast<uint8_t>(255));
        registry.emplace<Projectile>(proj, lifeTime, damage, element, playerEntity); });

    lua.set_function("SpawnParticles", [&](float x, float y, int count, int r, int g, int b)
                     {
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> angleDist(0.0f, 6.28318f);
        std::uniform_real_distribution<float> speedDist(20.0f, 150.0f);
        std::uniform_real_distribution<float> lifeDist(0.2f, 0.6f);

        for(int i = 0; i < count; ++i) {
            auto p = registry.create();
            registry.emplace<Transform>(p, x + 10.0f, y + 10.0f);
            float angle = angleDist(rng);
            float speed = speedDist(rng);
            float life = lifeDist(rng);
            registry.emplace<Particle>(p, life, life, std::cos(angle)*speed, std::sin(angle)*speed, static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b));
        } });

    loadedEnemies = DataLoader::loadEnemyDefs(ENEMY_DATA_PATH);
    loadedItems = DataLoader::loadItemDefs(ITEM_DATA_PATH);
    auto loadedSpells = DataLoader::loadSpellDefs(SPELL_DATA_PATH);

    movementSystem = std::make_unique<MovementSystem>(registry, gameMap, spatialGrid, MAP_WIDTH, MAP_HEIGHT);
    combatSystem = std::make_unique<CombatSystem>(game, registry, dispatcher, lua, spatialGrid, MAP_WIDTH, MAP_HEIGHT);
    itemSystem = std::make_unique<ItemSystem>(registry, dispatcher);
    spellSystem = std::make_unique<SpellSystem>(registry, dispatcher, lua);
    aiSystem = std::make_unique<AISystem>(registry, dispatcher, lua, gameMap, spatialGrid, MAP_WIDTH, MAP_HEIGHT);

    dispatcher.sink<DescendEvent>().connect<&PlayState::onDescend>(this);

    playerEntity = WorldBuilder::generateFloor(registry, gameMap, spatialGrid, MAP_WIDTH, MAP_HEIGHT, loadedEnemies, loadedItems);
}

void PlayState::processInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            game.quit();

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                float mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                dispatcher.trigger(SpellCastEvent{playerEntity, SpellCastEvent::SpellType::MELEE, mouseX + cameraX, mouseY + cameraY, nullptr});
            }
        }

        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_ESCAPE)
            {
                game.getStateMachine().popState();
                return;
            }
            if (event.key.key == SDLK_E)
            {
                float mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                dispatcher.trigger(SpellCastEvent{playerEntity, SpellCastEvent::SpellType::CRYO, mouseX + cameraX, mouseY + cameraY, nullptr});
            }
            if (event.key.key == SDLK_F)
            {
                float mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                dispatcher.trigger(SpellCastEvent{playerEntity, SpellCastEvent::SpellType::FIRE, mouseX + cameraX, mouseY + cameraY, nullptr});
            }
            if (event.key.key == SDLK_U)
            {
                auto &inv = registry.get<Inventory>(playerEntity);
                if (!inv.items.empty())
                {
                    dispatcher.trigger(ItemUseEvent{playerEntity, inv.items[0]});
                }
                else
                {
                    std::cout << "Nothing to use.\n";
                }
            }
            if (event.key.key == SDLK_G)
            {
                bool success = false;
                dispatcher.trigger(ItemPickupEvent{playerEntity, &success});
            }
            if (event.key.key == SDLK_PERIOD)
            {
                auto &playerPos = registry.get<Position>(playerEntity);
                auto stairsView = registry.view<Position, Stairs>();
                bool onStairs = false;
                for (auto [entity, pos] : stairsView.each())
                {
                    if (pos.x == playerPos.x && pos.y == playerPos.y)
                    {
                        onStairs = true;
                        break;
                    }
                }
                if (onStairs)
                    dispatcher.trigger(DescendEvent{});
            }
        }
    }

    int numKeys;
    const bool *keyState = SDL_GetKeyboardState(&numKeys);

    if (registry.valid(playerEntity) && registry.all_of<Velocity>(playerEntity))
    {
        auto &vel = registry.get<Velocity>(playerEntity);
        vel.dx = 0.0f;
        vel.dy = 0.0f;

        if (keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP])
            vel.dy -= 1.0f;
        if (keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN])
            vel.dy += 1.0f;
        if (keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_LEFT])
            vel.dx -= 1.0f;
        if (keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_RIGHT])
            vel.dx += 1.0f;

        if (vel.dx != 0.0f || vel.dy != 0.0f)
        {
            float length = std::sqrt(vel.dx * vel.dx + vel.dy * vel.dy);
            vel.dx /= length;
            vel.dy /= length;
        }
    }
}

void PlayState::update(float dt)
{
    movementSystem->update(dt);
    auto &pos = registry.get<Position>(playerEntity);

    if (registry.all_of<Transform>(playerEntity))
    {
        auto &pTransform = registry.get<Transform>(playerEntity);
        cameraX = std::clamp(static_cast<int>(pTransform.x) - (game.getWindowWidth() / 2), 0, (MAP_WIDTH * 20) - game.getWindowWidth());
        cameraY = std::clamp(static_cast<int>(pTransform.y) - (game.getWindowHeight() / 2), 0, (MAP_HEIGHT * 20) - game.getWindowHeight());
    }

    static int lastFovX = -1, lastFovY = -1;
    if (pos.x != lastFovX || pos.y != lastFovY || needsFOVUpdate)
    {
        int dynamicFOV = registry.get<EntropyStats>(playerEntity).fovRadius;
        gameMap.calculateFOV(pos.x, pos.y, dynamicFOV);
        lastFovX = pos.x;
        lastFovY = pos.y;
        needsFOVUpdate = false;
    }

    aiSystem->update(playerEntity, floorDepth, dt);

    static float entropyTimer = 0.0f;
    entropyTimer += dt;
    if (entropyTimer >= 1.0f)
    {
        if (registry.all_of<EntropyStats>(playerEntity))
        {
            auto &eStats = registry.get<EntropyStats>(playerEntity);
            if (eStats.entropy > 0)
                eStats.entropy--;
        }
        entropyTimer = 0.0f;
    }

    auto &eStats = registry.get<EntropyStats>(playerEntity);
    if (eStats.entropy >= 100)
    {
        game.getStateMachine().pushState(std::make_unique<VowState>(game, registry, playerEntity));
    }

    combatSystem->update(dt);
}

void PlayState::render()
{
    renderSystem.update(game.getRenderer(), registry, gameMap, cameraX, cameraY,
                        game.getWindowWidth(), game.getWindowHeight(), uiRenderer, playerEntity, floorDepth);
}

void PlayState::updateSpatialGrid(entt::entity entity, int oldX, int oldY, int newX, int newY)
{
    if (oldX >= 0 && oldY >= 0 && oldX < MAP_WIDTH && oldY < MAP_HEIGHT)
    {
        int oldIndex = oldY * MAP_WIDTH + oldX;
        if (spatialGrid[oldIndex] == entity)
            spatialGrid[oldIndex] = entt::null;
    }
    if (newX >= 0 && newY >= 0 && newX < MAP_WIDTH && newY < MAP_HEIGHT)
    {
        spatialGrid[newY * MAP_WIDTH + newX] = entity;
    }
}

void PlayState::onDescend(const DescendEvent &event)
{
    std::vector<entt::entity> toDestroy;
    auto view = registry.view<entt::entity>();
    for (auto entity : view)
    {
        if (entity == playerEntity)
            continue;
        if (!registry.all_of<Position>(entity) && registry.all_of<Item>(entity))
            continue;
        toDestroy.push_back(entity);
    }

    for (auto entity : toDestroy)
        registry.destroy(entity);

    floorDepth++;
    WorldBuilder::repopulateFloor(registry, gameMap, spatialGrid, MAP_WIDTH, MAP_HEIGHT, loadedEnemies, loadedItems, playerEntity, floorDepth);
    needsFOVUpdate = true;
}

entt::entity PlayState::getBlockingEntityAt(int x, int y)
{
    if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT)
        return entt::null;
    return spatialGrid[y * MAP_WIDTH + x];
}