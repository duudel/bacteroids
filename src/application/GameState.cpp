
#include "GameState.h"
#include "../renderer/Renderer.h"
#include "../math/Functions.h"

#include "../String.h"

namespace rob
{

    GameState::GameState()
        : m_ticker()
        , m_time(m_ticker)
        , m_gameTime()
        , m_textInput()
        , m_alloc(nullptr)
        , m_cache(nullptr)
        , m_renderer(nullptr)
        , m_quit(false)
        , m_nextState(0)
        , m_fps(0)
        , m_frames(0)
        , m_lastTime(0)
        , m_accumulator(0)
    {
        m_ticker.Init();
        m_time.Restart();
        m_lastTime = m_time.GetTimeMicros();
    }

    void GameState::DoUpdate()
    {
        const Time_t lastTime = m_time.GetTimeMicros();
        m_time.Update();
        Time_t frameTime = m_time.GetTimeMicros() - lastTime;
        if (frameTime > 25000)
            frameTime = 25000;

        m_gameTime.Update(frameTime);
        while (m_gameTime.Step())
        {
            m_time.Update();
            Update(m_gameTime);
        }
    }

    void GameState::DoRender()
    {
        Render();
        const Time_t time = m_time.GetTimeMicros();
        const Time_t frameTime = time - m_lastTime;
        m_lastTime = time;

        m_frames++;

        m_accumulator += frameTime;

        if (m_accumulator >= 1000000)
        {
            m_accumulator -= 1000000;
            m_fps = m_frames;
            m_frames = 0;
        }

        char buf[30];
        StringPrintF(buf, "FPS: %i", m_fps);

        int w, h;
        m_renderer->GetScreenSize(&w, &h);
        const float tw = m_renderer->GetTextWidth(buf);
        const float x = float(w) - Max(tw, 120.0f);

        m_renderer->BindFontShader();
        m_renderer->SetColor(Color::White);
        m_renderer->DrawText(x, 0.0f, buf);
    }

} // rob
