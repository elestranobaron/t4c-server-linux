/******************************************************************************
Modify for vs2008 (30/04/2009)
Add GetNbrRegisteredUnit() function by NightMare (27/06/2009)
/******************************************************************************/
//      File Name: DynObjManager.cpp
//      Project:   TFC Server
//      Plateform: Windows NT 4.0 Workstation/Server
//      Creation:  20/5/1998
//      Author:    Francois Leblanc (FL)
/******************************************************************************/
//      Change History
//
//         Date            Ver.      Author         Purpose
//         ----            ----      ------         -------
//         20/5/1998      1.0        FL             Initial developpement
//
//      Description
//          This file manages dynamicaly loaded objects via database. Object structures come
//		from dynamic link libraries, therefor the server needs to have a way to control
//		which object class in a DLL corresponds to an object structure ID in the database
//		( Field "ObjectStructure" )
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "DynObjManager.h"
#include "Unit.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
TemplateList< DynObjManager::DYNLIST>	DynObjManager::tlDynObjects;
TemplateList< ObjectStructure >			DynObjManager::tlRegisteredUnits;

/******************************************************************************/
// Registers an object structure for use by dynamically added objects.
void DynObjManager::RegisterObjectStructureCallback(
 WORD wStructureID,			// ID to assign to structure.
 DYNOBJCALLBACK lpCallback	// Callback function which will create the object.
)
/******************************************************************************/
{
	DYNLIST *lpDyn = new DYNLIST;
	lpDyn->wStructureID = wStructureID;
	lpDyn->lpCallback = lpCallback;
		
	tlDynObjects.AddToTail( lpDyn );
}
/******************************************************************************/
// Called after object initialization
void DynObjManager::PostInitDestroy( void )
/******************************************************************************/
{
	tlDynObjects.AnnihilateList();
}
/******************************************************************************/
// Destroy the dynamic object manager. Called at server termination.
void DynObjManager::Destroy( void )
/******************************************************************************/
{
	tlRegisteredUnits.AnnihilateList();
}
/******************************************************************************/
// Creates an object of type wStructureID and registers it as unit ID wUnitID
ObjectStructure *DynObjManager::GetRegisteredUnit(
 WORD wStructureID,		// The structureID of the class to use for the object creation
 WORD wReferenceID,		// The unit ID to assign to created unit. (BaseReferenceID)
 LPCTSTR lpszUnitName	// Units' string ID.
)
/******************************************************************************/
{
	BOOL boFound = FALSE;	
	DYNLIST *lpDyn;
	ObjectStructure *lpItem = NULL;
	
	tlDynObjects.ToHead();

	// Find the structure ID
	while( tlDynObjects.QueryNext() && !boFound )
	{
		lpDyn = tlDynObjects.Object();

		if( lpDyn->wStructureID == wStructureID )
		{
			boFound = TRUE;
		}		
	}

	// If ID was found
	if( boFound )
	{
		// Get created object from callback
		lpItem = lpDyn->lpCallback();

		// If item was registered.
		if( lpItem != NULL )
		{
			// Register units' base reference.
			if( Unit::RegisterUnitMessageHandler( 
				wReferenceID,							// Base reference ID of unit.
				lpItem,									// Message handler.
				lpszUnitName,							// Pointer to the name of the unit.
				U_OBJECT,								// Kind of unit to register
				FALSE									// TRUE if function should assign the next valid ID.
			) == 0 )
			{
				// If unit couldn't be registered, delete its reference.
				delete lpItem;
				lpItem = NULL;
			}
			else
			{
				// Add to dynamically registered units
				tlRegisteredUnits.AddToTail( lpItem );
			}
		}
	}

	return lpItem;
}

int DynObjManager::GetNbrRegisteredUnit()
{
   return tlRegisteredUnits.NbObjects();
}

ObjectStructure *DynObjManager::GetRegisteredUnitIndex(int iIdx)
{
   ObjectStructure *lpItem = NULL;


   if(iIdx <0 || iIdx >= GetNbrRegisteredUnit())
      return lpItem;
	
   tlRegisteredUnits.Lock();
	tlRegisteredUnits.ToHead();

	for(int i=0;i<iIdx;i++)
      tlRegisteredUnits.QueryNext();

   lpItem = tlRegisteredUnits.Object();
   tlRegisteredUnits.Unlock();

   return lpItem;
}

ObjectStructure *DynObjManager::GetRegisteredUnitID(WORD wID)
{
   ObjectStructure *lpItem = NULL;


   tlRegisteredUnits.Lock();
	tlRegisteredUnits.ToHead();

	while( tlRegisteredUnits.QueryNext() )
   {
      lpItem = tlRegisteredUnits.Object();


      if(lpItem->wBindedReferenceID == wID)
      {
         tlRegisteredUnits.Unlock();   
         return lpItem;
      }
   }
   tlRegisteredUnits.Unlock();

   return NULL;
}