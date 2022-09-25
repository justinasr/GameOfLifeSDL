#include "Game.h"
#include <iostream>
#include <stdlib.h>
#include <math.h>

Game::Game()
{
    fps = 20;
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

void Game::initialize(std::string name)
{
    // Create two empty worlds
    world = new uint8_t[BOARD_H * BOARD_W];
    oldWorld = new uint8_t[BOARD_H * BOARD_W];
    // Initialize one world
    for (uint16_t y = 0; y < BOARD_H; ++y)
    {
        for (uint16_t x = 0; x < BOARD_W; ++x)
        {
            // Populate ~20% of cells
            setCell(world, x, y, rand() % 5 ? DEAD : ALIVE);
        }
    }

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
                    count = 0;
                }
                else if (event.key.keysym.sym == SDLK_c)
                {
                    // Clear all
                    memset(world, DEAD, BOARD_H * BOARD_W * sizeof(uint8_t));
                }
                else if (event.key.keysym.sym == SDLK_n)
                {
                    // Do one step
                    step();
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                setCell(world,
                        event.button.x / CELL_SIZE,
                        event.button.y / CELL_SIZE,
                        getCellBinary(world, event.button.x / CELL_SIZE, event.button.y / CELL_SIZE) ? DEAD : ALIVE);
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
    x = (x + BOARD_W) % BOARD_W;
    y = (y + BOARD_H) % BOARD_H;
    return board[y * BOARD_W + x];
}

void Game::setCell(uint8_t *board, int16_t x, int16_t y, uint8_t value)
{
    x = (x + BOARD_W) % BOARD_W;
    y = (y + BOARD_H) % BOARD_H;
    board[y * BOARD_W + x] = value;
}

void Game::step()
{
    uint8_t neighbours;
    uint8_t oldCell;
    uint8_t *tmp = oldWorld;
    uint16_t alive = 0;
    oldWorld = world;
    world = tmp;
    for (uint16_t y = 0; y < BOARD_H; ++y)
    {
        for (uint16_t x = 0; x < BOARD_W; ++x)
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
    ++iteration;
    // Every 1024 - random alien injection
    if (iteration % 1024 == 0)
    {
        uint16_t aliens = (BOARD_W * BOARD_H / 30);
        for (uint16_t i = 0; i < aliens; i++)
        {
            setCell(world, rand() % BOARD_W, rand() % BOARD_H, ALIEN);
        }
    }
}

void Game::render()
{
    SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
    SDL_RenderClear(renderer);

    uint32_t color;
    SDL_Rect rect;
    rect.w = std::max(1, CELL_SIZE - 1);
    rect.h = std::max(1, CELL_SIZE - 1);
    for (uint16_t y = 0; y < BOARD_H; ++y)
    {
        for (uint16_t x = 0; x < BOARD_W; ++x)
        {
            color = colors[getCell(world, x, y)];
            SDL_SetRenderDrawColor(renderer,
                                   (color >> 24) & 0xFF,
                                   (color >> 16) & 0xFF,
                                   (color >> 8) & 0xFF,
                                   color & 0xFF);
            rect.x = CELL_SIZE * x;
            rect.y = CELL_SIZE * y;
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    SDL_RenderPresent(renderer);
}
