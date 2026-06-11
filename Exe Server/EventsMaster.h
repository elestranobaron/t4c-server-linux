// EventsMaster.h: interface for the Professions class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "VDList.h"
#include "IntlText.h"
#include "NMEvents.h"

class EventsMaster;

// The class
class __declspec(dllexport) EventsMaster
{
public:
	
	static void Create();  // Creates all internal structures
   static void Destroy(); // Destroys all internal structures

   static void ManageEvents();


   static int GetNbrEvent();
   static int GetEventFlagByIndex(int iIdx);
   static void SendEventList(Character *pUser);
   static void SummonMonsterE(int iEventID, CString strMID, int iQty, int iX, int iY, int iW);
   static void SummonItemE(int iEventID, DWORD dwID, int iQty, int iX, int iY, int iW);
   static void KillObjectID(DWORD dwID, int iX, int iY, int iW);
    
	
 
private:

     
   
private:
   static CPtrArray c_aEvents;

   
};


