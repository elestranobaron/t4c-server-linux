#include "stdafx.h"
#include "Minions.h"
#include "TFC_MAIN.H"
#include "Broadcast.h"
#include "ObjectListing.h"
#include "GameDefs.h"
#include "Random.h"
#include "MonsterStructure.h"
#include "T4CLog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern Random rnd;

Minions::Minions(Unit *pParent)
{		
   m_pParent = pParent;
   boCanMove = TRUE;
   SetSystemDestroy( TRUE );

   m_strName    = "";
   LastMoveTime = 0;
}
/******************************************************************************/
Minions::~Minions()
/******************************************************************************/
{
   
}

void Minions::Death(LPATTACK_STRUCTURE Blow,Unit *WhoHit)
{
   //set need to Destroy Creature
   KillCreature();

   WorldPos pos = GetWL();
   WorldMap *wl;
   wl = TFCMAIN::GetWorld((WORD)pos.world);
   if( wl != NULL )
   {
      wl->remove_world_unit(pos, GetID());
   }
}

int Minions::hit(LPATTACK_STRUCTURE Blow,Unit *WhoHit)
{
   return 0;
}


DWORD Minions::GetTrueMaxHP()
{
   return MaxHP ? MaxHP : 1;
}
void Minions::SetMaxHP(DWORD newHP)
{
   MaxHP = newHP;
}
DWORD Minions::GetHP()
{
   return HP;
}
void Minions::SetHP(DWORD newHP, bool boUpdate )
{
   HP = newHP;
}

CString Minions::GetName( WORD wLang )
{
   Character *chSelf = (Character *)m_pParent;
   CString strTmp;
   strTmp.Format(_STR( 15364, chSelf->GetLang() ),m_strName,chSelf->GetName());

   return strTmp;
}

void Minions::SetName(CString newname)
{
   m_strName = newname;
}

CString Minions::GetRealName()
{
    return  m_strName;
}



void Minions::SetCanMove( BOOL boNewCanMove)
{
   boCanMove = boNewCanMove;
}

BOOL Minions::CanMove( void )
{
   return boCanMove;
}

void Minions::SetLastMoveTime(UINT newTime)
{
   LastMoveTime = newTime;
}

UINT Minions::GetLastMoveTime()
{
   return LastMoveTime;
}


WorldPos Minions::MoveUnit(DIR::MOVE where,BOOL boAbsolute,bool boCompressMove,bool boBroadcastMove)
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
         //dois ajouter quil ne vas pas plus loin que le PJ associer
         //a voir si ici ou direct dan asla gestion des move...
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


void Minions::VaporizeUnit( bool bLog )
{
   ATTACK_STRUCTURE Blow;
   memset( &Blow, 0, sizeof( Blow ) );

   if(bLog)
   {
      _LOG_GAMEOP
         LOG_SYSOP,
         "Minion named %s (ID %u) was vaporized.",
         (LPCTSTR)GetName( _DEFAULT_LNG ),
         GetID()
         LOG_
   }
   

   // Tell everyone that this creature died.
   Broadcast::BCObjectRemoved( GetWL(), _DEFAULT_RANGE_REMOVE,GetID()); //Vaporize unit de creature

   // Kill creature.
   Death( &Blow, NULL );
}
