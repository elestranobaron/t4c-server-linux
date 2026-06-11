/******************************************************************************
Modify for vs2008 (30/04/2009)
/******************************************************************************
Thread Monitor
Written by Carlos Lima <carlos@dialsoft.com>
2006-03-17

This should register all running threads IDs and Names
so we can track what crashed and what else was running 
by that time.
/******************************************************************************/
#include "stdafx.h"
#include "ThreadMonitor.h"
#include "T4CLog.h"

/******************************************************************************/
CThreadMonitor::CThreadMonitor()
/******************************************************************************/
{
}
/******************************************************************************/
void CThreadMonitor::RegisterThread(std::string sThreadName)
/******************************************************************************/
{
	_LOG_DEBUG
		LOG_DEBUG_LVL4,
		"Starting thread %s ID %u", sThreadName.c_str(), GetCurrentThreadId()
	LOG_

	CAutoLock(this);
	runningThreadsMap[GetCurrentThreadId()] = sThreadName;
}
/******************************************************************************/
void CThreadMonitor::UnregisterThread()
/******************************************************************************/
{
	_LOG_DEBUG
		LOG_DEBUG_LVL4,
		"Finishing thread %u", GetCurrentThreadId()
	LOG_

	CAutoLock(this);

   ThreadListIterator i = runningThreadsMap.find(GetCurrentThreadId());
   if (i == runningThreadsMap.end()) 
   {
      return;
   }
   runningThreadsMap.erase(i);

	//runningThreadsMap.erase(GetCurrentThreadId());
}
/******************************************************************************/
bool CThreadMonitor::GetThreadName(DWORD dwThreadID, std::string &sThreadName) 
/******************************************************************************/
{
	CAutoLock(this);
	ThreadListIterator i = runningThreadsMap.find(dwThreadID);
	if (i == runningThreadsMap.end()) 
	{
		return false;
	}
	sThreadName = (*i).second;
	return true;
}
/******************************************************************************/
void CThreadMonitor::GetRunningThreadsList(ThreadList &containerForListOfThreads) 
/******************************************************************************/
{
	CAutoLock(this);
	containerForListOfThreads = runningThreadsMap;
}
/******************************************************************************/
CThreadMonitor& CThreadMonitor::GetInstance() 
/******************************************************************************/
{
	static CThreadMonitor threadMonitorInstance;
	return threadMonitorInstance;
}
/******************************************************************************/
CAutoThreadMonitor::CAutoThreadMonitor(std::string sThreadName) 
/******************************************************************************/
{
	CThreadMonitor::GetInstance().RegisterThread(sThreadName);
#ifdef _WIN32
	if (cr_thread_install_helper.m_nInstallStatus!=0) {
		// Something goes wrong. Get error message.
		char szErrorMsg[512]; szErrorMsg[0]=0;
		crGetLastErrorMsg(szErrorMsg, 512);
		_LOG_DEBUG
			LOG_CRIT_ERRORS,
			"Failed to install CrashRpt on thread %s ID %u: %s", sThreadName.c_str(), GetCurrentThreadId(), szErrorMsg
		LOG_
	} else {
		_LOG_DEBUG
			LOG_DEBUG_LVL4,
			"Installed CrashRpt on thread %s ID %u", sThreadName.c_str(), GetCurrentThreadId()
		LOG_
	}
#endif

}
/******************************************************************************/
CAutoThreadMonitor::~CAutoThreadMonitor() 
/******************************************************************************/
{
	CThreadMonitor::GetInstance().UnregisterThread();
}
