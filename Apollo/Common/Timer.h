#pragma once
#include "Types.h"

namespace Apollo {
    class Timer {
    public:
	    static void Init();

	    Timer();
	    void    Start();
	    UINT32  Stop();

        UINT32  Pause();
        void    Resume();
    
        UINT32  GetTimeElapsed() const;

    private:
	    INT64	m_time;
        UINT32  m_timeElapsed;
    };
}
