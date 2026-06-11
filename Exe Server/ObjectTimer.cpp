/******************************************************************************
Modify for vs2008 (01/05/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "ObjectTimer.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
#define LOCK	csThreadLock.Lock();
#define UNLOCK	csThreadLock.Unlock();

/******************************************************************************/
TemplateList <Unit>		ObjectTimer::tluObjectTimers;
CLock                   ObjectTimer::csThreadLock;

/******************************************************************************/
// Creates the object timers.
void ObjectTimer::Create( void )
/******************************************************************************/
{
}
/******************************************************************************/
void ObjectTimer::Destroy( void )
// Destroys the object timers.
/******************************************************************************/
{
}
/******************************************************************************/
// Adds an object to the timer objects
void ObjectTimer::AddObject( Unit *lpuUnit )// the Unit to add
/******************************************************************************/
{
	LOCK
	// If unit is an object..!
	if( lpuUnit->GetType() == U_OBJECT )
	{
		// If unit isn't already in list.
		if( !ObjectInList( lpuUnit ) )
		{
			// Add the item.
			tluObjectTimers.AddToTail( lpuUnit );
		}
	}
	UNLOCK
}
/******************************************************************************/
// Removes an object from the object timers.
void ObjectTimer::RemoveObject( Unit *lpuUnit ) // The unit to remove
/******************************************************************************/
{
	LOCK

	BOOL boFound = FALSE;

	tluObjectTimers.ToHead();
	while( tluObjectTimers.QueryNext() && !boFound )
	{
		if( tluObjectTimers.Object() == lpuUnit )
		{
			tluObjectTimers.Remove();
			boFound = TRUE;
		}
	}

	UNLOCK
}
/******************************************************************************/
// Removes an object from the timers.
void ObjectTimer::RemoveObject( DWORD dwID )// The unit ID to remove.
/******************************************************************************/
{
	LOCK

	BOOL boFound = FALSE;

	tluObjectTimers.ToHead();
	while( tluObjectTimers.QueryNext() && !boFound )
	{
		if( tluObjectTimers.Object()->GetID() == dwID )
		{
			tluObjectTimers.Remove();
			boFound = TRUE;
		}
	}

	UNLOCK
}
/******************************************************************************/
// Verifies the timers of all the object in the object timer list.
void ObjectTimer::VerifyTimers( void )
/******************************************************************************/
{
	LOCK	
	tluObjectTimers.ToHead();
	while( tluObjectTimers.QueryNext() )
	{
		//Verify the timers of each objects in the list.
		tluObjectTimers.Object()->VerifyTimers();
	}
	UNLOCK
}
/******************************************************************************/
// Queries the existence of an object in the list.
BOOL ObjectTimer::ObjectInList( Unit *lpuUnit )// the object to query
/******************************************************************************/
{
	LOCK
	BOOL boFound = FALSE;

	tluObjectTimers.ToHead();
	while( tluObjectTimers.QueryNext() && !boFound )
	{
		if( tluObjectTimers.Object() == lpuUnit )
		{
			boFound = TRUE;
		}
	}

	UNLOCK

	return boFound;
}
/******************************************************************************/
// Queries the existance of an object in the list.
BOOL ObjectTimer::ObjectInList( DWORD dwID )// The object's ID to query
/******************************************************************************/
{
	LOCK
	BOOL boFound = FALSE;

	tluObjectTimers.ToHead();
	while( tluObjectTimers.QueryNext() && !boFound )
	{
		if( tluObjectTimers.Object()->GetID() == dwID )
		{
			boFound = TRUE;
		}
	}

	UNLOCK

	return boFound;
}