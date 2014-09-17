
#include "Keyboard.h"

#include <SDL2/SDL.h>

namespace rob
{

    const char* GetKeyName(Key key)
    { return ::SDL_GetScancodeName(static_cast<SDL_Scancode>(key)); }

} // rob
