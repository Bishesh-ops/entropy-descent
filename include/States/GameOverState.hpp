#pragma once
#include "../FontRenderer.hpp"
#include "../Game.hpp"
#include "State.hpp"
#include <memory>

class GameOverState : public State {
public:
  GameOverState(Game &gameRef, int reachedFloor);
  void processInput() override;
  void update(float dt) override;
  void render() override;
  void onEnter() override;

private:
  Game &game;
  int floorDepth;
  std::unique_ptr<FontRenderer> fontTitle;
  std::unique_ptr<FontRenderer> fontSub;
};
