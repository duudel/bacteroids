
#include <SDL2/SDL.h>

#include "application/Game.h"

#include "resource/Builder/MasterBuilder.h"

int main(int argc, char *argv[])
{
    rob::MasterBuilder builder;
    builder.Build("data_source", "data");

    rob::Game game;
    game.Run();

    return 0;
}
