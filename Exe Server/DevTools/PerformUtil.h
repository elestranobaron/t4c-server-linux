// PerformUtil.h: interface for the CPerformUtil class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PERFORMUTIL_H__6F38D9E6_9F70_4321_962C_9EB79CC53FAF__INCLUDED_)
#define AFX_PERFORMUTIL_H__6F38D9E6_9F70_4321_962C_9EB79CC53FAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <chrono>
#endif

class CPerformUtil  
{
public:
	CPerformUtil();
	virtual ~CPerformUtil();

public:
   void	 PerformanceStart(void);
   void	 PerformanceStop(void);
   double PerformanceGetElapsedTime(void);
   int    PerformanceGetElapsedTimeMS(void);

protected:
#ifdef _WIN32
   LARGE_INTEGER m_StartTime;
   LARGE_INTEGER m_StopTime;
#else
   std::chrono::steady_clock::time_point m_StartTime;
   std::chrono::steady_clock::time_point m_StopTime;
#endif
};

#endif // !defined(AFX_PERFORMUTIL_H__6F38D9E6_9F70_4321_962C_9EB79CC53FAF__INCLUDED_)
