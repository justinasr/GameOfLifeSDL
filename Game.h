
#ifndef GAME_H
#define GAME_H

#define WINDOW_W 800
#define WINDOW_H 800
#define CELL_SIZE 8
#define BOARD_W (WINDOW_W/CELL_SIZE)
#define BOARD_H (WINDOW_H/CELL_SIZE)

#define DEAD 0
#define ALIVE 8
#define OLD (ALIVE*2)
#define ALIEN (OLD+1)

#include <stdint.h>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

class Game {
    private:
        bool isRunning;
        bool isSimulating;
        SDL_Window *window;
        SDL_Renderer *renderer;
        SDL_Texture *texture;
        uint8_t *world;
        uint8_t *oldWorld;
        uint8_t fps;
        uint32_t *colors;
        uint64_t iteration;
        uint8_t getCell(uint8_t*, int16_t, int16_t);
        uint8_t getCellBinary(uint8_t*, int16_t, int16_t);
        void setCell(uint8_t*, int16_t, int16_t, uint8_t);
        void step();
        void render();
    public:
        Game();
        ~Game();
        void initialize(std::string);
        void run();
};

#endif
