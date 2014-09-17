
#include "GameState.h"

namespace rob
{

    GameState::GameState()
        : m_ticker()
        , m_time(m_ticker)
        , m_gameTime()
        , m_alloc(nullptr)
        , m_cache(nullptr)
        , m_renderer(nullptr)
        , m_quit(false)
        , m_nextState(0)
    {
        m_ticker.Init();
        m_time.Restart();
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

} // rob
