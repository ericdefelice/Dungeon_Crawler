/*!
  @file
  fps_count.cpp

  @brief
  Maintains a count of the number of frames per second.

  @detail
*/

//--------------------------------------------
// Includes
//--------------------------------------------
#include "fps_count.h"

namespace Gumshoe {

FpsCount::FpsCount()
{
}


FpsCount::~FpsCount()
{
}


void FpsCount::Init()
{
	m_fps = 0;
	m_count = 0;
	m_startTime = timeGetTime();
	return;
}


void FpsCount::Update()
{
	m_count++;

	if(timeGetTime() >= (m_startTime + 1000))
	{
		m_fps = m_count;
		m_count = 0;
		
		m_startTime = timeGetTime();
	}
}


int FpsCount::GetFps()
{
	return m_fps;
}

} // end of namespace Gumshoe
