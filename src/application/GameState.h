
#ifndef H_ROB_GAME_STATE_H
#define H_ROB_GAME_STATE_H

#include "../input/Keyboard.h"
#include "../input/Mouse.h"

namespace rob
{

    class GameTime;
    class LinearAllocator;
    class MasterCache;
    class Renderer;

    class GameState
    {
    public:
        GameState()
            : m_alloc(nullptr)
            , m_cache(nullptr)
            , m_renderer(nullptr)
            , m_quit(false)
            , m_nextState(0)
        { }

        virtual ~GameState() { }

        void SetAllocator(LinearAllocator &alloc) { m_alloc = &alloc; }
        LinearAllocator& GetAllocator() { return *m_alloc; }

        void SetCache(MasterCache *cache) { m_cache = cache; }
        MasterCache& GetCache() { return *m_cache; }

        void SetRenderer(Renderer *renderer) { m_renderer = renderer; }
        Renderer& GetRenderer() { return *m_renderer; }

        virtual bool Initialize() { return true; }

        virtual void Update(const GameTime &gameTime) { }
        virtual void Render(const GameTime &gameTime) { }

        virtual void OnResize(int w, int h) { }

        virtual void OnKeyPress(Key key, uint32_t mods) { }
        virtual void OnKeyDown(Key key, uint32_t mods) { }
        virtual void OnKeyUp(Key key, uint32_t mods) { }

        virtual void OnMouseDown(MouseButton button, int x, int y) { }
        virtual void OnMouseUp(MouseButton button, int x, int y) { }
        virtual void OnMouseMove(int x, int y) { }

        void QuitState() { m_quit = true; }
        bool IsQuiting() const { return m_quit; }

        void ChangeState(int state) { m_nextState = state; }
        int NextState() const { return m_nextState; }

    private:
        LinearAllocator *   m_alloc;
        MasterCache *       m_cache;
        Renderer *          m_renderer;
        bool m_quit;
        int m_nextState;
    };

} // rob

#endif // H_ROB_GAME_STATE_H

