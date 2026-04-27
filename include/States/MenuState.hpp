#pragma once
#include "State.hpp"
#include "../Game.hpp"

class MenuState : public State
{
public:
    MenuState(Game &gameRef);
    void processInput() override;
    void update() override;
    void render() override;

private:
    Game &game;
};