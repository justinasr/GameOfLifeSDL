#include "Game.h"
#include <iostream>
#include <stdlib.h>
#include <math.h>

Game::Game()
{
    fps = 20;
}

Game::~Game()
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

void Game::initialize(std::string name)
{
    world = new uint8_t[BOARD_H * BOARD_W];
    oldWorld = new uint8_t[BOARD_H * BOARD_W];
    for (uint16_t y = 0; y < BOARD_H; ++y)
    {
        for (uint16_t x = 0; x < BOARD_W; ++x)
        {
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
                break;
            case SDL_MOUSEBUTTONDOWN:
                setCell(world,
                        event.button.x / CELL_SIZE,
                        event.button.y / CELL_SIZE,
                        getBinaryCell(world, event.button.x / CELL_SIZE, event.button.y / CELL_SIZE) ? DEAD : ALIVE);
            default:
                break;
            }
        }

        if (isSimulating)
        {
            step();
            if (count % 1024 == 0)
            {
                uint16_t aliens = (BOARD_W * BOARD_H / 30);
                for (uint16_t i = 0; i < aliens; i++)
                {
                    setCell(world, rand() % BOARD_W, rand() % BOARD_H, ALIEN);
                }
            }
            count++;
        }
        render();
        SDL_Delay(1000 / fps);
    }
}

uint8_t Game::getBinaryCell(uint8_t *board, int16_t x, int16_t y)
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
    uint8_t sum;
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
            sum = getBinaryCell(oldWorld, x - 1, y - 1) +
                  getBinaryCell(oldWorld, x - 1, y) +
                  getBinaryCell(oldWorld, x - 1, y + 1) +
                  getBinaryCell(oldWorld, x, y - 1) +
                  getBinaryCell(oldWorld, x, y + 1) +
                  getBinaryCell(oldWorld, x + 1, y - 1) +
                  getBinaryCell(oldWorld, x + 1, y) +
                  getBinaryCell(oldWorld, x + 1, y + 1);

            if (sum <= 1 || sum > 3)
            {
                // Any live cell with fewer than two live neighbours dies
                // Any live cell with more than three live neighbours dies
                if (oldCell == DEAD)
                {
                    setCell(world, x, y, DEAD);
                }
                else
                {
                    setCell(world, x, y, std::min(ALIVE, (int)oldCell) - 1);
                }
            }
            else if (sum == 3 && oldCell < ALIVE)
            {
                // Any dead cell with exactly three live neighbours becomes a live cell
                setCell(world, x, y, ALIVE);
                alive++;
            }
            else
            {
                // Any live cell with two or three live neighbours lives on
                if (oldCell >= ALIVE)
                {
                    // Getting older
                    setCell(world, x, y, std::min(ALIVE_OLD, oldCell + 1));
                    alive++;
                }
                else
                {
                    if (oldCell == DEAD)
                    {
                        setCell(world, x, y, DEAD);
                    }
                    else
                    {
                        setCell(world, x, y, std::min(ALIVE, (int)oldCell) - 1);
                    }
                }
            }
        }
    }
}

void Game::render()
{
    SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 192, 0, 255);
    uint8_t value;
    for (uint16_t y = 0; y < BOARD_H; ++y)
    {
        for (uint16_t x = 0; x < BOARD_W; ++x)
        {
            value = getCell(world, x, y);
            if (value == 0)
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
            else if (value < ALIVE)
            {
                SDL_SetRenderDrawColor(renderer, value * 16, value * 16, value * 16, 255);
            }
            else if (value < ALIVE_OLD)
            {
                SDL_SetRenderDrawColor(renderer, 0, 192, 0, 255);
            }
            else if (value < ALIEN)
            {
                SDL_SetRenderDrawColor(renderer, 0, 32, 192, 255);
            }
            else if (value == ALIEN)
            {
                SDL_SetRenderDrawColor(renderer, 192, 0, 0, 255);
            }

            SDL_Rect rect;
            rect.x = CELL_SIZE * x;
            rect.y = CELL_SIZE * y;
            rect.w = CELL_SIZE - 1;
            rect.h = CELL_SIZE - 1;
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    SDL_RenderPresent(renderer);
}
