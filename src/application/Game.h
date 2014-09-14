
#ifndef H_ROB_GAME_H
#define H_ROB_GAME_H

#include "../memory/LinearAllocator.h"
#include "../input/Keyboard.h"
#include "../input/Mouse.h"

namespace rob
{

    class Window;
    class Graphics;
    class AudioSystem;
    class MasterCache;
    class Renderer;
    class GameState;

    class Game
    {
    public:
        Game();
        ~Game();

        void Run();

        void OnResize(int w, int h);
        void OnKeyPress(Key key, uint32_t mods);
        void OnKeyDown(Key key, uint32_t mods);
        void OnKeyUp(Key key, uint32_t mods);
        void OnMouseDown(MouseButton button, int x, int y);
        void OnMouseUp(MouseButton button, int x, int y);
        void OnMouseMove(int x, int y);

    private:
        LinearAllocator m_staticAlloc;
        Window *m_window;
        Graphics *m_graphics;
        AudioSystem *m_audio;
        MasterCache *m_cache;
        Renderer *m_renderer;

        GameState *m_state;
        LinearAllocator m_stateAlloc;
    };

} // rob

#endif // H_ROB_GAME_H

