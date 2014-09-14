
#ifndef H_ROB_MOUSE_H
#define H_ROB_MOUSE_H

#include <SDL2/SDL_mouse.h>

namespace rob
{

    enum class MouseButton
    {
        Left = SDL_BUTTON_LEFT,
        Middle = SDL_BUTTON_MIDDLE,
        Right = SDL_BUTTON_RIGHT,
        X1 = SDL_BUTTON_X1,
        X2 = SDL_BUTTON_X2
    };

} // rob

#endif // H_ROB_MOUSE_H

