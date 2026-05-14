#pragma once
#include "../FontRenderer.hpp"
#include "../Game.hpp"
#include "State.hpp"

class MenuState : public State {
public:
  MenuState(Game &gameRef);
  void processInput() override;
  void update(float dt) override;
  void render() override;

private:
  Game &game;
  std::unique_ptr<FontRenderer> font;
};
