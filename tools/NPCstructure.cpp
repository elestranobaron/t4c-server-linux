/******************************************************************************
Modify for vs2008 (01/05/2009)
Add Scroll XP management
Add Init structure for GM NPC List by NightMare (27/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "NPCstructure.h"
#include "TFC_MAIN.h"
#include "T4CLog.h"
#include "GameDefs.h"
#include "IntlText.h"
#include "RegKeyHandler.h"
#include "TFC Server.h"
#include "PlayerManager.h"
#include "format.h"

/******************************************************************************/
const int _PHYSICAL = 0;
const int _SPELL	= 1;

extern Random rnd;
extern Broadcast BCast;
extern CTFCServerApp theApp;

/******************************************************************************/
NPCstructure::NPCstructure()
/******************************************************************************/
{
    npc.boCanExitBuilding = FALSE;
    npc.boInit = false;
    npc.name = NULL;	
}
/******************************************************************************/
NPCstructure::~NPCstructure()
/******************************************************************************/
{
	//delete npc.name;
}
/******************************************************************************/
//  Called when a NPC instance is being created.
void NPCstructure::OnInitialise( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
	bool *lpboInit = static_cast< bool * >( valueIN );

    // If swap NPC wasn't initialized, creation fails.
    if( !npc.boInit )
	{
        *lpboInit = false;
        return;
    }
    
    npc.boCanExitBuilding = FALSE;
	
	self->SetSystemDestroy(FALSE); // avoid system idle destruction
	// self->SetStaticFlagsReference(this);
	self->SetFlag(__FLAG_CANNOT_DISPELL, 1);
	self->SetFlag(__FLAG_BLOCKING, 1);

	self->SetLevel( npc.level );
    self->SetMaxHP(npc.HP);
	self->SetHP( npc.HP, false );
	self->SetDODGE( npc.DodgeSkill );
	self->SetATTACK( 0 );
	self->SetTarget(NULL);
	self->SetSpeed(npc.speed);
	self->SetAgressivness(npc.agressive);
	self->SetClan(npc.clan);
	self->SetCorpse(npc.Corpse);
	self->SetAppearance(npc.appearance);
	//self->SetName(npc.isName.get_str( DEFAULT_LNG ) );	
	self->SetINT( npc.INT );
	self->SetEND( npc.END );
	self->SetSTR( npc.STR );
	self->SetWIS( npc.WIS );
	self->SetAGI( npc.AGI );
	self->SetAttack( npc.boCanAttack );
   self->SetChangeTargetAA( npc.boChangeTargetAA);
   self->SetFriendlyID(npc.iFriendlyID);
   

   if(npc.appearance >= 10001 && npc.appearance <= 10012)
      self->SetRadiance(100);
    
	// TRACE( "%s Private? %u.", npc.isName.get_str(), npc.boPrivateTalk );	
	self->SetPrivateTalk( npc.boPrivateTalk );
   self->SetForceAttack( npc.boForceAttack );
    self->SetStatus( CAN_TALK );

	// test
	WorldPos wlUpperLimit = npc.InitialPos;
	wlUpperLimit.X += npc.wHiXrange;
	wlUpperLimit.Y += npc.wHiYrange;
	WorldPos wlLowerLimit = npc.InitialPos;
	wlLowerLimit.X -= npc.wLoXrange;
	wlLowerLimit.Y -= npc.wLoYrange;

	self->SetUpperLimit(wlUpperLimit);
	self->SetLowerLimit(wlLowerLimit);

	WorldPos dest = {-1,-1,-1};
	self->Do(wandering,"NPCstructure::OnInitialise");
	self->SetDestination(dest);
    self->SetBlock( __BLOCK_CAN_FLY_OVER );

    if( npc.FireResist  != 0 ){ self->SetFireResist ( npc.FireResist ); }
    if( npc.WaterResist != 0 ){ self->SetWaterResist( npc.WaterResist ); }
    if( npc.AirResist   != 0 ){ self->SetAirResist  ( npc.AirResist ); }
    if( npc.EarthResist != 0 ){ self->SetEarthResist( npc.EarthResist ); }
    if( npc.LightResist != 0 ){ self->SetLightResist( npc.LightResist ); }
    if( npc.DarkResist  != 0 ){ self->SetDarkResist ( npc.DarkResist  ); }

    if( npc.FirePower  != 0 ){ self->SetFirePower ( npc.FirePower ); }
    if( npc.WaterPower != 0 ){ self->SetWaterPower( npc.WaterPower ); }
    if( npc.AirPower   != 0 ){ self->SetAirPower  ( npc.AirPower ); }
    if( npc.EarthPower != 0 ){ self->SetEarthPower( npc.EarthPower ); }
    if( npc.LightPower != 0 ){ self->SetLightPower( npc.LightPower ); }    
    if( npc.DarkPower  != 0 ){ self->SetDarkPower( npc.DarkPower ); }	

    /*
    _LOG_NPCS
        LOG_MISC_1,
        "NPC %s was spawned.",
        self->GetName( _DEFAULT_LNG ),
        self->GetWL().X,
        self->GetWL().Y,
        self->GetWL().world
    LOG_
    */
}
/******************************************************************************/
//  Called when the server first initializes.
void NPCstructure::OnServerInitialisation(
 UNIT_FUNC_PROTOTYPE,
 WORD wBaseReferenceID // The unit's base referenceID.
)
/******************************************************************************/
{
	npc.BaseReferenceID = wBaseReferenceID;

	TRACE( "\r\nInit NPC #%u  %s", wBaseReferenceID ,npc.name);

	if(npc.appearance != -1)
	{
		WorldMap *world = TFCMAIN::GetWorld(npc.InitialPos.world);	
		if(world)
		{
			Unit *newNPC = world->create_world_unit(U_NPC, wBaseReferenceID, npc.InitialPos, FALSE);
			if(newNPC)
			{
				// Then put it in the creature list for it to become alive
				TFCMAIN::AddMonster(newNPC);

            sSvrItemWDA newNpc;

            char szID[ 256 ];
            Unit::GetNameFromID( wBaseReferenceID, szID );
            
            newNpc.ushID = theApp.m_iIndexCntDLLNPC;
            newNpc.SummonName.Format("%s",szID);
            
            bool bFound = false;
            for(int i=0;i<theApp.m_aServerNPCList.GetSize();i++)
            {
               if(newNpc.SummonName == theApp.m_aServerNPCList[i].SummonName)
               { 
                  bFound = true;
                  i = theApp.m_aServerNPCList.GetSize()+1;
               }
            }
            if(!bFound)
            {
               theApp.m_aServerNPCList.Add(newNpc);
               theApp.m_iIndexCntDLLNPC++;
            }
			}
         else
         {
            char szID[ 256 ];
            if(!Unit::GetNameFromID( wBaseReferenceID, szID ))
            {
               sprintf_s(szID,256,"Unknown");
            }

            // If this NPC has no 'position' (perhaps in a hive).
            if( npc.InitialPos.X == 0 || npc.InitialPos.Y == 0 )
            {
               // Only log at lvl 4.
               _LOG_DEBUG
                  LOG_DEBUG_LVL4,
                  "Could not create NPC \"%s\" ID( %s ) at <POSSIBLY ON BLOCK> (%u, %u, %u)", 
                  npc.name,/*npc.isName.get_str( _DEFAULT_LNG )*/
                  szID,
                  npc.InitialPos.X, 
                  npc.InitialPos.Y, 
                  npc.InitialPos.world
                  LOG_
            }
            else
            {
               _LOG_DEBUG
                  LOG_DEBUG_LVL1,
                  "Could not create NPC \"%s\" ID( %s ) at <POSSIBLY ON BLOCK> (%u, %u, %u)", 
                  npc.name,/*npc.isName.get_str( _DEFAULT_LNG )*/
                  szID,
                  npc.InitialPos.X, 
                  npc.InitialPos.Y, 
                  npc.InitialPos.world
                  LOG_
            }
         }
		}
		else
		{
            char szID[ 256 ];
            Unit::GetNameFromID( wBaseReferenceID, szID );

            _LOG_DEBUG
				LOG_DEBUG_LVL1,
				"Could not create NPC ID(%u, %s) \"%s\" at <WRONG, UNDEFINED WORLD> (%u, %u, %u)\r\n\r\n", 
				wBaseReferenceID,
                szID,
                npc.name,//npc.isName.get_str( _DEFAULT_LNG ), 
				npc.InitialPos.X, 
				npc.InitialPos.Y, 
				npc.InitialPos.world
			LOG_
		}
	}
}
/******************************************************************************/
//  If NPC attacks.
void NPCstructure::OnAttack( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
	LPATTACK_STRUCTURE Blow = (LPATTACK_STRUCTURE)valueIN;
	int  WhichAttack = 0;

	// If there are monster attacks.
    if( !npc.vlpMonsterAttacks.empty() )
	{
		WhichAttack = rnd(0, npc.vlpMonsterAttacks.size() - 1);
		
        // Deal damage.
        Blow->Strike += npc.vlpMonsterAttacks[ WhichAttack ]->DamageRoll.GetBoost( self, target );			
		
		self->SetATTACK( npc.vlpMonsterAttacks[ WhichAttack ]->AttackSkill );
	}
			
	TRACE("Value of Strike(attack)=%u\r\n", Blow->Strike);	
}
/******************************************************************************/
//  When NPC gets attacked
void NPCstructure::OnAttacked( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
	LPATTACK_STRUCTURE Blow = (LPATTACK_STRUCTURE)valueIN;

	Blow->Strike -= self->GetAC(); // remove monster's AC
	
	TRACE("Value of Strike(after defense)=%u %X\r\n", Blow->Strike,this);
}
/******************************************************************************/
// When NPC gets struck
void NPCstructure::OnHit( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
   LPATTACK_STRUCTURE Blow = (LPATTACK_STRUCTURE)valueIN;

   if( npc.boCanAttack )
   {
      if( self != NULL && target != self )
      {
         self->SetTarget( target );
         self->Do( fighting );
      }
   }

   if( target->GetType() != U_PC ) //DC for GPs
   {
      return;
   }

   // Il semble qu'il peut y avoir un target=U_PC pour un Player=NULL		//DC for GPs
   if ( static_cast< Character * >( target )->GetPlayer() == NULL )
   {
      _LOG_DEBUG
         LOG_CRIT_ERRORS,
         "DC - GPs - target->GetPlayer() is a null pointer while NPCstructure::OnHit."
         LOG_
      return;
   }
   if( static_cast< Character * >( target )->GetPlayer() != NULL ) //DC for GPs
   {	
      TRACE("Value of Strike(hit)=%f\r\n", Blow->Strike);
      __int64 xp = target->GetXP();
      __int64 lastxp = target->GetXP();
      TRACE("XP before %u  ", xp);	
      if(Blow->Strike > 0)
      {
         //Ajoute le user a la liste des gens qui on attaquer ce mob...
         if(theApp.dwShareXPDropEnable)
            self->AddHitUnit(target->GetID(),target);

         __int64 n64XPGain;

         DWORD dwHP = self->GetHP();
         // If blow does more damage than the monster's current HP
         if( dwHP < Blow->Strike )
         {
            // Give XP equal to the creature's XP
            n64XPGain = (int)( npc.XPperHit * dwHP );
         }
         else
         {
            // Get the blow's xp.
            n64XPGain = (int)( npc.XPperHit * Blow->Strike );
         }

         if( n64XPGain > 0 )
         {
            RegKeyHandler regKey;
            regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY );

            //NMNMNM_SCROLL XP MANAGEMENT HERE........
            if(target->GetType() == U_PC)
            {
               Players *lpPlayer = static_cast< Character *>( target )->GetPlayer();
               if(lpPlayer->self->ViewFlag(__FLAG_SCROLL_XP_TIMESTAMP) != 0)
               {
                  lpPlayer->self->AddXPScrollBonnus(n64XPGain);
               }


               int XPNormalRatio = regKey.GetProfileInt( "XPNormalRatio", 1 );

               // Apply the ratio
               n64XPGain *= XPNormalRatio;

               //NMS Flag Boust XP...
               int iNbsBoustXPFlag = lpPlayer->self->ViewFlag(__FLAG_NMS_BOUST_XP);
               if(iNbsBoustXPFlag >0 && iNbsBoustXPFlag <= 10)
                  n64XPGain *= iNbsBoustXPFlag;

            }

            Players *lpPlayer = static_cast< Character *>( target )->GetPlayer();

            if( lpPlayer != NULL && lpPlayer->GetGodFlags() & GOD_BOOST_XP )
            {
               int XPBoostRatio = regKey.GetProfileInt( "XPBoostRatio", 10 );

               //n64XPGain *= 10;
               n64XPGain *= XPBoostRatio;
            }

            regKey.Close();

            if( lpPlayer != NULL && lpPlayer->GetGodFlags() & GOD_DEVELOPPER ){
               TFormat format;
               target->SendSystemMessage(
                  format(
                  "Gained %I64u xp (hit).",
                  n64XPGain
                  )
                  );
            }

            xp += n64XPGain;
         }

         TRACE("XP after %u\r\n", xp);
         // If player gained xp.
         if( xp > lastxp && target->GetLevel() < MAX_LEVEL_XP )
         {
            // Cannot gain more then 2 levels..!
            if( xp < Character::sm_n64XPchart[ target->GetLevel() + 1 ] )
            {
               if( theApp.dwLogXPGains != 0 && n64XPGain > 10 * Blow->Strike )
               {
                  _LOG_PC
                     LOG_WARNING,
                     "Player %s gained %I64u xp striking monster %s ( ID %u ).",
                     (LPCTSTR)target->GetName( _DEFAULT_LNG ),
                     n64XPGain,
                     (LPCTSTR)self->GetName( _DEFAULT_LNG ),
                     self->GetStaticReference()
                     LOG_
               }

               target->SetXP(xp);
            }
         }
      }
   }
}
/******************************************************************************/
// When monster dies
void NPCstructure::OnDeath( UNIT_FUNC_PROTOTYPE ) // self/medium etc
/******************************************************************************/
{	
   BOOL bLogDrop = FALSE;

   if( target != NULL && target->GetType() == U_PC )
   {
      if( static_cast< Character * >( target )->GetPlayer() == NULL )
      {
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "DC - GPs - target->GetPlayer() is a null pointer while NPCstructure::OnDeath."
            LOG_
            return;
      }
      else
      {
         if(theApp.dwShareXPDropEnable)
         {
            //On loop pour toute les target pour creer les drop + XP...
            std::vector <Unit*> vUnit;
            self->GetUnitVector(vUnit);
           
            for(int vu=0;vu<vUnit.size();vu++)
            {
               if(vUnit[vu])
               {
                  Unit *pTarget = vUnit[vu];
                  pTarget->Lock(); 
                  Character *ch       = static_cast< Character *>( pTarget );
                  Players   *lpPlayer = static_cast< Character *>( pTarget )->GetPlayer();
                  //if(pTarget->GetLCK() >150)
                  {
                     bLogDrop = TRUE;
                     _LOG_MONSTERS
                        LOG_DEBUG_LVL1,
                        "Player %s (%s) (luck = %d) killed(Shared) creature %s at %u,%u,%u",
                        (LPCTSTR)pTarget->GetName( _DEFAULT_LNG ),
                        lpPlayer->GetFullAccountName(),
                        pTarget->GetLCK(),
                        (LPCTSTR)self->GetName( _DEFAULT_LNG ),
                        self->GetWL().X,self->GetWL().Y,self->GetWL().world
                        LOG_
                  }

                  __int64 xp = pTarget->GetXP();

                  __int64 n64XPGain = (__int64)( ( npc.XPperDeath * npc.level ) / pTarget->GetLevel() );

                  if( n64XPGain > npc.XPperDeath )
                  {
                     n64XPGain = (__int64)npc.XPperDeath;
                  }
                  // If the killer is a PC higher then level 10
                  if( pTarget->GetType() == U_PC && pTarget->GetLevel() > 10 )
                  {
                     // Make sure that he doesn't get more than 5% of the quantity
                     // of XP required between two levels.            
                     __int64 maxXpDiff = (ch->NextLevelXP() - ch->PreviousLevelXP()) / 20;
                     if( n64XPGain > maxXpDiff )
                     {
                        n64XPGain = maxXpDiff - 1;
                     }
                  }

                  __int64 boostGain = 0;

                  RegKeyHandler regKey;
                  regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY );


                  __int64 boostXPScroll = 0;
                  //NMNMNM_SCROLL XP MANAGEMENT HERE........
                  if(pTarget->GetType() == U_PC)
                  {
                     boostXPScroll = n64XPGain; // gain reel sans boust xp externe a sa...
                     int XPNormalRatio = regKey.GetProfileInt( "XPNormalRatio", 1 );
                     // Set the ratio
                     n64XPGain *= XPNormalRatio;

                     //NMS Flag Boust XP...
                     int iNbsBoustXPFlag = lpPlayer->self->ViewFlag(__FLAG_NMS_BOUST_XP);
                     if(iNbsBoustXPFlag >0 && iNbsBoustXPFlag <= 10)
                        n64XPGain *= iNbsBoustXPFlag;
                  }

                  // If this is a player that has boost xp flag.
                  if( pTarget->GetType() == U_PC && lpPlayer->GetGodFlags() & GOD_BOOST_XP )
                  {
                     int XPBoostRatio = regKey.GetProfileInt( "XPBoostRatio", 10 );

                     boostGain = n64XPGain * XPBoostRatio;
                     xp += boostGain;

                     // Set player's xp now.
                     pTarget->SetXP( xp );            
                  }
                  else
                  {				
                     xp += n64XPGain;
                  }

                  regKey.Close();


                  if( theApp.dwLogXPGains != 0 && n64XPGain > 1000 )
                  {
                     _LOG_PC
                        LOG_WARNING,
                        "Player %s gained %I64u xp killing(Shared) creature %s ( ID %u ).",
                        (LPCTSTR)pTarget->GetName( _DEFAULT_LNG ),
                        n64XPGain,
                        (LPCTSTR)self->GetName( _DEFAULT_LNG ),
                        self->GetStaticReference()
                        LOG_        
                  }

                  // If XP was gained and pTarget is a Character.
                  if( n64XPGain > 0 && pTarget->GetType() == U_PC )
                  {
                     if( lpPlayer != NULL && lpPlayer->GetGodFlags() & GOD_DEVELOPPER )
                     {
                        if( boostGain != 0 )
                        {
                           // Send debug information.
                           TFormat format;
                           pTarget->SendSystemMessage(format("Gained %I64u xp (death).",boostGain));                        
                        }
                        else
                        {
                           // Send debug information.
                           TFormat format;
                           pTarget->SendSystemMessage(format("Gained %I64u xp (death).",n64XPGain));
                        }
                     }

                     if(lpPlayer->self->ViewFlag(__FLAG_SCROLL_XP_TIMESTAMP) != 0)
                     {
                        lpPlayer->self->AddXPScrollBonnus(boostXPScroll);
                     }

                     // Set player's xp.
                     ch->SetXP( xp );            

                  }

                  //Calcule les drop et tout

                  UINT i;
                  vector< ItemToGive > &vDeathItems = npc.vDeathItems;	// for ease of use.
                  TemplateList <Unit> *Holdings = (TemplateList<Unit> *)valueOUT;

                  CString strItemAll = "";
                  CString strItemTmp;

                  const int DeathItemPrecision = 1000000;
                  for(i = 0; i < npc.vDeathItems.size(); i++)
                  {
                     double dProb = 0;

                     if( pTarget != NULL )
                     {
                        dProb = (( vDeathItems[ i ].ItemGivePercentage * DeathItemPrecision ) * pTarget->GetLCK() / 100);
                     }
                     else
                     {
                        dProb = ( vDeathItems[ i ].ItemGivePercentage * DeathItemPrecision );
                     }


                     if(rnd(0, 100 * DeathItemPrecision ) < dProb )
                     {
                        Objects *item = new Objects;
                        if( item->Create(U_OBJECT, vDeathItems[ i ].ItemID) )
                        {
                           item->SetLockedID(pTarget->GetID());
                           Holdings->AddToTail(item);
                           strItemTmp.Format("%s\r\n",item->GetName(_DEFAULT_LNG));
                           strItemAll+=strItemTmp;
                        }
                        else
                        {
                           item->DeleteUnit();
                        }
                     }
                  }

                  if(bLogDrop)
                  {
                     _LOG_MONSTERS
                        LOG_DEBUG_LVL1,
                        "Creature drop(Shared) item(s): %s",strItemAll
                        LOG_
                  }

                  for(i=0; i<npc.vOnDeathFlag.size();i++)
                  {
                     // Checks flags to give
                     if( npc.vOnDeathFlag[i].nDeathGiveFlag != 0 )
                     {
                        // If flag value should be incremented.
                        if( npc.vOnDeathFlag[i].boDeathFlagIncrement )
                        {
                           // Set increment flag value.
                           pTarget->SetFlag( npc.vOnDeathFlag[i].nDeathGiveFlag, pTarget->ViewFlag( npc.vOnDeathFlag[i].nDeathGiveFlag ) + npc.vOnDeathFlag[i].nDeathFlagValue );
                        }
                        else
                        {
                           // Sets the flag value.
                           pTarget->SetFlag( npc.vOnDeathFlag[i].nDeathGiveFlag, npc.vOnDeathFlag[i].nDeathFlagValue );
                        }
                     }
                  }
                  // Give gold to the target	
                  DWORD dwGold = rnd( npc.MinGold, npc.MaxGold );    

                  // If character is NOT grouped.
                  if( ch->GetGroup() == NULL )
                  {
                     if(ch->ViewFlag(__FLAG_SCROLL_OR_TIMESTAMP) != 0)
                     {
                        ch->AddORScrollBonnus(dwGold);
                     }

                     // Simply update its gold.
                     int iGoldMultiply = ch->ViewFlag(__FLAG_NMS_BOUST_GOLD);
                     if(iGoldMultiply >0 && iGoldMultiply < 11)
                        dwGold *=iGoldMultiply;

                     ch->SetGold( target->GetGold() + dwGold );
                  }
                  else
                  {
                     // Otherwise let the group distribute the gold.
                     ch->GetGroup()->DistributeKillGold( ch, dwGold );
                  }
                  pTarget->Unlock(); 
               }
            }

            self->DelAllHitUnit();
            return;
         }
         else
         {
            target->Lock(); 
            if(target->GetLCK() >150)
            {
               bLogDrop = TRUE;
               Players *lpPlayer = static_cast< Character *>( target )->GetPlayer();

               _LOG_MONSTERS
                  LOG_DEBUG_LVL1,
                  "Player %s (%s) (luck = %d) killed creature %s at %u,%u,%u",
                  (LPCTSTR)target->GetName( _DEFAULT_LNG ),
                  lpPlayer->GetFullAccountName(),
                  target->GetLCK(),
                  (LPCTSTR)self->GetName( _DEFAULT_LNG ),
                  self->GetWL().X,self->GetWL().Y,self->GetWL().world
                  LOG_
            }

            __int64 xp = target->GetXP();
            TRACE("XP before %u  ", xp);	

            __int64 n64XPGain = (__int64)( ( npc.XPperDeath * npc.level ) / target->GetLevel() );

            if( n64XPGain > npc.XPperDeath )
            {
               n64XPGain = (__int64)npc.XPperDeath;
            }
            // If the killer is a PC higher then level 10
            if( target->GetType() == U_PC && target->GetLevel() > 10 )
            {
               Character *ch = static_cast< Character * >( target );
               // Make sure that he doesn't get more than 5% of the quantity
               // of XP required between two levels.            
               __int64 maxXpDiff = (ch->NextLevelXP() - ch->PreviousLevelXP()) / 20;
               if( n64XPGain > maxXpDiff )
               {
                  n64XPGain = maxXpDiff - 1;
               }
            }

            __int64 boostGain = 0;

            RegKeyHandler regKey;
            regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY );


            __int64 boostXPScroll = 0;
            //NMNMNM_SCROLL XP MANAGEMENT HERE........
            if(target->GetType() == U_PC)
            {
               Players *lpPlayer = static_cast< Character *>( target )->GetPlayer();

               boostXPScroll = n64XPGain; // gain reel sans boust xp externe a sa...
               int XPNormalRatio = regKey.GetProfileInt( "XPNormalRatio", 1 );
               // Set the ratio
               n64XPGain *= XPNormalRatio;

               //NMS Flag Boust XP...
               int iNbsBoustXPFlag = lpPlayer->self->ViewFlag(__FLAG_NMS_BOUST_XP);
               if(iNbsBoustXPFlag >0 && iNbsBoustXPFlag <= 10)
                  n64XPGain *= iNbsBoustXPFlag;
            }

            // If this is a player that has boost xp flag.
            if( target->GetType() == U_PC && static_cast< Character * >( target )->GetPlayer()->GetGodFlags() & GOD_BOOST_XP )
            {
               int XPBoostRatio = regKey.GetProfileInt( "XPBoostRatio", 10 );

               boostGain = n64XPGain * XPBoostRatio;
               xp += boostGain;

               // Set player's xp now.
               target->SetXP( xp );            
            }
            else
            {				
               xp += n64XPGain;
            }

            regKey.Close();


            if( theApp.dwLogXPGains != 0 && n64XPGain > 1000 )
            {
               _LOG_PC
                  LOG_WARNING,
                  "Player %s gained %I64u xp killing creature %s ( ID %u ).",
                  (LPCTSTR)target->GetName( _DEFAULT_LNG ),
                  n64XPGain,
                  (LPCTSTR)self->GetName( _DEFAULT_LNG ),
                  self->GetStaticReference()
                  LOG_        
            }

            // If XP was gained and target is a Character.
            if( n64XPGain > 0 && target->GetType() == U_PC )
            {
               // Convert the attacker to character.
               Character *lpCharacter = static_cast< Character * >( target );

               // If the character is NOT grouped
               if( lpCharacter->GetGroup() == NULL )
               {
                  // If the character is a GOD_DEVELOPPER
                  Players *lpPlayer = lpCharacter->GetPlayer();
                  if( lpPlayer != NULL && lpPlayer->GetGodFlags() & GOD_DEVELOPPER )
                  {
                     if( boostGain != 0 )
                     {
                        // Send debug information.
                        TFormat format;
                        target->SendSystemMessage(format("Gained %I64u xp (death).",boostGain));                        
                     }
                     else
                     {
                        // Send debug information.
                        TFormat format;
                        target->SendSystemMessage(format("Gained %I64u xp (death).",n64XPGain));
                     }
                  }

                  if(lpPlayer->self->ViewFlag(__FLAG_SCROLL_XP_TIMESTAMP) != 0)
                  {
                     lpPlayer->self->AddXPScrollBonnus(boostXPScroll);
                  }

                  // Set player's xp.
                  lpCharacter->SetXP( xp );            
               }
               else
               {
                  // Otherwise let the group distribute the XP.
                  lpCharacter->GetGroup()->DistributeKillXP( lpCharacter, n64XPGain );
               }
            }
            target->Unlock(); 
         }
      }
   }

   UINT i;
   vector< ItemToGive > &vDeathItems = npc.vDeathItems;	// for ease of use.
   TemplateList <Unit> *Holdings = (TemplateList<Unit> *)valueOUT;

   CString strItemAll = "";
   CString strItemTmp;

   const int DeathItemPrecision = 1000000;
   for(i = 0; i < npc.vDeathItems.size(); i++)
   {
      double dProb = 0;

      if( target != NULL )
      {
         dProb = (( vDeathItems[ i ].ItemGivePercentage * DeathItemPrecision ) * target->GetLCK() / 100);
      }
      else
      {
         dProb = ( vDeathItems[ i ].ItemGivePercentage * DeathItemPrecision );
      }

      
      if(rnd(0, 100 * DeathItemPrecision ) < dProb )
      {
         Objects *item = new Objects;
         if( item->Create(U_OBJECT, vDeathItems[ i ].ItemID) )
         {
            Holdings->AddToTail(item);
            strItemTmp.Format("%s\r\n",item->GetName(_DEFAULT_LNG));
            strItemAll+=strItemTmp;
         }
         else
         {
            item->DeleteUnit();
         }
      }
   }

   if(bLogDrop)
   {
      _LOG_MONSTERS
         LOG_DEBUG_LVL1,
         "Creature drop item(s): %s",strItemAll
         LOG_
   }
   

   // Il semble qu'il peut y avoir un target=U_PC pour un Player=NULL		//DC for GPs
   if( target == NULL )	//DC for GPs
   {
      return;
   }

   //NMNMNM????... on doit vraiment incrementer les flag de cette creature ?
   // Avant apres la distribution des flags...
   // If target is a character.
   if( target->GetType() != U_PC )
   {
      return;
   }


   target->Lock(); 

   for(i=0; i<npc.vOnDeathFlag.size();i++)
   {
      // Checks flags to give
      if( npc.vOnDeathFlag[i].nDeathGiveFlag != 0 )
      {
         // If flag value should be incremented.
         if( npc.vOnDeathFlag[i].boDeathFlagIncrement )
         {
            // Set increment flag value.
            target->SetFlag( npc.vOnDeathFlag[i].nDeathGiveFlag, target->ViewFlag( npc.vOnDeathFlag[i].nDeathGiveFlag ) + npc.vOnDeathFlag[i].nDeathFlagValue );
         }
         else
         {
            // Sets the flag value.
            target->SetFlag( npc.vOnDeathFlag[i].nDeathGiveFlag, npc.vOnDeathFlag[i].nDeathFlagValue );
         }
      }
   }



   // Cast character.
   Character *lpCharacter = static_cast< Character * >( target );

   if ( lpCharacter->GetPlayer() == NULL )	//DC for GPs
   {
      _LOG_DEBUG
         LOG_CRIT_ERRORS,
         "DC - GPs - target->GetPlayer() is a null pointer while NPCstructure::OnDeath."
         LOG_
   }
   else
   {
      // Give gold to the target	
      DWORD dwGold = rnd( npc.MinGold, npc.MaxGold );    

      // If character is NOT grouped.
      if( lpCharacter->GetGroup() == NULL )
      {
         if(lpCharacter->ViewFlag(__FLAG_SCROLL_OR_TIMESTAMP) != 0)
         {
            lpCharacter->AddORScrollBonnus(dwGold);
         }

         // Simply update its gold.
         int iGoldMultiply = lpCharacter->ViewFlag(__FLAG_NMS_BOUST_GOLD);
         if(iGoldMultiply >0 && iGoldMultiply < 11)
            dwGold *=iGoldMultiply;

         lpCharacter->SetGold( target->GetGold() + dwGold );
      }
      else
      {
         // Otherwise let the group distribute the gold.
         lpCharacter->GetGroup()->DistributeKillGold( lpCharacter, dwGold );
      }
   }

   target->Unlock(); 
}
/******************************************************************************/
// Returns the monster structure associated with a NPC
MonsterStructure *NPCstructure::NPC::GetMonsterStructure( void )
/******************************************************************************/
{
	return this;
}
/******************************************************************************/
// Dead function, only there for virtual purposes
void NPCstructure::OnNPCDataExchange( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{}
/******************************************************************************/
// Returns the structure of the NPC
void NPCstructure::OnGetUnitStructure( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
	(*(MonsterStructure **)valueOUT) = &npc;
}
/******************************************************************************/
// This function is polled to see if the NPC has any "time-relative" behavior
void NPCstructure::OnQuerySchedule( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
}
/******************************************************************************/
void NPCstructure::OnAttackHit( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{}
