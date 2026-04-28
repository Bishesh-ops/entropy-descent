#include "../../include/States/PlayState.hpp"
#include "../../include/Events.hpp"
#include "../../include/Components.hpp"
#include <iostream>
#include <random>
#include <cmath>

PlayState::PlayState(Game &gameRef)
    : game(gameRef), gameMap(200, 150, 20), cameraX(0), cameraY(0),
      currentTurnState(TurnState::WAITING_FOR_PLAYER), needsFOVUpdate(true)
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

    int startX = 100, startY = 75;
    while (!gameMap.isFloor(startX, startY))
    {
        startX++;
        if (startX >= 200)
        {
            startX = 0;
            startY++;
            if (startY >= 150)
                startY = 0;
        }
    }
    registry.emplace<Position>(playerEntity, startX, startY);
    spatialGrid[startY * 200 + startX] = playerEntity;

    // --- Enemy Initialization ---
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> distX(0, 199);
    std::uniform_int_distribution<int> distY(0, 149);

    int numberOfEnemies = 60;
    for (int i = 0; i < numberOfEnemies; ++i)
    {
        int ex = distX(rng);
        int ey = distY(rng);

        while (!gameMap.isFloor(ex, ey) || (std::abs(ex - startX) < 10 && std::abs(ey - startY) < 10))
        {
            ex = distX(rng);
            ey = distY(rng);
        }

        auto enemyEntity = registry.create();
        registry.emplace<Position>(enemyEntity, ex, ey);
        registry.emplace<Enemy>(enemyEntity);
        registry.emplace<Collider>(enemyEntity);
        registry.emplace<Health>(enemyEntity, 20, 20);
        registry.emplace<CombatStats>(enemyEntity, 5, 2);

        spatialGrid[ey * 200 + ex] = enemyEntity;
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

            // Debug: Regenerate Map
            if (event.key.key == SDLK_SPACE)
            {
                gameMap.generateCaves(45, 5);
                gameMap.processMap();

                int oldX = pos.x;
                int oldY = pos.y;
                while (!gameMap.isFloor(pos.x, pos.y))
                {
                    pos.x++;
                    if (pos.x >= 200)
                    {
                        pos.x = 0;
                        pos.y++;
                    }
                    if (pos.y >= 150)
                        pos.y = 0;
                }
                updateSpatialGrid(playerEntity, oldX, oldY, pos.x, pos.y);
                playerActed = true;
            }

            // Movement Handling
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
                    int nextX = nextIndex % 200;
                    int nextY = nextIndex / 200;

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
        if (cameraX > (200 * 20) - game.getWindowWidth())
            cameraX = (200 * 20) - game.getWindowWidth();
        if (cameraY > (150 * 20) - game.getWindowHeight())
            cameraY = (150 * 20) - game.getWindowHeight();

        needsFOVUpdate = false;
    }
}

void PlayState::render()
{
    SDL_Renderer *renderer = game.getRenderer();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    gameMap.render(renderer, cameraX, cameraY);

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
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
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

    SDL_RenderPresent(renderer);
}

void PlayState::updateSpatialGrid(entt::entity entity, int oldX, int oldY, int newX, int newY)
{
    int oldIndex = oldY * 200 + oldX;
    int newIndex = newY * 200 + newX;
    if (spatialGrid.count(oldIndex) && spatialGrid[oldIndex] == entity)
    {
        spatialGrid.erase(oldIndex);
    }
    spatialGrid[newIndex] = entity;
}

entt::entity PlayState::getBlockingEntityAt(int x, int y)
{
    int index = y * 200 + x;
    auto it = spatialGrid.find(index);
    if (it != spatialGrid.end())
    {
        return it->second;
    }
    return entt::null;
}

// --- Event Listeners ---

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

        if (registry.all_of<Position>(event.deadEntity))
        {
            auto &pos = registry.get<Position>(event.deadEntity);
            int index = pos.y * 200 + pos.x;
            if (spatialGrid.count(index) && spatialGrid[index] == event.deadEntity)
            {
                spatialGrid.erase(index);
            }
        }

        registry.destroy(event.deadEntity);
    }
}