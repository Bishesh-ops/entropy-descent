#include "../../include/States/PlayState.hpp"
#include "../../include/Components.hpp"
#include <iostream>
#include <random>
#include <cmath>

PlayState::PlayState(Game &gameRef)
    : game(gameRef), gameMap(200, 150, 20), cameraX(0), cameraY(0),
      currentTurnState(TurnState::WAITING_FOR_PLAYER), needsFOVUpdate(true)
{

    std::cout << "Generating massive world..." << std::endl;
    gameMap.generateCaves(45, 5);
    gameMap.processMap();

    playerEntity = registry.create();
    registry.emplace<Player>(playerEntity);
    registry.emplace<Collider>(playerEntity);
    registry.emplace<Health>(playerEntity, 100, 100);   // 100 HP
    registry.emplace<CombatStats>(playerEntity, 10, 5); // 10 Atk 5 Def
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
        registry.emplace<Health>(enemyEntity, 20, 20);    // 20 HP
        registry.emplace<CombatStats>(enemyEntity, 5, 2); // 5 Atk 2 def
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
            // Don't process movement/actions if enemies are currently taking their turn
            if (currentTurnState != TurnState::WAITING_FOR_PLAYER)
                continue;

            bool playerActed = false; // Flag to check if we actually spent a turn

            auto &pos = registry.get<Position>(playerEntity);
            int nextX = pos.x;
            int nextY = pos.y;

            if (event.key.key == SDLK_SPACE)
            {
                gameMap.generateCaves(45, 5);
                gameMap.processMap();
                while (!gameMap.isFloor(pos.x, pos.y))
                {
                    pos.x++;
                    if (pos.x >= 200)
                    {
                        pos.x = 0;
                        pos.y++;
                    }
                    if (pos.y >= 150)
                    {
                        pos.y = 0;
                    }
                }
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
                if (!gameMap.isFloor(nextX, nextY))
                {
                    // Bumped into a static wall
                    std::cout << "Bumped into a wall!" << std::endl;
                    // Bumping a wall shouldn't spend a turn, so playerActed remains false
                }
                else
                {
                    // Tile is floor, check for dynamic entities
                    entt::entity blocker = getBlockingEntityAt(nextX, nextY);

                    if (blocker != entt::null)
                    {
                        // We bumped into an entity. We can check what it is using the registry!
                        if (registry.all_of<Enemy>(blocker))
                        {
                            auto &pStats = registry.get<CombatStats>(playerEntity);
                            auto &eHealth = registry.get<Health>(blocker);
                            auto &eStats = registry.get<CombatStats>(blocker);

                            int damage = std::max(1, pStats.attack - eStats.defense);
                            eHealth.current -= damage;
                            std::cout << "You hit enemy for " << damage << " dmg! (HP: " << eHealth.current << "/" << eHealth.max << ")" << std::endl;
                            // Death Check
                            if (eHealth.current <= 0)
                            {
                                std::cout << "Enemy shattered into entropy!" << std::endl;
                                registry.destroy(blocker); // Removes entity and all components from memory!
                            }
                            playerActed = true; // Attacking spends a turn
                        }
                    }
                    else
                    {
                        // Tile is walkable and clear
                        pos.x = nextX;
                        pos.y = nextY;
                        playerActed = true; // Moving spends a turn
                    }
                }
            }

            // If the player successfully moved, attacked, or used a skill, hand the turn to the enemies
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
                auto &eStats = registry.get<CombatStats>(entity);
                auto &pHealth = registry.get<Health>(playerEntity);
                auto &pStats = registry.get<CombatStats>(playerEntity);

                int damage = std::max(1, eStats.attack - pStats.defense);
                pHealth.current -= damage;

                std::cout << "Enemy hits you for " << damage << " dmg! (Your HP: " << pHealth.current << ")" << std::endl;

                if (pHealth.current <= 0)
                {
                    std::cout << "YOU HAVE DESCENDED. GAME OVER." << std::endl;
                    // For now, just quit. Later we can trigger a Game Over State.
                    game.quit();
                }
            }

            else if (distToPlayer <= 15)
            {
                auto path = gameMap.findPath(enemyPos.x, enemyPos.y, pos.x, pos.y);

                if (!path.empty())
                {
                    int nextIndex = path[0];
                    int nextX = nextIndex % 200;
                    int nextY = nextIndex / 200;

                    if (getBlockingEntityAt(nextX, nextY) == entt::null &&
                        !(nextX == pos.x && nextY == pos.y))
                    {
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
        {
            continue; // Skip the rest of the loop, don't draw this entity!
        }

        if (registry.all_of<Player>(entity))
        {
            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); // Cyan Player
        }
        else if (registry.all_of<Enemy>(entity))
        {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red Enemy
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow fallback
        }

        SDL_FRect rect = {
            static_cast<float>((pos.x * 20) - cameraX),
            static_cast<float>((pos.y * 20) - cameraY),
            20.0f, 20.0f};
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_RenderPresent(renderer);
}

entt::entity PlayState::getBlockingEntityAt(int x, int y)
{
    auto view = registry.view<Position, Collider>();
    for (auto entity : view)
    {
        auto &pos = view.get<Position>(entity);
        if (pos.x == x && pos.y == y)
        {
            return entity;
        }
    }
    return entt::null; // Return a null entity if the tile is clear
}
