#pragma once
#include "State.hpp"
#include "../Game.hpp"
#include "../Definitions.hpp"
#include <entt/entt.hpp>
#include <vector>

class VowState : public State
{
public:
    VowState(Game &gameRef, entt::registry &reg, entt::entity player);
    void processInput() override;
    void update(float dt) override;
    void render() override;
    void onEnter() override;

private:
    Game &game;
    entt::registry &registry;
    entt::entity playerEntity;

    std::vector<VowDef> allVows;
    VowDef choice1;
    VowDef choice2;

    void applyVow(const VowDef &vow);
};