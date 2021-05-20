/*******************************************************************************
 * Author:      Zhang Li
 * Description:
 ******************************************************************************/
#ifndef HEADER_DEBUG_TIMER_H
#define HEADER_DEBUG_TIMER_H

#include <sys/time.h>
#include <stdio.h>

#if DEBUG_TIMER
class DebugTimer {
    public:
        inline DebugTimer(const char* msgformat): m_msgformat(msgformat) {
            m_sec = 0;
        }
        inline ~DebugTimer() {
            fprintf(stderr, m_msgformat, m_sec);
        }

    private:
        const char* m_msgformat;
        double m_sec;

        friend class DebugTimerObject;
};

class DebugTimerObject {
    public:
        DebugTimerObject(DebugTimer& timer): m_timer(timer) {
            gettimeofday(&time_beg, 0);
        }
        ~DebugTimerObject() {
            gettimeofday(&time_end, 0);
            m_timer.m_sec += (time_end.tv_sec - time_beg.tv_sec) + (time_end.tv_usec - time_beg.tv_usec) / 1000000.0;
        }

    private:
        struct timeval time_beg;
        struct timeval time_end;
        DebugTimer& m_timer;
};
#else // DEBUG_TIMER

class DebugTimer {
    public:
        DebugTimer(const char* msgformat)   {}
        ~DebugTimer()                       {}
};
class DebugTimerObject {
    public:
        DebugTimerObject(DebugTimer& timer) {}
        ~DebugTimerObject()                 {}
};
#endif // DEBUG_TIMER
#endif
