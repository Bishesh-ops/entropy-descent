#include "../../include/States/MenuState.hpp"
#include "../../include/States/PlayState.hpp"
#include <iostream>

MenuState::MenuState(Game &gameRef) : game(gameRef)
{
    std::cout << "Entered Menu State! Press ENTER to play." << std::endl;
}

void MenuState::processInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            game.quit();
        }
        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_RETURN)
            {
                game.getStateMachine().pushState(std::make_unique<PlayState>(game), false);
            }
        }
    }
}

void MenuState::update()
{
    // no update logic for the menu state yet
}

void MenuState::render()
{
    SDL_SetRenderDrawColor(game.getRenderer(), 10, 20, 40, 255);
    SDL_RenderClear(game.getRenderer());
    SDL_RenderPresent(game.getRenderer());
}