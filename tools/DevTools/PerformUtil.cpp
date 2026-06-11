// PerformUtil.cpp: implementation of the CPerformUtil class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PerformUtil.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPerformUtil::CPerformUtil()
{

}

CPerformUtil::~CPerformUtil()
{

}

void CPerformUtil::PerformanceStart(void)
{
    QueryPerformanceCounter(&m_StartTime);
}

void CPerformUtil::PerformanceStop(void)
{
    QueryPerformanceCounter(&m_StopTime);
}

double CPerformUtil::PerformanceGetElapsedTime(void)
{
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
}

int CPerformUtil::PerformanceGetElapsedTimeMS(void)
{
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
}


