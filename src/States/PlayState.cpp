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

PlayState::PlayState(Game &gameRef)
    : game(gameRef), gameMap(MAP_WIDTH, MAP_HEIGHT, 20), cameraX(0), cameraY(0),
      currentTurnState(TurnState::WAITING_FOR_PLAYER), needsFOVUpdate(true),
      spatialGrid(MAP_WIDTH * MAP_HEIGHT, static_cast<entt::entity>(entt::null))
{
    // Load Data
    auto loadedEnemies = DataLoader::loadEnemyDefs(ENEMY_DATA_PATH);
    auto loadedItems = DataLoader::loadItemDefs(ITEM_DATA_PATH);
    auto loadedSpells = DataLoader::loadSpellDefs(SPELL_DATA_PATH);

    // Initialize Systems
    combatSystem = std::make_unique<CombatSystem>(game, registry, dispatcher, spatialGrid, MAP_WIDTH, MAP_HEIGHT);
    itemSystem = std::make_unique<ItemSystem>(registry, dispatcher);
    spellSystem = std::make_unique<SpellSystem>(registry, dispatcher, gameMap, spatialGrid, MAP_WIDTH, MAP_HEIGHT, loadedSpells);
    aiSystem = std::make_unique<AISystem>(registry, dispatcher, gameMap, spatialGrid, MAP_WIDTH, MAP_HEIGHT);

    // Generate World
    playerEntity = WorldBuilder::generateFloor(registry, gameMap, spatialGrid, MAP_WIDTH, MAP_HEIGHT, loadedEnemies, loadedItems);
}

void PlayState::processInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            game.quit();

        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_ESCAPE)
            {
                game.getStateMachine().popState();
                return;
            }

            if (currentTurnState != TurnState::WAITING_FOR_PLAYER)
                continue;

            bool playerActed = false;
            auto &pos = registry.get<Position>(playerEntity);
            int nextX = pos.x;
            int nextY = pos.y;

            if (event.key.key == SDLK_E)
            {
                bool success = false;
                dispatcher.trigger(SpellCastEvent{playerEntity, SpellCastEvent::SpellType::CRYO, &success});
                playerActed = success;
            }
            if (event.key.key == SDLK_F)
            {
                bool success = false;
                dispatcher.trigger(SpellCastEvent{playerEntity, SpellCastEvent::SpellType::FIRE, &success});
                playerActed = success;
            }
            if (event.key.key == SDLK_U)
            {
                auto &inv = registry.get<Inventory>(playerEntity);
                if (!inv.items.empty())
                {
                    dispatcher.trigger(ItemUseEvent{playerEntity, inv.items[0]});
                    playerActed = true;
                }
                else
                {
                    std::cout << "Nothing to use.\n";
                }
            }
            if (event.key.key == SDLK_G)
            {
                dispatcher.trigger(ItemPickupEvent{playerEntity});
                playerActed = true;
            }

            if (event.key.key == SDLK_W || event.key.key == SDLK_UP)
                nextY--;
            if (event.key.key == SDLK_S || event.key.key == SDLK_DOWN)
                nextY++;
            if (event.key.key == SDLK_A || event.key.key == SDLK_LEFT)
                nextX--;
            if (event.key.key == SDLK_D || event.key.key == SDLK_RIGHT)
                nextX++;

            if (nextX != pos.x || nextY != pos.y)
            {
                if (gameMap.isFloor(nextX, nextY))
                {
                    entt::entity blocker = getBlockingEntityAt(nextX, nextY);
                    if (blocker != entt::null)
                    {
                        if (registry.all_of<Enemy>(blocker))
                        {
                            dispatcher.trigger(MeleeAttackEvent{playerEntity, blocker});
                            playerActed = true;
                        }
                    }
                    else
                    {
                        updateSpatialGrid(playerEntity, pos.x, pos.y, nextX, nextY);
                        pos.x = nextX;
                        pos.y = nextY;
                        playerActed = true;
                    }
                }
            }

            if (playerActed)
            {
                currentTurnState = TurnState::ENEMY_TURN;
                needsFOVUpdate = true;
            }
        }
    }
}

void PlayState::update()
{
    auto &pos = registry.get<Position>(playerEntity);

    if (currentTurnState == TurnState::ENEMY_TURN)
    {
        aiSystem->update(playerEntity);

        // --- Entropy Decay ---
        if (registry.all_of<EntropyStats>(playerEntity))
        {
            auto &eStats = registry.get<EntropyStats>(playerEntity);
            if (eStats.entropy > 0)
                eStats.entropy--;
        }

        auto &eStats = registry.get<EntropyStats>(playerEntity);

        if (eStats.entropy >= 100)
        {
            game.getStateMachine().pushState(std::make_unique<VowState>(game, registry, playerEntity));
        }
        else if (eStats.entropy >= 86)
        {
            std::mt19937 rng(std::random_device{}());
            std::uniform_int_distribution<int> distX(0, MAP_WIDTH - 1);
            std::uniform_int_distribution<int> distY(0, MAP_HEIGHT - 1);
            int cx = distX(rng), cy = distY(rng);

            if (gameMap.isFloor(cx, cy) && gameMap.getTileState(cx, cy) == TileState::EMPTY)
            {
                gameMap.setTileState(cx, cy, TileState::FIRE);
                std::cout << "Reality degrades... a tile combusts into FIRE.\n";
            }
        }
        else if (eStats.entropy >= 61)
        {
            int phantomCount = 0;
            for (auto e : registry.view<Phantom>())
                phantomCount++;

            if (phantomCount < 2)
            {
                std::vector<int> visibleFloor;
                for (int y = 0; y < MAP_HEIGHT; ++y)
                {
                    for (int x = 0; x < MAP_WIDTH; ++x)
                    {
                        if (gameMap.isFloor(x, y) && gameMap.isVisible(x, y) && spatialGrid[y * MAP_WIDTH + x] == entt::null)
                        {
                            visibleFloor.push_back(y * MAP_WIDTH + x);
                        }
                    }
                }
                if (!visibleFloor.empty())
                {
                    std::mt19937 rng(std::random_device{}());
                    std::uniform_int_distribution<int> dist(0, visibleFloor.size() - 1);
                    int targetIndex = visibleFloor[dist(rng)];

                    auto phantom = registry.create();
                    registry.emplace<Position>(phantom, targetIndex % MAP_WIDTH, targetIndex / MAP_WIDTH);
                    registry.emplace<Enemy>(phantom);
                    registry.emplace<Phantom>(phantom);
                    registry.emplace<RenderColor>(phantom, static_cast<uint8_t>(180), static_cast<uint8_t>(0), static_cast<uint8_t>(180), static_cast<uint8_t>(180));
                }
            }
        }

        // Handoff back to player
        currentTurnState = TurnState::WAITING_FOR_PLAYER;
    }

    // FOV & Camera follow outside the turn gate so it stays smooth
    if (needsFOVUpdate)
    {
        int dynamicFOV = registry.get<EntropyStats>(playerEntity).fovRadius;
        gameMap.calculateFOV(pos.x, pos.y, dynamicFOV);

        cameraX = std::clamp(pos.x * 20 - (game.getWindowWidth() / 2), 0, (MAP_WIDTH * 20) - game.getWindowWidth());
        cameraY = std::clamp(pos.y * 20 - (game.getWindowHeight() / 2), 0, (MAP_HEIGHT * 20) - game.getWindowHeight());
        needsFOVUpdate = false;
    }

    combatSystem->update();
}
void PlayState::render()
{
    renderSystem.update(game.getRenderer(), registry, gameMap, cameraX, cameraY,
                        game.getWindowWidth(), game.getWindowHeight(), uiRenderer, playerEntity);
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

entt::entity PlayState::getBlockingEntityAt(int x, int y)
{
    if (x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT)
        return entt::null;
    return spatialGrid[y * MAP_WIDTH + x];
}