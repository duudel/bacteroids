
#include "Window.h"
#include "Game.h"

#include "../Log.h"

#include <SDL2/SDL.h>

namespace rob
{

    struct WindowData
    {
        SDL_Window *    m_window;
        SDL_GLContext   m_context;
    };

    Window::Window()
        : m_windowDataBuffer()
        , m_data(nullptr)
    {
        m_data = reinterpret_cast<WindowData*>(m_windowDataBuffer.m_value);
        m_data->m_window = nullptr;
        m_data->m_context = nullptr;

        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

        m_data->m_window = SDL_CreateWindow("Robbin in the Hood",
                                            SDL_WINDOWPOS_UNDEFINED,
                                            SDL_WINDOWPOS_UNDEFINED,
                                            1280, 720,
                                            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
//                                            0, 0,
//                                            SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
        if (!m_data->m_window)
        {
            log::Error("Could not create a window");
            return;
        }

        m_data->m_context = ::SDL_GL_CreateContext(m_data->m_window);
        if (!m_data->m_context)
        {
            log::Error("Could not create GL context");
            return;
        }
    }

    Window::~Window()
    {
        if (m_data->m_context)
        {
            ::SDL_GL_DeleteContext(m_data->m_context);
        }
        if (m_data->m_window)
        {
            ::SDL_DestroyWindow(m_data->m_window);
        }
    }

    bool Window::HandleEvents(Game *game)
    {
        ::SDL_PumpEvents();
        SDL_Event event;
        while (::SDL_PollEvent(&event) == 1)
        {
            switch (event.type)
            {
            case SDL_QUIT:
                return false;
            case SDL_KEYDOWN:
//                if (event.key.keysym.sym == SDLK_ESCAPE)
//                    return false;
                if (event.key.repeat == 0)
                    game->OnKeyPress(static_cast<Key>(event.key.keysym.sym), ::SDL_GetModState());
                game->OnKeyDown(static_cast<Key>(event.key.keysym.sym), ::SDL_GetModState());
                break;
            case SDL_KEYUP:
                game->OnKeyUp(static_cast<Key>(event.key.keysym.sym), ::SDL_GetModState());
                break;
            case SDL_MOUSEBUTTONDOWN:
                game->OnMouseDown(static_cast<MouseButton>(event.button.button), event.button.x, event.button.y);
                break;
            case SDL_MOUSEBUTTONUP:
                game->OnMouseUp(static_cast<MouseButton>(event.button.button), event.button.x, event.button.y);
                break;
            case SDL_MOUSEMOTION:
                game->OnMouseMove(event.motion.x, event.motion.y);
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED:
                    game->OnResize(event.window.data1, event.window.data2);
                    break;
                }
            }
        }
        return true;
    }

    void Window::SwapBuffers()
    { ::SDL_GL_SwapWindow(m_data->m_window); }

    void Window::GetSize(int *w, int *h) const
    { ::SDL_GetWindowSize(m_data->m_window, w, h); }

} // rob
