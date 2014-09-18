
#ifndef H_ROB_GAME_STATE_H
#define H_ROB_GAME_STATE_H

#include "GameTime.h"
#include "../time/MicroTicker.h"
#include "../time/VirtualTime.h"

#include "../input/Keyboard.h"
#include "../input/Mouse.h"
#include "../input/TextInput.h"
#include "../Log.h"
#include "../String.h"

namespace rob
{

    class GameTime;
    class LinearAllocator;
    class MasterCache;
    class Renderer;

    class GameState
    {
    public:
        GameState();
        GameState(const GameState&) = delete;
        GameState& operator = (const GameState&) = delete;
        virtual ~GameState() { }

        void SetAllocator(LinearAllocator &alloc) { m_alloc = &alloc; }
        LinearAllocator& GetAllocator() { return *m_alloc; }

        void SetCache(MasterCache *cache) { m_cache = cache; }
        MasterCache& GetCache() { return *m_cache; }

        void SetRenderer(Renderer *renderer) { m_renderer = renderer; }
        Renderer& GetRenderer() { return *m_renderer; }

        /// Gets called from Game. Handles fixed step update and calls virtual method Update.
        void DoUpdate();
        /// Gets called from Game. Calls virtual method Render.
        void DoRender();


        virtual bool Initialize() { return true; }

        virtual void Update(const GameTime &gameTime) { }
        virtual void Render() { }

        virtual void OnResize(int w, int h) { }


        virtual void OnTextInput(const char *str)
        { TextInputHandler(m_textInput).OnTextInput(str); }

        virtual void OnKeyPress(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods) { }

        virtual void OnKeyDown(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods)
        { TextInputHandler(m_textInput).OnKeyDown(key, mods); }
        virtual void OnKeyUp(Keyboard::Key key, Keyboard::Scancode scancode, uint32_t mods) { }

        virtual void OnMouseDown(MouseButton button, int x, int y) { }
        virtual void OnMouseUp(MouseButton button, int x, int y) { }
        virtual void OnMouseMove(int x, int y) { }

        void QuitState() { m_quit = true; }
        bool IsQuiting() const { return m_quit; }

        void ChangeState(int state) { m_nextState = state; }
        int NextState() const { return m_nextState; }

    protected:
        MicroTicker m_ticker;
        VirtualTime m_time;
        GameTime m_gameTime;

        TextInput m_textInput;

    private:
        LinearAllocator *   m_alloc;
        MasterCache *       m_cache;
        Renderer *          m_renderer;
        bool m_quit;
        int m_nextState;

        int m_fps;
        int m_frames;
        Time_t m_lastTime;
        Time_t m_accumulator;
    };

} // rob

#endif // H_ROB_GAME_STATE_H

