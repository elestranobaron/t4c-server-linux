/******************************************************************************
Modify for vs2008 (06/05/2009)
Add Combat Mode on BCAttack by Nightmare (06/27/2009)
/******************************************************************************/
#if !defined(AFX_BROADCAST_H__533D36A3_D2A7_11D0_B549_86A3B4000000__INCLUDED_)
#define AFX_BROADCAST_H__533D36A3_D2A7_11D0_B549_86A3B4000000__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "TFCPacket.h"
#include "SharedStructures.h"
#include "GameDefs.h"
#include "SendPacketVisitor.h"

/******************************************************************************/
class __declspec(dllexport) Broadcast  
/******************************************************************************/
{
public:
	static void Create();

   static void BCast(WorldPos where, unsigned char range, TFCPacket &packet, SendPacketVisitor *packetVisitor = NULL,
      bool bUselevelRange = false,int iLevelMin = 0,int iLevelMax = 0);

   static void BCObjectChanged( WorldPos where, unsigned char range, 
                                WORD  objectType,
                                DWORD objectId, 
                                DWORD objDead,
                                SendPacketVisitor *packetVisitor = NULL );

   static void BCObjectRemoved( WorldPos where, unsigned char range,DWORD objectId);

   static void BCAttack(WorldPos where, unsigned char range, 
                        DWORD attackerId, 
                        DWORD defenderId,        // The target's ID.
                        char  hpPercentage,    // The Hp percentage.
                        char  CombatMode,
                        WorldPos attackerPos,  // The attacker's position.
                        WorldPos defenderPos,  // The defender's position.
                        SendPacketVisitor *packetVisitor = NULL );

    static void BCMiss( WorldPos where, unsigned char range,
                    DWORD attackerId,
                    DWORD defenderId,      // The defender's ID.
                    WorldPos attackerPos,  // The attacker's position.
                    WorldPos defenderPos,  // The defender's position.
                    SendPacketVisitor *packetVisitor = NULL );

    static void BCSpellEffect( WorldPos where, unsigned char range,
                    WORD spellId,
                    DWORD casterId,
                    DWORD targetId,
                    WorldPos casterPos,
                    WorldPos targetPos,
                    DWORD spellEffectId,
                    DWORD spellChildId);

    static void BCServerMessage( WorldPos where, unsigned char range,
                    CString serverMsg,
                    SendPacketVisitor *packetVisitor = NULL,
					     DWORD color = CL_BLUE_LIGHT,
                    bool bUselevelRange = false,
                    int iLevelMin = 0,
                    int iLevelMax = 0);
	static void BCWeatherMsg( WorldPos where, unsigned char range, 
						   DWORD weatherEffect,
						   short value);

};

//extern Broadcast *__declspec(dllexport)bcast;

#endif // !defined(AFX_BROADCAST_H__533D36A3_D2A7_11D0_B549_86A3B4000000__INCLUDED_)
