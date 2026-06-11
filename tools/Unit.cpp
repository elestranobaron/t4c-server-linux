/******************************************************************************
Modify for vs2008 (26/04/2009)
Add Crime, Honnor, Invisibility 2, Combat Mode by NightMare (27/06/2009)
Add Firewall Check to test teleport Zone by Nightmare (27/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "Unit.h"
#include "TFC_MAIN.H"
#include "WorldMap.h"
#include "MonsterStructure.h"
#include "Random.h"
#include "_item.h"
#include "GameDefs.h"
#include "IntlText.h"
#include "SpellMessageHandler.h"
#include "QuestFlagsListing.h"
#include "SpellListing.h"
#include "PlayerManager.h"
#include "StatModifierFlagsListing2.h"
#include "NPCmacroScriptLng.h"
#ifdef _WIN32
#include <mmsystem.h>
#endif

// Maximum quantity of recursive calls to allow when querying effects.
// Effects may trigger other effects on the same unit, which can easely 
// cause stack overflows.
#define MAX_EFFECT_STACK_LEVELS 1
#define MAX_MESSAGE_STACK_LEVELS 1

#pragma warning(disable:4786)

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern TFC_MAIN *TFCServer;
extern CTFCServerApp theApp;

#define MAX_STARTUP_IDS				0x00100000  // 1048576 max static objects/NPCs
#define GLOBAL_ID_OFFSET			0x00100000

UINT							Unit::CurrentGlobalID	= GLOBAL_ID_OFFSET; 
BaseReferenceMessages *			        Unit::lpMessagesProc[65536];
TemplateList <Unit::UNIT_TYPE>	     Unit::tlUnitTypes;
std::map<DWORD,Unit *>                Unit::unit_map;
CLock                                 Unit::csUnitMap;
extern Random rnd;

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::InitializeMessagesProcs( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Initializes the message procs.
// 
//////////////////////////////////////////////////////////////////////////////////////////
{	
	int i;
	for( i = 0; i < 65536; i++ ){
		// No message handling by default
		lpMessagesProc[ i ] = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
WORD Unit::RegisterUnitMessageHandler
//////////////////////////////////////////////////////////////////////////////////////////
// Registers a message handler for message handling on a unit.
// 
(
 WORD wBaseReferenceID,					// Base reference ID of unit.
 BaseReferenceMessages *lpMessageUnit,	// Message handler.
 LPCTSTR lpszUnitName,					// Pointer to the name of the unit.
 BYTE bUnitType,						// Kind of unit to register
 BOOL boFindNextValidID,				// TRUE if function should assign the next valid ID.
 BOOL boForceRegistration,
 BOOL boOverwrite
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	if( wBaseReferenceID == 0 )
   {
		return 0;
	}

   bool bExcluded = false;
   //Look if this usit is EXCLUDED...
   for(int x=0;x<theApp.m_aNPCEXclusion.GetCount();x++)
   {
      if(_stricmp(theApp.m_aNPCEXclusion[x].GetBuffer(0),lpszUnitName)==0)
      {
         return 0; //this NPC is excluded...
      }
   }


   if(boOverwrite)
   {
      WORD wdID = GetIDFromName( lpszUnitName, 0, TRUE );
      if(wdID != 0)
      {
         _LOG_DEBUG LOG_DEBUG_LVL1, "Overwrite units... Unregister same global string ID %s.", lpszUnitName LOG_
         //Unregister old unit...
         UnregisterUnit(wdID);
      }
   }


	if( !boForceRegistration && GetIDFromName( lpszUnitName, 0, TRUE ) != 0 )
   {
		_LOG_DEBUG LOG_DEBUG_LVL1, "Two units are using the same global string ID %s.", lpszUnitName LOG_
		wBaseReferenceID = 0;
	}
   else
   {
		if( boFindNextValidID )
      {
			while( lpMessagesProc[ wBaseReferenceID ] )
         {
				wBaseReferenceID++;
			}
		}
		if( lpMessagesProc[ wBaseReferenceID ] == NULL )
      {
			_LOG_DEBUG
				LOG_DEBUG_HIGH,
				"Registering unit %u name %s type %u.",
				wBaseReferenceID,
				lpszUnitName,
				bUnitType
			LOG_
			
			UNIT_TYPE *lpUnitType = new UNIT_TYPE;
			lpUnitType->bUnitType = bUnitType;
			lpUnitType->csName = lpszUnitName;
			lpUnitType->wBaseReferenceID = wBaseReferenceID;

			lpMessageUnit->wBindedReferenceID = wBaseReferenceID;

			tlUnitTypes.AddToTail( lpUnitType );

			lpMessagesProc[ wBaseReferenceID ] = lpMessageUnit;
		}
      else
      {
			wBaseReferenceID = 0;
			_LOG_DEBUG LOG_DEBUG_LVL1, "Two units are using the same numerical ID %u. [%s]", 
								 wBaseReferenceID,
								 lpszUnitName
			LOG_
		}
	}

	//TRACE( "\r\nRegistering ID %u", wBaseReferenceID );

	return wBaseReferenceID;	// Returns the actual ID of the newly created unit.
}

//////////////////////////////////////////////////////////////////////////////////////////
BOOL Unit::GetNameFromID
//////////////////////////////////////////////////////////////////////////////////////////
// Returns the registered unit name from its static reference ID.
// 
(
 WORD wID,			// The static reference ID of the unit
 LPTSTR lpszName,	// The buffer where the name will be put.
 BYTE bUnitType		// The unit type to search for.
)
// Return: BOOL, TRUE if function found the name, FALSE otherwise.
//////////////////////////////////////////////////////////////////////////////////////////
{
	UNIT_TYPE *lpUnitType;
	
	tlUnitTypes.Lock();
	// Find the ID
	tlUnitTypes.ToHead();
	while( tlUnitTypes.QueryNext() ){
		lpUnitType = tlUnitTypes.Object();

		// If we found the kind of unit we were searching for.
		if( lpUnitType->bUnitType == bUnitType || bUnitType == 0 ){
			// If name matches the unit name ( case sensitive )
			if( lpUnitType->wBaseReferenceID == wID ){
            strcpy_s( lpszName,256, (LPCTSTR)lpUnitType->csName );
				//sprintf_s(lpszName,256,lpUnitType->csName );
				tlUnitTypes.Unlock();
				return TRUE;	// name found
			}

		}
	}
	tlUnitTypes.Unlock();

	return FALSE;	// Name not found
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::UnRegisterUnits( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Unregisters and cleans all registered units
// 
//////////////////////////////////////////////////////////////////////////////////////////
{
	tlUnitTypes.ToHead();
	while( tlUnitTypes.QueryNext() ){
		tlUnitTypes.DeleteAbsolute();
	}
   int i;
   for( i = 0; i < 65536; i++ )
   {
      // No message handling by default
      lpMessagesProc[ i ] = NULL;
   }
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::UnregisterUnit( WORD wBaseReferenceID )
{
    tlUnitTypes.ToHead();
    while( tlUnitTypes.QueryNext() ){
        UNIT_TYPE *unitType = tlUnitTypes.Object();
        if( unitType->wBaseReferenceID == wBaseReferenceID ){
            tlUnitTypes.DeleteAbsolute();
        }
    }

    lpMessagesProc[ wBaseReferenceID ] = NULL;
}

Unit *Unit::GetByID(DWORD id)
{
   csUnitMap.Lock();
   Unit *pUnit = NULL;
	std::map<DWORD, Unit*>::iterator it = unit_map.find(id);
	if (it != unit_map.end()) 
      pUnit = (*it).second;
   csUnitMap.Unlock();
   return pUnit;
}

UINT Unit::GetNbrUnitMap()
{
   return unit_map.size();
}

//////////////////////////////////////////////////////////////////////////////////////////
WORD Unit::GetIDFromName
//////////////////////////////////////////////////////////////////////////////////////////
// This fonction finds the base reference ID of a registered NPC using its textual name.
// 
(
 CString csName,			// The name of the NPC.
 BYTE bUnitType,			// Unit type to search for.
 BOOL boInsensitiveSearch	// TRUE if unit name search is case insensitive.
)
// Return: WORD, The base reference ID of the registered NPC, 0 if name wasn't found.
//////////////////////////////////////////////////////////////////////////////////////////
{
	UNIT_TYPE *lpUnitType;
	
	// Find the ID
	tlUnitTypes.Lock();
	tlUnitTypes.ToHead();
	while( tlUnitTypes.QueryNext() ){
		lpUnitType = tlUnitTypes.Object();

		// If we found the kind of unit we were searching for.
		if( lpUnitType->bUnitType == bUnitType || bUnitType == 0 ){
			// If name matches the unit name ( case sensitive )
			if( !boInsensitiveSearch ){
				if( lpUnitType->csName == csName ){
					tlUnitTypes.Unlock();
					return lpUnitType->wBaseReferenceID;
				}
			}else{
				if( _stricmp( (LPCTSTR)lpUnitType->csName, (LPCTSTR)csName ) == 0 ){
					tlUnitTypes.Unlock();
					return lpUnitType->wBaseReferenceID;
				}
			}

		}
	}
	tlUnitTypes.Unlock();
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////
void Unit::SendGlobalUnitMessage
//////////////////////////////////////////////////////////////////////////////////////////
// Sends a message to all units registered.
// 
(
 UINT MessageID,		// The ID of the message
 UNIT_FUNC_PROTOTYPE
)
/////////////////////////////////////////////////////////////////////////////////////////
{
	DWORD wI;

	// Send all units this message
	for( wI = 0; wI < 65536; wI++ ){
		try{
		if( lpMessagesProc[ wI ] ){
#ifdef CHANGED_MESSAGES
#error "Change messages in Unit::SendGlobalUnitMessage()"
#endif			
			switch( MessageID ){
			case MSG_OnAttack: 
					lpMessagesProc[ wI ]->OnAttack( UNIT_FUNC_PARAM );
				break;
			case MSG_OnAttacked:
					lpMessagesProc[ wI ]->OnAttacked( UNIT_FUNC_PARAM );
				break;
			case MSG_OnDisturbed:
					lpMessagesProc[ wI ]->OnDisturbed( UNIT_FUNC_PARAM );
				break;
			case MSG_OnUse:
					lpMessagesProc[ wI ]->OnUse( UNIT_FUNC_PARAM );
				break;
			case MSG_OnTimer:
					lpMessagesProc[ wI ]->OnTimer( UNIT_FUNC_PARAM );
				break;
			case MSG_OnInitialise:
					lpMessagesProc[ wI ]->OnInitialise( UNIT_FUNC_PARAM );
				break;
			case MSG_OnTalk:
					lpMessagesProc[ wI ]->OnTalk( UNIT_FUNC_PARAM );
				break;
			case MSG_OnQuerySchedule:
					lpMessagesProc[ wI ]->OnQuerySchedule( UNIT_FUNC_PARAM );
				break;
			case MSG_OnView:
					lpMessagesProc[ wI ]->OnView( UNIT_FUNC_PARAM );
				break;
			case MSG_OnNoMoreShots:
					lpMessagesProc[ wI ]->OnNoMoreShots( UNIT_FUNC_PARAM );
				break;
			case MSG_OnMove:
					lpMessagesProc[ wI ]->OnMove( UNIT_FUNC_PARAM );
				break;
			case MSG_OnDeath:
					lpMessagesProc[ wI ]->OnDeath( UNIT_FUNC_PARAM );
				break;
			case MSG_OnHit:
					lpMessagesProc[ wI ]->OnHit( UNIT_FUNC_PARAM );
				break;
			case MSG_OnEquip:
					lpMessagesProc[ wI ]->OnEquip( UNIT_FUNC_PARAM );
				break;
			case MSG_OnUnequip:
					lpMessagesProc[ wI ]->OnUnequip( UNIT_FUNC_PARAM );
				break;
			case MSG_OnServerInitialisation:
					lpMessagesProc[ wI ]->OnServerInitialisation( UNIT_FUNC_PARAM, (WORD)wI );
				break;
			case MSG_OnServerTermination:
					lpMessagesProc[ wI ]->OnServerTermination( UNIT_FUNC_PARAM );
				break;
			case MSG_OnNPCDataExchange:
					lpMessagesProc[ wI ]->OnNPCDataExchange( UNIT_FUNC_PARAM );
				break;
			case MSG_OnGetUnitStructure:
					lpMessagesProc[ wI ]->OnGetUnitStructure( UNIT_FUNC_PARAM );
				break;
			case MSG_OnPopup:
					lpMessagesProc[ wI ]->OnPopup( UNIT_FUNC_PARAM );
				break;
            case MSG_OnAttackHit:
                    lpMessagesProc[ wI ]->OnAttackHit( UNIT_FUNC_PARAM );
                break;
            case MSG_OnDestroy:
                    lpMessagesProc[ wI ]->OnDestroy( UNIT_FUNC_PARAM );
                break;
			}
		}
		}catch(...){
			_LOG_DEBUG
				LOG_CRIT_ERRORS,
				"Crashed sending message %u to structure %u (self=0x%x, medium=0x%x, target=0x%x, valueIN=0x%x, valueOUT=0x%x)",
				MessageID,
				wI,
				self, medium, target,
				valueIN, valueOUT
			LOG_
			throw;
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////
// Default message mapping
#ifdef CHANGED_MESSAGES
#error "Change messages in BaseReferenceMessages(), at start of Unit.cpp"
#endif

void BaseReferenceMessages::OnAttack( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnAttacked( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnDisturbed( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnUse( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnTimer( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnTalk( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnQuerySchedule( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnInitialise( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnView( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnNoMoreShots( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnMove( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnDeath( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnHit( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnEquip( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnUnequip( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnServerInitialisation( UNIT_FUNC_PROTOTYPE, WORD wBaseReferenceID ){};
void BaseReferenceMessages::OnServerTermination( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnNPCDataExchange( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnGetUnitStructure( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnPopup( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnAttackHit( UNIT_FUNC_PROTOTYPE ){};
void BaseReferenceMessages::OnDestroy( UNIT_FUNC_PROTOTYPE ){};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Unit::Unit( bool boDbgLock )
{
   queryInvisible.SetUnit( this );

   // Unit always has a reference count of 1 by default.
   dwRefCount = 1;

   // Unit isn't 'dead' by default.
   boDead = false;

   bStatus = 0;
   wlOriginalPos.X = wlOriginalPos.Y = wlOriginalPos.world = -1;

   WL.X = WL.Y = WL.world = 0;
   speed = 50; // default speed
   doing = nothing;
   ToBeKilled = TRUE;  // Only NPCs should not be killed by the system
   Bond = NULL;
   Target = NULL;
   GlobalID = 0xFFFFFFFF;
   wlUpperLimit.X = wlUpperLimit.Y = wlUpperLimit.world = -1;
   wlLowerLimit.X = wlLowerLimit.Y = wlLowerLimit.world = -1;

   lptlEffectList = new TemplateList <UNIT_EFFECT>;
   lptlBoostList  = new TemplateList <BOOST>;
   bBlock = bCurrentBlocking = __BLOCK_NONE;
   cRadiance = 0;
   wLevel = 0;

   m_dwImmobilizationCycle = 0;
   m_dwPrimalScreamCycle   = 0;

   UnitType = 0;

   m_dwLockedID = 0;

   StatINT = StatSTR = StatEND = StatAGI = StatWIL = StatWIS = StatLCK = StatDODGE = StatATTACK = 0;

   boInvisible = false;

   // Stack level for effects starts at 0.
   effectStackLevelDispell = 0;
   effectStackLevelQuery   = 0;
   messageStackLevel = 0;

   loadMentalExhaust = 0;
   loadMoveExhaust = 0;
   loadAttackExhaust = 0;

   userSpeed = 1;

}

Unit::~Unit()
{	
	if( GetType() == U_OBJECT )
   {
		ObjectTimer::RemoveObject( this );
	}
    
   LPUNIT_EFFECT lpEffect;
   if( lptlEffectList != NULL )
   {
      lptlEffectList->ToHead();
      while( lptlEffectList->QueryNext() )
      {
         lpEffect = lptlEffectList->Object();
         if( lpEffect->lpFunc )
         {
            lpEffect->lpFunc( MSG_OnDestroy, lpEffect->dwEffect, this, NULL, NULL, lpEffect->lpData, NULL );
         }
         lptlEffectList->DeleteAbsolute();
      }
      if (lptlEffectList != NULL)
      {
         delete lptlEffectList;
         lptlEffectList = NULL;
      }
   }

	if( lptlBoostList != NULL )
   {
		lptlBoostList->ToHead();
		while( lptlBoostList->QueryNext() )
      {
			lptlBoostList->DeleteAbsolute();
		}
		if (lptlBoostList != NULL)
		{
			delete lptlBoostList;
			lptlBoostList = NULL;
		}
	}

   //unit_map[GlobalID]=0;

   csUnitMap.Lock();
   std::map<DWORD, Unit*>::iterator it = unit_map.find(GlobalID);
   if (it != unit_map.end()) 
      unit_map.erase(it);
   csUnitMap.Unlock();
   

	//TRACE(_T("Destroyed a unit with address 0x%x "), this);
}

//////////////////////////////////////////////////////////////////////////////////////////
bool Unit::DeleteUnit( void )
//////////////////////////////////////////////////////////////////////////////////////////
//  Deletes a unit. If the unit has a zero reference count, it is removed from memory.
// 
// Return: bool, true if the unit was deleted from memory.
//////////////////////////////////////////////////////////////////////////////////////////
{    
    // Synchronize all unit deletions.
    static CLock globalUnitDeletionLock;
    CAutoLock globalLock( &globalUnitDeletionLock );

    
    dwRefCount--;
    if( dwRefCount == 0 )
    {
        SendUnitMessage( MSG_OnDestroy, this, NULL, NULL );
        // Use global operator delete.
        ::delete this;
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::CreateVirtualUnit( void )
//////////////////////////////////////////////////////////////////////////////////////////
//  Simply increase the reference count so next delete will only 'virtually' destroy
//  the unit.
// 
//////////////////////////////////////////////////////////////////////////////////////////
{
    dwRefCount++;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// This is a TIMERCALLBACK function which clean corpses after a while
/*
void Unit::CleanCorpse( TIMERCALLBACK_PROTOTYPE ){
	Unit *puCorpse = (Unit *)lpData;
	WorldPos wlPos = puCorpse->GetWL();
	
	WorldMap *wl = TFCMAIN::GetWorld(wlPos.world);
	
	if(wl)
   {
		wl->remove_world_unit(wlPos, puCorpse->GetID());

      Broadcast::BCObjectRemoved( wlPos, _DEFAULT_RANGE_REMOVE,puCorpse->GetID()); //clean corpse pas utoliser
	}
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
// Creates a unit (You MUST call this function!!)
BOOL Unit::Create(UINT Type, UINT Reference)
{
   // If a base message handler has been defined for this unit
   // or if this unit is a character (doesn't need a base message handler).

   if( lpMessagesProc[ Reference ] || Type == U_PC )
   {
      BOOL ReturnValue = TRUE;
      //void *StaticFlagAddr = NULL;

      // if CurrentGlobalID has reach the end of the string
      if(CurrentGlobalID == 0xFFFFFFFF)
      {
         // Then go back to the lowest offset of the global IDs and skip the static IDs
         CurrentGlobalID = GLOBAL_ID_OFFSET + MAX_STARTUP_IDS;
      }

      csUnitMap.Lock();
      unit_map[CurrentGlobalID] = this;
      csUnitMap.Unlock();

      SetID( CurrentGlobalID);
      CurrentGlobalID++;


      // Sets the basic stats of the unit
      UnitType = Type;
      Appearance = BaseReferenceID = Reference;

      // Then dispatch an initialisation message.	
      bool boInit = true;
      if( SendUnitMessage(MSG_OnInitialise, this, NULL, NULL, &boInit, NULL) )
      {
         ReturnValue = boInit;
         if( !boInit )
         {
            _LOG_DEBUG
               LOG_DEBUG_LVL1,
               "Failed to create unit ID %u (Type %u).", Reference, Type
               LOG_
         }
      }
      else
      {
         ReturnValue = false;
      }

      return ReturnValue;
   }
   BaseReferenceID = 0;
   return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
BOOL Unit::SendUnitMessage
//////////////////////////////////////////////////////////////////////////////////////////
// 
// 
(
 UINT MessageID,		// Message to send
 UNIT_FUNC_PROTOTYPE	// self/medium/target/valueIN/valueOUT
)
// Return: BOOL, TRUE if message could be sent
//////////////////////////////////////////////////////////////////////////////////////////
{
    // Returns FALSE if the dispatcher failed to find the static unit
	ASSERT( BaseReferenceID <= 65535 );
    if( BaseReferenceID > 65535 ){
		return FALSE;
	}

if( lpMessagesProc[ BaseReferenceID ] ){
#ifdef CHANGED_MESSAGES
#error "Change messages in Unit::SendUnitMessage()"
#endif			
		bMarker = 0;	// clears the marker before any message call.
		
		switch( MessageID ){
		case MSG_OnAttack: 
                if( messageStackLevel >= MAX_MESSAGE_STACK_LEVELS ){
                    return FALSE;
                }
                messageStackLevel++;
				lpMessagesProc[ BaseReferenceID ]->OnAttack( UNIT_FUNC_PARAM );
                messageStackLevel--;
			break;
		case MSG_OnAttacked:
                if( messageStackLevel >= MAX_MESSAGE_STACK_LEVELS ){
                    return FALSE;
                }
                messageStackLevel++;
				lpMessagesProc[ BaseReferenceID ]->OnAttacked( UNIT_FUNC_PARAM );
                messageStackLevel--;
			break;
		case MSG_OnDisturbed:
				lpMessagesProc[ BaseReferenceID ]->OnDisturbed( UNIT_FUNC_PARAM );
			break;
		case MSG_OnUse:
				lpMessagesProc[ BaseReferenceID ]->OnUse( UNIT_FUNC_PARAM );
			break;
		case MSG_OnTimer:
				lpMessagesProc[ BaseReferenceID ]->OnTimer( UNIT_FUNC_PARAM );
			break;
		case MSG_OnInitialise:
				lpMessagesProc[ BaseReferenceID ]->OnInitialise( UNIT_FUNC_PARAM );
			break;
		case MSG_OnTalk:
				lpMessagesProc[ BaseReferenceID ]->OnTalk( UNIT_FUNC_PARAM );
			break;
		case MSG_OnQuerySchedule:
				lpMessagesProc[ BaseReferenceID ]->OnQuerySchedule( UNIT_FUNC_PARAM );
			break;
		case MSG_OnView:
				lpMessagesProc[ BaseReferenceID ]->OnView( UNIT_FUNC_PARAM );
			break;
		case MSG_OnNoMoreShots:
				lpMessagesProc[ BaseReferenceID ]->OnNoMoreShots( UNIT_FUNC_PARAM );
			break;
		case MSG_OnMove:
				lpMessagesProc[ BaseReferenceID ]->OnMove( UNIT_FUNC_PARAM );
			break;
		case MSG_OnDeath:
				lpMessagesProc[ BaseReferenceID ]->OnDeath( UNIT_FUNC_PARAM );
			break;
		case MSG_OnHit:
                if( messageStackLevel >= MAX_MESSAGE_STACK_LEVELS ){
                    return FALSE;
                }
                messageStackLevel++;
				lpMessagesProc[ BaseReferenceID ]->OnHit( UNIT_FUNC_PARAM );
                messageStackLevel--;
			break;
		case MSG_OnEquip:
				lpMessagesProc[ BaseReferenceID ]->OnEquip( UNIT_FUNC_PARAM );
			break;
		case MSG_OnUnequip:
				lpMessagesProc[ BaseReferenceID ]->OnUnequip( UNIT_FUNC_PARAM );
			break;
		case MSG_OnNPCDataExchange:
				// We cannot allow this to happen when the units are not close enough!
				// BUY, SELL, TEACH, TRAIN and LEARN can only happen face to face!
				if (GetWL().AreInRange(target->GetWL(), 13) == FALSE) {
					return FALSE;
				}
				lpMessagesProc[ BaseReferenceID ]->OnNPCDataExchange( UNIT_FUNC_PARAM );
			break;
		case MSG_OnGetUnitStructure:
				lpMessagesProc[ BaseReferenceID ]->OnGetUnitStructure( UNIT_FUNC_PARAM );
			break;
		case MSG_OnPopup:
				lpMessagesProc[ BaseReferenceID ]->OnPopup( UNIT_FUNC_PARAM );
			break;
		case MSG_OnDestroy:
				lpMessagesProc[ BaseReferenceID ]->OnDestroy( UNIT_FUNC_PARAM );
			break;
        case MSG_OnAttackHit:
                if( messageStackLevel >= MAX_MESSAGE_STACK_LEVELS ){
                    return FALSE;
                }
                messageStackLevel++;
                lpMessagesProc[ BaseReferenceID ]->OnAttackHit( UNIT_FUNC_PARAM );
                messageStackLevel--;
            break;
        default:
            return FALSE;
            break;
		}
	}else{
        if( GetType() == U_HIVE || GetType() == U_MINIONS || GetType() == 0 ){
            return TRUE;
        }

        DWORD callerAddr;
        GET_CALLER_ADDR( callerAddr );

        // A message was sent to a unit which does not have a message handler.
        //ASSERT( false );

        /*
        _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "Tried to send unit message %u to unit %s, type %u staticID %u. Caller=0x%x.",
            MessageID,
            (LPCTSTR)GetName( _DEFAULT_LNG ),
            GetType(),
            BaseReferenceID,
            callerAddr
        LOG_
        */

		return FALSE;
	}

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns U_OBJECT, U_PC or U_NPC
char Unit::GetType(){
	return UnitType;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the position of the unit
WorldPos Unit::GetWL(){
	return WL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Sets the position of a unit (warning, this does not teleport the unit!)
void Unit::SetWL(WorldPos pos){
    // Unit position setting is critical.
    if( wlOriginalPos.X == -1 || wlOriginalPos.Y == -1 || wlOriginalPos.world == -1 ){
		wlOriginalPos = pos;
	}
	WL = pos;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the DWORD global ID of the unit
UINT Unit::GetID(){
	return GlobalID;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Sets the ID of the unit
void Unit::SetID(UINT ID)
{
	GlobalID = ID;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the reference of the unit
UINT Unit::GetStaticReference(){
	return BaseReferenceID;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Sends an 'attack' message to the base reference.
int Unit::attack(LPATTACK_STRUCTURE blow, Unit *Target){	
	int attackID = 0;	
	QueryEffects( MSG_OnAttack, blow, NULL, Target );
	SendUnitMessage(MSG_OnAttack, this, NULL, Target, blow, &attackID);
	blow->Strike += QueryBoost( STAT_DAMAGE );
return attackID;
}
////////////////////////////////////////////////////////////////////////////////////////////////////	
// Sends an 'attacked' message to the base reference.
int Unit::attacked(LPATTACK_STRUCTURE blow, Unit *Mechant){	
	int dodgeID;
	QueryEffects( MSG_OnAttacked, blow, NULL, Mechant );
	SendUnitMessage(MSG_OnAttacked, this, NULL, Mechant, blow, &dodgeID);
return dodgeID;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Sends a 'hit' message to the base reference.
int Unit::hit(LPATTACK_STRUCTURE blow, Unit *WhoHit){
	int status;
	QueryEffects( MSG_OnHit, blow, NULL, WhoHit );
	SendUnitMessage(MSG_OnHit, this, NULL, WhoHit, blow, &status);
return status;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
// Sent message when an attack successfully hits
int Unit::attack_hit(LPATTACK_STRUCTURE s_asBlow, Unit *lpuTarget){
    QueryEffects( MSG_OnAttackHit, s_asBlow, NULL, lpuTarget );
    SendUnitMessage( MSG_OnAttackHit, this, NULL, lpuTarget, s_asBlow );
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////
void Unit::Death( LPATTACK_STRUCTURE lpBlow, Unit *lpuKiller ){
}

//////////////////////////////////////////////////////////////////////////////////////////
CString Unit::GetName
//////////////////////////////////////////////////////////////////////////////////////////
//  Gets the name of a unit.
// 
(
 WORD wLang // The language in which to fetch the unit's name
)
// Return: CString, 
//////////////////////////////////////////////////////////////////////////////////////////
{
    char szName[ 256 ];        
    bool boNameFound = false;

	switch(GetType()){
	case U_NPC:{
		MonsterStructure *npc = NULL;
		SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &npc);
        if(npc){
            if( npc->name != NULL ){
                strcpy_s( szName, 256, npc->name );
                boNameFound = true;
            }else{
                szName[ 0 ] = 0;                
            }
        }else{
            return("[anonymous NPC]");
        }
	}break;
	case U_OBJECT:{
		_item *obj = NULL;
		SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &obj);
        if(obj){
            if( obj->name != NULL ){
                strcpy_s( szName, 256, obj->name );
                boNameFound = true;
            }else{
                szName[ 0 ] = 0;                
            }
        }else{
            return("[unnamed object]");
        }
	}break;
    default:
        szName[ 0 ] = 0;
    break;
	}
    
    // If a name was found.
    if( boNameFound ){
        // Check if it was defined in another language.
        return IntlText::ParseString( szName, wLang );
    }

    return szName;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// This function is implemented only for virtual purposes
void Unit::SetName(CString newname){
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// ID of the icon showed on the server
UINT Unit::GetAppearance(){
	return Appearance; // since usually the reference ID will be the same as the appearance
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Set the ID of the icon
void Unit::SetAppearance(UINT new_appearance){
	Appearance = new_appearance;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Only players need XP, this function is provided for virtual purposes
void Unit::SetXP(__int64 xp,bool bforce){
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Only players need XP, this function is provided for virtual purposes
__int64 Unit::GetXP(){
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////
// Only U_OBJECT and U_PC have backpacks, this function is provided for virtual purposes
TemplateList <Unit> *Unit::GetBackpack(){
	return NULL;
}
//////////////////////////////////////////////////////////////////////////////////////////////
// This function is provided for virtual purposes
void Unit::SetBackpack(TemplateList <Unit> *LList){	
}
//////////////////////////////////////////////////////////////////////////////////////////////
// This function is provided for virtual purposes
TemplateList <Unit> *Unit::GetEquipped(){
	return NULL;
}
//////////////////////////////////////////////////////////////////////////////////////////////
// This function is provided for virtual purposes
void Unit::SetEquipped(TemplateList <Unit> *LList){	
}
//////////////////////////////////////////////////////////////////////////////////////////////
// This function is provided for virtual purposes
EXHAUST Unit::GetExhaust(){
	EXHAUST tmp = {0,0,0};
	return tmp;
}
//////////////////////////////////////////////////////////////////////////////////////////////
// This function is provided for virtual purposes
void Unit::SetExhaust(EXHAUST newExhaust){
}
//////////////////////////////////////////////////////////////////////////////////////////////
// Returns current unit behavior (for U_NPCs and U_OBJECT)
BYTE Unit::IsDoing(){
	return doing;
}
//////////////////////////////////////////////////////////////////////////////////////////////
// Sets a unit behavior
void Unit::Do(BYTE newDoing,CString strFromWho)
{
	doing = newDoing;
	strDoFromWho = strFromWho;
}
//////////////////////////////////////////////////////////////////////////////////////////////
// This function is provided for virtual purposes
WorldPos Unit::Destination(){
	WorldPos dummy;
	dummy.X = 0;		// 0 is getting nowhere, -1 is wandering aimlessly
	dummy.Y = 0;
	dummy.world = 0;
	return dummy;
}
//////////////////////////////////////////////////////////////////////////////////////////////
// This function is provided for virtual purposes
void Unit::SetDestination(WorldPos newPos){
}
//////////////////////////////////////////////////////////////////////////////////////////////
// Moves a unit in the map
// Returns the new position of the unit
WorldPos Unit::MoveUnit(DIR::MOVE where, BOOL boAbsolute, bool boCompressMove, bool boBroadcastMove )
{	
   Lock();	

   WorldPos WL = GetWL();
   BOOL blocked = FALSE;	

   WorldPos CurrentWL = GetWL();

   

	if(!ViewFlag(__FLAG_STUN))
   {
		WorldMap *world = TFCMAIN::GetWorld(WL.world);
	
		switch(where)
		{
		   case DIR::north: 
				WL.Y -= userSpeed;
			break;
   		case DIR::northeast:
				WL.Y -= userSpeed;
				WL.X += userSpeed;
			break;
   		case DIR::east:
				WL.X += userSpeed;
			break;
   		case DIR::southeast:
				WL.Y += userSpeed;
				WL.X += userSpeed;
			break;
   		case DIR::south:
				WL.Y += userSpeed;
			break;
   		case DIR::southwest:
				WL.Y += userSpeed;
				WL.X -= userSpeed;
			break;
   		case DIR::west:
				WL.X -= userSpeed;
			break;
   		case DIR::northwest:
				WL.X -= userSpeed;
				WL.Y -= userSpeed;
			break;
		}

      boBroadcast = !boBroadcast;


      bool boSendMove = true;
      // If the user wants to use compressed movement sending.
      if( boCompressMove )
      {
         // Set move broadcasting to the current broadcast state (true or false).
         boSendMove = boBroadcast;
      }

      // If movement is NOT to be sent
      if( !boBroadcastMove  )
      {
         boSendMove = false;
      }        

		if( !world->move_world_unit(CurrentWL, WL, GetID(), (char)where, boAbsolute, boSendMove ) )
		{
			WL = CurrentWL;
		}
		else
		{	
			world->QueryEffects(WL, this); // Checks for any "area" effects that might affect what we stepped in
			WL = GetWL();	// Worldpos may have changed
			// search move effects
			QueryEffects( MSG_OnMove, NULL, NULL, NULL );
		}
			
	}
	else
	{
		WL = CurrentWL;
	}

    Unlock();

//	SetWL(WL);
return WL;
}
////////////////////////////////////////////////////////////////////////////////////////////
// Get the exhaust relative to the speed of the unit (when moving)
BYTE Unit::GetSpeedExhaust(){
	if(!speed) speed = 1;
	return (200 / speed); // returns speed in rounds of exhaust
}
////////////////////////////////////////////////////////////////////////////////////////
// Returns the speed (moves per 10 seconds)
BYTE Unit::GetSpeed(){
	return speed;
}
////////////////////////////////////////////////////////////////////////////////////////
// Sets a new speed (moves per 10 seconds)
void Unit::SetSpeed(BYTE newSpeed){
	speed = newSpeed;
}
/////////////////////////////////////////////////////////////////////////////////////////
// Units are _NEUTRAL by default, This function is provided for virtual purposes
char Unit::GetAgressivness(){
	return _NEUTRAL;
}
/////////////////////////////////////////////////////////////////////////////////////////
// This function is provided for virtual purposes
void Unit::SetAgressivness(char newAgressive){	
}
/////////////////////////////////////////////////////////////////////////////////////////
// This function is provided for virtual purposes
WORD Unit::GetClan(){
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
// This function is provided for virtual purposes
void Unit::SetClan(WORD newClan){
}
//////////////////////////////////////////////////////////////////////////////////////////////
// Returns the unit targetted (talk or fight) by the unit.
Unit *Unit::GetTarget(){
	return Target;
}
//////////////////////////////////////////////////////////////////////////////////////////////
// Sets a target (talk or fight, depending on what the unit DOes)
void Unit::SetTarget(Unit *newTarget){
	// If the target changed.
    if( Target != newTarget )
    {
        if( newTarget != NULL && ( newTarget->GetType() == U_OBJECT || newTarget->GetType() == U_HIVE || newTarget->GetType() == U_MINIONS) )
        {
            return;
        }

        if(newTarget != NULL && newTarget->IsDead())
        {
           return;
        }
        Target = newTarget;

        DWORD callerAddr = 0;
        __asm{
            push eax
            mov eax, dword ptr[ ebp + 4 ]
            mov callerAddr, eax
            pop eax
        };

        _LOG_DEBUG
            LOG_DEBUG_LVL4,
            "Unit %u 0x%u set target to 0x%x. ThreadId=%u Caller=0x%08x",
            GetID(),
            this,
            Target,
            GetCurrentThreadId(),
            callerAddr
        LOG_
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////
// Returns the corpse (Icon) of the unit
WORD Unit::GetCorpse(){
	return Corpse;
}
//////////////////////////////////////////////////////////////////////////////////////////////
// Sets a new corpse (Icon) for the unit
void Unit::SetCorpse(WORD newCorpse){
	Corpse = newCorpse;
}
////////////////////////////////////////////////////////////////////////////////////////////
// This function is provided for virtual purposes
UINT Unit::GetIdleTime(){
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////
// This function is provided for virtual purposes
void Unit::SetIdleTime(UINT newTime){
}
/////////////////////////////////////////////////////////////////////////////////////////
// This funtions returns TRUE if the system is allowed to kill the unit because it was idle
BOOL Unit::SystemDestroy(){
	return ToBeKilled;
}
/////////////////////////////////////////////////////////////////////////////////////////
// This functions sets the condition. See Unit::SystemDestroy()
void Unit::SetSystemDestroy(BOOL value){
	ToBeKilled = value;
}
/////////////////////////////////////////////////////////////////////////////////////////
// This functions is here only for virtual purposes
DWORD Unit::GetTrueMaxHP(){
	return 1;
}

DWORD Unit::GetMaxHP(){
    DWORD maxHP = GetTrueMaxHP();
	int boost = QueryBoost( STAT_MAX_HP );
    
    // If the boost give positive max HP
    if( (int)maxHP + boost > 0 ){
        maxHP += boost;
    }else{
        maxHP = 1;
    }
	
    return maxHP;
}

/////////////////////////////////////////////////////////////////////////////////////////
// This functions is here only for virtual purposes
void Unit::SetMaxHP(DWORD NewHP){
}

/////////////////////////////////////////////////////////////////////////////////////////
// This functions is here only for virtual purposes
DWORD Unit::GetHP(){
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
// This functions is here only for virtual purposes
void Unit::SetHP(DWORD NewHP, bool boUpdate ){	
}


/////////////////////////////////////////////////////////////////////////////////////////
// This functions is here only for virtual purposes
WORD Unit::GetTrueMaxMana(){
	return 1;
}

WORD Unit::GetMaxMana(){
    WORD maxMana = GetTrueMaxMana();
	int boost = QueryBoost( STAT_MAX_MANA );
    
    // If the boost give positive max HP
    if( (int)maxMana + boost > 0 ){
        maxMana += boost;
    }else{
        maxMana = 1;
    }
	
    return maxMana;
}

/////////////////////////////////////////////////////////////////////////////////////////
// This functions is here only for virtual purposes
void Unit::SetMaxMana(WORD NewHP){
}

/////////////////////////////////////////////////////////////////////////////////////////
// This functions is here only for virtual purposes
WORD Unit::GetMana(){
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
// This functions is here only for virtual purposes
void Unit::SetMana(WORD NewHP, BOOL boEcho ){	
}

/////////////////////////////////////////////////////////////////////////////////////////
// This functions is provided for virtual purposes
int Unit::GetGold(){
	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
// This functions is provided for virtual purposes
void Unit::SetGold(int newGold, BOOL boEcho ){
}

///////////////////////////////////////////////////////////////////////////////////////
// This function teleports a unit anywhere in the world.
BOOL Unit::Teleport(WorldPos to, BYTE How,BOOL bByPassAreneBlock)
{
   BOOL bBlockTP = FALSE;
   if( UnitType == U_PC )
   {
      //si on est en arene et que la team est creer c que larene est LANCE...
      Character *ThisCharacter = static_cast<Character *>( this );
      if(ThisCharacter->GetArenaID() >0 && ThisCharacter->GetArenaTeam() > 0 &&!bByPassAreneBlock)
      {
         if(ThisCharacter->GetArenaType() ==ARENE1_TYPE)
         {
            //on valide que le TO est dans la meem zone arene que le current...
            list< sCombatArenaLocation >::iterator itA = theApp.CombatArenaLocationList1.begin();
            for(int i=0;i<ThisCharacter->GetArenaID()-1;i++)
               itA++;

            if(to.X >= (*itA).wlTopLeft.X && to.X <= (*itA).wlBottomRight.X &&
               to.Y >= (*itA).wlTopLeft.Y && to.Y <= (*itA).wlBottomRight.Y && to.world == (*itA).wlTopLeft.world)
            {
               //OKI dans la zone
            }
            else
            {
               bBlockTP = TRUE;
            }
         }
         else if(ThisCharacter->GetArenaType() ==ARENE2_TYPE)
         {
            //on valide que le TO est dans la meem zone arene que le current...
            list< sCombatArenaLocation >::iterator itA = theApp.CombatArenaLocationList2.begin();
            for(int i=0;i<ThisCharacter->GetArenaID()-1;i++)
               itA++;

            if(to.X >= (*itA).wlTopLeft.X && to.X <= (*itA).wlBottomRight.X &&
               to.Y >= (*itA).wlTopLeft.Y && to.Y <= (*itA).wlBottomRight.Y && to.world == (*itA).wlTopLeft.world)
            {
               //OKI dans la zone
            }
            else
            {
               bBlockTP = TRUE;
            }
         }
         
      }
   }

   if(bBlockTP)
      return FALSE;


   WorldPos from = GetWL();
   WorldMap *SourceWorld = TFCMAIN::GetWorld( from.world );
   WorldMap *TargetWorld = TFCMAIN::GetWorld( to.world );



   //	TRACE("Teleporting from ( %u %u %u ) to ( %u %u %u )\r\n",from.X, from.Y, from.world, to.X, to.Y, to.world);
   _LOG_WORLD
      LOG_DEBUG_LVL1,
         "Unit %s teleported from ( %u, %u, %u ) to ( %u, %u, %u ) addr( 0x%x )",
         (LPCTSTR)GetName( _DEFAULT_LNG ),
         from.X,
         from.Y,
         from.world,
         to.X,
         to.Y,
         to.world,
         this
      LOG_

      Lock();
   if(SourceWorld && TargetWorld)
   {
      if(SourceWorld->IsValidPosition(from) && TargetWorld->IsValidPosition(to))
      {

         SetLastTeleport( to );

         SourceWorld->remove_world_unit(from, GetID());			

         SetWL( to );

         // then send a broadcast..
         Broadcast::BCObjectRemoved( from, _DEFAULT_RANGE_REMOVE, GetID()); //quand on se teleporte

         // Only handle this part if its a PC which entered the teleport
         if( UnitType == U_PC ){
            TFCPacket sending;

            sending << (RQ_SIZE)RQ_TeleportPlayer;
            sending << (short)to.X;
            sending << (short)to.Y;
            sending << (short)to.world;
            SendPlayerMessage( sending );

            // Get the Character structure
            Character *ThisCharacter = static_cast<Character *>( this );

            // Stop the AutoCombat to prevent possibility of xp
            // if player isn't on the WorldMap
            ThisCharacter->StopAutoCombat();

            

            {
               Players *pPlayer = (Players *)ThisCharacter->GetPlayer();

               if( pPlayer != NULL )
               {
                  //NMNMNM???... on flush toute les reference de ce joueur car il quitte le monde un court instant...
                  CPlayerManager::RemoveTargetReferences( pPlayer->self ,false);
                  pPlayer->self->SetTarget(NULL);

                  //theApp.ValidTeleportPlayer(pPlayer); //NMNMNM a mettre peut etre plus tard...

                  // Remove player from the game and put it in pre-in game waiting state.
                  pPlayer->in_game = FALSE;
                  pPlayer->boPreInGame = TRUE;

                  // If the god doesn't have the GOD_NO_MONSTERS flag.
                  if( !( pPlayer->GetGodFlags() & GOD_NO_MONSTERS ) ){
                     TargetWorld->VerifyInviewHives( to );
                  }
               }
            }

            sending.Destroy();
            int read;
            read = TargetWorld->packet_inview_units( to, sending, _DEFAULT_RANGE, this );
            // If items were packetted.
            if( read > 0 ){								
               SendPlayerMessage( sending );
            }
         }else{
            TargetWorld->deposit_unit( to, this );

            // If unit isn't a player teleport it rightaway
            BroadcastPopup( to );
         }

         Unlock();
         return TRUE;
      }

   }
   Unlock();
   return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////
// These function set "bonds" (a unit to which they "belong")
Unit *Unit::GetBond(){return Bond;};
/////////////////////////////////////////////////////////////////////////////////////////
void Unit::SetBond(Unit *newBond){
	Bond = newBond;
};


///////////////////////////////////////////////////////////////////////////////////////////
// This functions returns TRUE if wlPos is whithin the limit of the wlUpper/LowerLimit.
BOOL Unit::IsInLimit(WorldPos wlPos){
	// If there is no limit (-1) then return TRUE
	if(wlLowerLimit.X == -1 || wlLowerLimit.Y == -1 || wlUpperLimit.X == -1 || wlUpperLimit.Y== 1)
		return TRUE;

	// else check if it is whitin the limit "box"
	if(wlPos.X >= wlLowerLimit.X && wlPos.X <= wlUpperLimit.X && 
		wlPos.Y >= wlLowerLimit.Y && wlPos.Y <= wlUpperLimit.Y) return TRUE;

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////////////////////
// This functions set the lower world limit of the unit
void Unit::SetLowerLimit(WorldPos wlPos){
	wlLowerLimit = wlPos;
}
/////////////////////////////////////////////////////////////////////////////////////////////
// This functions set the upper world limit 
void Unit::SetUpperLimit(WorldPos wlPos){
	wlUpperLimit = wlPos;
}
////////////////////////////////////////////////////////////////////////////////////////////
// Sets the destination of the monster to the center of it's limit
void Unit::GotoLimit(void){	
	WorldPos wlNewDest = {((wlUpperLimit.X - wlLowerLimit.X) / 2) + wlLowerLimit.X,
						  ((wlUpperLimit.Y - wlLowerLimit.Y) / 2) + wlLowerLimit.Y, 
							GetWL().world};
	
	SetDestination(wlNewDest);
}

////////////////////////////////////////////////////////////////////////////////////////////
// this function should return the ThisPlayer member of a Character (virtual)
void *Unit::GetPlayer(){return NULL;};

///////////////////////////////////////////////////////////////////////////////////////////
// This function, well, trains a unit :) virtual function
void Unit::CheckIFLevelUP(){}

/////////////////////////////////////////////////////////////////////////////////////////////
// These function get and set the level
WORD Unit::GetLevel(){
	return wLevel;
}
void Unit::SetLevel(WORD wNewLevel){
	wLevel = wNewLevel;
}
/////////////////////////////////////////////////////////////////////////////////////////////
// This function returns the class of the unit, U_NPC and _PLAYER only..
BYTE Unit::GetClass(){
	return nClass;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Returns the class points of a unit, for U_PC only
LPWORD Unit::GetClassPoints(){
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////
// virtual fonction
void Unit::SendPlayerMessage(TFCPacket &sending){
}

//NMNMNM_XP_NEW MEthod
/////////////////////////////////////////////////////////////////////////////////////////////
// virtual fonction
void Unit::SendPlayerXP(bool bForceUpdate)
{

}

//////////////////////////////////////////////////////////////////////////////////////////////
// virtual function
BOOL Unit::UseSkillPnts(WORD wQuantity){return FALSE;};
BOOL Unit::UseStatPnts(WORD wQuantity){return FALSE;};

BOOL Unit::UseSkill(int nID, Unit *target, LPVOID lpValueOUT ){return FALSE;};

BOOL Unit::UseSkill(int nID, WorldPos wlPos){return FALSE;};

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::Talk
//////////////////////////////////////////////////////////////////////////////////////////
// Allows a unit, whichever type, to talk
// 
(
 LPCTSTR Message, // Message to say.
 DWORD dwColor,   // Color of message.
 Unit *lpuTalkTo  // Unit to who we talk to.
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	BYTE bDirection = 0;	
	signed int Xoff, Yoff;

	if( lpuTalkTo ){
		WorldPos wlUs   = GetWL();
		WorldPos wlThem = lpuTalkTo->GetWL();
		
		Xoff = (wlThem.X - wlUs.X + 11) * 3;
		Yoff = (wlThem.Y - wlUs.Y + 16) * 2;
	
		if( Yoff > 30 ){
			if( Xoff > 30 ){
				Xoff -= 30;
				Yoff -= 30;
				if( Xoff > 2 * Yoff ){
					bDirection = KP_EAST;
				}else if( Yoff > Xoff * 2 ){
					bDirection = KP_SOUTH;
				}else{
					bDirection = KP_SOUTHEAST;
				}
			}else{
				Yoff -= 30;
				Xoff = 30 - Xoff;
				if( Xoff > 2*Yoff ){
					bDirection = KP_WEST;
				}else if( Yoff > 2 * Xoff ){
					bDirection = KP_SOUTH;
				}else {
					bDirection = KP_SOUTHWEST;
				}
			}
		}else{
			if( Xoff > 30 ){
				Xoff -= 30;
				Yoff = 30 - Yoff;
				if( Xoff > 2 * Yoff ){
					bDirection = KP_EAST;
				}else if( Yoff > Xoff * 2 ){
					bDirection = KP_NORTH;
				}else{
					bDirection = KP_NORTHEAST;
				}
			}else{
				Yoff = 30 - Yoff;
				Xoff = 30 - Xoff;
				if( Xoff > 2*Yoff ){
					bDirection = KP_WEST;
				}else if( Yoff > 2 * Xoff ){
					bDirection = KP_NORTH;
				}else {
					bDirection = KP_NORTHWEST;
				}
			}
		}
	}

    CString msg( Message );

	DWORD dwNameColor = CL_RED;
    TFCPacket sending;
    sending << (RQ_SIZE)__EVENT_SHOUT;
    sending << (long)GetID();
    sending << (char)bDirection;
    sending << (long)dwColor;
    if( GetType() != U_PC )
    {
       sending << (char)1; // an NPC.
       dwNameColor = U_NPC_COLOR;
    }
    else
    {
       sending << (char)0; // not an NPC.
       Players *TargetPlayer = static_cast< Players * >(GetPlayer());
       dwNameColor = TargetPlayer->self->ViewFlag(__FLAG_UNIT_COLOR);
       if(!dwNameColor)
       {
          if(theApp.m_dwPVPSyetem2Actif == 1) //PVP SYSTEM
          {
             if ( TargetPlayer->IsGod() ) 
             {
                dwNameColor = U_GOD_COLOR
             }
             else
             {
                if(TargetPlayer->self->GetCrime() == TargetPlayer->self->GetHonor())
                   dwNameColor = U_PCRP_COLOR
                else if(TargetPlayer->self->GetCrime() > TargetPlayer->self->GetHonor())
                  dwNameColor = U_PC_COLOR
                else
                  dwNameColor = U_PCRP_COLOR
             }
          }
          else
          {
             if ( TargetPlayer->IsGod() ) 
                dwNameColor = U_GOD_COLOR
             else if(TargetPlayer->self->ViewFlag(__FLAG_RPHRP_STATUS) == 1)
                dwNameColor = U_PCRP_COLOR
             else
                dwNameColor = U_PC_COLOR
          }
       }
    }
    sending << msg;
    sending << GetName( GetLang() );
	sending << (long)dwNameColor;
	
	//TRACE( "\r\nSent color %u and direction %u\r\n", dwColor, bDirection );

    Broadcast::BCast( GetWL(), _DEFAULT_RANGE, sending );
}

// virtual
void Unit::SetDetectRange( BYTE ){};
BYTE Unit::GetDetectRange( void ){ return 0; };


//////////////////////////////////////////////////////////////////////////////////////////
Unit *Unit::GetIndentItem
//////////////////////////////////////////////////////////////////////////////////////////
// Returns the item content of the item within the item whithin the item [...]
// of the indent item list.
// 
(
 DWORD *lpdwIndent
)
// Return: TemplateList <Unit>, The content of the last item in the indent list.
//////////////////////////////////////////////////////////////////////////////////////////
{				
	TemplateList <Unit> *tluBackpack;
	Unit *lpuUnit = this;
	BOOL boFound = TRUE;

	// If we have a null hierarchy list
	if( lpdwIndent[ 0 ] == 0 ){
		// return the user as the backpack owner
		return this;
	}

	int i = 0;
	if( lpdwIndent && lpuUnit ){

		// Start of indent list	
		do{
			// Get the backpack of the current item 
			tluBackpack = lpuUnit->GetBackpack();
			
			boFound = FALSE;
			// if it has a backpack
			if( tluBackpack ){
				// searches in the backpack		
				tluBackpack->Lock();
				tluBackpack->ToHead();
				while( tluBackpack->QueryNext() && !boFound ){
					// If this backpack has the next item in the indent list
					if( tluBackpack->Object()->GetID() == lpdwIndent[i] ){
						boFound = TRUE;
						lpuUnit = tluBackpack->Object();
					}
				}
				tluBackpack->Unlock();
			}
			i++;
		}while( lpdwIndent[i] && boFound );
	}

	if( boFound ){
		// packet the last units backpack
		return lpuUnit;
	}else{
		return NULL;
	}

}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::PacketBackpack
//////////////////////////////////////////////////////////////////////////////////////////
// Puts the content of the unit's backpack into a packet. Does NOT add the leading 
// packet type ID.
//
(
 TFCPacket &sending, bool gameop// The packet.
)
//////////////////////////////////////////////////////////////////////////////////////////
{
}
//////////////////////////////////////////////////////////////////////////////////////////
double Unit::GetAC( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Unit's AC
// 
// Return: WORD, the AC.
//////////////////////////////////////////////////////////////////////////////////////////
{
    double dblValue = QueryBoost( STAT_AC );
    
    switch(GetType()){
	case U_OBJECT:{
		_item *obj = NULL;
		SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &obj);
		if(obj){
			double ret = obj->armor.AC + dblValue;
            if( ret <= 0 ){ return 0; }else{ return ret; };
		}
    }break;
    case U_NPC:{
        MonsterStructure *monster = NULL;
        SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &monster );
        if( monster ){
			double ret = monster->AC + dblValue;
            if( ret <= 0 ){ return 0; }else{ return ret; };
        }
    } break;	
	}

return( 0 );
}

//////////////////////////////////////////////////////////////////////////////////////////
double Unit::GetParryPC( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Unit's Parry's points
// 
// Return: WORD, the Parry's points of the objet.
//////////////////////////////////////////////////////////////////////////////////////////
{
    //double dblValue = QueryBoost( STAT_AC );
	double dblValue = 0;
    
    switch(GetType()){
	case U_OBJECT:{
		_item *obj = NULL;
		SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &obj);
		if(obj){
			double ret = obj->armor.dParadePC + dblValue;
            if( ret <= 0 ){ return 0; }else{ return ret; };
		}
    }break;
	}

return( 0 );
}



//////////////////////////////////////////////////////////////////////////////////////////
LPUSER_SKILL Unit::GetSkill
//////////////////////////////////////////////////////////////////////////////////////////
// This function returns the user-skill associated to the player.
// 
(
 DWORD dwSkill // The skill to query.
)
{ return NULL; }

//////////////////////////////////////////////////////////////////////////////////////////
WORD Unit::GetSkillPoints( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Returns the number of skill points
//
{ return 0; };

void Unit::SetLastTeleport( WorldPos WL ){
}

WorldPos Unit::GetLastTeleport( void ){
	WorldPos wlPos = { 0, 0, 0 };
	return wlPos;
}

//////////////////////////////////////////////////////////////////////////////////////////
WorldPos Unit::OriginalWorldPos( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Returns the first set position of the unit.
// 
// Return: WorldPos, The first set position of the unit.
//////////////////////////////////////////////////////////////////////////////////////////
{
	return wlOriginalPos;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Adds a spell effect to a unit.
// 
void Unit::AddEffect(LPUNIT_EFFECT lpSpellEffect)
{
	Lock();
   if( lptlEffectList != NULL )
   {
	   lptlEffectList->Lock();
	   lptlEffectList->AddToTail( lpSpellEffect );
	   lptlEffectList->Unlock();
   }
   Unlock();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Removes a specific effect from a unit.
// 
void Unit::RemoveEffect(DWORD dwEffect,bool bBroadcast)
{
	Lock();
	if( lptlEffectList != NULL )
   {
		LPUNIT_EFFECT lpEffect;
		lptlEffectList->Lock();
		lptlEffectList->ToHead();
		while( lptlEffectList->QueryNext() )
      {
			lpEffect = lptlEffectList->Object();
			if( lpEffect->dwEffect == dwEffect )
         {
				// Send a destroy message to the effect
				if( lpEffect->lpFunc && bBroadcast)
            {
					lpEffect->lpFunc( MSG_OnDestroy, dwEffect, this, NULL, NULL, lpEffect->lpData, NULL );
				}
				// Destroy the effect item
				lptlEffectList->DeleteAbsolute();
			}
		}
		lptlEffectList->Unlock();
	}
	Unlock();
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//  Dispells an effect which has the specified binded flag
void Unit::DispellEffectWithFlag(DWORD flagId)
{
   CAutoLock autoLock( this ); 

   if( lptlEffectList == NULL )
      return;

   // If the functions hasn't been recursively called too many times.
   if( effectStackLevelDispell >= MAX_EFFECT_STACK_LEVELS )
      return;
   effectStackLevelDispell++; // Function called, augment stack level.


   lptlEffectList->Lock();
   lptlEffectList->ToHead();
   while( lptlEffectList->QueryNext() )
   {
      UNIT_EFFECT *effect = lptlEffectList->Object();
      if( effect->bindedFlag == flagId )
      {
         if( effect->lpFunc )
         {
            effect->lpFunc( MSG_OnDispell, effect->dwEffect, this, NULL, NULL, effect->lpData, NULL );
            lptlEffectList->DeleteAbsolute();
         }
      }
   }
   lptlEffectList->Unlock();
   // Function is ending, decrease stack level.
   effectStackLevelDispell--;	

#if 0
   list< UNIT_EFFECT * > dispellListFlag;

   lptlEffectList->Lock();
   lptlEffectList->ToHead();

   while( lptlEffectList->QueryNext() )
   {
      UNIT_EFFECT *effect = lptlEffectList->Object();
      if( effect->bindedFlag == flagId )
      {
         if( effect->lpFunc )
         {
            dispellListFlag.push_back( effect );
         }
      }
   }
   lptlEffectList->Unlock();

   list< UNIT_EFFECT * >::iterator i;
   for( i = dispellListFlag.begin(); i != dispellListFlag.end(); i++ )
   {
      // Send a dispell message to the effect.
      (*i)->lpFunc( MSG_OnDispell, (*i)->dwEffect, this, NULL, NULL, (*i)->lpData, NULL );

   }
   dispellListFlag.clear();

   // Function is ending, decrease stack level.
   effectStackLevelDispell--;	
   #endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// Returns a specific effect.
// Return: LPUNIT_EFFECT, the returned spell effect, NULL if none found.
//////////////////////////////////////////////////////////////////////////////////////////
LPUNIT_EFFECT Unit::GetEffect(DWORD dwEffect)
{
	Lock();
	LPUNIT_EFFECT lpEffect = NULL;
	if( lptlEffectList != NULL )
   {
		BOOL boFound = FALSE;
		lptlEffectList->Lock();
		lptlEffectList->ToHead();
		while( lptlEffectList->QueryNext() && !boFound )
      {
			lpEffect = lptlEffectList->Object();
			if( lpEffect->dwEffect == dwEffect )
         {
				boFound = TRUE;
			}
		}
		lptlEffectList->Unlock();

		if( !boFound )
      {
			lpEffect = NULL;
		}
	}
	Unlock();

	return lpEffect;
}
//////////////////////////////////////////////////////////////////////////////////////////
// Returns the list of all spells on a unit
// 
// Return: TemplateList<UNIT_EFFECT>*, pointer to this list. NULL if no effects
//////////////////////////////////////////////////////////////////////////////////////////
TemplateList<UNIT_EFFECT>* Unit::GetAllEffects( void )
{
	return lptlEffectList;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Destroys every effects on the unit.
// 
//////////////////////////////////////////////////////////////////////////////////////////
void Unit::DestroyEffects( void )
{
	Lock();

	LPUNIT_EFFECT lpEffect;

	if( lptlEffectList != NULL )
   {
		lptlEffectList->Lock();
		lptlEffectList->ToHead();
		while( lptlEffectList->QueryNext() )
      {
			lpEffect = lptlEffectList->Object();
			if( lpEffect->lpFunc )
         {
				lpEffect->lpFunc( MSG_OnDestroy, lpEffect->dwEffect, this, NULL, NULL, lpEffect->lpData, NULL );
			}
			// Destroy the effect item
			lptlEffectList->DeleteAbsolute();
		}
		lptlEffectList->Unlock();
	}
	Unlock();
}

void Unit::DispellEffects(Unit *caster, Unit *target, DWORD dwEffectID)
/******************************************************************************/
{
   Lock();
   // Get the target's effects
   if( lptlEffectList != NULL )
   {
      lptlEffectList->Lock();
      lptlEffectList->ToHead();
      while( lptlEffectList->QueryNext() )
      {
         UNIT_EFFECT *effect = lptlEffectList->Object();
         // If the spell ID was found
         if( effect->bindedSpellID == dwEffectID )
         {
            // Send a dispell message to the effect
            effect->lpFunc( MSG_OnDispell, effect->dwEffect, target, NULL, caster, effect->lpData, NULL );
            lptlEffectList->DeleteAbsolute();
         }        
      }
      lptlEffectList->Unlock();
   }
   Unlock();

   #if 0
   Lock();
   // Get the target's effects
   if( lptlEffectList != NULL )
   {
      list< UNIT_EFFECT * > dispellList;

      lptlEffectList->Lock();
      lptlEffectList->ToHead();
      while( lptlEffectList->QueryNext() )
      {
         UNIT_EFFECT *effect = lptlEffectList->Object();
         // If the spell ID was found
         if( effect->bindedSpellID == dwEffectID )
         {
            // Send a dispell message to the effect
            dispellList.push_back( effect );
         }        
      }
      lptlEffectList->Unlock();

      list< UNIT_EFFECT * >::iterator i;
      for( i = dispellList.begin(); i != dispellList.end(); i++ )
      {
         // Send a dispell message to the effect.
         (*i)->lpFunc( MSG_OnDispell, (*i)->dwEffect, target, NULL, caster, (*i)->lpData, NULL );
      }
      dispellList.clear();
   }
   #endif
   
}

//////////////////////////////////////////////////////////////////////////////////////////
// Sends a message to all effects on a unit.
// 
void Unit::SendGlobalEffectMessage(BYTE bMsgType,LPVOID lpMsgData,Unit *medium,Unit *target)
{
	try
   {
      list< UNIT_EFFECT * > SendList;
		Lock();
		if( lptlEffectList != NULL )
      {
			lptlEffectList->Lock();

			// Add the effect to the call list.
			lptlEffectList->ToHead();
			while( lptlEffectList->QueryNext() )
         {		
            UNIT_EFFECT *effect = lptlEffectList->Object();
            SendList.push_back(effect);
			}
			lptlEffectList->Unlock();
		}    

      list< UNIT_EFFECT * >::iterator i;
      for( i = SendList.begin(); i != SendList.end(); i++ )
      {
         if ( (*i)->lpFunc == NULL )
         {	//DC for GPs
            _LOG_DEBUG
               LOG_CRIT_ERRORS,
               "DC - GPs - lpEffect->lpFunc is a null pointer while SendGlobalEffectMessage."
               LOG_
         }
         else
         {								//DC for GPs
            (*i)->lpFunc( bMsgType, (*i)->dwEffect, this, NULL, NULL, (*i)->lpData, NULL );
            if(bMsgType == MSG_OnDispell)
               RemoveEffect((*i)->dwEffect,true);
         }
      }
      SendList.clear();
	   Unlock();
	} 
   catch(...)
   {
		_LOG_DEBUG LOG_CRIT_ERRORS, "Crashed at Unit::SendGlobalEffectMessage" LOG_
		throw;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Queries all the effects and sends them the specific message.
// 
void Unit::QueryEffects(BYTE bMsgType,LPVOID lpUserData,Unit *medium,Unit *target)
{
   CAutoLock autoLock( this );

   // If the functions hasn't been recursively called too many times.
   if( effectStackLevelQuery < MAX_EFFECT_STACK_LEVELS )
   {
      effectStackLevelQuery++; // Function called, augment stack level.

      // Query all attack effects	
      if( lptlEffectList )
      {
         list< UNIT_EFFECT * > QueryEffectList;
         lptlEffectList->Lock();
         lptlEffectList->ToHead();
         while( lptlEffectList->QueryNext() )
         {
            UNIT_EFFECT *effect = lptlEffectList->Object();
            if( effect->bEffectType == bMsgType )
            {
               if( effect->lpFunc )
               {
                  QueryEffectList.push_back( effect );
               }
            }
         }
         lptlEffectList->Unlock();

         list< UNIT_EFFECT * >::iterator i;
         for( i = QueryEffectList.begin(); i != QueryEffectList.end(); i++ )
         {
            if((*i) == NULL) ////DC for GPs
            {
               _LOG_DEBUG
                  LOG_CRIT_ERRORS,
                  "DC - GPs - lpEffect is a null pointer while QueryEffects."
                  LOG_
            }
            else if ( (*i)->lpFunc == NULL )//DC for GPs
            {	
               _LOG_DEBUG
                  LOG_CRIT_ERRORS,
                  "DC - GPs - lpEffect->lpFunc is a null pointer while QueryEffects."
                  LOG_
            }
            else
            {	//DC for GPs
               try
               {
                  (*i)->lpFunc( bMsgType, (*i)->dwEffect, this, medium, target, (*i)->lpData, lpUserData );
                  if(bMsgType == MSG_OnDispell)
                     RemoveEffect((*i)->dwEffect,true);
               }
               catch(...)
               {
                  _LOG_DEBUG
                     LOG_CRIT_ERRORS,
                     "DC - GPs - lpEffect->lpFunc is a FUBAR pointer while QueryEffects."
                     LOG_
               }
            }
           
         }
         QueryEffectList.clear();
      }
      // Function is ending, decrease stack level.
      effectStackLevelQuery--;	
   }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Verifies all the timer effects on the unit
// 
void Unit::VerifyTimers( void )
{
   if( PickLock() )
   {
      try
      {
         // Query all attack effects	
         list< UNIT_EFFECT * > VerifyTimersList;
         if( lptlEffectList != NULL )
         {
            lptlEffectList->Lock();
            lptlEffectList->ToHead();
            while( lptlEffectList->QueryNext() )
            {
               UNIT_EFFECT *effect = lptlEffectList->Object();

               if( effect->dwTimer != 0 )
               {
                  if( effect->dwTimer <= TFCMAIN::GetRound() )
                  {
                     if( effect->lpFunc )
                     {					
                        VerifyTimersList.push_back( effect );
                        lptlEffectList->Remove();// Only remove timer, it will be destroyed later
                     }
                     else
                     {
                        // destroy effect if it isn't going to trigger.
                        lptlEffectList->DeleteAbsolute();
                     }
                  }
               }
            }	
            lptlEffectList->Unlock();


            list< UNIT_EFFECT * >::iterator i;
            for( i = VerifyTimersList.begin(); i != VerifyTimersList.end(); i++ )
            {
               if ( (*i) == NULL )//DC for GPs
               {				
                  _LOG_DEBUG
                     LOG_CRIT_ERRORS,
                     "DC - GPs - lpEffect is a null pointer while VerifyTimers."
                     LOG_
               }
               else if ( (*i)->lpFunc == NULL )//DC for GPs
               {	
                  _LOG_DEBUG
                     LOG_CRIT_ERRORS,
                     "DC - GPs - lpEffect->lpFunc is a null pointer while VerifyTimers."
                     LOG_
               }
               else
               {									//DC for GPs
                  if(!IsDead())
                  {
                     (*i)->lpFunc( MSG_OnTimer  , (*i)->dwEffect, this, NULL, NULL, (*i)->lpData, NULL );
                     (*i)->lpFunc( MSG_OnDestroy, (*i)->dwEffect, this, NULL, NULL, (*i)->lpData, NULL );
                  }
               }
            }
            VerifyTimersList.clear();
         }
      }
      catch (...)
      {
         FILE *pft = NULL;
         fopen_s(&pft,"c:\\__SvrException.txt","a+");
         fprintf(pft,"VerifyTimers...\n");
         fclose(pft);
      }

      Unlock();
   }
}

//////////////////////////////////////////////////////////////////////////////////////////
signed char Unit::GetRadiance( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Get's the unit's radiance
// 
// Return: signed char, the unit's radiance
//////////////////////////////////////////////////////////////////////////////////////////
{
    int nRadiance = cRadiance;

    // Add up any radiance boost.
    nRadiance += (int)( QueryBoost( STAT_RADIANCE ) );

    // Radiance cannot be less than -100% and not more than 100%
    nRadiance = nRadiance < -100 ? -100 : nRadiance;
    nRadiance = nRadiance >  100 ?  100 : nRadiance;

	return (char)( nRadiance );
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::SetRadiance
//////////////////////////////////////////////////////////////////////////////////////////
// Sets the radiance of a unit
// 
(
 signed char cNewRadiance // The new radiance
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	cRadiance = cNewRadiance;
}

//////////////////////////////////////////////////////////////////////////////////////////
DIR::MOVE Unit::QueryDirection
//////////////////////////////////////////////////////////////////////////////////////////
// Returns the direction which the unit should go to reach wlTarget
// 
( 
 WorldPos wlTarget	// The target position.
)
// Return: DIR::MOVE, the direction the unit should move to
//////////////////////////////////////////////////////////////////////////////////////////
{
	DIR::MOVE direction;	
	if(wlTarget.X < WL.X) direction = DIR::west;
	if(wlTarget.X > WL.X) direction = DIR::east;

	if(wlTarget.Y < WL.Y && direction == DIR::west) direction = DIR::northwest;
	else if(wlTarget.Y < WL.Y && direction == DIR::east) direction = DIR::northeast;
	else if(wlTarget.Y < WL.Y) direction = DIR::north;
		if(wlTarget.Y > WL.Y && direction == DIR::west) direction = DIR::southwest;
	else if(wlTarget.Y > WL.Y && direction == DIR::east) direction = DIR::southeast;
	else if(wlTarget.Y > WL.Y) direction = DIR::south;
	
	return direction;
}

//////////////////////////////////////////////////////////////////////////////////////////
BOOL Unit::UseSpellEnergy
//////////////////////////////////////////////////////////////////////////////////////////
// Uses spell energy, purely virtual
// 
(
 WORD wMana
)
// Return: BOOL, always FALSE
//////////////////////////////////////////////////////////////////////////////////////////
{
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::SetMark
//////////////////////////////////////////////////////////////////////////////////////////
// Sets the marker
// 
(
 BYTE bNewMark // the current marker
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	bMarker = bNewMark;
}

//////////////////////////////////////////////////////////////////////////////////////////
BYTE Unit::GetMark( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Gets the current marker
// 
// Return: BYTE, the marker
//////////////////////////////////////////////////////////////////////////////////////////
{
	return bMarker;
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::AddBoost
//////////////////////////////////////////////////////////////////////////////////////////
// Adds a boost to the unit
// 
(
 LPBOOST lpBoost // The boost structure
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	lptlBoostList->Lock();
	lptlBoostList->AddToTail( lpBoost );
	lptlBoostList->Unlock();
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::AddBoost
//////////////////////////////////////////////////////////////////////////////////////////
// Adds a boost
// 
(
 DWORD dwBoostID,	// The boost ID
 WORD wStat,	// The stat to boost 
 int nBoost		// The boost to assign to the stat.
)
//////////////////////////////////////////////////////////////////////////////////////////
{
    //TRACE( "\r\nAdding boost ID %u for stat %u of value %d", 
    //    dwBoostID,
    //    wStat,
    //    nBoost
    //);
    
    if( nBoost != 0 ){
        LPBOOST lpBoost = new BOOST;
	    lpBoost->dwBoostID = dwBoostID;
	    lpBoost->wStat = wStat;
        char str[ 14 ];
    
        // If formula was ok
		_itoa_s( nBoost, str, 14, 10 );
        if( lpBoost->bfBoost.SetFormula( str ) ){
            // Add boost.
            AddBoost( lpBoost );
        }else{
            _LOG_DEBUG
                LOG_DEBUG_LVL3,
                "Unit::AddBoost2| Boost %u's formula could not be set for stat %u: %s.",
                dwBoostID,
                wStat,
                str
            LOG_
    	    // Otherwise destroy it.
			if (lpBoost != NULL)
			{
				delete lpBoost;
				lpBoost = NULL;
			}
        }
    }else{
        _LOG_DEBUG
            LOG_DEBUG_LVL3,
            "Unit::AddBoost2| Stat %u's boost %u on unit %s was 0 and not added.",
            wStat,
            dwBoostID,
            (LPCTSTR)GetName(_DEFAULT_LNG)
        LOG_
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::AddBoost
//////////////////////////////////////////////////////////////////////////////////////////
// Adds a boost
// 
(
 DWORD dwBoostID,             // The boost ID.
 WORD wStat,               // The boosted stat.
 const char *lpszFormula    // The boost formula.
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	LPBOOST lpBoost = new BOOST;
	lpBoost->dwBoostID = dwBoostID;
	lpBoost->wStat = wStat;
    if( lpBoost->bfBoost.SetFormula( lpszFormula ) ){
        AddBoost( lpBoost );
    }else{
        _LOG_DEBUG
            LOG_DEBUG_LVL3,
            "Unit::AddBoost3| Boost %u's formula could not be set for stat %u: %s.",
            dwBoostID,
            wStat,
            lpszFormula
        LOG_

		if (lpBoost != NULL)
		{
			delete lpBoost;
			lpBoost = NULL;
		}
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::RemoveBoost
//////////////////////////////////////////////////////////////////////////////////////////
// Removes a boost
// 
(
 DWORD dwBoostID // The boost ID to remove
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	if( lptlBoostList != NULL ){
		BOOL boFound = FALSE;

		lptlBoostList->Lock();
		lptlBoostList->ToHead();
		while( lptlBoostList->QueryNext()/* && !boFound*/ ){
			if( lptlBoostList->Object()->dwBoostID == dwBoostID ){
				lptlBoostList->DeleteAbsolute();
				//boFound = TRUE;
			}
		}
		lptlBoostList->Unlock();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::RemoveBoostFromStat
//////////////////////////////////////////////////////////////////////////////////////////
//  Removes all boosts associated to the specified stat.
// 
(
 WORD wStat // 
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	if( lptlBoostList != NULL ){
		BOOL boFound = FALSE;

		lptlBoostList->Lock();
		lptlBoostList->ToHead();
		while( lptlBoostList->QueryNext()/* && !boFound*/ ){
			if( lptlBoostList->Object()->wStat == wStat ){
				lptlBoostList->DeleteAbsolute();
				//boFound = TRUE;
			}
		}
		lptlBoostList->Unlock();
	}
}
//////////////////////////////////////////////////////////////////////////////////////////
void Unit::RemoveAllBoost()
{
	if( lptlBoostList != NULL )
   {
      lptlBoostList->Lock();
		lptlBoostList->ToHead();
		while( lptlBoostList->QueryNext() )
      {
			lptlBoostList->DeleteAbsolute();
		}
      lptlBoostList->Unlock();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
LPBOOST Unit::GetBoost
//////////////////////////////////////////////////////////////////////////////////////////
// Returns the boost structure of a given boost
// 
(
 DWORD dwBoostID // The boost ID to search for
)
// Return: LPBOOST, The boost structure. NULL if none found.
//////////////////////////////////////////////////////////////////////////////////////////
{
	LPBOOST lpBoost = NULL;
	
	if( lptlBoostList != NULL ){
		BOOL boFound = FALSE;

		lptlBoostList->Lock();
		lptlBoostList->ToHead();
		while( lptlBoostList->QueryNext() && !boFound ){
			lpBoost = lptlBoostList->Object();
			if( lpBoost->dwBoostID == dwBoostID ){				
				boFound = TRUE;
			}
		}
		lptlBoostList->Unlock();
		// If boost wasn't found.
		if( !boFound ){
			// Set a NULL boost
			lpBoost = NULL;
		}
	}

	return lpBoost;
}

//////////////////////////////////////////////////////////////////////////////////////////
int Unit::QueryBoost
//////////////////////////////////////////////////////////////////////////////////////////
// Queries all the boosts associated with a given stat and returns the total.
// 
(
 WORD wStat // The stat.
)
// Return: int, the resulting boost
//////////////////////////////////////////////////////////////////////////////////////////
{
	double dblBoost = 0;

	if( lptlBoostList != NULL ){

		LPBOOST lpBoost;
		lptlBoostList->Lock();
		lptlBoostList->ToHead();
		while( lptlBoostList->QueryNext() ){
			lpBoost = lptlBoostList->Object();
            //TRACE( "\r\nBoosted stat %u vs wanted stat %u.", lpBoost->wStat, wStat );
			if( lpBoost->wStat == wStat ){
				dblBoost += lpBoost->bfBoost.GetBoost( this );
			}
		}
		lptlBoostList->Unlock();
	}

	return (int)( dblBoost );
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::SetBoost
//////////////////////////////////////////////////////////////////////////////////////////
// Sets the boost of an already existing boost.
// 
(
 DWORD dwBoostID,	// The boost ID to set.
 WORD wStat,	// The stat to boost, only used if boost didn't already exist.
 int nNewBoost	// The new boost.
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	BOOL boFound = FALSE;

	if( lptlBoostList != NULL ){
		LPBOOST lpBoost;
		lptlBoostList->Lock();
		lptlBoostList->ToHead();
		while( lptlBoostList->QueryNext() && !boFound ){
			lpBoost = lptlBoostList->Object();

			if( lpBoost->dwBoostID == dwBoostID ){
                char str[ 14 ];
				_itoa_s( nNewBoost, str, 14, 10 );
				lpBoost->bfBoost.SetFormula( str );
				boFound = TRUE;
			}
		}
		lptlBoostList->Unlock();
	}

	// If skill wasn't set, create it.
	if( !boFound ){
		AddBoost( dwBoostID, wStat, nNewBoost );
	}
}


#define GET_STAT( stat ) WORD Unit::Get##stat\
( void ){\
	int nStat = Stat##stat\
	; nStat += QueryBoost( STAT_##stat\
	); nStat = nStat < 0 ? 0 : nStat;\
	return (WORD)nStat;\
}

WORD Unit::GetSTR( void ){
	int nStat = StatSTR;
	nStat += QueryBoost( STAT_STR );
	nStat = nStat < 0 ? 0 : nStat;
	return (WORD)nStat;
}


GET_STAT( INT )
GET_STAT( END )

GET_STAT( AGI )
GET_STAT( WIS )
GET_STAT( ATTACK )
GET_STAT( DODGE )
GET_STAT( LCK )

WORD Unit::GetTrueINT( void ){ return StatINT; }
WORD Unit::GetTrueEND( void ){ return StatEND; }
WORD Unit::GetTrueSTR( void ){ return StatSTR; }
WORD Unit::GetTrueAGI( void ){ return StatAGI; }
WORD Unit::GetTrueWIS( void ){ return StatWIS; }
WORD Unit::GetTrueATTACK( void ){ return StatATTACK; }
WORD Unit::GetTrueDODGE( void ){ return StatDODGE; }
WORD Unit::GetTrueLCK( void ) { return StatLCK; }

void Unit::SetINT( WORD bStat ){ StatINT = bStat; }
void Unit::SetEND( WORD bStat ){ StatEND = bStat; }
void Unit::SetSTR( WORD bStat ){ StatSTR = bStat; }
void Unit::SetAGI( WORD bStat ){ StatAGI = bStat; }
void Unit::SetWIS( WORD bStat ){ StatWIS = bStat; }
void Unit::SetATTACK( WORD bStat ){ StatATTACK = bStat; }
void Unit::SetDODGE( WORD bStat ){ StatDODGE = bStat; }
void Unit::SetLCK( WORD bStat ) 
{ 
   StatLCK = bStat;
   if(StatLCK <=1)
      StatLCK = 100;
      
}


void Unit::SetUndead( WORD wUndead ){};
WORD Unit::GetUndead( void ){ return 0; };


//////////////////////////////////////////////////////////////////////////////////////////
void BinaryToString
//////////////////////////////////////////////////////////////////////////////////////////
// Converts a binary buffer into a user readable string (in hexadecimal format).
// 
(
 LPBYTE lpBuffer,	// The binary buffer
 LPBYTE lpString,	// Pointer to the resulting string data, must be large enough (dwBufferSize * 2) !.
 DWORD dwBufferSize // The buffer size
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	DWORD i;
	BYTE bData;

	for( i = 0; i < dwBufferSize * 2; i++ ){
		
		// Fetch first 4 bits
		if( !( i % 2 ) ){
			bData = (BYTE)( lpBuffer[ i >> 1 ] & 0xF0 ) >> 4;
		}
		// Fetch second 4 bits
		else{
			bData = (BYTE)( lpBuffer[ i >> 1 ] & 0x0F );
		}

		switch( bData ){
		case 0:  lpString[ i ] = '0'; break;
		case 1:  lpString[ i ] = '1'; break;
		case 2:  lpString[ i ] = '2'; break;
		case 3:  lpString[ i ] = '3'; break;	
		case 4:  lpString[ i ] = '4'; break;
		case 5:  lpString[ i ] = '5'; break;
		case 6:  lpString[ i ] = '6'; break;
		case 7:  lpString[ i ] = '7'; break;	
		case 8:  lpString[ i ] = '8'; break;
		case 9:  lpString[ i ] = '9'; break;
		case 10: lpString[ i ] = 'A'; break;
		case 11: lpString[ i ] = 'B'; break;	
		case 12: lpString[ i ] = 'C'; break;
		case 13: lpString[ i ] = 'D'; break;
		case 14: lpString[ i ] = 'E'; break;
		case 15: lpString[ i ] = 'F'; break;	
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////
// Add a query to the batch requests
#define ADD_QUERY	{ \
	LPSQL_REQUEST lpSql = new SQL_REQUEST;\
	lpSql->csQuery = csQuery;\
	lptlSQLRequests->AddToTail( lpSql );\
}

//////////////////////////////////////////////////////////////////////////////////////////
// Saves all effects binded with the unit into the provided vdArray
// 
BOOL Unit::SaveEffects(TemplateList< SQL_REQUEST > *lptlSQLRequests,DWORD dwBaseOwnerID)
{		
	CString csQuery;
	BOOL boDBError = FALSE;
		
   Lock();
   if( lptlEffectList != NULL )
   {
		lptlEffectList->Lock();
		lptlEffectList->ToHead();
		while( lptlEffectList->QueryNext() )
      {
			LPUNIT_EFFECT lpEffect = lptlEffectList->Object();
         DATA_SAVE sSaveData;
         ZeroMemory( &sSaveData, sizeof( DATA_SAVE ) );

			if( lpEffect->lpFunc != NULL )
         {
				lpEffect->lpFunc( MSG_OnSavePlayer, lpEffect->dwEffect,this, NULL, NULL, lpEffect->lpData, &sSaveData );
			}

			if( sSaveData.bSave != DO_NOT_SAVE ){
				
				if( sSaveData.bBufferSize > 0 && sSaveData.bBufferSize < 128 )
            {
					// Alloc buffer and make conversion.
					BYTE lpbTextData[ 256 ]; // Maximum SQL string length
               //TRACE( "\r\nBuffer size %u", sSaveData.bBufferSize );
					BinaryToString( sSaveData.lpbData, lpbTextData, sSaveData.bBufferSize );
               lpbTextData[ sSaveData.bBufferSize * 2 ] = 0;
					if (sSaveData.lpbData != NULL)
					{
						delete sSaveData.lpbData;
						sSaveData.lpbData = NULL;
					}

				    csQuery.Format( 
                               "INSERT INTO Effects(OwnerID,BaseOwnerID,EffectID,EffectType,Timer,EffectData,TotalDuration,BindedSpellID,BindedFlagID) VALUES ( "
                               "%d,"	// OwnerID
                               "%d,"   // BaseOwnerID
                               "%d,"	// EffectID
                               "%d,"	// EffectType
                               "%d,"	// Timer
                               "'%s'," // EffectData
                               "%d,"   // TotalDuration
                               "%u,"   // BindedSpellID
                               "%u)",  // BindedFlagID
                               GetID(),
                               dwBaseOwnerID,
                               lpEffect->dwEffect,
                               lpEffect->bEffectType,
                               ( lpEffect->dwTimer - TFCMAIN::GetRound() ),
                               lpbTextData,
                               lpEffect->dwTotalDuration,
                               lpEffect->bindedSpellID,
                               lpEffect->bindedFlag
                               );

                _LOG_DEBUG
                   LOG_DEBUG_LVL3,
                      "Saved effect %u, bufsize=%u, lpbData=0x%x, bSave=%u",
                      lpEffect->dwEffect,
                      sSaveData.bBufferSize,
                      sSaveData.lpbData,
                      sSaveData.bSave
                   LOG_

                   ADD_QUERY
                }
			}			
		}

		lptlEffectList->Unlock();
	}

	Unlock();

	return boDBError;
}


//////////////////////////////////////////////////////////////////////////////////////////
void StringToBinary
//////////////////////////////////////////////////////////////////////////////////////////
// Converts an hexadecimal data string into a binary buffer
// 
(
 LPBYTE lpszString, // The string to convert from.
 LPBYTE lpbBuffer,	// The resulting buffer. Must be able to hold dwSize / 2 bytes.
 DWORD dwSize		// The size of the string data.
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	DWORD i;	

	for( i = 0; i < ( dwSize >> 1 ); i++ ){
		// First byte is the higher 4 bits
		switch( lpszString[ i << 1 ] ){
		case '0': lpbBuffer[ i ] = 0x0 << 4; TRACE("0"); break;
		case '1': lpbBuffer[ i ] = 0x1 << 4; TRACE("1"); break;
		case '2': lpbBuffer[ i ] = 0x2 << 4; TRACE("2"); break;
		case '3': lpbBuffer[ i ] = 0x3 << 4; TRACE("3"); break;
		case '4': lpbBuffer[ i ] = 0x4 << 4; TRACE("4"); break;
		case '5': lpbBuffer[ i ] = 0x5 << 4; TRACE("5"); break;
		case '6': lpbBuffer[ i ] = 0x6 << 4; TRACE("6"); break;
		case '7': lpbBuffer[ i ] = 0x7 << 4; TRACE("7"); break;
		case '8': lpbBuffer[ i ] = 0x8 << 4; TRACE("8"); break;
		case '9': lpbBuffer[ i ] = 0x9 << 4; TRACE("9"); break;
		case 'A': lpbBuffer[ i ] = 0xA << 4; TRACE("a"); break;
		case 'B': lpbBuffer[ i ] = 0xB << 4; TRACE("b"); break;
		case 'C': lpbBuffer[ i ] = 0xC << 4; TRACE("c"); break;
		case 'D': lpbBuffer[ i ] = 0xD << 4; TRACE("d"); break;
		case 'E': lpbBuffer[ i ] = 0xE << 4; TRACE("e"); break;
		case 'F': lpbBuffer[ i ] = 0xF << 4; TRACE("f"); break;
		default:  lpbBuffer[ i ] = 0x0 << 4; TRACE("-"); break;
		}
		// Second byte is the lower 4 bits
		switch( lpszString[ ( i << 1 ) + 1 ] ){
		case '0': lpbBuffer[ i ] |= 0x0; TRACE("0"); break;
		case '1': lpbBuffer[ i ] |= 0x1; TRACE("1"); break;
		case '2': lpbBuffer[ i ] |= 0x2; TRACE("2"); break;
		case '3': lpbBuffer[ i ] |= 0x3; TRACE("3"); break;
		case '4': lpbBuffer[ i ] |= 0x4; TRACE("4"); break;
		case '5': lpbBuffer[ i ] |= 0x5; TRACE("5"); break;
		case '6': lpbBuffer[ i ] |= 0x6; TRACE("6"); break;
		case '7': lpbBuffer[ i ] |= 0x7; TRACE("7"); break;
		case '8': lpbBuffer[ i ] |= 0x8; TRACE("8"); break;
		case '9': lpbBuffer[ i ] |= 0x9; TRACE("9"); break;
		case 'A': lpbBuffer[ i ] |= 0xA; TRACE("a"); break;
		case 'B': lpbBuffer[ i ] |= 0xB; TRACE("b"); break;
		case 'C': lpbBuffer[ i ] |= 0xC; TRACE("c"); break;		
		case 'D': lpbBuffer[ i ] |= 0xD; TRACE("d"); break;
		case 'E': lpbBuffer[ i ] |= 0xE; TRACE("e"); break;
		case 'F': lpbBuffer[ i ] |= 0xF; TRACE("f"); break;
		default:  lpbBuffer[ i ] |= 0x0; TRACE("-"); break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::LoadEffects
//////////////////////////////////////////////////////////////////////////////////////////
// Loads the effects earlier saved inside the given vdArray
// 
(
 cODBCMage &Connection,
 DWORD dwBaseOwnerID
)
//////////////////////////////////////////////////////////////////////////////////////////
{
   const int DB_EffectID		 = 1;
   const int DB_EffectType	  	 = 2;
   const int DB_Timer			 = 3;
   const int DB_EffectData		 = 4;
   const int DB_TotalDuration	 = 5;
   const int DB_BindedSpellID  = 6;
   const int DB_BindedFlagID   = 7;

   CString csQuery;

   csQuery.Format(	"SELECT EffectID, EffectType, Timer, EffectData, TotalDuration, BindedSpellID, BindedFlagID FROM Effects WHERE OwnerID=%d AND BaseOwnerID=%d", GetID(), dwBaseOwnerID );

   if( Connection.SendRequest( (LPCTSTR)csQuery ) )
   {
      // Scroll through the fetched records.
      while( Connection.Fetch() )
      {
         LOAD_EFFECT_DATA eff;
         char szData[ 256 ];

         Connection.GetDWORD ( DB_EffectID, &eff.effectId );
         Connection.GetWORD  ( DB_EffectType, &eff.effectType );
         Connection.GetDWORD ( DB_Timer, &eff.effectTimer );
         Connection.GetString( DB_EffectData, (LPTSTR)szData, 256 );
         Connection.GetSDWORD( DB_TotalDuration, (long*)&eff.totalDuration );
         Connection.GetDWORD ( DB_BindedSpellID, &eff.bindedSpellId );
         Connection.GetDWORD ( DB_BindedFlagID, &eff.bindedFlagId );
         eff.effectData = szData;

         deferredEffectLoad.push_back( eff );
      }
   }
}

//////////////////////////////////////////////////////////////////////////////////////////
//  Loads the unit effects
// 
//////////////////////////////////////////////////////////////////////////////////////////
void Unit::DeferredLoadEffects( void )
{
   Lock();

   list< LOAD_EFFECT_DATA >::iterator i;
   for( i = deferredEffectLoad.begin(); i != deferredEffectLoad.end(); i++ )
   {
      LPUNIT_EFFECT lpEffect = new UNIT_EFFECT;
      lpEffect->dwTimer = (*i).effectTimer TDELAY;
      lpEffect->dwEffect = (*i).effectId;
      lpEffect->bEffectType = (BYTE)(*i).effectType;
      lpEffect->dwTotalDuration = (*i).totalDuration;
      lpEffect->bindedSpellID = (*i).bindedSpellId;
      lpEffect->bindedFlag = (*i).bindedFlagId;

      DWORD dwStrLen = (*i).effectData.GetLength();

      // Query the spell effect manager to get the binded effect function.
      lpEffect->lpFunc = UnitEffectManager::GetEffectProc( lpEffect->dwEffect );

      if( dwStrLen > 0 )
      {
         BYTE lpszData[ 512 ];
         strcpy_s( (char *)lpszData, 512, (*i).effectData );

         DATA_SAVE sSaveData;
         ZeroMemory( &sSaveData, sizeof( DATA_SAVE ) );
         // If there was string data, convert it
         sSaveData.bBufferSize = (BYTE)( dwStrLen / 2 );
         sSaveData.lpbData = new BYTE[ dwStrLen / 2 + 1 ];

         StringToBinary( lpszData, sSaveData.lpbData, dwStrLen );
         // Then tell the particular effect that data has been loaded.
         if( lpEffect->lpFunc != NULL )
         {
            _LOG_DEBUG
               LOG_DEBUG_LVL3,
               "Loading effect %u with data %s.",
               lpEffect->dwEffect,
               lpszData
               LOG_

            lpEffect->lpFunc( MSG_OnLoadPlayer, lpEffect->dwEffect, this, NULL, NULL, lpEffect, &sSaveData );
            // Add the effect to the player
            AddEffect( lpEffect );
         }
         else
         {
            _LOG_DEBUG
               LOG_DEBUG_LVL1,
               "Effect function for effect %u does not exist.",
               lpEffect->dwEffect
               LOG_
         }

         if (sSaveData.lpbData != NULL)
         {
            delete sSaveData.lpbData;
            sSaveData.lpbData = NULL;
         }
      }
   }
   deferredEffectLoad.clear();

   if( lptlBoostList != NULL && deferredBoostLoad.size() > 0)
   {
      lptlBoostList->Lock();
      list< LOAD_BOOST_DATA >::iterator q;
      for( q = deferredBoostLoad.begin(); q != deferredBoostLoad.end(); q++ )
      {
         LPBOOST lpBoost = new BOOST;

         lpBoost->dwBoostID = (*q).dwBoostID;
         lpBoost->wStat = (*q).wStat;

         // If formula was ok.
         if( lpBoost->bfBoost.SetFormula( (*q).boostFormula ) )
         {
            // Add the boost.
            lptlBoostList->AddToTail( lpBoost );
         }
         else
         {
            // otherwise remove boost.
            if (lpBoost != NULL)
            {
               delete lpBoost;
               lpBoost = NULL;
            }
         }
      }
      lptlBoostList->Unlock();
   }
   deferredBoostLoad.clear();

   if( GetType() == U_PC )
   {
      Character *pc = static_cast< Character * >( this );
   }

   DealExhaust( loadAttackExhaust, loadMentalExhaust, loadMoveExhaust );

   loadAttackExhaust = loadMentalExhaust = loadMoveExhaust = 0;

   if( GetType() == U_PC )
   {
      Character *pc = static_cast< Character * >( this );

      // If the player has already remorted.
      if( ViewFlag( __FLAG_NUMBER_OF_REMORTS ) != 0 )
      {
         WorldPos wlPos = { 0, 0, 0 };
         // Cast the permanent remort spell.
         SpellMessageHandler::ActivateSpell( __SPELL_REMORT_AURA, this, NULL, NULL, wlPos );
      }

      // AddOn spell
      if( ViewFlag( __FLAG_ADDON_STORYLINE_PROGRESS ) >= 4 )
      {
         WorldPos wlPos = { 0, 0, 0 };
         // Cast the permanent wrath of the ancients spell.
         SpellMessageHandler::ActivateSpell( __SPELL_WRATH_OF_THE_ANCIENTS, this, NULL, this, wlPos );           
      }


      //Boucle pour toute la liste des infinite spell
      for(int i=0;i<theApp.m_aInfiniteSpell.GetSize();i++)
      {
         if( ViewFlag( theApp.m_aInfiniteSpell[i].uiFlagID ) != 0 )
         {
            WorldPos wlPos = { 0, 0, 0 };
            // Cast the permanent wrath of the ancients spell.
            SpellMessageHandler::ActivateSpell( theApp.m_aInfiniteSpell[i].uiSpellID, this, NULL, this, wlPos );   //OnLoading        
         }
      }

      /*
      if( ViewFlag( __FLAG_NMS_DECHU ) != 0 )
      {
         WorldPos wlPos = { 0, 0, 0 };
         // Cast the permanent wrath of the ancients spell.
         SpellMessageHandler::ActivateSpell( 10414, this, NULL, this, wlPos );   //OnLoading        
      }
      */
   }


   Unlock();
}

//////////////////////////////////////////////////////////////////////////////////////////
BOOL Unit::SaveBoosts
//////////////////////////////////////////////////////////////////////////////////////////
// 
// 
(
 TemplateList< SQL_REQUEST > *lptlSQLRequests,
 DWORD dwBaseOwnerID
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	LPBOOST lpBoost;
	CString csQuery;
	BOOL boDBError = FALSE;
		
	if( lptlBoostList != NULL ){
		
		lptlBoostList->Lock();
		lptlBoostList->ToHead();

		while( lptlBoostList->QueryNext() && !boDBError ){
			lpBoost = lptlBoostList->Object();

			csQuery.Format(
				"INSERT INTO Boosts(OwnerID,BaseOwnerID,BoostID,Stat,Boost) VALUES ( "
				"%d,"	  // OwnerID
				"%d,"   // BaseOwnerID
            "%d,"	  // BoostID
				"%d,"	  // Stat
				"'%s'"  // Boost
				" )",
				GetID(),
            dwBaseOwnerID,
				lpBoost->dwBoostID,
				lpBoost->wStat,
				lpBoost->bfBoost.GetOriginalFormula()
			);

			ADD_QUERY
		}
		lptlBoostList->Unlock();
	}


	return boDBError;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Loads the boosts from a a given array and stores them on this unit.
void Unit::LoadBoosts(cODBCMage &Connection,DWORD dwBaseOwnerID)
{	
	const int DB_BoostID = 1;
	const int DB_Stat = 2;
	const int DB_Boost = 3;
	
	CString csQuery;
	csQuery.Format(	"SELECT BoostID, Stat, Boost FROM Boosts WHERE OwnerID=%d AND BaseOwnerID=%d", GetID(), dwBaseOwnerID );
	    
    lptlBoostList->Lock();
    lptlBoostList->AnnihilateList();
    lptlBoostList->Unlock();

   if( Connection.SendRequest( (LPCTSTR)csQuery ) )
    {
       // Scroll through the fetched records.
       while( Connection.Fetch() )
       {
          LOAD_BOOST_DATA lbd;
          char str[ 256 ];

          Connection.GetDWORD(  DB_BoostID, &lbd.dwBoostID );
          Connection.GetWORD(   DB_Stat,    &lbd.wStat );
          Connection.GetString( DB_Boost,  str, 256 );

          lbd.boostFormula = str;

          deferredBoostLoad.push_back( lbd );
       }
    }	
}

//////////////////////////////////////////////////////////////////////////////////////////
BOOL Unit::SaveFlags
//////////////////////////////////////////////////////////////////////////////////////////
// 
// 
(
 TemplateList< SQL_REQUEST > *lptlSQLRequests,
 DWORD dwBaseOwnerID
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	CString csQuery;
	BOOL boDBError = FALSE;
//	LPINT FlagValues;
//	UINT *FlagIDs;
//	UINT  FlagLength;

	// First save the dynamic flags.
	//FlagLength = GetFlags(&FlagValues, &FlagIDs);
    FlagCont cFlags;
    GetFlags( cFlags );

    FlagCont::iterator i;
	
	for( i = cFlags.begin(); i != cFlags.end(); i++ )	{
		
      csQuery.Format( 
         "INSERT INTO Flags(OwnerID,BaseOwnerID,FlagID,FlagValue,DynamicFlag) VALUES ("
         "%d,"	// OwnerID
         "%d,"   // BaseOwnerID
         "%u,"	// FlagID
         "%d,"	// FlagValue
         "1"		// DynamicFlag
         " )",
         GetID(),
         dwBaseOwnerID,
         (*i).first,   // FlagID
         (*i).second   // FlagValue
         );
	
		ADD_QUERY
	}
    
	return boDBError;
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::LoadFlags
//////////////////////////////////////////////////////////////////////////////////////////
// Load the flags associated to a unit.
//
(
 cODBCMage &Connection,
 DWORD dwBaseOwnerID
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	const int DB_FlagID			= 1;
	const int DB_FlagValue		= 2;
	const int DB_DynamicFlag	= 3;

	CString csQuery;
	DWORD dwID;
	long lValue;
	BYTE  bType;

	csQuery.Format( "SELECT FlagID, FlagValue, DynamicFlag FROM Flags WHERE OwnerID=%d AND BaseOwnerID=%d", GetID(), dwBaseOwnerID );

	if( Connection.SendRequest( (LPCTSTR)csQuery ) )
   {
		// Fetch the found record.
		while( Connection.Fetch() )
      {
			Connection.GetDWORD( DB_FlagID, &dwID );
			Connection.GetSDWORD( DB_FlagValue, &lValue );
			Connection.GetBYTE ( DB_DynamicFlag, &bType );
            
   		SetFlag( dwID, lValue );
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Determines if a unit can attack
// 
// Return: BOOL, TRUE if this unit can attack.
//////////////////////////////////////////////////////////////////////////////////////////

BOOL Unit::CanAttack( void )
{
	return FALSE;
}
BOOL Unit::CanChangeTargetAA( void )
{
   return FALSE;
}

DWORD Unit::GetFriendlyID( void )
{
   return 0;
}


void Unit::SetAttack( BOOL boCanAttack ){};
void Unit::SetChangeTargetAA( BOOL boChangeTargetAA ){};
void Unit::SetFriendlyID( DWORD iFriendlyID ){};

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::SetBlock
//////////////////////////////////////////////////////////////////////////////////////////
// Sets the block
// 
(
 BYTE bNewBlock
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	bBlock = bNewBlock;
}

//////////////////////////////////////////////////////////////////////////////////////////
BYTE Unit::GetBlock( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Returns the block
// 
// Return: BYTE, the block
//////////////////////////////////////////////////////////////////////////////////////////
{
	return bBlock;
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::SetUnderBlock
//////////////////////////////////////////////////////////////////////////////////////////
// Sets the under block
// 
(
 BYTE bBlock
)
//////////////////////////////////////////////////////////////////////////////////////////
{
	bCurrentBlocking = bBlock;
}

//////////////////////////////////////////////////////////////////////////////////////////
BYTE Unit::GetUnderBlock( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Returns the under block
// 
// Return: BYTE, The under block
//////////////////////////////////////////////////////////////////////////////////////////
{
	return bCurrentBlocking;
}
 
BYTE Unit::GetUnderBlockMap( void )
{
   //return bCurrentBlocking;

   WorldPos wlPos = GetWL();
   WorldMap *pWorld = TFCMAIN::GetWorld( wlPos.world );
   BYTE underBlock = __BLOCK_NONE;
   if(pWorld)
      underBlock = pWorld->QueryAreaTypeMap( wlPos );

   return underBlock;
}


// Used by Creatures.cpp
//////////////////////////////////////////////////////////////////////////////////////////
void Unit::SetCanMove
//////////////////////////////////////////////////////////////////////////////////////////
// Sets if creature can move or stands still.
// 
(
 BOOL boNewCanMove
)
//////////////////////////////////////////////////////////////////////////////////////////
{}

//////////////////////////////////////////////////////////////////////////////////////////
BOOL Unit::CanMove( void )
//////////////////////////////////////////////////////////////////////////////////////////
// TRUE if creature has the ability to actually move!!
// 
//////////////////////////////////////////////////////////////////////////////////////////
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
WORD Unit::GetLang( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Units return the default language.
// 
// Return: WORD, the default language.
//////////////////////////////////////////////////////////////////////////////////////////
{
    return IntlText::GetDefaultLng();
}

//////////////////////////////////////////////////////////////////////////////////////////
BYTE Unit::GetStatus( void )
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
{
    return bStatus;
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::SetStatus
//////////////////////////////////////////////////////////////////////////////////////////
// Sets the status.
// 
(
 BYTE bNewStatus // The status
)
//////////////////////////////////////////////////////////////////////////////////////////
{
    bStatus = bNewStatus;
}

//////////////////////////////////////////////////////////////////////////////////////////
int Unit::GetWeight( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Returns the weight of the unit.
// 
// Return: int, the weight.
//////////////////////////////////////////////////////////////////////////////////////////
{
    int nWeight = 0;

    if( GetType() == U_OBJECT ){
        _item *lpItem = NULL;

        SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItem );

        nWeight = lpItem->size;
    }

    return nWeight;
}

//////////////////////////////////////////////////////////////////////////////////////////
int Unit::GetMaxWeight( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Returns 0
// 
// Return: int, 
//////////////////////////////////////////////////////////////////////////////////////////
{
    return 0;
}

void Unit::SetPrivateTalk( BOOL boPrivateTalk ){
}
BOOL Unit::IsPrivateTalk( void ){
    return FALSE;
}

void Unit::SetForceAttack( BOOL boFA ){
}
BOOL Unit::IsForceAttack( void ){
   return FALSE;
}

void Unit::SendPrivateMessage( const CString &csMessage, Unit *lpuUnit, DWORD dwColor ){};


//////////////////////////////////////////////////////////////////////////////////////////
void Unit::PacketUnitInformation
//////////////////////////////////////////////////////////////////////////////////////////
// Appends a unit's information to a packet which needs it, like Move or AddObject
// 
(
 TFCPacket &sending // The packet to append the unit's data to.
)
//////////////////////////////////////////////////////////////////////////////////////////
{
   DWORD dwFVal = CheckGlobalFlag(3000000+GetFriendlyID());
   char chStatus = GetStatus();
   if(IsForceAttack()) //is an NPC, but we force like a monster...
      chStatus = 0x00;
   sending << (long)dwFVal;
   sending << (short)GetFriendlyID();
   sending << (short)GetAppearance();
   sending << (long) GetID();
   sending << (char) GetRadiance();
   sending << (char) chStatus;
   sending << (char) (GetMaxHP() == 0 ? 0 : GetHP() * 100 / GetMaxHP() );
   if(GetType() == U_PC)
   {
      Character *ThisCharacter = static_cast<Character *>( this );
      if(ThisCharacter)
         sending << (char) (ThisCharacter->GetNMCombatMode());   
      else
      {
         char CombatMode = 0;
         sending << (char) (CombatMode);   
      }
   }
   else
   {
      char CombatMode = 0;
      sending << (char) (CombatMode);
   }

   char chHiddenInv2 =  0;
   if(ViewFlag(__FLAG_INVISIBILITY2))
      chHiddenInv2 =  1;
   sending << (char) (chHiddenInv2);
}

char Unit::GetNMCombatMode()
{
   return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::PacketPuppetInfo
//////////////////////////////////////////////////////////////////////////////////////////
//  Packets packet information for a normal unit.
// 
(
 TFCPacket &sending // 
)
//////////////////////////////////////////////////////////////////////////////////////////
{
    // Fill empty puppet packet.
    sending << (RQ_SIZE)RQ_PuppetInformation;
    sending << (long) GetID();
    sending << (short)0;
    sending << (short)0;
    sending << (short)0;
    sending << (short)0;
    sending << (short)0;
    sending << (short)0;
    sending << (short)0;
    sending << (short)0;
    sending << (short)0;
    sending << (short)0;
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::DealExhaust
//////////////////////////////////////////////////////////////////////////////////////////
//  Deals exhaust
// 
(
 DWORD dwAttack, // Attack exhaust in milliseconds
 DWORD dwMental, // Mental exhaust in milliseconds
 DWORD dwMove    // Move exhaust in milliseconds
)
//////////////////////////////////////////////////////////////////////////////////////////
{
    EXHAUST sExhaust = GetExhaust();

    // If previous mental exhaustion is off.
    if( sExhaust.mental < TFCMAIN::GetRound() ){
        // Create a new exhaust.
        sExhaust.mental = TFCMAIN::GetRound() + ( dwMental MILLISECONDS );
    }else{
        DWORD dwNewExhaust = (DWORD)dwMental MILLISECONDS;
        // If old exhaust is smaller than this spell's induced exhaust
        if( sExhaust.mental - TFCMAIN::GetRound() < dwNewExhaust ){
            // Set exhaust to spell exhaust.
            sExhaust.mental = TFCMAIN::GetRound() + dwNewExhaust;
        }
    }
    // If previous move exhaustion is off.
    if( sExhaust.move < TFCMAIN::GetRound() )
    {
        // Create a new exhaust.
        sExhaust.move = TFCMAIN::GetRound() + ( dwMove MILLISECONDS );
    }
    else
    {
        DWORD dwNewExhaust = dwMove MILLISECONDS;
        // If old exhaust is smaller than this spell's induced exhaust
        if( sExhaust.move - TFCMAIN::GetRound() < dwNewExhaust )
        {
            // Set exhaust to spell exhaust.
            sExhaust.move = TFCMAIN::GetRound() + dwNewExhaust;
        }
    }

    // If previous mental exhaustion is off.
    if( sExhaust.attack < TFCMAIN::GetRound() ){
        // Create a new exhaust.
        sExhaust.attack = TFCMAIN::GetRound() + ( dwAttack MILLISECONDS );
    }else{
        DWORD dwNewExhaust = dwAttack MILLISECONDS;
        // If old exhaust is smaller than this spell's induced exhaust
        if( sExhaust.attack - TFCMAIN::GetRound() < dwNewExhaust ){
            // Set exhaust to spell exhaust.
            sExhaust.attack = TFCMAIN::GetRound() + dwNewExhaust;
        }
    }

    // Set new exhaust.
    SetExhaust( sExhaust );
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::BroadcastPopup
//////////////////////////////////////////////////////////////////////////////////////////
//  Broadcasts that the unit just appeared.
// 
(
 WorldPos wlAppearPos,
 int nBroadcast
)
//////////////////////////////////////////////////////////////////////////////////////////
{
    TFCPacket sending;
    // Packet popup information
    PacketPopup( wlAppearPos, sending );

    // If we broadcast popup
    if( nBroadcast ){
        Broadcast::BCast( wlAppearPos, _DEFAULT_RANGE, sending, GetInvisibleQuery() );
    }else{
        SendPlayerMessage( sending );
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::PacketPopup
//////////////////////////////////////////////////////////////////////////////////////////
//  Puts the popup information into a packet, with header.
// 
(
 WorldPos wlAppearPos, // The appearance position
 TFCPacket &sending    // The packet to put the information in.
)
//////////////////////////////////////////////////////////////////////////////////////////
{
    DWORD dwFVal = CheckGlobalFlag(3000000+GetFriendlyID());
    char chStatus = GetStatus();
    if(IsForceAttack()) //is an NPC, but we force like a monster...
       chStatus = 0x00;
    sending << (RQ_SIZE)10004;
    sending << (short)wlAppearPos.X;
    sending << (short)wlAppearPos.Y;
    sending << (long)dwFVal;
    sending << (short)GetFriendlyID();
    sending << (short)GetAppearance();
    sending << (long)GetID();
    sending << (char)GetRadiance();
    sending << (char)chStatus;
    sending << (char)( GetMaxHP() == 0 ? 0 : GetHP() * 100 / GetMaxHP() );
    char CombatMode = 0;
    if(GetType() == U_PC)
    {
       Character *ThisCharacter = static_cast<Character *>( this );
       if(ThisCharacter)
          sending << (char) (ThisCharacter->GetNMCombatMode());   
       else
          sending << (char) (CombatMode);
    }
    else
    {

       sending << (char) (CombatMode);
    }

   char chHiddenInv2 =  0;
   if(ViewFlag(__FLAG_INVISIBILITY2) == 1)
   {
      chHiddenInv2 =  1;
   }
   sending << (char) (chHiddenInv2);
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::SetKarma
//////////////////////////////////////////////////////////////////////////////////////////
//  Sets the user's karma to a new value.
// 
(
 int nInt // The new karma value.
)
//////////////////////////////////////////////////////////////////////////////////////////
{ 
    // If karma went down,
    if( nInt < nKarma ){
        SendSystemMessage( _STR( 7127, GetLang() ) );

        _LOG_PC
            LOG_MISC_1,
            "Player %s's karma went down to %d",
            (LPCTSTR)GetName( GetLang() ),
            nInt
        LOG_
    }
    // If karma went up.
    else if( nInt > nKarma ){
        SendSystemMessage( _STR( 7126, GetLang() ) );
        _LOG_PC
            LOG_MISC_1,
            "Player %s's karma went up to %d",
            (LPCTSTR)GetName( GetLang() ),
            nInt
        LOG_
    }
    nKarma = nInt; 
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::SetCrime(int nInt)
{ 
   if( nInt < nCrime ) // If nCrime went down,
      SendSystemMessage( _STR( 15002, GetLang() ) );
   
   else if( nInt > nCrime )// If nCrime went up.
      SendSystemMessage( _STR( 15001, GetLang() ) );
   
   nCrime = nInt; 
}

void Unit::SetHonor(int nInt)
{ 
   if( nInt < nHonor ) // If nHonor went down,
      SendSystemMessage( _STR( 15004, GetLang() ) );
   
   else if( nInt > nHonor )// If nHonor went up.
      SendSystemMessage( _STR( 15003, GetLang() ) );
   nHonor = nInt; 
}

//////////////////////////////////////////////////////////////////////////////////////////
bool Unit::QueryInvisible::SendPacketTo
//////////////////////////////////////////////////////////////////////////////////////////
//  Determines whether the packet can be sent to the target player or not. Checks
// for invisibility flag.
// 
(
 Unit *target
)
// Return: bool, true or false. 
//////////////////////////////////////////////////////////////////////////////////////////
{
    if( unit == target )
    {
        return true;
    }

    
    if(((Character*)target)->GetGodFlags() & GOD_SEE_ALL)
    {
       if( unit->GetType() == U_PC )
       {
          Character *ch = static_cast< Character * >( unit );                
          if( ch->ViewFlag(__FLAG_JUST_DO_IT) == 666 ) //oki//block this unit...
             return false;
       }

       return true;
    }


    if( unit->GetType() == U_PC )
    {
        Character *ch = static_cast< Character * >( unit );                
        if( ch->GetPlayer()->GetGodFlags() & GOD_TRUE_INVISIBILITY ){
            return false;
        }
    }

    //valid if this unit have invisible skin...
    if(unit->GetAppearance() == 0)
    {
       return false;
    }

    
    bool ret = true;
    // If this unit is invisible and the target doesn't detect it.
    if( unit->ViewFlag( __FLAG_INVISIBILITY ) != 0 && 
        target->ViewFlag( __FLAG_DETECT_INVISIBILITY ) == 0 ){
        ret = false;
    }

    if( unit->ViewFlag( __FLAG_INVISIBILITY2 ) != 0 && 
       target->ViewFlag( __FLAG_DETECT_INVISIBILITY ) == 0 ){
       ret = false;
    }

    // If the unit is hidden and the target doesn't detect it.
    if( unit->ViewFlag( __FLAG_HIDDEN ) != 0 &&
        target->ViewFlag( __FLAG_DETECT_HIDDEN ) == 0 ){
        ret = false;
    }

    return ret;
}
//////////////////////////////////////////////////////////////////////////////////////////
void Unit::QueryInvisible::SetUnit
//////////////////////////////////////////////////////////////////////////////////////////
// Sets the character for the query invisible class.
// 
(
 Unit *ich
)
//////////////////////////////////////////////////////////////////////////////////////////
{
    unit = ich;
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::DispellInvisibility( void )
//////////////////////////////////////////////////////////////////////////////////////////
//  Removes any hidden or invisibility status from the unit.
// 
//////////////////////////////////////////////////////////////////////////////////////////
{
    DispellEffectWithFlag( __FLAG_INVISIBILITY );
    DispellEffectWithFlag( __FLAG_INVISIBILITY2 );
    Unhide();
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::Unhide( void )
//////////////////////////////////////////////////////////////////////////////////////////
//  Unhides a unit.
// 
//////////////////////////////////////////////////////////////////////////////////////////
{
    // If unit isn't already hidden.
    if( ViewFlag( __FLAG_HIDDEN ) == 0 ){
        return;
    }
    
    // Remove the hidden flag.
    RemoveFlag( __FLAG_HIDDEN );

    WorldPos wlPos = GetWL();

    // Broadcast the unit's popup.
    BroadcastPopup( wlPos );

	SendSystemMessage( _STR(12956, GetLang() ) );
    

	if (theApp.dwHideUncoverEffectDisabled == 0) {
		// Broadcast the unhide effect
		Broadcast::BCSpellEffect( wlPos, _DEFAULT_RANGE,
			30100, // <-- Effect ID
			GetID(),
			GetID(),
			wlPos,
			wlPos,
			GetNextGlobalEffectID(),
			0
		);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
void Unit::RemoveReferenceTo
//////////////////////////////////////////////////////////////////////////////////////////
// Removes any reference to the pointer to unit.
// 
(
 Unit *theUnit // The unit.
)
//////////////////////////////////////////////////////////////////////////////////////////
{
    if( GetBond() == theUnit ){
        SetBond( NULL );
    }
    if( GetTarget() == theUnit ){
        SetTarget( GetBond() );
    }
}

void  Unit::AddHitUnit(DWORD dwID,Unit *pUnit)
{
   m_csUnitMap.Lock();
   std::map<DWORD, HITData*>::iterator it = m_HitList.find(dwID);
   if (it != m_HitList.end())
   {
      HITData *pHit = NULL;
      pHit = (*it).second;
      pHit->dwTimeStamp = timeGetTime(); //reset timeout
      _LOG_CHEAT
         LOG_ALWAYS,
         "** Ajout(UPDATE) a la liste mob[%d] user[%d]  timestamp[%d]   Count[%d]",
         GetID(),pUnit->GetID(),pHit->dwTimeStamp,m_HitList.size()
         LOG_


   }
   else
   {
      //on doit ajouter
      HITData *pHit = new HITData;
      pHit->pUnit = pUnit;
      pHit->dwTimeStamp = timeGetTime(); //reset timeout;
      m_HitList[dwID] = pHit;

      _LOG_CHEAT
         LOG_ALWAYS,
         "** Ajout(ADD) a la liste mob[%d] user[%d]  timestamp[%d]  Count[%d]",
         GetID(),pUnit->GetID(),pHit->dwTimeStamp,m_HitList.size()
         LOG_
   }

   m_csUnitMap.Unlock();
}

void Unit::DelAllHitUnit()
{
   HITData *pHit = NULL;

   m_csUnitMap.Lock();
   std::map<DWORD, HITData*>::iterator it = m_HitList.begin();
   while (it != m_HitList.end())
   {
      pHit = (*it).second;
      if(pHit) 
         delete pHit;
      pHit = NULL;
      it++;
   }
   m_HitList.clear();
   m_csUnitMap.Unlock();
}

void Unit::ValidHitTime()
{
   HITData *pHit = NULL;

   m_csUnitMap.Lock();
   std::map<DWORD, HITData*>::iterator it = m_HitList.begin();
   while (it != m_HitList.end())
   {
      pHit = (*it).second;
      if(timeGetTime() - pHit->dwTimeStamp > 10000) //> 10 sec
      {
         it = m_HitList.erase( it );    

         _LOG_CHEAT
            LOG_ALWAYS,
            "** Attack Expire(DEL) de la liste mob[%d] user[%d]  Count[%d]",
            GetID(),pHit->pUnit->GetID(),m_HitList.size()
            LOG_


         if(pHit) delete pHit;
         pHit = NULL;
      }
      else
        it++;
   }
   m_csUnitMap.Unlock();
}

void Unit::GetUnitVector(std::vector <Unit*> &vUnit)
{
   HITData *pHit = NULL;

   m_csUnitMap.Lock();
   std::map<DWORD, HITData*>::iterator it = m_HitList.begin();
   while (it != m_HitList.end())
   {
      pHit = (*it).second;
      vUnit.push_back(pHit->pUnit);
      it++;
   }
   m_csUnitMap.Unlock();
}