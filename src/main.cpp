
#include <SDL2/SDL.h>

#include "bacteroids/BacteroidsGame.h"

#include "resource/Builder/MasterBuilder.h"

int main(int argc, char *argv[]) __attribute__((force_align_arg_pointer));

int main(int argc, char *argv[])
{
    rob::MasterBuilder builder;
    builder.Build("data_source", "data");

    bact::Bacteroids game;
    game.Run();

    return 0;
}
