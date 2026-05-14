#include "../include/FontRenderer.hpp"

FontRenderer::FontRenderer(const std::string &path, int size) {
  if (!TTF_WasInit() && !TTF_Init()) {
    std::cerr << "TTF_Init Error:" << SDL_GetError() << "\n";
    return;
  }
  font = TTF_OpenFont(path.c_str(), size);
  if (!font) {
    std::cerr << "Failed to load font " << path << ": " << SDL_GetError()
              << "\n";
  }
}

FontRenderer::~FontRenderer() {
  if (font) {
    TTF_CloseFont(font);
  }
}

void FontRenderer::draw(SDL_Renderer *renderer, const std::string &text,
                        float x, float y, SDL_Color color) {
  if (!font || text.empty())
    return;

  // Create high-quality blended surface
  SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
  if (!surface)
    return;

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture) {
    SDL_FRect dstRect = {x, y, static_cast<float>(surface->w),
                         static_cast<float>(surface->h)};
    SDL_RenderTexture(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
  }
  SDL_DestroySurface(surface);
}
