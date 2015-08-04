/*!
  @file
  cpu_load.cpp

  @brief
  Maintains the amout of time the CPU is being used by the game.

  @detail
*/

//--------------------------------------------
// Includes
//--------------------------------------------
#include "cpu_load.h"


namespace Gumshoe {

CpuLoad::CpuLoad()
{
}


CpuLoad::~CpuLoad()
{
}


void CpuLoad::Init()
{
	PDH_STATUS status;


	// Initialize the flag indicating whether this object can read the system cpu usage or not.
	m_canReadCpu = true;

	// Create a query object to poll cpu usage.
	status = PdhOpenQuery(NULL, 0, &m_queryHandle);
	if(status != ERROR_SUCCESS)
	{
		m_canReadCpu = false;
	}

	// Set query object to poll all cpus in the system.
	status = PdhAddCounter(m_queryHandle, TEXT("\\Processor(_Total)\\% processor time"), 0, &m_counterHandle);
	if(status != ERROR_SUCCESS)
	{
		m_canReadCpu = false;
	}

	m_lastSampleTime = GetTickCount(); 

	m_cpuUsage = 0;

	return;
}


void CpuLoad::Shutdown()
{
	if(m_canReadCpu)
	{
		PdhCloseQuery(m_queryHandle);
	}

	return;
}


void CpuLoad::Update()
{
	PDH_FMT_COUNTERVALUE value; 

	if(m_canReadCpu)
	{
		if((m_lastSampleTime + 1000) < GetTickCount())
		{
			m_lastSampleTime = GetTickCount(); 

			PdhCollectQueryData(m_queryHandle);
        
			PdhGetFormattedCounterValue(m_counterHandle, PDH_FMT_LONG, NULL, &value);

			m_cpuUsage = value.longValue;
		}
	}

	return;
}


int CpuLoad::GetCpuPercentage()
{
	int usage;

	if(m_canReadCpu)
	{
		usage = (int)m_cpuUsage;
	}
	else
	{
		usage = 0;
	}

	return usage;
}

} // end of namespace Gumshoe