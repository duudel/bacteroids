
#ifndef H_ROB_VIRTUAL_TIME_H
#define H_ROB_VIRTUAL_TIME_H

#include "MicroTicker.h"

namespace rob
{

    class VirtualTime
    {
    public:
        VirtualTime();

        void Restart(MicroTicker &ticker);
        void Pause();
        void Resume(MicroTicker &ticker);
        void Update(MicroTicker &ticker);

        double GetTime() const;
        Time_t GetTimeMicros() const;

    private:
        Time_t m_time;
        Time_t m_last;
        bool m_paused;
    };

} // rob

#endif // H_ROB_VIRTUAL_TIME_H
