
#include "VirtualTime.h"

namespace rob
{

    VirtualTime::VirtualTime()
        : m_time(0)
        , m_last(0)
        , m_paused(true)
    { }

    void VirtualTime::Restart(MicroTicker &ticker)
    {
        m_time = 0;
        Resume(ticker);
    }

    void VirtualTime::Pause()
    {
        m_paused = true;
    }

    void VirtualTime::Resume(MicroTicker &ticker)
    {
        m_last = ticker.GetTicks();
        m_paused = false;
    }

    void VirtualTime::Update(MicroTicker &ticker)
    {
        if (!m_paused)
        {
            Time_t t = ticker.GetTicks();
            m_time += t - m_last;
            m_last = t;
        }
    }

    double VirtualTime::GetTime() const
    {
        return m_time / 1000000.0;
    }

    Time_t VirtualTime::GetTimeMicros() const
    {
        return m_time;
    }

} // rob
