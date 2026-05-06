#pragma once

#include "State.hpp"
#include "../Game.hpp"
#include <SDL3_mixer/SDL_mixer.h>

class WinState : public State
{
public:
    WinState(Game &gameRef);
    ~WinState() override;

    void processInput() override;
    void update(float dt) override;
    void render() override;
    void onEnter() override;

private:
    Game &game;
    Mix_Chunk *victorySound = nullptr;
};