#pragma once
#include <vector>
#include <random>
#include <SDL3/SDL.h>
#include <queue>
#include <algorithm>
#include <cmath>

enum class TileState
{
    EMPTY,
    WATER,
    FIRE,
    FROZEN,
    SCORCHED,
    FLOOR
};

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
    std::vector<TileState> tileStates;

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
    std::vector<int> findPath(int startX, int startY, int goalX, int goalY) const
    {
        std::vector<int> path;

        if (!isFloor(goalX, goalY))
            return path;

        int startIndex = getIndex(startX, startY);
        int goalIndex = getIndex(goalX, goalY);

        auto heuristic = [](int x1, int y1, int x2, int y2)
        {
            int dx = std::abs(x1 - x2);
            int dy = std::abs(y1 - y2);
            return (dx + dy) * 10 + std::abs(dx - dy);
        };

        std::vector<int> gScore(width * height, 1e9);
        std::vector<int> cameFrom(width * height, -1);

        gScore[startIndex] = 0;

        using P = std::pair<int, int>;
        std::priority_queue<P, std::vector<P>, std::greater<P>> openSet;
        openSet.push({heuristic(startX, startY, goalX, goalY), startIndex});

        while (!openSet.empty())
        {
            int current = openSet.top().second;
            openSet.pop();

            if (current == goalIndex)
            {
                int curr = goalIndex;
                while (curr != startIndex)
                {
                    path.push_back(curr);
                    curr = cameFrom[curr];
                }
                std::reverse(path.begin(), path.end());
                return path;
            }

            int cx = current % width;
            int cy = current / width;

            int dx[] = {0, 0, -1, 1};
            int dy[] = {-1, 1, 0, 0};

            for (int i = 0; i < 4; ++i)
            {
                int nx = cx + dx[i];
                int ny = cy + dy[i];

                if (isFloor(nx, ny))
                {
                    int neighborIndex = getIndex(nx, ny);
                    int tentative_gScore = gScore[current] + 10;

                    if (tentative_gScore < gScore[neighborIndex])
                    {
                        cameFrom[neighborIndex] = current;
                        gScore[neighborIndex] = tentative_gScore;

                        int fScore = tentative_gScore + heuristic(nx, ny, goalX, goalY);
                        openSet.push({fScore, neighborIndex});
                    }
                }
            }
        }
        return path;
    }

    Map(int w, int h, int cSize) : width(w), height(h), cellSize(cSize)
    {
        grid.resize(width * height, 0);
        visible.resize(width * height, false);
        explored.resize(width * height, false);
        tileStates.resize(width * height, TileState::EMPTY);
    }

    TileState getTileState(int x, int y) const
    {
        if (x < 0 || y < 0 || x >= width || y >= height)
            return TileState::EMPTY;
        return tileStates[getIndex(x, y)];
    }

    void setTileState(int x, int y, TileState state)
    {
        if (x >= 0 && y >= 0 && x < width && y < height)
        {
            tileStates[getIndex(x, y)] = state;
        }
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
        std::fill(tileStates.begin(), tileStates.end(), TileState::EMPTY);
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

        visible[getIndex(playerX, playerY)] = true;
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

                if (gridX < 0 || gridX >= width || gridY < 0 || gridY >= height)
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

    void render(SDL_Renderer *renderer, int cameraX, int cameraY, int windowWidth, int windowHeight) const
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
                    {
                        switch (tileStates[index])
                        {
                        case TileState::WATER:
                            SDL_SetRenderDrawColor(renderer, 50, 100, 200, 255);
                            break;
                        case TileState::FIRE:
                            SDL_SetRenderDrawColor(renderer, 200, 80, 20, 255);
                            break;
                        case TileState::FROZEN:
                            SDL_SetRenderDrawColor(renderer, 150, 200, 255, 255);
                            break;
                        case TileState::SCORCHED:
                            SDL_SetRenderDrawColor(renderer, 60, 30, 20, 255);
                            break;
                        case TileState::EMPTY:
                        default:
                            SDL_SetRenderDrawColor(renderer, 40, 40, 50, 255);
                            break;
                        }
                    }
                    else
                    {
                        switch (tileStates[index])
                        {
                        case TileState::WATER:
                            SDL_SetRenderDrawColor(renderer, 25, 50, 100, 255);
                            break;
                        case TileState::FIRE:
                            SDL_SetRenderDrawColor(renderer, 100, 40, 10, 255);
                            break;
                        case TileState::FROZEN:
                            SDL_SetRenderDrawColor(renderer, 75, 100, 125, 255);
                            break;
                        case TileState::SCORCHED:
                            SDL_SetRenderDrawColor(renderer, 30, 15, 10, 255);
                            break;
                        case TileState::EMPTY:
                        default:
                            SDL_SetRenderDrawColor(renderer, 15, 15, 20, 255);
                            break;
                        }
                    }
                }

                SDL_FRect rect = {
                    static_cast<float>((x * cellSize) - cameraX),
                    static_cast<float>((y * cellSize) - cameraY),
                    static_cast<float>(cellSize),
                    static_cast<float>(cellSize)};

                // Dynamic culling using passed parameters
                if (rect.x + rect.w > 0 && rect.x < windowWidth && rect.y + rect.h > 0 && rect.y < windowHeight)
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