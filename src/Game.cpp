#include "../include/Game.hpp"
#include "../include/States/MenuState.hpp"
#include <iostream>

#include "../third_party/imgui/imgui.h"
#include "../third_party/imgui/imgui_impl_sdl3.h"
#include "../third_party/imgui/imgui_impl_sdlrenderer3.h"

Game::Game() : isRunning(true), windowWidth(800), windowHeight(600) {
  // Initialize SDL core subsystems using modern boolean return checks
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
  }

  // Initialize modern SDL3_mixer library
  if (!MIX_Init()) {
    std::cerr << "MIX_Init Error: " << SDL_GetError() << "\n";
  } else {
    // Instantiate the playback device using default system parameters
    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (!mixer) {
      std::cerr << "MIX_CreateMixerDevice Error: " << SDL_GetError() << "\n";
    }
  }

  window =
      SDL_CreateWindow("Narrative Roguelike", windowWidth, windowHeight, 0);
  renderer = SDL_CreateRenderer(window, NULL);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();

  ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer3_Init(renderer);

  stateMachine.pushState(std::make_unique<MenuState>(*this));
}

Game::~Game() {
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  // Cleanly drop device allocations before dropping subsytems
  if (mixer) {
    MIX_DestroyMixer(mixer);
  }
  MIX_Quit();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void Game::run() {
  Uint64 lastTime = SDL_GetTicks();
  while (isRunning) {
    Uint64 currentTime = SDL_GetTicks();
    float dt = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    if (dt > 0.05f)
      dt = 0.05f;

    stateMachine.processStateChanges();

    if (!stateMachine.isEmpty()) {
      stateMachine.getActiveState().processInput();

      ImGui_ImplSDLRenderer3_NewFrame();
      ImGui_ImplSDL3_NewFrame();
      ImGui::NewFrame();

      stateMachine.getActiveState().update(dt);
      stateMachine.getActiveState().render();

      ImGui::Render();
      ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
      SDL_RenderPresent(renderer);
    } else {
      isRunning = false;
    }
  }
}
