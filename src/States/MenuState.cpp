#include "../../include/States/MenuState.hpp"
#include "../../include/States/PlayState.hpp"
#include <iostream>

MenuState::MenuState(Game &gameRef) : game(gameRef) {
  std::cout << "Entered Menu State! Press ENTER to play." << std::endl;
  font = std::make_unique<FontRenderer>("assets/fonts/JetBrainsMono.ttf", 24);
}

void MenuState::processInput() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      game.quit();
    }
    if (event.type == SDL_EVENT_KEY_DOWN) {
      if (event.key.key == SDLK_RETURN) {
        game.getStateMachine().pushState(std::make_unique<PlayState>(game),
                                         false);
      }
    }
  }
}

void MenuState::update(float dt) {
  // no update logic for the menu state yet
}

void MenuState::render() {
  SDL_Renderer *renderer = game.getRenderer();
  SDL_SetRenderDrawColor(renderer, 10, 15, 20, 255);
  SDL_RenderClear(renderer);

  if (font) {
    font->draw(renderer, "ENTROPY DESCENT", 280, 100, {255, 100, 100, 255});
    font->draw(renderer, "WASD        - Move", 250, 220);
    font->draw(renderer, "Left Click  - Melee Attack", 250, 260);
    font->draw(renderer, "E / F       - Cast Cryo / Fire", 250, 300);
    font->draw(renderer, "G / U       - Pick up / Use Item", 250, 340);
    font->draw(renderer, ".           - Descend Stairs", 250, 380);

    font->draw(renderer, "PRESS ENTER TO BEGIN", 260, 500,
               {150, 255, 150, 255});
  }
}
