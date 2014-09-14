
#include "Game.h"
#include "GameState.h"
#include "Window.h"
#include "../graphics/Graphics.h"
#include "../audio/AudioSystem.h"
#include "../resource/MasterCache.h"
#include "../renderer/Renderer.h"

#include "MicroTicker.h"

#include "../Log.h"

#include <SDL2/SDL.h>

namespace rob
{

    class VirtualTime
    {
    public:
        VirtualTime()
            : m_time(0)
            , m_last(0)
            , m_paused(true)
        { }

        void Restart(MicroTicker &ticker)
        {
            m_time = 0;
            Resume(ticker);
        }

        void Pause()
        {
            m_paused = true;
        }

        void Resume(MicroTicker &ticker)
        {
            m_last = ticker.GetTicks();
            m_paused = false;
        }

        void Update(MicroTicker &ticker)
        {
            if (!m_paused)
            {
                Time_t t = ticker.GetTicks();
                m_time += t - m_last;
                m_last = t;
            }
        }

        double GetTime() const
        {
            return m_time / 1000000.0;
        }

        Time_t GetTimeMillis() const
        {
            return m_time / 1000;
        }

        Time_t GetTimeMicros() const
        {
            return m_time;
        }

    private:
        Time_t m_time;
        Time_t m_last;
        bool m_paused;
    };

    class GameTime
    {
    };

    static const size_t STATIC_MEMORY_SIZE = 4 * 1024 * 1024;

    Game::Game()
        : m_staticAlloc(STATIC_MEMORY_SIZE)
        , m_window(nullptr)
        , m_graphics(nullptr)
        , m_audio(nullptr)
        , m_cache(nullptr)
        , m_renderer(nullptr)
        , m_state(nullptr)
        , m_stateAlloc()
    {
        ::SDL_Init(SDL_INIT_EVERYTHING);

        m_window = m_staticAlloc.new_object<Window>();
        m_graphics = m_staticAlloc.new_object<Graphics>(m_staticAlloc);
        m_audio = m_staticAlloc.new_object<AudioSystem>(m_staticAlloc);
        m_cache = m_staticAlloc.new_object<MasterCache>(m_graphics, m_audio, m_staticAlloc);
        m_renderer = m_staticAlloc.new_object<Renderer>(m_graphics, m_staticAlloc);

        int w, h;
        m_window->GetSize(&w, &h);
        m_graphics->SetViewport(0, 0, w, h);

        log::Info("Static memory used: ", m_staticAlloc.GetAllocatedSize(), " bytes");

        const size_t freeMemory = STATIC_MEMORY_SIZE - m_staticAlloc.GetAllocatedSize();
        m_stateAlloc.SetMemory(m_staticAlloc.Allocate(freeMemory), freeMemory);
    }

    Game::~Game()
    {
        m_stateAlloc.del_object(m_state);
        m_staticAlloc.del_object(m_renderer);
        m_staticAlloc.del_object(m_cache);
        m_staticAlloc.del_object(m_audio);
        m_staticAlloc.del_object(m_graphics);
        m_staticAlloc.del_object(m_window);
        ::SDL_Quit();
    }

    bool Game::Setup()
    {
        if (!m_graphics->IsInitialized())
        {
            log::Error("Graphics was not initialized, aborting...");
            return false;
        }

        if (!Initialize())
        {
            log::Error("Could not initialize game, aborting...");
            return false;
        }

        if (m_state == nullptr)
        {
            log::Error("Game state not initialized by Game::Initialize(), aborting...");
            return false;
        }
        return true;
    }

    void Game::Run()
    {
        if (!Setup())
            return;

        GameTime gameTime;
        while (m_window->HandleEvents(this))
        {
            m_graphics->Clear();

            m_state->Update(gameTime);
            m_state->Render(gameTime);

            m_window->SwapBuffers();

            if (m_state->IsQuiting())
                break;
        }
    }

    void Game::OnResize(int w, int h)
    { m_graphics->SetViewport(0, 0, w, h); }

    void Game::OnKeyPress(Key key, uint32_t mods)
    { m_state->OnKeyPress(key, mods); }

    void Game::OnKeyDown(Key key, uint32_t mods)
    { m_state->OnKeyDown(key, mods); }

    void Game::OnKeyUp(Key key, uint32_t mods)
    { m_state->OnKeyUp(key, mods); }

    void Game::OnMouseDown(MouseButton button, int x, int y)
    { m_state->OnMouseDown(button, x, y); }

    void Game::OnMouseUp(MouseButton button, int x, int y)
    { m_state->OnMouseUp(button, x, y); }

    void Game::OnMouseMove(int x, int y)
    { m_state->OnMouseMove(x, y); }

} // rob
