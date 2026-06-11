/******************************************************************************
Modify for vs2008 (30/04/2009)
Add Attack BOW
Add Hair color by Nightmare (27/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "Creatures.h"
#include "TFC_MAIN.H"
#include "Broadcast.h"
#include "ObjectListing.h"
#include "GameDefs.h"
#include "Random.h"
#include "MonsterStructure.h"
#include "T4CLog.h"

extern CTFCServerApp theApp;

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
extern Random rnd;

/******************************************************************************/
Creatures::Creatures()
/******************************************************************************/
{		
    boCanMove = TRUE;
	boCanAttack = TRUE;
   boChangeTargetAA = FALSE;
   iFriendlyID = 0;
	exhaust.attack = 0;
	exhaust.move = 0;	
	clan = 0;
	//bScanRange = rnd( 15, 20 );
	bScanRange = rnd( 20, 25 ); // More adapted for a 1024 resolution
	SetSystemDestroy( TRUE );

	SetBlock( __BLOCK_CAN_FLY_OVER );

	wlLastTeleport.X = wlLastTeleport.Y = wlLastTeleport.world = 0;

	lpHive = NULL;

    boPrivate = FALSE;
    boForceAttack = FALSE;
}
/******************************************************************************/
Creatures::~Creatures()
/******************************************************************************/
{
	if( lpHive != NULL )
	{
		lpHive->DecreaseMonsterCount();		
	}
}
/******************************************************************************/
// Called when dead.
void Creatures::Death(
 LPATTACK_STRUCTURE Blow,	// The blow structure
 Unit *WhoHit				// Who hit.
)
/******************************************************************************/
{
	// sets it's HP to hp, for protocol ;)
	SetHP(0, false);				

    // If the unit was already killed sooner in the attack phase.
    if( IsDead() ){
        return;
    }
    // Set creature in 'killed' mode.
    KillCreature();
    
    // Creature who hit no longer is in combat
    if( WhoHit != NULL )
    {
       WhoHit->SetTarget(WhoHit->GetBond());
       WhoHit->Do(wandering,"Creatures::Death");
    }

	TRACE(_T("DEATH at (%u, %u) adress 0x%x!!"), GetWL().X, GetWL().Y, this);
	TemplateList <Unit> CorpseHolding;
		
	SendUnitMessage(MSG_OnDeath, this, NULL, WhoHit, Blow, &CorpseHolding);
	
	WorldPos pos = GetWL();
	WorldMap *wl;
	//Unit *theCorpse;

	wl = TFCMAIN::GetWorld((WORD)pos.world);
	if( wl != NULL )
	{
		wl->remove_world_unit(pos, GetID());

        _LOG_DEBUG
            LOG_DEBUG_LVL4,
            "Unit 0x%x died.",
            this
        LOG_

		TFCPacket sending;		

		Unit *lpuItem = NULL;		
		// Drop the corpse's holding on the floor, around the body.
      int k = 0;
      WorldPos wlFoundPos = { 0,0,0 };
      CorpseHolding.ToHead();
      while( CorpseHolding.QueryNext() )
      {
         lpuItem = CorpseHolding.Object();
         wlFoundPos = pos;
         switch( k++ )
         {
            case 0: wlFoundPos.Y = pos.Y + rnd( 1, 2 ); break;
            case 1: wlFoundPos.X = pos.X + rnd( 1, 2 ); break;
            case 2: wlFoundPos.Y = pos.Y - rnd( 1, 2 ); break;
            case 3: wlFoundPos.X = pos.X - rnd( 1, 2 ); break;
            case 4: wlFoundPos.X = pos.X + rnd( 1, 2 );
                    wlFoundPos.Y = pos.Y + rnd( 1, 2 ); break;
            case 5: wlFoundPos.X = pos.X + rnd( 1, 2 );
                    wlFoundPos.Y = pos.Y - rnd( 1, 2 ); break;
            case 6: wlFoundPos.X = pos.X - rnd( 1, 2 ); 
                    wlFoundPos.Y = pos.Y + rnd( 1, 2 ); break;
            case 7: wlFoundPos.X = pos.X - rnd( 1, 2 );
                    wlFoundPos.Y = pos.Y - rnd( 1, 2 ); break;
         } 
         k = k > 7 ? 0 : k;

         // If spot isn't a valid one, make the world find it..!
         if( wl->IsBlocking( wlFoundPos ) )
         {
            wlFoundPos = wl->FindValidSpot( wlFoundPos, 3 );
         }
         if( wl->IsValidPosition( wlFoundPos ) )
         {
            DWORD dwID = 0;
            if(theApp.dwShareXPDropEnable)
            {
               dwID = lpuItem->GetLockedID();
            }

            wl->deposit_unit( wlFoundPos, lpuItem ,dwID);

            lpuItem->BroadcastPopup( wlFoundPos );
         }
         else
         {
            lpuItem->DeleteUnit();
         }
      }
    }
	else
	{
        _LOG_DEBUG
            LOG_DEBUG_LVL1,
            "Could not find world for unit at pos %u, %u, %u! (Creatures::OnDeath)",
            pos.X,
            pos.Y,
            pos.world
        LOG_
    }
}
/******************************************************************************/
// Hit a creature with a succesful blow.
int Creatures::hit(
 LPATTACK_STRUCTURE Blow,   // The blow.
 Unit *WhoHit               // The unit who attacked the creature.
)
/******************************************************************************/
{
	SendUnitMessage(MSG_OnHit, this, NULL, WhoHit, (void *)Blow);

    MonsterStructure *lpMob = NULL;
    SendUnitMessage( MSG_OnGetUnitStructure, this, NULL, NULL, NULL, &lpMob );
    if( lpMob == NULL )
	{
        return 0;
    }

    if( lpMob->AC >= 100000 )
	{
        return 0;
    }

	signed int HP = GetHP();    
    HP -= (int)Blow->Strike;
	
	// If death occured
	if(HP <= 0)
	{
		Death( Blow, WhoHit );
		return -1;
	}
	else
	{
		SetHP(HP, false);
	}
	
    return 0;
}
/******************************************************************************/
EXHAUST Creatures::GetExhaust()
/******************************************************************************/
{
	return exhaust;
}
/******************************************************************************/
void Creatures::SetExhaust(EXHAUST dfd)
/******************************************************************************/
{
	exhaust = dfd;
}
/******************************************************************************/
char Creatures::GetAgressivness()
/******************************************************************************/
{
	return agressive;
}
/******************************************************************************/
void Creatures::SetAgressivness(char newAgressive)
/******************************************************************************/
{
	agressive = newAgressive;
}
/******************************************************************************/
WORD Creatures::GetClan()
/******************************************************************************/
{
	return clan;
}
/******************************************************************************/
void Creatures::SetClan(WORD newClan)
/******************************************************************************/
{
	clan = newClan;
}
/******************************************************************************/
WorldPos Creatures::Destination()
/******************************************************************************/
{
	return dest;
}
/******************************************************************************/
void Creatures::SetDestination(WorldPos newDestination)
/******************************************************************************/
{
	dest = newDestination;
}
/******************************************************************************/
UINT Creatures::GetIdleTime()
/******************************************************************************/
{
	return IdleTime;
}
/******************************************************************************/
void Creatures::SetIdleTime(UINT newTime)
/******************************************************************************/
{
	IdleTime = newTime;
}
/******************************************************************************/
// This functions returns the max HP of a player
DWORD Creatures::GetTrueMaxHP()
/******************************************************************************/
{
	return MaxHP ? MaxHP : 1;
}
/******************************************************************************/
// This functions returns the max HP of a player
void Creatures::SetMaxHP(DWORD newHP)
/******************************************************************************/
{
	MaxHP = newHP;
}
/******************************************************************************/
// This functions returns the max HP of a player
DWORD Creatures::GetHP()
/******************************************************************************/
{
	return HP;
}
/******************************************************************************/
// This functions returns the max HP of a player
void Creatures::SetHP(DWORD newHP, bool boUpdate )
/******************************************************************************/
{
	HP = newHP;
}
/******************************************************************************/
// This functions returns the max HP of a player
WORD Creatures::GetTrueMaxMana()
/******************************************************************************/
{
	return MaxMana;
}
/******************************************************************************/
// This functions returns the max HP of a player
void Creatures::SetMaxMana(WORD newMaxMana)
/******************************************************************************/
{
	MaxMana = newMaxMana;
}
/******************************************************************************/
// This functions returns the max HP of a player
WORD Creatures::GetMana()
/******************************************************************************/
{
	return Mana;
}
/******************************************************************************/
// This functions returns the max HP of a player
void Creatures::SetMana(WORD newMana)
/******************************************************************************/
{
	Mana = newMana;
}
/******************************************************************************/
// Sets the new detection range of the monster.
void Creatures::SetDetectRange(
 BYTE bNewRange // new range
)
/******************************************************************************/
{
	bScanRange = bNewRange;
}
/******************************************************************************/
// Returns the scan range.
BYTE Creatures::GetDetectRange( void )
/******************************************************************************/
{
	return bScanRange;
}
/******************************************************************************/
// Sets the last teleport taken by this unit so this unit can come back from whence it came
void Creatures::SetLastTeleport( WorldPos WL )// Worldpos of teleport
/******************************************************************************/
{
	wlLastTeleport = WL;
}
/******************************************************************************/
// Returns the last teleport taken by this unit.
WorldPos Creatures::GetLastTeleport( void )
/******************************************************************************/
{
	return wlLastTeleport;
}
/******************************************************************************/
// Determines if the NPC can attack
BOOL Creatures::CanAttack( void )
/******************************************************************************/
{
	return boCanAttack;
}
/******************************************************************************/
// Sets if the NPC can attack or not.
void Creatures::SetAttack( BOOL boNewAttack )// TRUE if the NPC can now attack. Default is TRUE.
/******************************************************************************/
{
	boCanAttack = boNewAttack;
}

BOOL Creatures::CanChangeTargetAA( void )
{
   return boChangeTargetAA;
}
void Creatures::SetChangeTargetAA( BOOL boNewVal )
{
   boChangeTargetAA = boNewVal;
}

DWORD Creatures::GetFriendlyID( void )
{
   return iFriendlyID;
}
void Creatures::SetFriendlyID( DWORD dwID )
{
   iFriendlyID = dwID;
}



/******************************************************************************/
// Sets if creature can move or stands still.
void Creatures::SetCanMove( BOOL boNewCanMove)
/******************************************************************************/
{
	boCanMove = boNewCanMove;
}
/******************************************************************************/
// TRUE if creature has the ability to actually move!!
BOOL Creatures::CanMove( void )
/******************************************************************************/
{
	return boCanMove;
}
/******************************************************************************/
// Sets the binded monster spawn
void Creatures::SetBindedHive( Hive *lpNewHive)
/******************************************************************************/
{
	lpHive = lpNewHive;
}
/******************************************************************************/
// Moves a creature
WorldPos Creatures::MoveUnit(
 DIR::MOVE where,               // Direction of movement.
 BOOL boAbsolute,               // TRUE if creature should move even on blockings.
 bool boCompressMove,
 bool boBroadcastMove
)
/******************************************************************************/
{
	WorldPos WL = GetWL();
	BOOL blocked = FALSE;
	
	WorldPos CurrentWL = GetWL();

	if(!ViewFlag(__FLAG_STUN))
	{
		WorldMap *world = TFCMAIN::GetWorld(WL.world);	
		switch(where)
		{
			case DIR::north: 
				WL.Y--;
				break;
			case DIR::northeast:
				WL.Y--;
				WL.X++;
				break;
			case DIR::east:
				WL.X++;
				break;
			case DIR::southeast:
				WL.Y++;
				WL.X++;
				break;
			case DIR::south:
				WL.Y++;
				break;
			case DIR::southwest:
				WL.Y++;
				WL.X--;
				break;
			case DIR::west:
				WL.X--;
				break;
			case DIR::northwest:
				WL.X--;
				WL.Y--;
				break;
		}

        BOOL boMove = TRUE;

        // If this isn't the same position.
        if( !SAME_POS( WL, GetWL() ) )
		{
            // Get the creature's NPC structure.
            MonsterStructure *lpMob;
            SendUnitMessage( MSG_OnGetUnitStructure, this, NULL, NULL, NULL, &lpMob );
            if( lpMob != NULL )
            {
               // If creature cannot exit building.
               if( lpMob->boCanExitBuilding == FALSE )
               {
                  WORD wUnderBlock = GetUnderBlockMap();            
                  WORD wAreaType   = world->QueryAreaTypeMap( WL ); 
                  // If creature is currently in a building.
                  if( wUnderBlock == __AREA_BUILDING || wUnderBlock == __INDOOR_SAFE_HAVEN )
                  {
                     // If creature would step outside of a building.
                     if( wAreaType != __AREA_BUILDING && wAreaType != __INDOOR_SAFE_HAVEN )
                     {
                        // Forbid it from going outside of the building.
                        boMove = FALSE;
                     }
                  }
                  // If the creature is outside a building.
                  else
                  {
                     // If creature would step inside a building.
                     if( wAreaType == __AREA_BUILDING || wAreaType == __INDOOR_SAFE_HAVEN )
                     {
                        // Forbid it from going inside the building.
                        boMove = FALSE;
                     }
                  }
               }
            }
        }
        // If monster could move.
        if( boMove )
		{        	
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
			
	}
	else
	{
		WL = CurrentWL;
	}

	SetWL(WL);
	return WL;
}
/******************************************************************************/
void Creatures::SetPrivateTalk( BOOL boPrivateTalk )
/******************************************************************************/
{
    boPrivate = boPrivateTalk;
}
/******************************************************************************/
BOOL Creatures::IsPrivateTalk( void )
/******************************************************************************/
{
    return boPrivate;
}

/******************************************************************************/
void Creatures::SetForceAttack( BOOL boFA )
/******************************************************************************/
{
   boForceAttack = boFA;
}
/******************************************************************************/
BOOL Creatures::IsForceAttack( void )
/******************************************************************************/
{
   return boForceAttack;
}

/******************************************************************************/
int _MONSTER_ATTACK::Attack( Unit *self, Unit *target )
/******************************************************************************/
{
    return 0;//TFCMAIN::Attack( self, target );
}
/******************************************************************************/
int _MONSTER_SPELL_ATTACK::Attack( Unit *self, Unit *target )
/******************************************************************************/
{
    EXHAUST sExhaust = self->GetExhaust();

     // If player isn't exhausted.
    if( sExhaust.mental <= TFCMAIN::GetRound() )
	{
        if( target != NULL )
		{
            _LOG_DEBUG
                LOG_DEBUG_HIGH,
                "Monster %s (0x%x) is casting spell %u on target %s (0x%x).",
                (LPCTSTR)self->GetName(_DEFAULT_LNG),
                self,
                wSpellID,                
                (LPCTSTR)target->GetName(_DEFAULT_LNG),
                target
            LOG_
        }
		else
		{
            _LOG_DEBUG
                LOG_DEBUG_HIGH,
                "Monster %s is casting spell %u on a NULL target.",
                (LPCTSTR)self->GetName(_DEFAULT_LNG),
                wSpellID                
            LOG_
        }

        self->DispellInvisibility();

        // Activate the spell!
        SpellMessageHandler::ActivateSpell(
    		wSpellID,			
	    	self,
		    NULL,
    		target,
	    	target->GetWL()
	    );
    }
    return 0;
}

int _MONSTER_BOW_ATTACK::Attack( Unit *self, Unit *target )
{  
   return 0;
}

//  Packets the puppet information of a monster.
void Creatures::PacketPuppetInfo( TFCPacket &sending)
/******************************************************************************/
{
    // Get the basic monster structure of the creature.
    MonsterStructure *lpMob;
    SendUnitMessage( MSG_OnGetUnitStructure, this, NULL, NULL, NULL, &lpMob );
    if( lpMob != NULL )
    {
        short shHairColor = ViewFlag(__FLAG_NMS_COLOR_HAIR);
	    short shTAGPlayer = ViewFlag(__FLAG_NMS_TAG_DISPLAY_OVER_HEAD);
        sending << (RQ_SIZE)RQ_PuppetInformation;
        sending << (long) GetID();
        sending << (short)( lpMob->wBody   );
        sending << (short)( lpMob->wFeet   );
        sending << (short)( lpMob->wGloves );
        sending << (short)( lpMob->wHelm   );
        sending << (short)( lpMob->wLegs   );
        sending << (short)( lpMob->wWeapon );
        sending << (short)( lpMob->wShield );
        sending << (short)( lpMob->wCape   );
        sending << (short)( shHairColor    );
		sending << (short)( shTAGPlayer    );
    }
    // If no monster structure was found (should *never* happen ).
    else
	{
        // Use the default unit puppet packetting.
        Unit::PacketPuppetInfo( sending );
    }
}
/******************************************************************************/
//  Called to terminate the creature's existence.
void Creatures::VaporizeUnit( bool bLog )
/******************************************************************************/
{
    ATTACK_STRUCTURE Blow;
    memset( &Blow, 0, sizeof( Blow ) );

    if(bLog)
    {
       _LOG_GAMEOP
           LOG_SYSOP,
           "Creature named %s (ID %u) was vaporized.",
           (LPCTSTR)GetName( _DEFAULT_LNG ),
           GetID()
       LOG_
    }

	// Tell everyone that this creature died.
    Broadcast::BCObjectRemoved( GetWL(), _DEFAULT_RANGE_REMOVE,GetID()); //Vaporize unit de creature
    
    // Kill creature.
    Death( &Blow, NULL );
}
