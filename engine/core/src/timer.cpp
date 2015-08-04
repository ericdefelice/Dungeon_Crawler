/*!
  @file
  timer.cpp

  @brief
  Functionality for timers in the game engine.

  @detail
*/

//--------------------------------------------
// Includes
//--------------------------------------------
#include "timer.h"


namespace Gumshoe {

Timer::Timer()
{
}


Timer::~Timer()
{
}


bool Timer::Init()
{
	// Check to see if this system supports high performance timers.
	QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency);
	if(m_frequency == 0)
	{
		return false;
	}

	// Find out how many times the frequency counter ticks every millisecond.
	m_ticksPerMs = (float)(m_frequency / 1000);

	QueryPerformanceCounter((LARGE_INTEGER*)&m_startTime);

	return true;
}


void Timer::Update()
{
	INT64 currentTime;
	float timeDifference;


	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	timeDifference = (float)(currentTime - m_startTime);

	m_frameTime = timeDifference / m_ticksPerMs;

	m_startTime = currentTime;

	return;
}


float Timer::GetTime()
{
	return m_frameTime;
}

} // end of namespace Gumshoe
