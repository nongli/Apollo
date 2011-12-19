#include "Timer.h"
#include <windows.h>

INT64 s_TimerFrequency;

namespace Apollo {

void Timer::Init() {
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	s_TimerFrequency = frequency.QuadPart;
}

Timer::Timer() {
    m_timeElapsed = 0;
}

void Timer::Start() {
    m_timeElapsed = 0;
    Resume();
}

UINT32 Timer::Stop() {
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);

	UINT64 diff = time.QuadPart - m_time;
	m_timeElapsed = (UINT32)(diff * 1000 / s_TimerFrequency);
    return m_timeElapsed;
}

UINT32 Timer::GetTimeElapsed() const {
    return m_timeElapsed;
}

UINT32 Timer::Pause() {
    UINT32 timePassed = this->Stop();
    m_timeElapsed += timePassed;
    return m_timeElapsed;
}

void Timer::Resume() {
    LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	m_time = time.QuadPart;	
}

}
