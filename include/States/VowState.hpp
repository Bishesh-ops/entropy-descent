#pragma once
#include "../Definitions.hpp"
#include "../FontRenderer.hpp"
#include "../Game.hpp"
#include "State.hpp"
#include <entt/entt.hpp>
#include <vector>

class VowState : public State {
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
  std::vector<VowDef> offeredChoices;

  std::unique_ptr<FontRenderer> fontRenderer;

  void applyVow(const VowDef &vow);
};
