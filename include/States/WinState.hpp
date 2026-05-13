#pragma once
#include "../Game.hpp"
#include "State.hpp"
#include <SDL3_mixer/SDL_mixer.h>

class WinState : public State {
public:
  WinState(Game &gameRef);
  ~WinState() override;

  void processInput() override;
  void update(float dt) override;
  void render() override;
  void onEnter() override;

private:
  Game &game;
  MIX_Audio *victoryAudio = nullptr;
};
