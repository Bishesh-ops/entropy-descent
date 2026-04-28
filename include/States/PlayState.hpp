#pragma once
#include "State.hpp"
#include "../Game.hpp"
#include "../Map.hpp"
#include <entt/entt.hpp>

class PlayState : public State
{
public:
    PlayState(Game &gameRef);
    void processInput() override;
    void update() override;
    void render() override;

private:
    Game &game;

    Map gameMap;
    entt::registry registry;
    entt::entity playerEntity;

    int cameraX;
    int cameraY;

    entt::entity getBlockingEntityAt(int x, int y);
};