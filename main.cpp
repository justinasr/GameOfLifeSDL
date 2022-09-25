#include "Game.h"

int main(int argc, char *argv[])
{
    Game *game = new Game("The Game of Life");
    game->initialize();
    game->run();
    return 0;
}
