/*!
  @file
  cpu_load.h

  @brief
  Maintains the amout of time the CPU is being used by the game.

  @detail
*/

#pragma once


//--------------------------------------------
// Linking
//--------------------------------------------
#pragma comment(lib, "pdh.lib")


//--------------------------------------------
// Includes
//--------------------------------------------
#include "gumshoe_typedefs.h"
#include <pdh.h>


namespace Gumshoe {

//--------------------------------------------
// CpuLoad class definition
//--------------------------------------------
class CpuLoad
{
public:
	CpuLoad();
	~CpuLoad();

	void Init();
	void Shutdown();
	void Update();
	int GetCpuPercentage();

private:
	bool m_canReadCpu;
	HQUERY m_queryHandle;
	HCOUNTER m_counterHandle;
	uint32 m_lastSampleTime;
	uint32 m_cpuUsage;
};

} // end of namespace Gumshoe
