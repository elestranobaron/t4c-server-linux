// PerformUtil.h: interface for the CPerformUtil class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PERFORMUTIL_H__6F38D9E6_9F70_4321_962C_9EB79CC53FAF__INCLUDED_)
#define AFX_PERFORMUTIL_H__6F38D9E6_9F70_4321_962C_9EB79CC53FAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <windows.h>

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
   

protected:
   LARGE_INTEGER m_StartTime;
   LARGE_INTEGER m_StopTime;

};

#endif // !defined(AFX_PERFORMUTIL_H__6F38D9E6_9F70_4321_962C_9EB79CC53FAF__INCLUDED_)
