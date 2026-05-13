#include "../../include/States/VowState.hpp"
#include "../../include/Components.hpp"
#include "../../include/DataLoader.hpp"
#include <algorithm>
#include <iostream>
#include <random>

VowState::VowState(Game &gameRef, entt::registry &reg, entt::entity player)
    : game(gameRef), registry(reg), playerEntity(player) {
  allVows = DataLoader::loadVowDefs("../data/vows.json");

  offeredChoices = allVows;
  std::mt19937 rng(std::random_device{}());
  std::shuffle(offeredChoices.begin(), offeredChoices.end(), rng);

  if (offeredChoices.size() > 3) {
    offeredChoices.resize(3);
  }
}

void VowState::onEnter() {
  std::cout << "\n=========================================\n";
  std::cout << "         ENTROPY CRITICAL (100)          \n";
  std::cout << "    THE ABYSS DEMANDS A SACRIFICE.       \n";
  std::cout << "=========================================\n\n";

  for (size_t i = 0; i < offeredChoices.size(); ++i) {
    std::cout << "PRESS [" << (i + 1) << "] FOR PATH:\n";
    std::cout << ">> " << offeredChoices[i].name << "\n";
    std::cout << ">> " << offeredChoices[i].description << "\n\n";
  }
  std::cout << "MAKE YOUR CHOICE...\n";
}

void VowState::applyVow(const VowDef &vow) {
  auto &eStats = registry.get<EntropyStats>(playerEntity);

  if (vow.type == "fov") {
    eStats.fovRadius = std::max(1, eStats.fovRadius - 3);
    eStats.bonusAoE += 2;
  } else if (vow.type == "combat") {
    if (registry.all_of<CombatStats>(playerEntity)) {
      registry.get<CombatStats>(playerEntity).attack = 1;
    }
    eStats.hasPassiveAura = true;
  } else if (vow.type == "healing") {
    eStats.healthLocked = true;
  }

  eStats.entropy = 50;
  std::cout << "\nYOU HAVE TAKEN THE " << vow.name << ". REALITY WARPS.\n";
  game.getStateMachine().popState();
}

void VowState::processInput() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT)
      game.quit();

    if (event.type == SDL_EVENT_KEY_DOWN) {
      if (!registry.valid(playerEntity) ||
          !registry.all_of<EntropyStats>(playerEntity)) {
        game.getStateMachine().popState();
        return;
      }

      int choiceIdx = -1;
      if (event.key.key == SDLK_1)
        choiceIdx = 0;
      else if (event.key.key == SDLK_2)
        choiceIdx = 1;
      else if (event.key.key == SDLK_3)
        choiceIdx = 2;

      if (choiceIdx >= 0 &&
          choiceIdx < static_cast<int>(offeredChoices.size())) {
        applyVow(offeredChoices[choiceIdx]);
      }
    }
  }
}

void VowState::update(float dt) {}

void VowState::render() {
  SDL_Renderer *renderer = game.getRenderer();

  SDL_SetRenderDrawColor(renderer, 10, 0, 15, 230);
  SDL_RenderClear(renderer);

  int w = game.getWindowWidth();
  int h = game.getWindowHeight();

  if (!offeredChoices.empty()) {
    float colWidth = static_cast<float>(w) / offeredChoices.size();
    float rectW = colWidth * 0.8f;
    float gap = colWidth * 0.1f;

    for (size_t i = 0; i < offeredChoices.size(); ++i) {
      SDL_FRect rect = {i * colWidth + gap, h * 0.2f, rectW, h * 0.6f};

      if (i == 0) {
        SDL_SetRenderDrawColor(renderer, 100, 20, 30, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
      } else if (i == 1) {
        SDL_SetRenderDrawColor(renderer, 20, 30, 100, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 50, 100, 255, 255);
      } else {
        SDL_SetRenderDrawColor(renderer, 60, 20, 80, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 180, 50, 255, 255);
      }
      SDL_RenderRect(renderer, &rect);
    }
  }

  SDL_RenderPresent(renderer);
}
