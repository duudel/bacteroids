
#include "Time.h"

#include <SDL2/SDL.h>

namespace rob
{

    uint32_t GetTicks()
    { return SDL_GetTicks(); }

} // rob
