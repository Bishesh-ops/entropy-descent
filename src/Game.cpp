#include "../include/Game.hpp"
#include "../include/States/MenuState.hpp"
#include <iostream>
#include <SDL3_mixer/SDL_mixer.h>

#include "../third_party/imgui/imgui.h"
#include "../third_party/imgui/imgui_impl_sdl3.h"
#include "../third_party/imgui/imgui_impl_sdlrenderer3.h"
//Testing
Game::Game() : isRunning(true), windowWidth(800), windowHeight(600)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
    }

    if (Mix_OpenAudio(0, nullptr) < 0) {
        std::cerr << "SDL_mixer Error: " << Mix_GetError() << "\n";
    }

    window = SDL_CreateWindow("Narrative Roguelike", windowWidth, windowHeight, 0);
    renderer = SDL_CreateRenderer(window, NULL);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // Boot up the game by throwing the Menu on the stack
    stateMachine.pushState(std::make_unique<MenuState>(*this));

    
}

Game::~Game()
{
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext(); 
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::run()
{
    Uint64 lastTime = SDL_GetTicks();
    while (isRunning)
    {
        Uint64 currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        if (dt > 0.05f)
            dt = 0.05f;

        stateMachine.processStateChanges();
        
        if (!stateMachine.isEmpty())
        {
            stateMachine.getActiveState().processInput();

            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            stateMachine.getActiveState().update(dt);
            stateMachine.getActiveState().render();

            ImGui::Render();
            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);            
            SDL_RenderPresent(renderer);
        }
        else
        {
            isRunning = false;
        }
    }
}
