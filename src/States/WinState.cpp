#include "../../include/States/WinState.hpp"
#include "../../third_party/imgui/imgui.h"
#include "../../third_party/imgui/imgui_impl_sdl3.h"
#include <iostream>

WinState::WinState(Game &gameRef) : game(gameRef)
{

    victorySound = Mix_LoadWAV("assets/audio/victory.wav");
    if (!victorySound)
    {
        std::cerr << "Warning: Failed to load victory.wav! " << Mix_GetError() << "\n";
    }
}

WinState::~WinState()
{
    if (victorySound)
    {
        Mix_FreeChunk(victorySound);
    }
}

void WinState::onEnter()
{
    std::cout << "Entered WinState!\n";
    if (victorySound)
    {
        Mix_PlayChannel(-1, victorySound, 0);
    }
}

void WinState::processInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL3_ProcessEvent(&event);

        if (event.type == SDL_EVENT_QUIT)
        {
            game.quit();
        }
        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_RETURN)
            {
                game.getStateMachine().popState();
                game.getStateMachine().popState();
            }
        }
    }
}

void WinState::update(float dt)
{
    // Static screen, no update logic needed
}

void WinState::render()
{
    SDL_Renderer* renderer = game.getRenderer();
    
    SDL_SetRenderDrawColor(renderer, 10, 15, 20, 255);
    SDL_RenderClear(renderer);

    ImGui::SetNextWindowPos(ImVec2(game.getWindowWidth() * 0.5f, game.getWindowHeight() * 0.5f), 
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                            
    ImGui::Begin("Victory Screen", nullptr, 
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | 
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
                 
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "VICTORY ACHIEVED");
    ImGui::Separator();
    ImGui::Text("All hostiles have been purged.");
    ImGui::Text("Press ENTER to return to the Main Menu.");
    ImGui::End();
}