/******************************************************************************
Modify for vs2008 (30/04/2009)
Add PVP level range No PVP Drop by NightMare (29/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "GAME_RULES.h"
#include "AutoConfig.h"
#include "Random.h"
#include "ObjectListing.h"
#include "GameDefs.h"
#include "QUestFlagsListing.h"
#include "NPCmacroScriptLng.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
extern Random rnd;
extern CTFCServerApp theApp;
BYTE GAME_RULES::bPVPRule = PVP_LEVEL;

/******************************************************************************/
GAME_RULES::GAME_RULES()
/******************************************************************************/
{    
}
/******************************************************************************/
GAME_RULES::~GAME_RULES()
/******************************************************************************/
{
}
/******************************************************************************/
// Helper inline functions.
inline void AddPVPVar( LPCTSTR lpszValue, int nDefault )
/******************************************************************************/
{
    CAutoConfig::AddRegInt( theApp.csT4CKEY+"PVPDeath", lpszValue, nDefault );
};
/******************************************************************************/
inline int QueryPVPVar( LPCTSTR lpszValue )
/******************************************************************************/
{
    return CAutoConfig::GetIntValue( theApp.csT4CKEY+"PVPDeath", lpszValue );
}
/******************************************************************************/
// Creates the game rule.
void GAME_RULES::Create( void )
/******************************************************************************/
{
   AddPVPVar( "MIN_PVP", 9 );
   AddPVPVar( "PVP_RANGE", 2 );
   AddPVPVar( "MOB_BACKPACK_LOSS", 5 );
   AddPVPVar( "MOB_EQUIP_LOSS", 0 );
   AddPVPVar( "MOB_XP_LOSS", 5 );
   AddPVPVar( "MOB_GOLD_LOSS", 0 );
   AddPVPVar( "MOB_GOLD_DROP", 0 );
   AddPVPVar( "PC_BACKPACK_LOSS", 1 );
   AddPVPVar( "PC_EQUIP_LOSS", 0 );
   AddPVPVar( "PC_XP_LOSS", 1 );
   AddPVPVar( "PC_GOLD_LOSS", 0 );
   AddPVPVar( "PC_GOLD_DROP", 0 );
   AddPVPVar( "CRIME_POINTS", 0 );
   AddPVPVar( "PVP_RANGE_NODROP", 0 );
}
/******************************************************************************/
// This function returns the precision of a blow
DWORD GAME_RULES::GetBlowPrecision(DWORD dwAttackSkill, DWORD dwDodgeSkill, DWORD dwAttackAgi, DWORD dwTargetAgi )
/******************************************************************************/
{
	dwAttackSkill = dwAttackSkill > 0 ? dwAttackSkill : 1;
	dwDodgeSkill = dwDodgeSkill > 0 ? dwDodgeSkill : 1;
	return( ( rnd( 1, dwAttackSkill ) + rnd( 1, dwAttackAgi / 3 ) ) - ( rnd( 1, dwDodgeSkill ) + rnd( 1, dwTargetAgi / 3 ) ) );
}
/******************************************************************************/
// Returns the strength (natural) damage of a unit
DWORD GAME_RULES::GetNaturalDamage(Unit *lpuUnit)
/******************************************************************************/
{
    DWORD rtn = 0;
    
    if( lpuUnit->GetType() == U_PC )
	{
        Character *ch = static_cast< Character * >( lpuUnit );

        if( ch->RangedAttack() )
		{
            if( lpuUnit->GetSTR() >= 20 )
			{
                rtn = ( lpuUnit->GetSTR() - 20 ) / 20;
            }
            if( lpuUnit->GetAGI() >= 20 )
			{
                rtn += ( lpuUnit->GetAGI() - 20 ) / 10;
            }

            return rtn;
        }
    }
    if( lpuUnit->GetSTR() >= 20 )
	{
        rtn = ( lpuUnit->GetSTR() - 20 ) / 5;   
	    rtn = rtn > 0 ? rtn : 0;
    }
	return( rtn );
}
/******************************************************************************/
// Regenerates the unit's HP
void GAME_RULES::HPregen( Unit *lpuRegen )// Unit on which we should regenerate HP
/******************************************************************************/
{
	DWORD HP	= lpuRegen->GetHP();
	DWORD MaxHP = lpuRegen->GetMaxHP();
	BOOL boUpdate = FALSE;

	//TRACE( "\r\n\r\n1-HP %u MaxHP %u\r\n", HP, MaxHP );
	// If HP is lower then maxHP, regen tends to pull it to it's normal state
	if(HP < MaxHP)
	{
        int nRegen = rnd( 0, lpuRegen->GetEND() / 40 ) + 1;

        nRegen = nRegen < 0 ? 0 : nRegen;

        if( rnd( 1, 100 ) <= 60 )
		{
            HP += nRegen;            

            HP = HP > MaxHP ? MaxHP : HP;
            lpuRegen->SetHP( HP, true );
        }        
    }
	// Otherwise regen tends to pull it back to it's normal state
	else if ( HP > MaxHP )
	{		
		HP -= ceil((double)MaxHP * 5 PERCENT);
		HP = HP < MaxHP ? MaxHP : HP; // Le ceil est pour être certain que je retire quelques chose en cas de valeur faible
        lpuRegen->SetHP( HP, true );
	}
}
/******************************************************************************/
// Regenerates the units mana.
void GAME_RULES::ManaRegen( Unit *lpuRegen )// The unit which wants to regenerate
/******************************************************************************/
{
	int nMana		= lpuRegen->GetMana();
	int nMaxMana	= lpuRegen->GetMaxMana();

	// If Mana is lower then maxHP, regen tends to pull it to it's normal state
	if( nMana < nMaxMana )
	{
        int nRegen = rnd( 0, lpuRegen->GetINT() / 160 ) + lpuRegen->GetINT() / 160 + 
                     rnd( 0, lpuRegen->GetWIS() / 160 ) + lpuRegen->GetWIS() / 160 + 1;

        
        if( rnd( 1, 100 ) <= 60 )
		{
            nMana += nRegen;

            nMana = nMana > nMaxMana ? nMaxMana : nMana;

            lpuRegen->SetMana((WORD)nMana);
        }
	}
	// Otherwise regen tends to pull it back to it's normal state
	else if ( nMana > nMaxMana )
	{
		nMana -= ceil((double)nMaxMana * 2 PERCENT); // Le ceil est pour être certain que je retire quelques chose en cas de valeur faible
		nMana = nMana < nMaxMana ? nMaxMana : nMana;

        lpuRegen->SetMana((WORD)nMana); 
	}	
}
/******************************************************************************/
// Applies the penalties of a player's death.
void GAME_RULES::DeathPenalties(
 Character *chUser,         // The character who died.
 Unit *lpAttacker,          // The attacker (may be NULL).
 TemplateList< Unit > *invSpillList,
 TemplateList< Unit > *equipSpillList,// The list of objects to spill.
 DWORD &goldLoss,
 BOOL bPVPChnageDrop,
 double dPVPNewDrop
)
/******************************************************************************/
{
   int wLevelRestriction  = QueryPVPVar( "PVP_RANGE" );
   int wRestrictionNoDrop = QueryPVPVar( "PVP_RANGE_NODROP" );
   enum{ DeathPVP, DeathMob };     

   int nDeathType = DeathMob;

   // Determine if this was a death by PVP.
   if( lpAttacker != NULL ){
      if( lpAttacker->GetType() == U_PC )
      {
         nDeathType = DeathPVP;
      }
   }

   double dXPLoss = 0;
   int    nBackpackLoss = 0;
   int    nEquipLoss = 0;
   int    nGoldLoss = 0;
   int	 nGoldDrop = 0;

   if( nDeathType == DeathMob )
   {
      // Query mob-death variables.
      dXPLoss       = (double)QueryPVPVar( "MOB_XP_LOSS" ) / 100.0;
      nBackpackLoss = QueryPVPVar( "MOB_BACKPACK_LOSS" );        
      nEquipLoss    = QueryPVPVar( "MOB_EQUIP_LOSS" );       
      nGoldLoss     = QueryPVPVar( "MOB_GOLD_LOSS" );
      nGoldDrop     = QueryPVPVar( "MOB_GOLD_DROP" );
   }
   else
   {
      // Query PC-death variables.
      dXPLoss       = (double)QueryPVPVar( "PC_XP_LOSS" ) / 100.0;
      nBackpackLoss = QueryPVPVar( "PC_BACKPACK_LOSS" );
      nEquipLoss    = QueryPVPVar( "PC_EQUIP_LOSS" );
      nGoldLoss     = QueryPVPVar( "PC_GOLD_LOSS" );
      nGoldDrop     = QueryPVPVar( "PC_GOLD_DROP" );
      if(bPVPChnageDrop)
      {
         nBackpackLoss = (int)dPVPNewDrop;
         nEquipLoss    = (int)dPVPNewDrop;
      }
   }

   // si restriction des drop et que attacker est un PLAYER...
   if(wRestrictionNoDrop && lpAttacker != NULL && lpAttacker->GetType() == U_PC) //block drop quand drop only
   {
      if( abs( chUser->GetLevel() - lpAttacker->GetLevel() ) >= wLevelRestriction ) //block drop quand drop only
      {
         dXPLoss       = 0;
         nBackpackLoss = QueryPVPVar( "PC_BACKPACK_LOSS" );
         nEquipLoss    = 0;
         nGoldLoss     = 0;
         nGoldDrop     = 0;

         if(bPVPChnageDrop)
         {
            nBackpackLoss = (int)dPVPNewDrop;
            nEquipLoss    = (int)dPVPNewDrop;
         }
      }
   }

   if( nDeathType == DeathMob && theApp.dwPVMDropDisabled)
   {
      dXPLoss       = 0;
      nBackpackLoss = 0;
      nEquipLoss    = 0;
      nGoldLoss     = 0;
      nGoldDrop     = 0;
   }
   else if( theApp.dwPVPDropDisabled )
   {
      dXPLoss       = 0;
      nBackpackLoss = 0;
      nEquipLoss    = 0;
      nGoldLoss     = 0;
      nGoldDrop     = 0;
   }


   // Get the level cap of the user.
   WORD wLevel = chUser->GetLevel();

   if(wLevel >= MAX_LEVEL_XP)
   {
      wLevel = MAX_LEVEL_XP - 1;
   }
   else if( wLevel == 0 )
   {
      wLevel = 1;
   }

   // Get the differenct
   __int64 qwXpLoss;
   if( chUser->GetXP() > Character::sm_n64XPchart[wLevel - 1] )
   {
      qwXpLoss = chUser->GetXP() - Character::sm_n64XPchart[wLevel - 1];
      qwXpLoss = static_cast< __int64 >( static_cast< double >( qwXpLoss ) * dXPLoss );
   }
   else
   {
      // If user is already under this level's XP.
      qwXpLoss = 0;
   }

   __int64 qwCurrentXP = chUser->GetXP();
	chUser->SetFlag(__FLAG_DEATH_LOST_XP, qwCurrentXP);
   if( qwCurrentXP > qwXpLoss )
   {
      qwCurrentXP -= qwXpLoss;
   }
   else
   {
      qwCurrentXP = 0;
   }

   chUser->SetXP( qwCurrentXP ,true);

   // Scroll through the backpack items to determine which ones are dropped.
   TemplateList< Unit > *lpBackpack = chUser->GetBackpack();

   if( lpBackpack != NULL )
   {
      lpBackpack->ToHead();
      while( lpBackpack->QueryNext() )
      {
         // If this item was lost.
         if( rnd( 0, 100 ) < nBackpackLoss )
         {
            Objects *lpObject = static_cast< Objects * >( lpBackpack->Object() );

            _item *lpItemStructure = NULL;

            lpObject->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItemStructure );               								

            // If the object can be dropped.
            if( !( lpItemStructure->dwDropFlags & CANNOT_DROP_ITEM ) && lpObject->GetStaticReference() != (UINT)__OBJ_GOLD )
            {
               lpObject->Remove();
               if( lpObject->GetQty() == 0 )
               {
                  // Remove it from the backpack
                  lpBackpack->Remove();

                  // Use at least one object.
                  lpObject->Add( 1 );
               }
               else
               {
                  // Create a copy item.
                  DWORD baseID = lpObject->GetStaticReference();
                  lpObject = new Objects;
                  if( !lpObject->Create( U_OBJECT, baseID ) )
                  {
                     lpObject->DeleteUnit();
                     lpObject = NULL;
                  }
               }

               if( lpObject != NULL )
               {
                  // And add it to the spilled objects list.
                  invSpillList->AddToTail( lpObject );
               }
            }
         }
      }
   }

   // Scroll through the equipped items.
   Unit **lpuEquipped = chUser->GetEquipment();

   int i = 0;
   int j;
   for( i = 0; i < EQUIP_POSITIONS; i++ )
   {
      // If there is an item at this position.
      if( lpuEquipped[ i ] != NULL )
      {
         // If this item gets lost.
         if( rnd( 0, 100 ) < nEquipLoss )
         {
            Unit *lpObject = lpuEquipped[ i ];

            _item *lpItemStructure = NULL;

            lpObject->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItemStructure );

            // If the object can be dropped.
            if( !( lpItemStructure->dwDropFlags & CANNOT_DROP_ITEM ) && lpObject->GetStaticReference() != (UINT)__OBJ_GOLD )
            {
               chUser->unequip_object( i, true );

               // Remove all occurences of this item (as it may appear twice) from the equipment.
               for( j = 0; j < EQUIP_POSITIONS; j++ )
               {
                  if( lpuEquipped[ j ] == lpObject )
                  {
                     lpuEquipped[ j ] = NULL;
                  }
               }

               // Add the object to the spilled list.
               equipSpillList->AddToTail( lpObject );
            }
         }
      }
   }   	

   DWORD oldGold = chUser->GetGold();
   goldLoss = oldGold * nGoldLoss / 100;

	//Moen_OK : deplacer ici aulieu de deathOld et NMS
	// S'il y a une perte d'or.
	if (goldLoss > 0) 
   {
		// On perd moins que ce qu'on possède
		if( goldLoss < oldGold ) 
      {
			chUser->SetGold( oldGold - goldLoss );
			chUser->SetFlag(__FLAG_DEATH_LOST_GOLD, goldLoss); // On sauvegarde l'or pour le restaure ( Perte)
		}
		// on perd tout
		else 
      {
			chUser->SetGold( 0 );
			chUser->SetFlag(__FLAG_DEATH_LOST_GOLD, oldGold); // On sauvegarde l'or pour le restaure, on a tout perdu on sauvegarde juste oldGold
		}
	}
	// Il n'y a pas de pertes, on laisse à 0
	else {
		chUser->SetFlag(__FLAG_DEATH_LOST_GOLD, 0); 
	}
    	
   // If some gold pieces must be dropped on the ground
   if( nGoldDrop > 0 )
   {
      DWORD goldDrop = goldLoss * nGoldDrop / 100;

      if( goldDrop > 1 )
      {			
         Objects *lpGold = new Objects;
         if( !lpGold->Create( U_OBJECT, __OBJ_GOLD ) )
         {				
            lpGold->DeleteUnit();
            lpGold = NULL;
         }
         else
         {				
            lpGold->Add( goldDrop );
            invSpillList->AddToTail( lpGold );
         }
      }
   }

   // Put player to half HP.
   chUser->SetHP( chUser->GetHP() / 2, true );
}

/******************************************************************************/
// Determines if two units are in PVP range.
BOOL GAME_RULES::InPVP(
 Unit *lpuFirst, // First unit.
 Unit *lpuSecond // Second unit.
)
/******************************************************************************/
{    
   int wLevelRestriction  = QueryPVPVar( "PVP_RANGE" );
   int wMinLevel          = QueryPVPVar( "MIN_PVP" );
   int wRestrictionNoDrop = QueryPVPVar( "PVP_RANGE_NODROP" );

   if(wRestrictionNoDrop)
      wLevelRestriction = 99999;
   // If one of the opponents doesn't exist or if the opponent is himself.
   if( lpuFirst == NULL || lpuSecond == NULL || ( lpuFirst == lpuSecond ) )
   {
      return TRUE;
   }

   // If both opponents are in a FULL PVP area, allow PVP interaction.
   if( lpuFirst->GetUnderBlockMap() == __AREA_FULL_PVP && lpuSecond->GetUnderBlockMap() == __AREA_FULL_PVP )
   {
      return TRUE;
   }

   // If both parties are players.
   if( lpuFirst->GetType() == U_PC && lpuSecond->GetType() == U_PC )
   {
      // Transfert them into characters.
      Character *lpFirstChar    = static_cast< Character * >( lpuFirst );
      Character *lpSecondChar   = static_cast< Character * >( lpuSecond );
      Players   *lpFirstPlayer  = lpFirstChar->GetPlayer();
      Players   *lpSecondPlayer = lpSecondChar->GetPlayer();

      // If any of the player structures have been deleted.
      if( lpFirstPlayer == NULL || lpSecondPlayer == NULL )
      {
         // Refuse pvp.
         return FALSE;
      }

      // If either of the player cannot PVP.
      if( !lpFirstPlayer->CanPVP() || !lpSecondPlayer->CanPVP() )
      {
         return FALSE;
      }

      // If any of them are in full-PVP mode.
      if( lpFirstPlayer->IsFullPVP() || lpSecondPlayer->IsFullPVP() )
      {
         return TRUE;
      }

      // If both parties are in a group.
      if( lpFirstChar->GetGroup() != NULL && lpSecondChar->GetGroup() != NULL )
      {
         // If the second character is a member of the first character's group.
         if( lpFirstChar->GetGroup()->IsGroupMember( lpSecondChar ) )
         {
            // No PVP allows.
            return FALSE;
         }
      }
   }

   BOOL boPVP = TRUE;
   if (bPVPRule == PVP_LEVEL)
   {
      // Both must be 
      if( lpuFirst->GetType() == U_PC && lpuSecond->GetType() == U_PC )
      {
         // If the unit's levels aren't in range.
         if( abs( lpuFirst->GetLevel() - lpuSecond->GetLevel() ) >= wLevelRestriction ||
            lpuFirst->GetLevel() < wMinLevel || lpuSecond->GetLevel() < wMinLevel )
         {
            boPVP = FALSE;
         }
      }
   }

	return boPVP;
}
/******************************************************************************/
// Determines if the PVP rules apply in this area.
BOOL GAME_RULES::InSafeHaven(
 Unit *lpuFirst, // 1st
 Unit *lpuSecond // 2nd
)
/******************************************************************************/
{

   // If either the attacker or the attacked is on a safe-haven
   // If both the attacker and target are players.
   if( (lpuFirst->GetUnderBlockMap()  == __SAFE_HAVEN        || 
        lpuFirst->GetUnderBlockMap()  == __INDOOR_SAFE_HAVEN ||
        lpuSecond->GetUnderBlockMap() == __SAFE_HAVEN        ||
        lpuSecond->GetUnderBlockMap() == __INDOOR_SAFE_HAVEN   )  && 
       (lpuFirst->GetType() == U_PC                            && 
        lpuSecond->GetType() == U_PC                        ))
   {
      return TRUE;
   }

   if((lpuFirst->GetType() == U_PC && lpuSecond->GetType() == U_PC) &&
      lpuFirst->ViewFlag(__FLAG_NMS_PLAYER_DEATH)!= 0 || lpuSecond->ViewFlag(__FLAG_NMS_PLAYER_DEATH)!= 0)
   {
      return TRUE;
      //un des PJ est mort donc pas daction....
   }

   return FALSE;
}



UINT GAME_RULES::NMPVPCanAttack    ( Unit *lpuFirst, Unit *lpuSecond )
{
   Character *chF = static_cast< Character * >( lpuFirst );
   Character *chS = static_cast< Character * >( lpuSecond );

   //valide le frinedly ici
   if(lpuFirst->GetType() == U_PC && lpuSecond->GetType() == U_NPC)
   {
      if(chF->ViewFlag(__FLAG_PJ_VS_MONSTER_FRIENDLY) > 0 && lpuSecond->GetFriendlyID() > 0)
      {
         DWORD dwFlagID      = 3000000+lpuSecond->GetFriendlyID();
         DWORD dwFactionMask = PLFactionMask(chF->ViewFlag(__FLAG_PJ_VS_MONSTER_FRIENDLY));
         if( CheckGlobalFlag(dwFlagID) & dwFactionMask)
            return 1000; //ami
      }
   }

   /////////////////////////////////////////////////////////////////////////////////
   // SAFE ZONE on ne MANAGE PAS...
   /////////////////////////////////////////////////////////////////////////////////
   if(InSafeHaven(lpuFirst,lpuSecond))
      return 0; // fait comme si pouvais se battre sa sera bloquerv ailleur...

   /////////////////////////////////////////////////////////////////////////////////
   // En ZONE ARENE 
   /////////////////////////////////////////////////////////////////////////////////
   if(lpuFirst ->GetUnderBlockMap() == __FULL_PVP_CANNOT_REALLY_DIE_DROP_ORROB &&
      lpuSecond->GetUnderBlockMap() == __FULL_PVP_CANNOT_REALLY_DIE_DROP_ORROB    )
   {
      // Zone de combat / Arene on peu se batte peu importe le status...
      return 0;
   }

   /////////////////////////////////////////////////////////////////////////////////
   // En ZONE ARENE CAST SPELL 
   /////////////////////////////////////////////////////////////////////////////////
   if(lpuFirst ->GetUnderBlockMap() == __FULL_PVP_CANNOT_REALLY_DIE_DROP_ORROB_CAST_SPELL &&
      lpuSecond->GetUnderBlockMap() == __FULL_PVP_CANNOT_REALLY_DIE_DROP_ORROB_CAST_SPELL    )
   {
      // Zone de combat / Arene on peu se batte peu importe le status...
      return 0;
   }

   /////////////////////////////////////////////////////////////////////////////////
   // En ZONE ARENE GAME
   /////////////////////////////////////////////////////////////////////////////////
   if(( lpuFirst ->GetUnderBlockMap()   == __ARENAGAME_FULL_PVP     ||
      lpuFirst ->GetUnderBlockMap()   == __ARENAGAME_BT_FULL_PVP  ||
      lpuFirst ->GetUnderBlockMap()   == __ARENAGAME_RT_FULL_PVP     )
      &&
      ( lpuSecond ->GetUnderBlockMap()   == __ARENAGAME_FULL_PVP     ||
      lpuSecond ->GetUnderBlockMap()   == __ARENAGAME_BT_FULL_PVP  ||
      lpuSecond ->GetUnderBlockMap()   == __ARENAGAME_RT_FULL_PVP     )
      )
   {
      //en zone arene game on peu se battre peu importe status, mais on valide si il sont dans la meme equipe ou pas...
      if(chF->GetArenaTeam() == chS->GetArenaTeam())
         return 99; //on ne peu attaquer un membre de la meme equipe...
      else
         return 0; //peu combattre...
   }

   //valide le frinedly ici
   if(lpuFirst->GetType() == U_PC && lpuSecond->GetType() == U_PC && theApp.m_dwFriendlyBlockPJAttack)
   {
      if(chF->ViewFlag(__FLAG_PJ_VS_MONSTER_FRIENDLY) > 0 && chS->ViewFlag(__FLAG_PJ_VS_MONSTER_FRIENDLY) > 0)
      {
         if( chF->ViewFlag(__FLAG_PJ_VS_MONSTER_FRIENDLY) == chS->ViewFlag(__FLAG_PJ_VS_MONSTER_FRIENDLY))
            return 1000; //ami
      }
   }


   if(theApp.m_dwPVPSyetem2Actif == 0) //NMPVPCanAttack
      return 0;
 
   if(lpuFirst->GetType() == U_PC && lpuSecond->GetType() == U_PC)
   {
      if(lpuFirst == lpuSecond)
         return 0;
      if(lpuFirst == NULL || lpuSecond == NULL)
         return 0;

      int iCurTime = time(NULL);
      lpuFirst->SetFlag(__FLAG_MODE_COMBAT_TIMES,iCurTime);
      lpuSecond->SetFlag(__FLAG_MODE_COMBAT_TIMES,iCurTime);


      /////////////////////////////////////////////////////////////////////////////////
      // UN des 2 est DEJA MORT on PERMET PAs DATTACK
      /////////////////////////////////////////////////////////////////////////////////
      if(lpuFirst ->ViewFlag(__FLAG_NMS_PLAYER_DEATH)!= 0  || 
         lpuSecond->ViewFlag(__FLAG_NMS_PLAYER_DEATH)!= 0)
      {
         return 98;
      }

      /////////////////////////////////////////////////////////////////////////////////
      // HLL couper de combat essaie attaquer... 
      /////////////////////////////////////////////////////////////////////////////////
	   if(lpuFirst  ->ViewFlag(__FLAG_NMS_CANT_ATTACK_OTHER_PLAYER) == 1  && lpuSecond  ->ViewFlag(__FLAG_NMS_CANT_ATTACK_OTHER_PLAYER) == 1 )
      {
         return 0;
      }
      else if(lpuFirst  ->ViewFlag(__FLAG_NMS_CANT_ATTACK_OTHER_PLAYER) == 1  && theApp.IsTargetOnList(lpuFirst->GetID(),lpuSecond->GetID()) )
      {
         return 0;
      }
      else if(lpuFirst  ->ViewFlag(__FLAG_NMS_CANT_ATTACK_OTHER_PLAYER) == 1)
      {
         return 1;
      }
      /////////////////////////////////////////////////////////////////////////////////
      // RP/HRP contre HRP bloquer
      /////////////////////////////////////////////////////////////////////////////////
      theApp.AddTargetOnList(lpuSecond->GetID(),lpuFirst->GetID());
      return 0;
   }
   return 0;
}






