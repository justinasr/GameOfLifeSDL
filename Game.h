
#ifndef GAME_H
#define GAME_H

#define WINDOW_W 768
#define WINDOW_H 768

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
        uint16_t alienCounter;
        uint8_t cellSize;
        uint16_t boardWidth;
        uint16_t boardHeight;
        uint8_t getCell(uint8_t*, int16_t, int16_t);
        uint8_t getCellBinary(uint8_t*, int16_t, int16_t);
        void setCell(uint8_t*, int16_t, int16_t, uint8_t);
        void step();
        void render();
    public:
        Game(std::string);
        ~Game();
        void initialize();
        void run();
};

#endif
