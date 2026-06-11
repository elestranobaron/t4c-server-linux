// PerformUtil.cpp: implementation of the CPerformUtil class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PerformUtil.h"

CPerformUtil::CPerformUtil()
{
}

CPerformUtil::~CPerformUtil()
{
}

void CPerformUtil::PerformanceStart(void)
{
#ifdef _WIN32
    QueryPerformanceCounter(&m_StartTime);
#else
    m_StartTime = std::chrono::steady_clock::now();
#endif
}

void CPerformUtil::PerformanceStop(void)
{
#ifdef _WIN32
    QueryPerformanceCounter(&m_StopTime);
#else
    m_StopTime = std::chrono::steady_clock::now();
#endif
}

double CPerformUtil::PerformanceGetElapsedTime(void)
{
#ifdef _WIN32
   LARGE_INTEGER timerFrequency;
   double ElapsedTime;
   long double ticksPerSecond;
   long double timeDifference;
   __int64 oldTicks;
   __int64 newTicks;
   
   QueryPerformanceFrequency(&timerFrequency);
   
   oldTicks = ((__int64)m_StartTime.HighPart << 32) + (__int64)m_StartTime.LowPart;
   newTicks = ((__int64)m_StopTime.HighPart << 32) + (__int64)m_StopTime.LowPart;
   timeDifference = (long double) (newTicks - oldTicks);
   
   ticksPerSecond = (long double) (((__int64)timerFrequency.HighPart << 32) 
      + (__int64)timerFrequency.LowPart);
   
   ElapsedTime = (double)(timeDifference / ticksPerSecond);

   return ElapsedTime;
#else
    const auto delta = std::chrono::duration_cast<std::chrono::duration<double>>(m_StopTime - m_StartTime);
    return delta.count();
#endif
}

int CPerformUtil::PerformanceGetElapsedTimeMS(void)
{
#ifdef _WIN32
   LARGE_INTEGER timerFrequency;
   int ElapsedTime;
   long double ticksPerSecond;
   long double timeDifference;
   __int64 oldTicks;
   __int64 newTicks;
   
   QueryPerformanceFrequency(&timerFrequency);
   
   oldTicks = ((__int64)m_StartTime.HighPart << 32) + (__int64)m_StartTime.LowPart;
   newTicks = ((__int64)m_StopTime.HighPart << 32) + (__int64)m_StopTime.LowPart;
   timeDifference = (long double) (newTicks - oldTicks);
   
   ticksPerSecond = (long double) (((__int64)timerFrequency.HighPart << 32) 
      + (__int64)timerFrequency.LowPart);
   
   ElapsedTime = (int)((double)((timeDifference / ticksPerSecond)*1000.00));

   return ElapsedTime;
#else
    return static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(m_StopTime - m_StartTime).count());
#endif
}
