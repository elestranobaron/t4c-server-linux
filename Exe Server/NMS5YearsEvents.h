// NMS5YearsEvents.h: interface for the Professions class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

class NMS5YearsEvents;

// The class
class __declspec(dllexport) NMS5YearsEvents
{
public:
	
	static void Create();  // Creates all internal structures
   static void Destroy(); // Destroys all internal structures

   static void Manage5YearEvents();
    
	
 
private:
   //static void StartEvent(Players *pPlayer, int iAreneID);
     
   
private:
   static DWORD            m_dwNextItemTime;
   static int              m_iLastPosUse;
   static WorldPos         m_wlPosition[50];
   
};


