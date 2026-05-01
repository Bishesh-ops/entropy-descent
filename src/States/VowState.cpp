#include "../../include/States/VowState.hpp"
#include "../../include/DataLoader.hpp"
#include "../../include/Components.hpp"
#include <iostream>
#include <random>

VowState::VowState(Game &gameRef, entt::registry &reg, entt::entity player)
    : game(gameRef), registry(reg), playerEntity(player)
{
    allVows = DataLoader::loadVowDefs("../data/vows.json");

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, allVows.size() - 1);

    int idx1 = dist(rng);
    int idx2 = dist(rng);
    while (idx2 == idx1 && allVows.size() > 1)
    {
        idx2 = dist(rng);
    }

    choice1 = allVows[idx1];
    choice2 = allVows[idx2];
}

void VowState::onEnter()
{
    std::cout << "\n=========================================\n";
    std::cout << "         ENTROPY CRITICAL (100)          \n";
    std::cout << "    THE ABYSS DEMANDS A SACRIFICE.       \n";
    std::cout << "=========================================\n\n";

    std::cout << "PRESS [1] FOR THE LEFT PATH:\n";
    std::cout << ">> " << choice1.name << "\n";
    std::cout << ">> " << choice1.description << "\n\n";

    std::cout << "PRESS [2] FOR THE RIGHT PATH:\n";
    std::cout << ">> " << choice2.name << "\n";
    std::cout << ">> " << choice2.description << "\n\n";
    std::cout << "MAKE YOUR CHOICE...\n";
}

void VowState::applyVow(const VowDef &vow)
{
    auto &eStats = registry.get<EntropyStats>(playerEntity);

    if (vow.type == "fov")
    {
        eStats.fovRadius = std::max(2, eStats.fovRadius - 3);
        eStats.bonusAoE += 2;
    }
    else if (vow.type == "combat")
    {
        registry.get<CombatStats>(playerEntity).attack = 0;
        eStats.hasPassiveAura = true;
    }
    else if (vow.type == "healing")
    {
        eStats.healthLocked = true;
    }

    eStats.entropy = 50;
    std::cout << "\nYOU HAVE TAKEN THE " << vow.name << ". REALITY WARPS.\n";
    game.getStateMachine().popState();
}

void VowState::processInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            game.quit();
        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_1)
                applyVow(choice1);
            if (event.key.key == SDLK_2)
                applyVow(choice2);
        }
    }
}

void VowState::update(float dt) {}

void VowState::render()
{
    SDL_Renderer *renderer = game.getRenderer();

    SDL_SetRenderDrawColor(renderer, 10, 0, 15, 230);
    SDL_RenderClear(renderer);

    int w = game.getWindowWidth();
    int h = game.getWindowHeight();

    SDL_FRect rect1 = {w * 0.1f, h * 0.2f, w * 0.35f, h * 0.6f};
    SDL_SetRenderDrawColor(renderer, 100, 20, 30, 255);
    SDL_RenderFillRect(renderer, &rect1);
    SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
    SDL_RenderRect(renderer, &rect1);

    SDL_FRect rect2 = {w * 0.55f, h * 0.2f, w * 0.35f, h * 0.6f};
    SDL_SetRenderDrawColor(renderer, 20, 30, 100, 255);
    SDL_RenderFillRect(renderer, &rect2);
    SDL_SetRenderDrawColor(renderer, 50, 100, 255, 255);
    SDL_RenderRect(renderer, &rect2);

    SDL_RenderPresent(renderer);
}