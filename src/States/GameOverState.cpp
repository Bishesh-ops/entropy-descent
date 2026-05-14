#include "../../include/States/GameOverState.hpp"
#include <iostream>

GameOverState::GameOverState(Game &gameRef, int reachedFloor)
    : game(gameRef), floorDepth(reachedFloor) {
  fontTitle =
      std::make_unique<FontRenderer>("assets/fonts/JetBrainsMono.ttf", 48);
  fontSub =
      std::make_unique<FontRenderer>("assets/fonts/JetBrainsMono.ttf", 24);
}

void GameOverState::onEnter() {
  std::cout << "Entered GameOverState. Floor reached: " << floorDepth << "\n";
}

void GameOverState::processInput() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      game.quit();
    }
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_RETURN) {
      game.getStateMachine().popState();
      game.getStateMachine().popState();
    }
  }
}

void GameOverState::update(float dt) {}

void GameOverState::render() {
  SDL_Renderer *renderer = game.getRenderer();
  SDL_SetRenderDrawColor(renderer, 15, 5, 5, 255);
  SDL_RenderClear(renderer);

  if (fontTitle) {
    fontTitle->draw(renderer, "YOU DIED", game.getWindowWidth() * 0.35f,
                    game.getWindowHeight() * 0.3f, {255, 50, 50, 255});
  }
  if (fontSub) {
    std::string depthStr = "Floor Reached: " + std::to_string(floorDepth);
    fontSub->draw(renderer, depthStr, game.getWindowWidth() * 0.38f,
                  game.getWindowHeight() * 0.45f, {200, 200, 200, 255});
    fontSub->draw(renderer, "Press ENTER to return to Menu",
                  game.getWindowWidth() * 0.28f, game.getWindowHeight() * 0.65f,
                  {100, 100, 100, 255});
  }
}
