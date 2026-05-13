#pragma once
#include "StateMachine.hpp"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

class Game {
public:
  Game();
  ~Game();
  void run();
  void quit() { isRunning = false; }

  SDL_Renderer *getRenderer() const { return renderer; }
  StateMachine &getStateMachine() { return stateMachine; }
  int getWindowWidth() const { return windowWidth; }
  int getWindowHeight() const { return windowHeight; }

  // Expose the core mixer device for downstream asset loading
  MIX_Mixer *getMixer() const { return mixer; }

private:
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;
  MIX_Mixer *mixer = nullptr;
  bool isRunning;

  StateMachine stateMachine;

  int windowWidth;
  int windowHeight;
};
