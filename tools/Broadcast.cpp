/******************************************************************************
Modify for vs2008 (29/04/2009)
Add combat mode Support by Nightmare (30/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "Broadcast.h"
#include "TFC_MAIN.h"
#include "TFCPacket.h"
#include "SharedStructures.h"
#include "PacketManager.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
static sockaddr_in sockDummyAddr;

/******************************************************************************/
// Create the broadcasting class.
void Broadcast::Create( void )
/******************************************************************************/
{
    sockDummyAddr;
    memset( &sockDummyAddr, 0, sizeof( sockaddr_in ) );
}
/******************************************************************************/
// Broadcast a packet. This is the best function to broadcast.
void Broadcast::BCast(
                      WorldPos where,        // For local broadcasts
                      unsigned char range,   // Range of local broadcasts, 0 for global broadcast.
                      TFCPacket &sending,    // The packet to send,
                      SendPacketVisitor *packetVisitor,
                      bool bUselevelRange,
                      int iLevelMin,
                      int iLevelMax
                      )
/******************************************************************************/
{
    // Old protocol needed range to be 0 for global broadcasts.
    int nRange = range == 0 ? -1 : range;

    // Send message.
    CPacketManager::SendPacket(sending,sockDummyAddr,sockDummyAddr,nRange,where,TRUE,packetVisitor,true,bUselevelRange,iLevelMin,iLevelMax); //OK
}
/******************************************************************************/
// Broadcast an object changed.
void Broadcast::BCObjectChanged(
 WorldPos where,        // For local broadcasts
 unsigned char range,   // Range of local broadcasts, 0 for global broadcast.
 WORD  objectType,      // The new object type
 DWORD objectId,        // The object's ID.
 DWORD objDead,         // have a 0 is only chnage obj or 1 if the obj is now dead...
 SendPacketVisitor *packetVisitor
)
/******************************************************************************/
{
    TFCPacket sending;
    sending << (RQ_SIZE)__EVENT_OBJECT_CHANGED;
    sending << (short)objectType;
    sending << (long)objectId;
    sending << (long)objDead;

    BCast( where, range, sending, packetVisitor);
}
/******************************************************************************/
// Broadcast an object removal.
void Broadcast::BCObjectRemoved(
                                WorldPos where,        // For local broadcasts
                                unsigned char range,   // Range of local broadcasts, 0 for global broadcast.
                                DWORD objectId         // The object's ID.
                                )
/******************************************************************************/
{
   TFCPacket sending;
   sending << (RQ_SIZE)__EVENT_OBJECT_REMOVED;
   sending << (char)0; // how the object was removed (effect) = 0
   sending << (long)objectId; // ID of the object
   BCast( where, range, sending, NULL);
}
/******************************************************************************/
// Broadcast an attack.
void Broadcast::BCAttack(
 WorldPos where,        // For local broadcasts
 unsigned char range,   // Range of local broadcasts, 0 for global broadcast.
 DWORD attackerId,        // The attacker's ID.
 DWORD defenderId,        // The target's ID.
 char  hpPercentage,    // The Hp percentage.
 char  CombatMode,
 WorldPos attackerPos,  // The attacker's position.
 WorldPos defenderPos,  // The defender's position.
 SendPacketVisitor *packetVisitor
)
/******************************************************************************/
{
    TFCPacket sending;
    sending << (RQ_SIZE)__EVENT_ATTACK;
    sending << (long)attackerId; // ID of the attacker
    sending << (long)defenderId; // ID of the attacked
    sending << (char)0; // kind of attack used
    sending << (char)0; // kind of dodge used		        
    sending << (char)hpPercentage; // Pourcentage of   HP vs MaxHP
    sending << (char)CombatMode; // Pourcentage of   HP vs MaxHP
    sending << (short)attackerPos.X; // Attacker X pos
    sending << (short)attackerPos.Y; // Attacker Y pos
    sending << (short)defenderPos.X;
    sending << (short)defenderPos.Y;
	
    BCast( where, range, sending, packetVisitor );
}
/******************************************************************************/
// Broadcast a miss.
void Broadcast::BCMiss(
 WorldPos where,        // For local broadcasts
 unsigned char range,   // Range of local broadcasts, 0 for global broadcast.
 DWORD attackerId,      // The attacker's ID.
 DWORD defenderId,      // The defender's ID.
 WorldPos attackerPos,  // The attacker's position.
 WorldPos defenderPos,  // The defender's position.
 SendPacketVisitor *packetVisitor
)
/******************************************************************************/
{
    TFCPacket sending;
    sending << (RQ_SIZE)__EVENT_MISS;
    sending << (long)attackerId; // attacker's ID
    sending << (long)defenderId; // defender's ID
    sending << (short)attackerPos.X; // attacker X pos
    sending << (short)attackerPos.Y; // attacker Y pos
    sending << (short)defenderPos.X;
    sending << (short)defenderPos.Y;

    BCast( where, range, sending, packetVisitor );
}

/******************************************************************************/
// Broadcast a spell effect.
void Broadcast::BCSpellEffect(
 WorldPos where,        // For local broadcasts
 unsigned char range,   // Range of local broadcasts, 0 for global broadcast.
 WORD spellId,
 DWORD casterId,
 DWORD targetId,
 WorldPos casterPos,
 WorldPos targetPos,
 DWORD spellEffectId,
 DWORD spellChildId
)
/******************************************************************************/
{
    TFCPacket sending;
    sending << (RQ_SIZE)__EVENT_SPELL_EFFECT;
    sending << (short)spellId;  // SpellID
    sending << (long) casterId;  // Caster's ID.
    sending << (long) targetId;  // Target's ID.
    sending << (short)targetPos.X;  // Target XPos.
    sending << (short)targetPos.Y;  // Target YPos.
    sending << (short)casterPos.X;  // Caster's XPos.
    sending << (short)casterPos.Y;  // Caster's YPos.
    sending << (long)spellEffectId;  // The spell effect's unique ID.
    sending << (long)spellChildId;  // The spell effect's child effect ID.

    BCast( where, range, sending, NULL/*packetVisitor */);
}
/******************************************************************************/
// Broadcast a server message
void Broadcast::BCServerMessage(
 WorldPos where,        // For local broadcasts
 unsigned char range,   // Range of local broadcasts, 0 for global broadcast.
 CString serverMsg,
 SendPacketVisitor *packetVisitor,
 DWORD color,
 bool bUselevelRange,
 int iLevelMin,
 int iLevelMax
)
/******************************************************************************/
{
   TFCPacket sending;
   sending << (RQ_SIZE)RQ_ServerMessage;
   sending << (short)30;
   sending << (short)3;
   sending << serverMsg;
   sending << (long)color;

   BCast( where, range, sending, packetVisitor ,bUselevelRange,iLevelMin,iLevelMax);
}
/******************************************************************************/
// Broadcast a rain message
void Broadcast::BCWeatherMsg(
 WorldPos where,        // For local broadcasts
 unsigned char range,   // Range of local broadcasts, 0 for global broadcast.
 DWORD weatherEffect,   // Snow, Fog, Rain, ...
 short value   			// Value of the msg (on/off)
)
/******************************************************************************/
{
   TFCPacket sending;
   sending << (RQ_SIZE)RQ_WeatherMsg;
   sending << (long)weatherEffect;
   sending << (short)value;

   BCast( where, range, sending, NULL );
}