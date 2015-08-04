/*!
  @file
  fps_count.h

  @brief
  Maintains a count of the number of frames per second.

  @detail
*/

#pragma once

//--------------------------------------------
// Linking
//--------------------------------------------
#pragma comment(lib, "winmm.lib")


//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include <mmsystem.h>


namespace Gumshoe {

//--------------------------------------------
// FpsCount class definition
//--------------------------------------------
class FpsCount
{
public:
	FpsCount();
	~FpsCount();

	void Init();
	void Update();
	int GetFps();

private:
	uint16 m_fps, m_count;
	uint32 m_startTime;
};

} // end of namespace Gumshoe