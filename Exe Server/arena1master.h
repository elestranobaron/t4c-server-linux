// Arena1Master.h: interface for the Professions class.
//
//////////////////////////////////////////////////////////////////////
#pragma once


#include "ArenaGlobal.h"


class Arena1Master;

// The class
class __declspec(dllexport) Arena1Master
{
public:
	
	static void Create(int iNbrArena);  // Creates all internal structures
   static void Destroy(); // Destroys all internal structures

   static void ManageArena();
   static void IncreasePoint(Players *pPlayer, int iAreneID);
   
   static void AddPlayer(Players *pPlayer, int iAreneID);
   static void RemovePlayer(Players *pPlayer,bool bUpdateList, int iAreneID,BOOL bPenalty);
   static void SendArenaList(Players *pPlayer, int iAreneID);
   static void SendArenaPlayStat(Players *pPlayer,bool bForceOffline,int iAreneID);
   static void SummonFlagRnd(int iAreneID);
   static void SummonFlag(int iAreneID,int iX, int iY, int iW);
   static void SendMessageToAll(CString strMessage,DWORD dwColor, int iAreneID);
   static void SendMessageToTeam(CString strPlayerName,int iTeamID,CString strMessage, int iAreneID);
   static void AddTakeList(Players *pPlayer,Unit *pItemUnit, int iAreneID);
   static void RemTakeList(Players *pPlayer, int iAreneID);
   static int  GetNumberOfArene();
   static CString GetAreneName(int iAreneID);
   static int  GetSummonItemID(int iAreneID);
   static WorldPos GetRecallDeathTeam(int iTeamID,int iAreneID);
   static int  GetDeathWaitTimeMS(int iAreneID);
   static int  GetDeathWaitTimeS(int iAreneID);
   static int  GetMaxMinutes(int iAreneID);
   static int  GetMaxPoints(int iAreneID);
   static int  GetArenaStatus(int iAreneID);
   static int  GetArenaMinLevel(int iAreneID);
   static int  GetArenaMaxLevel(int iAreneID);
   static int  GetArenaWaitTimeGen(int iAreneID,int &iNbrSec,int &iNbrMin);
   
   
	
 
private:
   static void SendArenaWaitingList(Players *pPlayer, int iAreneID);
   static void SendPlayerFakeGroup(Players *pPlayer, int iAreneID);
   static void ResetPlayerArene(Players *pPlayer, int iAreneID);
   static void SendFlagTakeStatus(Players *pPlayer, char chCode, char chDecompte, int iAreneID,bool bForceZero = false);
      
   
private:
   static int              m_iNbrArena;
   static sArenaFlagGame  *m_pArenaList;
   
};


