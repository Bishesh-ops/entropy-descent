#pragma once
#include <vector>
#include <random>
#include <SDL3/SDL.h>
#include <queue>
#include <algorithm>
#include <cmath>

class Map
{
private:
    std::vector<int> getRegion(int startX, int startY, std::vector<bool> &visited) const
    {
        std::vector<int> region;
        std::queue<int> queue;

        int startIndex = getIndex(startX, startY);
        queue.push(startIndex);
        visited[startIndex] = true;

        while (!queue.empty())
        {
            int current = queue.front();
            queue.pop();
            region.push_back(current);

            int cx = current % width;
            int cy = current / width;

            int dx[] = {0, 0, -1, 1};
            int dy[] = {-1, 1, 0, 0};

            for (int i = 0; i < 4; ++i)
            {
                int nx = cx + dx[i];
                int ny = cy + dy[i];

                if (nx >= 0 && nx < width && ny >= 0 && ny < height)
                {
                    int neighborIndex = getIndex(nx, ny);
                    if (grid[neighborIndex] == 0 && !visited[neighborIndex])
                    {
                        visited[neighborIndex] = true;
                        queue.push(neighborIndex);
                    }
                }
            }
        }
        return region;
    }
    int width, height;
    int cellSize;
    std::vector<int> grid;
    std::vector<bool> visible;
    std::vector<bool> explored;

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
        visible.resize(width * height, false);
        explored.resize(width * height, false);
    }
    void processMap()
    {
        std::vector<std::vector<int>> regions;
        std::vector<bool> visited(width * height, false);

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int index = getIndex(x, y);
                if (grid[index] == 0 && !visited[index])
                {
                    regions.push_back(getRegion(x, y, visited));
                }
            }
        }

        if (regions.empty())
            return;

        size_t largestRegionIndex = 0;
        size_t maxSize = 0;
        for (size_t i = 0; i < regions.size(); ++i)
        {
            if (regions[i].size() > maxSize)
            {
                maxSize = regions[i].size();
                largestRegionIndex = i;
            }
        }

        for (size_t i = 0; i < regions.size(); ++i)
        {
            if (i != largestRegionIndex)
            {
                for (int index : regions[i])
                {
                    grid[index] = 1;
                }
            }
        }
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

    void calculateFOV(int playerX, int playerY, int radius)
    {
        std::fill(visible.begin(), visible.end(), false);

        visible[getIndex(playerX, playerX)] = true;
        explored[getIndex(playerX, playerY)] = true;

        for (float angle = 0; angle < 360; angle += 0.5f)
        {
            float rad = angle * (M_PI / 180.0f);
            float dirX = std::cos(rad);
            float dirY = std::sin(rad);

            float rayX = playerX;
            float rayY = playerY;

            for (int r = 0; r < radius; ++r)
            {
                rayX += dirX;
                rayY += dirY;

                int gridX = static_cast<int>(std::round(rayX));
                int gridY = static_cast<int>(std::round(rayY));

                if (gridX < 0 || gridX > width || gridY < 0 || gridY > height)
                    break;
                int index = getIndex(gridX, gridY);

                visible[index] = true;
                explored[index] = true;

                if (grid[index] == 1)
                {
                    break;
                }
            }
        }
    }

    void render(SDL_Renderer *renderer, int cameraX, int cameraY) const
    {
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int index = getIndex(x, y);

                if (!explored[index])
                    continue;

                if (grid[index] == 1)
                {
                    if (visible[index])
                        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
                    else
                        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
                }
                else
                {

                    if (visible[index])
                        SDL_SetRenderDrawColor(renderer, 40, 40, 50, 255);
                    else
                        SDL_SetRenderDrawColor(renderer, 15, 15, 20, 255);
                }

                SDL_FRect rect = {
                    static_cast<float>((x * cellSize) - cameraX),
                    static_cast<float>((y * cellSize) - cameraY),
                    static_cast<float>(cellSize),
                    static_cast<float>(cellSize)};

                if (rect.x + rect.w > 0 && rect.x < 800 && rect.y + rect.h > 0 && rect.y < 600)
                {
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }
    }

    bool isFloor(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= width || y >= height)
            return false;
        return grid[getIndex(x, y)] == 0;
    }

    bool isVisible(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= width || y >= height)
            return false;
        return visible[getIndex(x, y)];
    }
};