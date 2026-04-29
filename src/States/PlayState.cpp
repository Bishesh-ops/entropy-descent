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

PlayState::PlayState(Game &gameRef)
    : game(gameRef), gameMap(MAP_WIDTH, MAP_HEIGHT, 20), cameraX(0), cameraY(0),
      currentTurnState(TurnState::WAITING_FOR_PLAYER), needsFOVUpdate(true),
      spatialGrid(MAP_WIDTH * MAP_HEIGHT, static_cast<entt::entity>(entt::null))
{
    // --- Event Bindings ---
    dispatcher.sink<MeleeAttackEvent>().connect<&PlayState::onMeleeAttack>(this);
    dispatcher.sink<EntityDeathEvent>().connect<&PlayState::onEntityDeath>(this);

    // --- World Generation ---
    std::cout << "Generating massive world..." << std::endl;
    gameMap.generateCaves(45, 5);
    gameMap.processMap();

    // --- Player Initialization ---
    playerEntity = registry.create();
    registry.emplace<Player>(playerEntity);
    registry.emplace<Collider>(playerEntity);
    registry.emplace<Health>(playerEntity, 100, 100);
    registry.emplace<CombatStats>(playerEntity, 10, 5);
    registry.emplace<Inventory>(playerEntity); // Initialize player inventory

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
                        registry.remove<Position>(pickedUp); // Safe removal!

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
        currentTurnState = TurnState::WAITING_FOR_PLAYER;
    }

    if (needsFOVUpdate)
    {
        gameMap.calculateFOV(pos.x, pos.y, 15);

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
    for (auto deadEntity : pendingDestroy)
    {
        if (registry.all_of<Position>(deadEntity))
        {
            auto &deadPos = registry.get<Position>(deadEntity);
            int index = deadPos.y * MAP_WIDTH + deadPos.x;

            if (index >= 0 && index < (MAP_WIDTH * MAP_HEIGHT) && spatialGrid[index] == deadEntity)
            {
                spatialGrid[index] = entt::null;
            }
        }
        registry.destroy(deadEntity);
    }
    pendingDestroy.clear();
}

void PlayState::render()
{
    SDL_Renderer *renderer = game.getRenderer();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    gameMap.render(renderer, cameraX, cameraY, game.getWindowWidth(), game.getWindowHeight());

    auto view = registry.view<Position>();
    for (auto entity : view)
    {
        auto &pos = view.get<Position>(entity);

        if (registry.all_of<Enemy>(entity) && !gameMap.isVisible(pos.x, pos.y))
            continue;

        if (registry.all_of<Player>(entity))
        {
            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        }
        else if (registry.all_of<Enemy>(entity))
        {
            if (registry.all_of<RenderColor>(entity))
            {
                auto &color = registry.get<RenderColor>(entity);
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            }
        }
        else if (registry.all_of<Item>(entity))
        {
            if (registry.all_of<RenderColor>(entity))
            {
                auto &color = registry.get<RenderColor>(entity);
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            }
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        }

        SDL_FRect rect = {
            static_cast<float>((pos.x * 20) - cameraX),
            static_cast<float>((pos.y * 20) - cameraY),
            20.0f, 20.0f};
        SDL_RenderFillRect(renderer, &rect);
    }

    uiRenderer.render(renderer, registry, playerEntity);
    SDL_RenderPresent(renderer);
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

void PlayState::onMeleeAttack(const MeleeAttackEvent &event)
{
    auto &aStats = registry.get<CombatStats>(event.attacker);
    auto &tHealth = registry.get<Health>(event.target);
    auto &tStats = registry.get<CombatStats>(event.target);

    int damage = std::max(1, aStats.attack - tStats.defense);
    tHealth.current -= damage;

    std::cout << "Entity " << static_cast<uint32_t>(event.attacker)
              << " hit Entity " << static_cast<uint32_t>(event.target)
              << " for " << damage << " dmg! (HP: " << tHealth.current << ")" << std::endl;

    if (tHealth.current <= 0)
    {
        dispatcher.trigger(EntityDeathEvent{event.target});
    }
}

void PlayState::onEntityDeath(const EntityDeathEvent &event)
{
    if (registry.all_of<Player>(event.deadEntity))
    {
        std::cout << "YOU HAVE DESCENDED. GAME OVER." << std::endl;
        game.quit();
    }
    else
    {
        std::cout << "Enemy shattered into entropy!" << std::endl;
        pendingDestroy.push_back(event.deadEntity); // Queue destruction for end of frame
    }
}