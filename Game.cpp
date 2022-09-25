#include "Game.h"
#include <iostream>
#include <stdlib.h>
#include <math.h>

Game::Game(std::string name)
{
    // returns zero on success else non-zero
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("error initializing SDL: %s\n", SDL_GetError());
    }
    window = SDL_CreateWindow(name.c_str(),
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              WINDOW_W,
                              WINDOW_H,
                              SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);
    fps = 20;
    cellSize = 8;
    // Calculate cell color for each state
    // from DEAD to ALIEN inclusive
    colors = new uint32_t[ALIEN + 1];
    for (uint8_t i = 0; i < ALIEN + 1; ++i)
    {
        if (i == 0)
        {
            colors[i] = 0xFF; // R=G=B=0 A=0xFF
        }
        else if (i < ALIVE)
        {
            colors[i] = ((i * 16) & (0xFF)) << 24 |
                        ((i * 16) & (0xFF)) << 16 |
                        ((i * 16) & (0xFF)) << 8 |
                        0xFF; // Gray
        }
        else if (i < OLD)
        {
            colors[i] = 0x00C000FF; // Green
        }
        else if (i < ALIEN)
        {
            colors[i] = 0x0020C0FF; // Blue
        }
        else if (i == ALIEN)
        {
            colors[i] = 0xC00000FF; // Red
        }
    }
}

Game::~Game()
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

void Game::initialize()
{
    boardWidth = WINDOW_W / cellSize;
    boardHeight = WINDOW_H / cellSize;
    // Create two empty worlds
    world = new uint8_t[boardWidth * boardHeight];
    oldWorld = new uint8_t[boardWidth * boardHeight];
    // Initialize one world
    for (uint16_t y = 0; y < boardHeight; ++y)
    {
        for (uint16_t x = 0; x < boardWidth; ++x)
        {
            // Populate ~20% of cells
            setCell(world, x, y, rand() % 5 ? DEAD : ALIVE);
        }
    }
}

void Game::run()
{
    isRunning = true;
    isSimulating = false;
    SDL_Event event;
    uint16_t count = 0;

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type != SDL_QUIT && event.type != SDL_KEYDOWN && event.type != SDL_MOUSEBUTTONDOWN)
            {
                continue;
            }
            switch (event.type)
            {
            case SDL_QUIT:
                isRunning = false;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q)
                {
                    isRunning = false;
                }
                else if (event.key.keysym.sym == SDLK_SPACE)
                {
                    isSimulating = !isSimulating;
                }
                else if (event.key.keysym.sym == SDLK_DOWN)
                {
                    fps = std::max(fps - 1, 1);
                }
                else if (event.key.keysym.sym == SDLK_UP)
                {
                    fps = std::min(fps + 1, 100);
                }
                else if (event.key.keysym.sym == SDLK_a)
                {
                    alienCounter = 1023;
                }
                else if (event.key.keysym.sym == SDLK_c)
                {
                    // Clear all
                    memset(world, DEAD, boardWidth * boardHeight * sizeof(uint8_t));
                }
                else if (event.key.keysym.sym == SDLK_n)
                {
                    // Do one step
                    step();
                }
                else if ((event.key.keysym.sym == SDLK_PLUS || event.key.keysym.sym == SDLK_EQUALS) && cellSize < 128)
                {
                    cellSize *= 2;
                    initialize();
                }
                else if (event.key.keysym.sym == SDLK_MINUS && cellSize > 1)
                {
                    cellSize /= 2;
                    initialize();
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                setCell(world,
                        event.button.x / cellSize,
                        event.button.y / cellSize,
                        getCellBinary(world, event.button.x / cellSize, event.button.y / cellSize) ? DEAD : ALIVE);
            default:
                break;
            }
        }

        if (isSimulating)
        {
            step();
        }
        render();
        SDL_Delay(1000 / fps);
    }
}

uint8_t Game::getCellBinary(uint8_t *board, int16_t x, int16_t y)
{
    return getCell(board, x, y) < ALIVE ? 0 : 1;
}

uint8_t Game::getCell(uint8_t *board, int16_t x, int16_t y)
{
    x = (x + boardWidth) % boardWidth;
    y = (y + boardHeight) % boardHeight;
    return board[y * boardWidth + x];
}

void Game::setCell(uint8_t *board, int16_t x, int16_t y, uint8_t value)
{
    x = (x + boardWidth) % boardWidth;
    y = (y + boardHeight) % boardHeight;
    board[y * boardWidth + x] = value;
}

void Game::step()
{
    uint8_t neighbours;
    uint8_t oldCell;
    uint8_t *tmp = oldWorld;
    uint16_t alive = 0;
    oldWorld = world;
    world = tmp;
    for (uint16_t y = 0; y < boardHeight; ++y)
    {
        for (uint16_t x = 0; x < boardWidth; ++x)
        {
            oldCell = getCell(oldWorld, x, y);
            neighbours = getCellBinary(oldWorld, x - 1, y - 1) +
                         getCellBinary(oldWorld, x - 1, y) +
                         getCellBinary(oldWorld, x - 1, y + 1) +
                         getCellBinary(oldWorld, x, y - 1) +
                         getCellBinary(oldWorld, x, y + 1) +
                         getCellBinary(oldWorld, x + 1, y - 1) +
                         getCellBinary(oldWorld, x + 1, y) +
                         getCellBinary(oldWorld, x + 1, y + 1);

            if (neighbours == 3 && oldCell < ALIVE)
            {
                // Any dead cell with exactly three live
                // neighbours becomes a live cell
                setCell(world, x, y, ALIVE);
                alive++;
            }
            else if (neighbours < 2 || neighbours > 3 || oldCell < ALIVE)
            {
                // Any live cell with fewer than two live neighbours dies
                // Any live cell with more than three live neighbours dies
                // Any dead cell stays dead
                if (oldCell == DEAD)
                {
                    // Dead stays dead
                    setCell(world, x, y, DEAD);
                }
                else
                {
                    // Fading out cell fades out by 1
                    // In case it is an OLD cell, it has to
                    // start fading out from ALIVE value
                    setCell(world, x, y, std::min(ALIVE, (int)oldCell) - 1);
                }
            }
            else if (oldCell >= ALIVE)
            {
                // Any live cell with two or three live neighbours lives on
                setCell(world, x, y, std::min(OLD, oldCell + 1));
                alive++;
            }
        }
    }
    ++alienCounter;
    // Every 1024 - random alien injection
    if (alienCounter % 1024 == 0)
    {
        uint16_t aliens = (boardWidth * boardHeight / 30);
        for (uint16_t i = 0; i < aliens; i++)
        {
            setCell(world, rand() % boardWidth, rand() % boardHeight, ALIEN);
        }
    }
}

void Game::render()
{
    SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
    SDL_RenderClear(renderer);

    uint32_t color;
    SDL_Rect rect;
    rect.w = std::max(1, cellSize - 1);
    rect.h = std::max(1, cellSize - 1);
    for (uint16_t y = 0; y < boardHeight; ++y)
    {
        for (uint16_t x = 0; x < boardWidth; ++x)
        {
            color = colors[getCell(world, x, y)];
            SDL_SetRenderDrawColor(renderer,
                                   (color >> 24) & 0xFF,
                                   (color >> 16) & 0xFF,
                                   (color >> 8) & 0xFF,
                                   color & 0xFF);
            rect.x = cellSize * x;
            rect.y = cellSize * y;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    SDL_RenderPresent(renderer);
}
