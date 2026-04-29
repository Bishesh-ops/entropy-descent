#include "../../include/States/PlayState.hpp"
#include "../../include/Events.hpp"
#include "../../include/Components.hpp"
#include "../../include/DataLoader.hpp"
#include <iostream>
#include <random>
#include <cmath>

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
    // --- Event Bindings ---
    combatSystem = std::make_unique<CombatSystem>(game, registry, dispatcher, spatialGrid, MAP_WIDTH, MAP_HEIGHT);
    std::vector<SpellDef> loadedSpells = DataLoader::loadSpellDefs(SPELL_DATA_PATH);
    itemSystem = std::make_unique<ItemSystem>(registry, dispatcher);
    spellSystem = std::make_unique<SpellSystem>(registry, dispatcher, gameMap, spatialGrid, MAP_WIDTH, MAP_HEIGHT, loadedSpells);

    // --- World Generation ---
    std::cout << "Generating massive world..." << std::endl;
    gameMap.generateCaves(45, 5);
    gameMap.processMap();

    //  Entropic Sources (Water Pools)
    std::mt19937 poolRng(std::random_device{}());
    std::uniform_int_distribution<int> poolDistX(2, MAP_WIDTH - 3);
    std::uniform_int_distribution<int> poolDistY(2, MAP_HEIGHT - 3);
    std::uniform_int_distribution<int> poolChance(0, 99);

    int numberOfPools = 40;
    for (int i = 0; i < numberOfPools; ++i)
    {
        int cx = poolDistX(poolRng);
        int cy = poolDistY(poolRng);

        // Find a random floor tile to act as the center of our puddle
        while (!gameMap.isFloor(cx, cy))
        {
            cx = poolDistX(poolRng);
            cy = poolDistY(poolRng);
        }

        // Splash water around the center tile organically
        for (int dy = -2; dy <= 2; ++dy)
        {
            for (int dx = -2; dx <= 2; ++dx)
            {
                int tx = cx + dx;
                int ty = cy + dy;

                if (gameMap.isFloor(tx, ty))
                {
                    // The closer to the center, the higher the chance of water
                    int dist = std::abs(dx) + std::abs(dy);
                    int probability = 100 - (dist * 25);

                    if (poolChance(poolRng) < probability)
                    {
                        gameMap.setTileState(tx, ty, TileState::WATER);
                    }
                }
            }
        }
    }

    // --- Entropic Sources (Fire Pools) ---
    int numberOfFirePools = 15;
    for (int i = 0; i < numberOfFirePools; ++i)
    {
        int cx = poolDistX(poolRng);
        int cy = poolDistY(poolRng);

        while (!gameMap.isFloor(cx, cy))
        {
            cx = poolDistX(poolRng);
            cy = poolDistY(poolRng);
        }

        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                int tx = cx + dx;
                int ty = cy + dy;

                if (gameMap.isFloor(tx, ty) && poolChance(poolRng) < 60)
                {
                    gameMap.setTileState(tx, ty, TileState::FIRE);
                }
            }
        }
    }

    // --- Player Initialization ---
    playerEntity = registry.create();
    registry.emplace<Player>(playerEntity);
    registry.emplace<Collider>(playerEntity);
    registry.emplace<Health>(playerEntity, 100, 100);
    registry.emplace<CombatStats>(playerEntity, 10, 5);
    registry.emplace<Inventory>(playerEntity);
    registry.emplace<EntropyStats>(playerEntity); // MVP Component

    int startX = 100, startY = 75;
    while (!gameMap.isFloor(startX, startY))
    {
        startX++;
        if (startX >= MAP_WIDTH)
        {
            startX = 0;
            startY++;
            if (startY >= MAP_HEIGHT)
                startY = 0;
        }
    }
    registry.emplace<Position>(playerEntity, startX, startY);
    spatialGrid[startY * MAP_WIDTH + startX] = playerEntity;

    // --- Enemy Initialization (Data-Driven) ---
    std::vector<EnemyDef> enemyArchetypes = DataLoader::loadEnemyDefs(ENEMY_DATA_PATH);
    if (enemyArchetypes.empty())
    {
        std::cerr << "WARNING: No enemy archetypes loaded! Using fallback." << std::endl;
        enemyArchetypes.push_back({"Fallback", 20, 5, 2, 255, 0, 0, 255});
    }

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> distX(0, MAP_WIDTH - 1);
    std::uniform_int_distribution<int> distY(0, MAP_HEIGHT - 1);
    std::uniform_int_distribution<int> distArchetype(0, enemyArchetypes.size() - 1);

    int numberOfEnemies = 60;
    for (int i = 0; i < numberOfEnemies; ++i)
    {
        int ex = distX(rng);
        int ey = distY(rng);

        while (!gameMap.isFloor(ex, ey) ||
               (std::abs(ex - startX) < 10 && std::abs(ey - startY) < 10) ||
               spatialGrid[ey * MAP_WIDTH + ex] != entt::null)
        {
            ex = distX(rng);
            ey = distY(rng);
        }

        const EnemyDef &def = enemyArchetypes[distArchetype(rng)];

        auto enemyEntity = registry.create();
        registry.emplace<Position>(enemyEntity, ex, ey);
        registry.emplace<Enemy>(enemyEntity);
        registry.emplace<Collider>(enemyEntity);
        registry.emplace<Health>(enemyEntity, def.hp, def.hp);
        registry.emplace<CombatStats>(enemyEntity, def.attack, def.defense);
        registry.emplace<RenderColor>(enemyEntity, def.r, def.g, def.b, def.a);

        spatialGrid[ey * MAP_WIDTH + ex] = enemyEntity;
    }

    // --- Item Initialization ---
    std::vector<ItemDef> itemArchetypes = DataLoader::loadItemDefs(ITEM_DATA_PATH);
    if (!itemArchetypes.empty())
    {
        std::uniform_int_distribution<int> distItemArch(0, itemArchetypes.size() - 1);

        for (int i = 0; i < 10; ++i)
        {
            int ix = distX(rng);
            int iy = distY(rng);

            while (!gameMap.isFloor(ix, iy))
            {
                ix = distX(rng);
                iy = distY(rng);
            }

            const ItemDef &def = itemArchetypes[distItemArch(rng)];

            auto itemEntity = registry.create();
            registry.emplace<Position>(itemEntity, ix, iy);
            registry.emplace<Item>(itemEntity);
            registry.emplace<ItemEffect>(itemEntity, def.effectType, def.magnitude);
            registry.emplace<RenderColor>(itemEntity, def.r, def.g, def.b, def.a);
        }
    }
}

void PlayState::processInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            game.quit();
        }

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

            if (event.key.key == SDLK_SPACE)
            {
                gameMap.generateCaves(45, 5);
                gameMap.processMap();

                int oldX = pos.x;
                int oldY = pos.y;
                while (!gameMap.isFloor(pos.x, pos.y))
                {
                    pos.x++;
                    if (pos.x >= MAP_WIDTH)
                    {
                        pos.x = 0;
                        pos.y++;
                    }
                    if (pos.y >= MAP_HEIGHT)
                        pos.y = 0;
                }
                updateSpatialGrid(playerEntity, oldX, oldY, pos.x, pos.y);
                playerActed = true;
            }

            // --- 'E' Key: Cryo Exchange ---
            if (event.key.key == SDLK_E)
            {
                bool success = false;
                dispatcher.trigger(SpellCastEvent{playerEntity, SpellCastEvent::SpellType::CRYO, &success});
                playerActed = success;
            }

            // --- 'F' Key: Fire Exchange ---
            if (event.key.key == SDLK_F)
            {
                bool success = false;
                dispatcher.trigger(SpellCastEvent{playerEntity, SpellCastEvent::SpellType::FIRE, &success});
                playerActed = success;
            }
            // --- 'U' For use item ---
            if (event.key.key == SDLK_U)
            {
                auto &inv = registry.get<Inventory>(playerEntity);
                if (!inv.items.empty())
                {
                    entt::entity itemToUse = inv.items[0];
                    dispatcher.trigger(ItemUseEvent{playerEntity, itemToUse});
                    playerActed = true;
                }
                else
                {
                    std::cout << "Nothing to use." << std::endl;
                }
            }
            // --- 'G' Key: Pick up item ---
            if (event.key.key == SDLK_G)
            {
                entt::entity pickedUp = entt::null;
                auto view = registry.view<Position, Item>();
                for (auto entity : view)
                {
                    auto &itemPos = view.get<Position>(entity);
                    if (itemPos.x == pos.x && itemPos.y == pos.y)
                    {
                        pickedUp = entity;
                        break;
                    }
                }
                if (pickedUp != entt::null)
                {
                    auto &inv = registry.get<Inventory>(playerEntity);
                    if (static_cast<int>(inv.items.size()) < inv.maxCapacity)
                    {
                        inv.items.push_back(pickedUp);
                        registry.remove<Position>(pickedUp);
                        std::cout << "Picked up item!" << std::endl;
                        playerActed = true;
                    }
                    else
                    {
                        std::cout << "Inventory is full!" << std::endl;
                    }
                }
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
        auto view = registry.view<Position, Enemy>();
        for (auto entity : view)
        {
            // Phantom entities do not pathfind or act
            if (registry.all_of<Phantom>(entity))
                continue;

            auto &enemyPos = view.get<Position>(entity);
            int distToPlayer = std::abs(enemyPos.x - pos.x) + std::abs(enemyPos.y - pos.y);

            if (distToPlayer == 1)
            {
                dispatcher.trigger(MeleeAttackEvent{entity, playerEntity});
            }
            else if (distToPlayer <= 15)
            {
                auto path = gameMap.findPath(enemyPos.x, enemyPos.y, pos.x, pos.y);

                if (!path.empty())
                {
                    int nextIndex = path[0];
                    int nextX = nextIndex % MAP_WIDTH;
                    int nextY = nextIndex / MAP_WIDTH;

                    if (getBlockingEntityAt(nextX, nextY) == entt::null && !(nextX == pos.x && nextY == pos.y))
                    {
                        updateSpatialGrid(entity, enemyPos.x, enemyPos.y, nextX, nextY);
                        enemyPos.x = nextX;
                        enemyPos.y = nextY;
                    }
                }
            }
        }

        if (registry.all_of<EntropyStats>(playerEntity))
        {
            auto &eStats = registry.get<EntropyStats>(playerEntity);
            if (eStats.entropy > 0)
            {
                eStats.entropy--;
            }
        }

        currentTurnState = TurnState::WAITING_FOR_PLAYER;
    }

    if (needsFOVUpdate)
    {
        int dynamicFOV = registry.get<EntropyStats>(playerEntity).fovRadius;
        gameMap.calculateFOV(pos.x, pos.y, dynamicFOV);

        int playerPixelX = pos.x * 20;
        int playerPixelY = pos.y * 20;

        cameraX = playerPixelX - (game.getWindowWidth() / 2);
        cameraY = playerPixelY - (game.getWindowHeight() / 2);

        if (cameraX < 0)
            cameraX = 0;
        if (cameraY < 0)
            cameraY = 0;
        if (cameraX > (MAP_WIDTH * 20) - game.getWindowWidth())
            cameraX = (MAP_WIDTH * 20) - game.getWindowWidth();
        if (cameraY > (MAP_HEIGHT * 20) - game.getWindowHeight())
            cameraY = (MAP_HEIGHT * 20) - game.getWindowHeight();

        needsFOVUpdate = false;
    }

    // --- Deferred Destruction Cleanup ---
    combatSystem->update();
    // --- Phantom Spawn Hook ---
    // --- ntropy Thresholds & Cascade Hook ---
    auto &eStats = registry.get<EntropyStats>(playerEntity);

    if (eStats.entropy >= 100)
    {
        std::cout << "\n--- ENTROPY CRITICAL: VOW REQUIRED! ---\n";
        std::cout << "(Placeholder for Phase 8: VowState Overlay)\n";
        eStats.entropy = 50; // Temporarily reset so it doesn't spam every frame
    }
    else if (eStats.entropy >= 86)
    {
        // Cascade Level: Reality fractures. Floor tiles randomly combust.
        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> distX(0, MAP_WIDTH - 1);
        std::uniform_int_distribution<int> distY(0, MAP_HEIGHT - 1);
        int cx = distX(rng);
        int cy = distY(rng);

        if (gameMap.isFloor(cx, cy) && gameMap.getTileState(cx, cy) == TileState::EMPTY)
        {
            gameMap.setTileState(cx, cy, TileState::FIRE);
            std::cout << "Reality degrades... a tile combusts into FIRE.\n";
        }
    }
    else if (eStats.entropy >= 61)
    {
        // Phantom Spawn Level
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
        {
            spatialGrid[oldIndex] = entt::null;
        }
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