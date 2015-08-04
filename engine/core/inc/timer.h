/*!
  @file
  timer.h

  @brief
  Functionality for timers in the game engine.

  @detail
*/

#pragma once


//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"


namespace Gumshoe {

//--------------------------------------------
// Timer class definition
//--------------------------------------------
class Timer
{
public:
	Timer();
	~Timer();

	bool Init();
	void Update();

	float GetTime();

private:
	INT64 m_frequency;
	float m_ticksPerMs;
	INT64 m_startTime;
	float m_frameTime;
};

} // end of namespace Gumshoe