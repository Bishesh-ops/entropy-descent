#include <SDL3/SDL.h>
#include <iostream>

int main(int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Failed to initialize SDL:" << SDL_GetError() << "\n";
    }
    return 0;
}