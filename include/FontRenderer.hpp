#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <iostream>
#include <string>

class FontRenderer {
public:
  FontRenderer(const std::string &path, int size);
  ~FontRenderer();
  void draw(SDL_Renderer *renderer, const std::string &text, float x, float y,
            SDL_Color color = {255, 255, 255, 255});

private:
  TTF_Font *font = nullptr;
};
