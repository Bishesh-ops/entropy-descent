#pragma once
#include <vector>
#include <random>
#include <SDL3/SDL.h>

class Map
{
private:
    int width, height;
    int cellSize;
    std::vector<int> grid;

    int getIndex(int x, int y) const
    {
        return y * width + x;
    }

    int countAliveNeighbours(int x, int y) const
    {
        int count = 0;
        for (int i = -1; i <= 1; ++i)
        {
            for (int j = -1; j <= 1; ++j)
            {
                int neighborX = x + i;
                int neighborY = y + j;
                if (i == 0 && j == 0)
                    continue;
                if (neighborX < 0 || neighborY < 0 || neighborX >= width || neighborY >= height)
                {
                    count++;
                }
                else if (grid[getIndex(neighborX, neighborY)] == 1)
                {
                    count++;
                }
            }
        }
        return count;
    }

public:
    Map(int w, int h, int cSize) : width(w), height(h), cellSize(cSize)
    {
        grid.resize(width * height, 0);
    }
    void generateCaves(int fillProbability, int iterations)
    {

        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, 100);

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                if (x == 0 || y == 0 || x == width - 1 || y == height - 1)
                {
                    grid[getIndex(x, y)] = 1;
                }
                else
                {
                    grid[getIndex(x, y)] = (dist(rng) < fillProbability) ? 1 : 0;
                }
            }
        }
        for (int i = 0; i < iterations; ++i)
        {
            std::vector<int> newGrid = grid;
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    int neighborWalls = countAliveNeighbours(x, y);

                    if (grid[getIndex(x, y)] == 1)
                    {
                        newGrid[getIndex(x, y)] = (neighborWalls >= 4) ? 1 : 0;
                    }
                    else
                    {
                        newGrid[getIndex(x, y)] = (neighborWalls >= 5) ? 1 : 0;
                    }
                }
            }
            grid = newGrid;
        }
    }

    void render(SDL_Renderer *renderer) const
    {
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                if (grid[getIndex(x, y)] == 1)
                {
                    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
                }
                else
                {

                    SDL_SetRenderDrawColor(renderer, 20, 20, 25, 255);
                }

                SDL_FRect rect = {
                    static_cast<float>(x * cellSize),
                    static_cast<float>(y * cellSize),
                    static_cast<float>(cellSize),
                    static_cast<float>(cellSize)};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    bool isFloor(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= width || y >= height)
            return false;
        return grid[getIndex(x, y)] == 0;
    }
};