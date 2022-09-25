#include "Game.h"

int main(int argc, char *argv[])
{
    Game *game = new Game();
    game->initialize("The Game of Life");
    game->run();
    return 0;
}
