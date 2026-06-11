/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
//      File Name: Autolock.cpp
//      Project:   The 4th Coming
//      Creation:  Friday, September 24, 1999
//      Author:    Ben Thomas (TH)
#include "stdafx.h"
#include "Autolock.h"

/******************************************************************************/
Autolock::Autolock(CRITICAL_SECTION *cs)           // The Critical Section to lock (unlock).
/******************************************************************************/
{
	m_cs = cs; // Remember the Critical section, needed for the destructor.
	EnterCriticalSection(m_cs);
}
/******************************************************************************/
Autolock::~Autolock( void )
/******************************************************************************/
{
	LeaveCriticalSection(m_cs);
}