/******************************************************************************
Modify for vs2008 (31/04/2009)
Add Incremental spell Flag, NM PVP System Check,Combat Mode Server Side by Nightmare (29/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "SpellMessageHandler.h"
#include "Broadcast.h"
#include "TFC_MAIN.h"
#include "GameDefs.h"
#include "format.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

LPSPELL_STRUCT SpellMessageHandler::lpSpellTable[ MAX_SPELLS ];
extern Random rnd;
extern CTFCServerApp theApp;

static CLock     g_ALockCastSpell;

/******************************************************************************/
// Creates the spell message handler
void SpellMessageHandler::Create( void )
/******************************************************************************/
{
	int i;
	// Init the proc table to NULL
	for( i = 0; i < MAX_SPELLS; i++ )
	{
		lpSpellTable[ i ] = NULL;
	}
}
/******************************************************************************/
// Destroys all registered spells.
void SpellMessageHandler::Destroy( void )
/******************************************************************************/
{
	int i;
	// Init the proc table to NULL
	for( i = 0; i < MAX_SPELLS; i++ )
	{
		if( lpSpellTable[ i ] != NULL )
		{
			// delete lpSpellTable[ i ]->lpszSpellName;
            lpSpellTable[ i ]->tlEffects.AnnihilateList();
            lpSpellTable[ i ]->saAttrib.tlskSkillRequired.AnnihilateList();
        
            lpSpellTable[ i ]->Delete();
        }
	}
}
/******************************************************************************/
// Registers a spell for use.
void SpellMessageHandler::RegisterSpell(
 WORD wSpellID,					// unique ID of the spell.
 LPSPELL_STRUCT lpSpellStruct	// Procedure attached to spell.
)
/******************************************************************************/
{
	if( wSpellID >= SPELL_ID_OFFSET )
	{
		if( wSpellID - SPELL_ID_OFFSET < MAX_SPELLS )
		{
			if( lpSpellTable[ wSpellID - SPELL_ID_OFFSET ] )
			{
                _LOG_DEBUG
                    LOG_DEBUG_LVL1,
                    "Found duplicate spell ID %u, unregistering previous one. (overriding it)",
                    wSpellID
                LOG_

                lpSpellTable[ wSpellID - SPELL_ID_OFFSET ]->Delete();
			}

            //TRACE( "\r\nRegistering spell ID %u", wSpellID ); //NMNMNM ben trop de log pour rien
			lpSpellTable[ wSpellID - SPELL_ID_OFFSET ] = lpSpellStruct;
			
		}
		else
		{
			CString TheText;
			TheText.Format(
				"Tried to register the spell ID %u which is beyond the maximum number of spells. Spell Name: %s (Desc: %s)",
				wSpellID,
				lpSpellStruct->GetName(_DEFAULT_LNG),
				lpSpellStruct->GetDesc(_DEFAULT_LNG).c_str()
			);
			//Removed the message box. Message is now getting sent to the DEBUG LOG
			//Messagebox makes server shutdown and never go back till the sysop go to the console and clicks ok.
			//MessageBox( NULL, TheText, "DLL error", MB_OK );
			_LOG_DEBUG
				LOG_CRIT_ERRORS,
				TheText
			LOG_
		}
	}
	else
	{
		CString TheText;
		TheText.Format( 
			"Tried to register the spell ID %u which is below the lowest available spell ID. Spell Name: %s (Desc: %s)",
			wSpellID,
			lpSpellStruct->GetName(_DEFAULT_LNG),
			lpSpellStruct->GetDesc(_DEFAULT_LNG)
		);
		//Removed the message box. Message is now getting sent to the DEBUG LOG
		//Messagebox makes server shutdown and never go back till the sysop go to the console and clicks ok.
		//MessageBox( NULL, TheText, "DLL error", MB_OK );
		_LOG_DEBUG
			LOG_CRIT_ERRORS,
			TheText
		LOG_
	}
}
/******************************************************************************/
inline bool IsTargetOK( LPSPELL_STRUCT lpSpell, Unit *self, Unit *target )
/******************************************************************************/
{
    if( target == NULL )
	{
        return false;
    }

    switch( lpSpell->bTarget )
	{
		case TARGET_UNIT_ANY_FAVOR_PC:    
		case TARGET_UNIT_ANY: break;			
		case TARGET_UNIT_OBJECT:
			if( target->GetType() != U_OBJECT )
			{   
				return false;
			}
			break;
		case TARGET_UNIT_NPC:
			if( target->GetType() != U_NPC )
			{
				return false;
			}
			break;
		case TARGET_UNIT_PC:
			if( target->GetType() != U_PC )
			{
				return false;
			}
			break;
		case TARGET_UNIT_LIVING_FAVOR_PC:
		case TARGET_UNIT_LIVING_FAVOR_NPC:
		case TARGET_UNIT_LIVING:
			if( target->GetType() != U_PC && target->GetType() != U_NPC )
			{
				return false;
			}
			break;
		case TARGET_SELF:    
			break; 
		case TARGET_POSITION: 
			break;
		case TARGET_UNIT_PC_NONSELF:                            
			if( target == self )
			{
				return false;
			}
			else if( target->GetType() != U_PC )
			{
				return false;
			}
			break;			
		case TARGET_UNIT_LIVING_FAVOR_NPC_NONSELF:
		case TARGET_UNIT_LIVING_NONSELF:
			if( target == self )
			{
				return false;
			}
			else if( target->GetType() != U_NPC && target->GetType() != U_PC )
			{
				return false;
			}
			break;			
		case TARGET_UNIT_ANY_NONSELF:
			if( target == self )
			{
				return false;
			}
			break;
		case TARGET_GROUP_SELF:
			if( self == target )
			{
				return true;
			}
		case TARGET_GROUP_UNIT:    
		case TARGET_GROUP_POSITION:
			{
				// Non-PC casters don't have groups.
				if( self->GetType() != U_PC )
				{
					return false;
				}

				// The caster is always a member of its own group.
				if( self == target )
				{           
					return true;
				}

				// Must target PCs.
				if( target->GetType() != U_PC )
				{
					return false;
				}
				Character *selfCh = static_cast< Character * >( self );
				Character *targetCh = static_cast< Character * >( target );

				// If the caster isn't in a group.
				if( selfCh->GetGroup() == NULL )
				{
					return false;
				}
				// If the target character isn't in the caster's group.
				if( !selfCh->GetGroup()->IsGroupMember( targetCh ) )
				{   
					return false;
				}
			}
			break;
		case TARGET_NONGROUP_SELF:
			if( target == self )
			{
				return false;
			}
		case TARGET_NONGROUP_UNIT:    
		case TARGET_NONGROUP_POSITION:
			// Cannot target self nor can caster be a non-PC.
         //if(!target->CanAttack())
         //{              
         //   return false;
         //}
			if( target == self )
			{
				return false;
			}
			// If caster isn't a PC it cannot be in a group.
			if( self->GetType() != U_PC )
			{
				return true;
			}

			// Only check PC characters.
			if( target->GetType() == U_PC )
			{
				Character *selfCh = static_cast< Character * >( self );
				Character *targetCh = static_cast< Character * >( target );
				// If the player isn't in a group.
				if( selfCh->GetGroup() == NULL )
				{
					return true;
				}                
				// If the target character is in the caster's group.
				if( selfCh->GetGroup()->IsGroupMember( targetCh ) )
				{              
					return false;
				}
			}
	}    
	return true;
}
/******************************************************************************/
inline bool SafeMultiLock( CLock *cLockL, CLock *cLockR, DWORD maxTry = 10 )
/******************************************************************************/
{
	DWORD tries = 0;
	cLockL->Lock();
	while( !cLockR->PickLock() )
	{
		cLockL->Unlock();
		Sleep( 0 );
		tries++;
		if( tries >= maxTry )
		{
			return false;
		}
		cLockL->Lock();
    }
    return true;
}
/******************************************************************************/
// Casts the spell
static bool CastSpell(
 LPSPELL_STRUCT lpSpell,
 Unit *self,
 Unit *target,
 WorldPos wlPos,
 double range
)
/******************************************************************************/
{
    // Lock the involved units

   if( target != NULL )
   {
      _LOG_DEBUG
         LOG_DEBUG_HIGH,
         "[spell]Self=0x%x, Target=0x%x",
         self,
         target
         LOG_       

         // If the two units were not locked within reasonable time, abort.
         if( !SafeMultiLock( self, target ) )
            return false;

         // Virtually create the unit.
         target->CreateVirtualUnit();
   }
   else
   {
      self->Lock();
   }

   if( target == NULL )
   {
      _LOG_DEBUG
         LOG_DEBUG_HIGH,
         "Spell %u triggered. Caster %s (ID %u). Target is caster.",
         lpSpell->wSpellID,
         self->GetName(_DEFAULT_LNG),
         self->GetID()
         LOG_
   }
   else
   {
      _LOG_DEBUG
         LOG_DEBUG_HIGH,
         "Spell %u triggered. Caster %s (ID %u). Target is %s (ID %u).",
         lpSpell->wSpellID,
         (LPCTSTR)self->GetName(_DEFAULT_LNG),
         self->GetID(),
         (LPCTSTR)target->GetName(_DEFAULT_LNG),
         target->GetID()
         LOG_
   }


   if(lpSpell)
   {

      //test de la maniere dont les effet sont caller car il etais possible qu<une liste call une liste de spell du meem
      //et restait coller car attendait queil finisse, mais la recursivite est pas gerer
      //on vas essayer de simplement boucler sur la liste etant donner que dans ce cas precis, les spell effect et spell
      //sont cree au loading et detruit au close dui serveur donc ils sont toujours valide suffit de les caller...

      for(int kk=0;kk<lpSpell->tlEffects.NbObjects();kk++)
      {
         if(lpSpell->tlEffects.GetObjectByIdx(kk))
            lpSpell->tlEffects.GetObjectByIdx(kk)->CallEffect( self, NULL, target, wlPos, range );
      }
      

      /*
      lpSpell->tlEffects.Lock();
      try
      {
         lpSpell->tlEffects.ToHead();

         // Call each effect on spell.
         while( lpSpell->tlEffects.QueryNext() )
         {
            lpSpell->tlEffects.Object()->CallEffect( self, NULL, target, wlPos, range );
         }
      }
      catch (...)
      {
         int crash = 0;
      }
      lpSpell->tlEffects.Unlock();
      */
   }
   else 
   {
   }

   // Unlock resources.
   self->Unlock();
   if( target != NULL )
   {
      target->Unlock();
      target->DeleteUnit();// Delete the unit's virtual image. See the preceeding target->CreateVirtualUnit()
   }

   return true;
}
/******************************************************************************/
// Activates a spell
BOOL SpellMessageHandler::ActivateSpell ( SPELL_PROC_PROTOTYPE_C )// Spell prototype
/******************************************************************************/
{
   //CAutoLock autoCastSpellLock( &g_ALockCastSpell);
   // *must* have a self.
   if( self == NULL )
      return FALSE;

   WorldMap *wlWorld = TFCMAIN::GetWorld( self->GetWL().world );

   // Spell must affect a valid world.
   if( wlWorld == NULL )
      return FALSE;


   TRACE( "\r\nTrying to cast spell ID %u.", wSpellID );

   // If the spellID is not within the spell table.
   if( wSpellID < SPELL_ID_OFFSET || ( wSpellID - SPELL_ID_OFFSET ) >= MAX_SPELLS )
      return FALSE;
   if( lpSpellTable[ wSpellID - SPELL_ID_OFFSET ] == NULL )
      return FALSE;

   LPSPELL_STRUCT lpSpell = lpSpellTable[ wSpellID - SPELL_ID_OFFSET ];

   // If the spell is a self spell
   if( lpSpell->bTarget == TARGET_SELF || lpSpell->bTarget == TARGET_GROUP_SELF || lpSpell->bTarget == TARGET_NONGROUP_SELF )
   {
      target = self;
   }

   // If the spell fails.
   //if( ( static_cast< double >( rnd( 1, 100000 ) ) / 1000 ) > lpSpell->successPercentage.GetBoost( self, target ) )
   //   return FALSE;
   int iSuccess = lpSpell->successPercentage.GetBoost( self, target ) ;
   if( rnd( 0, 100 ) > iSuccess || iSuccess == 0)
     return FALSE;

   // If this spell requires PVP check before casting.
   if( lpSpell->boPVPcheck && target != NULL && !bSelfValidationFREE)
   {
      if( !GAME_RULES::InPVP( self, target ) )
      {
         // Sends a 'cannot cast spell because of pvp blabla' msg to caster.
         self->SendSystemMessage( _STR( 463, self->GetLang() ) );
         return FALSE;
      }

      // Return if any of the two opponents are in a safe area.
      if( GAME_RULES::InSafeHaven( self, target ) )
      {
         // Sends a 'cannot cast spell because of pvp blabla' msg to caster.
         self->SendSystemMessage( _STR( 463, self->GetLang() ) );
         return FALSE;
      }


      Character *chSelf = static_cast< Character * >( self );
      if(chSelf)
      {
         if(self->GetType() == U_PC && target->GetType() == U_PC && !chSelf->GetNMCombatMode() && !theApp.IsTargetOnList(self->GetID(),target->GetID()))
         {
            // cannot send spell PVP if combat mode not activated...
            if(self == target)
               self->SendSystemMessage( _STR( 15047, self->GetLang() ) );
            else
               self->SendSystemMessage( _STR( 15036, self->GetLang() ) );
            return FALSE;
         }
      }


      UINT uiRet = GAME_RULES::NMPVPCanAttack( self, target );
      if( uiRet >0 )
      {
         if(uiRet == 1)
            self->SendInfoMessage(_STR( 15037 , self->GetLang() ),0x0570D5);
         else if(uiRet == 2)
            self->SendInfoMessage(_STR( 15038 , self->GetLang() ),0x0570D5);
         else if(uiRet == 3)
            self->SendInfoMessage(_STR( 15039 , self->GetLang() ),0x0570D5);
         else if(uiRet == 98)
            self->SendInfoMessage(_STR( 15040 , self->GetLang() ),0x0570D5);
         else if(uiRet == 99)
            self->SendInfoMessage(_STR( 15345 , self->GetLang() ),0x0570D5);
         else if(uiRet == 1000)
            self->SendInfoMessage(_STR( 15511 , self->GetLang() ),0x0570D5);
         else 
            self->SendInfoMessage(_STR( 15041 , self->GetLang() ),0x0570D5);
         return FALSE;
      }

   }

   // Determine the starting and ending course
   WorldPos wlFrom = self->GetWL();
   WorldPos wlTo   = target == NULL ? wlPos : target->GetWL();

   // If spell requires a line of sight, and spell isn't targetted to self.
   if( lpSpell->nLineOfSight != 0 && target != self )
   {
      // Determine the starting and ending course
      Unit *lpCollisionUnit = NULL;
      WorldPos wlCollidePos = { 0, 0, 0 };
      // If a collision was detected.
      if( wlWorld->GetCollisionPos( wlFrom, wlTo, &wlCollidePos, &lpCollisionUnit, false ) )
      {                            
         // Setup new target according to the collision pos.
         target = lpCollisionUnit;
         // Set the collide pos as the spell's target pos.
         wlPos = wlCollidePos;                                
      }
   }
   else if( target != NULL )
   {
      wlPos = target->GetWL();
   }

   // If there is a target which is not targettable.
   if(!bSelfValidationFREE)
   {
      if( target != NULL && !IsTargetOK( lpSpell, self, target ) && lpSpell->bTarget != TARGET_NONGROUP_SELF )
      {
         // Send a 'wrong unit' message.
         self->SendSystemMessage( _STR( 470, self->GetLang() ) );
         return FALSE;
      }
   }
   

   if( lpSpell->bTarget == TARGET_NONGROUP_SELF )
   {
      wlPos = self->GetWL();
      target = NULL;
   }

   DWORD masterEffectId = 0;

   // If there is a visual effect associated with the spell.
   if( lpSpell->wVisualEffect != 0 )
   {
      DWORD dwTargetID;
      WorldPos wlTargetPos;

      // If a victim was selected.
      if( target != NULL )
      {
         // The target is the victim.
         dwTargetID  = target->GetID();
         wlTargetPos = target->GetWL();
      }
      else
      {
         // Otherwise use the specified position as target.
         dwTargetID  = 0;
         wlTargetPos = wlPos;
      }

      TRACE( " effect %u from ID %u to ID %u at ( %u, %u )\n",lpSpell->wVisualEffect,self->GetID(),dwTargetID,wlTargetPos.X,wlTargetPos.Y);

      masterEffectId = GetNextGlobalEffectID();
      Broadcast::BCSpellEffect( self->GetWL(), _DEFAULT_RANGE,lpSpell->wVisualEffect,self->GetID(),dwTargetID,self->GetWL(),wlTargetPos,masterEffectId,0);
   }

   // Deal the spell's exhaust.
   self->DealExhaust(lpSpell->sInducedExhaust.bfAttack.GetBoost( self, target ),
                     lpSpell->sInducedExhaust.bfMental.GetBoost( self, target ),
                     lpSpell->sInducedExhaust.bfMove  .GetBoost( self, target )     );

   // Cast the spell which hits the center unit.
   bool success = CastSpell( lpSpell, self, target, wlPos, 0 );

   if(success && lpSpell->nIncrementedFlag >0)
   {
      //on doit incrementer le flag du caster...
      self->SetFlag(lpSpell->nIncrementedFlag,self->ViewFlag(lpSpell->nIncrementedFlag)+1);
   }

   // --- Area effects
   // If this is not an area effect.
   if( lpSpell->bRange == 0 )
   {
      return success;
   }

   // The spell's center area.
   WorldPos wlCenter = wlPos;

   // Get X & Y starting position as well as X & Y extension.
   int nMinX = wlCenter.X - lpSpell->bRange;
   nMinX = nMinX < 0 ? 0 : nMinX;

   int nMinY = wlCenter.Y - lpSpell->bRange;
   nMinY = nMinY < 0 ? 0 : nMinY;

   int nMaxX = wlCenter.X + lpSpell->bRange;
   nMaxX = nMaxX >= (int)wlWorld->GetMAXX() ? (int)wlWorld->GetMAXX() - 1 : nMaxX;

   int nMaxY = wlCenter.Y + lpSpell->bRange;
   nMaxY = nMaxY >= (int)wlWorld->GetMAXY() ? (int)wlWorld->GetMAXY() - 1 : nMaxY;

   // Scroll through the area.
   int x, y;

   DWORD centeredId;    
   WorldPos centeredPos;
   if( target == NULL )
   {
      centeredPos = wlPos;
      centeredId = 0;
   }
   else
   {
      centeredPos = target->GetWL();
      centeredId = target->GetID();
   }

   for( x = nMinX; x < nMaxX; x++ )
   {
      for( y = nMinY; y < nMaxY; y++ )
      {
         // Get relative X and Y
         int relX = ( x - wlCenter.X );
         int relY = ( y - wlCenter.Y );

         //NMNMNM remets le bug des spells
         // If position is dead center.
         /*
         if( relX == 0 && relY == 0 )
         {
         continue;
         }
         */

         // Find the ray length of this point.
         double dblRay = sqrt( double(relX * relX + relY * relY) );

         // If the ray is not within the spell's range.
         if( dblRay >= lpSpell->bRange )
         {
            continue;
         }

         WorldPos wlTopUnitPos = { x, y, wlCenter.world };

         // Get the unit standing on this spot.
         target = wlWorld->ViewTopUnit( wlTopUnitPos );

         // If a no unit was found at this position.
         if( target == NULL/*  || target == self*/)
         {
            continue;
         }
         // Do not target hives.
         if(target->GetType() == U_MINIONS ||  target->GetType() == U_HIVE || target->GetType() == U_OBJECT || target->GetAppearance() == 0)
         {
            continue;
         }
         // If its a player
         if( target->GetType() == U_PC )
         {
            // Check if it has true invisibility or not.
            Character *ch = static_cast< Character * >( target );
            Players *pl = reinterpret_cast< Players * >( ch->GetPlayer() );
            if( pl != NULL )
            {
               if( pl->GetGodFlags() & GOD_TRUE_INVISIBILITY ){
                  continue;
               }
            }

            Character *chSelf = static_cast< Character * >( self );
            if(chSelf && self->GetType() == U_PC)
            {
               if( lpSpell->boPVPcheck && !chSelf->GetNMCombatMode())
               {
                  continue;
               }
            }
         }

         // If unit is not targettable by spell.
         if( !IsTargetOK( lpSpell, self, target ) )
         {
            continue;
         }
         // If the spell requires PVP check with target.
         if( lpSpell->boPVPcheck )
         {
            // If players are not in PVP.
            if( !GAME_RULES::InPVP( self, target ) )
            {
               continue;
            }
         }

         // Determine the starting and ending course
         Unit *lpCollisionUnit = NULL;                       
         WorldPos wlCollidePos = { 0, 0, 0 };

         // If a collision was detected between the center of the spell effect and the target position                
         if( wlWorld->GetCollisionPos( wlCenter, target->GetWL(), &wlCollidePos, &lpCollisionUnit, false, false ) )
         {
            // Do not cast effect.
            continue;
         }

         wlPos = target->GetWL();

         // If a range spell blast effect was specified.
         if( lpSpell->wRangeVisualEffect != 0 )
         {
            Broadcast::BCSpellEffect( wlPos, _DEFAULT_RANGE,
                                       lpSpell->wRangeVisualEffect, // Spell effect ID.
                                       centeredId,           // Caster's ID.
                                       target->GetID(),         // Target ID
                                       centeredPos,         // Caster's position.
                                       wlPos,                // Target position
                                       GetNextGlobalEffectID(),
                                       masterEffectId
                                       );
         }

         // Cast the spell in the unit that got blasted.
         CastSpell( lpSpell, self, target, wlPos, dblRay );
      }
   }

   return success;
}

/******************************************************************************/
// Returns the spell structure of a given spell.
LPSPELL_STRUCT SpellMessageHandler::GetSpell( WORD wSpellID ) // The spell Id if the spell to get.
/******************************************************************************/
{
	LPSPELL_STRUCT lpSpellStruct = NULL;
	if( wSpellID >= SPELL_ID_OFFSET )
	{
		if( ( wSpellID - SPELL_ID_OFFSET ) < MAX_SPELLS )
		{
			TRACE( "\r\nQueried spell structure %u", wSpellID );
			lpSpellStruct = lpSpellTable[ wSpellID - SPELL_ID_OFFSET ];
		}
		else
		{
			TRACE( "\r\nFailed to obtain spell structure %u", wSpellID );
		}
	}
	return lpSpellStruct;
}
/******************************************************************************/
//  Gets the spell structure which has the specified name.
LPSPELL_STRUCT SpellMessageHandler::GetSpellByName(
 std::string spellName, // The spell name.
 WORD wLang
)
/******************************************************************************/
{
    LPSPELL_STRUCT lpSpellStruct = NULL;
    int i;
    for( i = 0; i < MAX_SPELLS; i++ )
	{
        LPSPELL_STRUCT lpSpellStruct = lpSpellTable[ i ];
        if( lpSpellStruct != NULL && _stricmp( lpSpellStruct->GetName( wLang ), spellName.c_str() ) == 0 )
		{
            return lpSpellStruct;
        }
    }
    return NULL;
}
/******************************************************************************/
// Tests if the skill can be learned.
BOOL SpellMessageHandler::IsSpellLearnable(
 WORD wSpellID,		// The ID of the skill to query.
 Unit *uLearner,	// Unit which should learn the skill.
 CString &reqText   // Holds the requirements description.
)
/******************************************************************************/
{
   if( wSpellID - SPELL_ID_OFFSET < MAX_SPELLS ){
      LPSPELL_STRUCT lpSpell = lpSpellTable[ wSpellID - SPELL_ID_OFFSET ];

      if( lpSpell )
      {
         TFormat format;

         LPSKILL_ATTRIBUTES lpAttrib = &lpSpell->saAttrib;

         // Form the description string.
         reqText = _STR( 7277, uLearner->GetLang() );
         bool prev = false;
         bool nothin = true;
         if( lpAttrib->skLevel != 0 )
         {
            reqText += format( _STR( 7278, uLearner->GetLang() ), lpAttrib->skLevel );
            prev = true;
            nothin = false;
         }
         if( lpAttrib->skINT != 0 )
         {
            if( prev ){
               reqText += ", ";
            }                
            reqText += format( _STR( 7282, uLearner->GetLang() ), lpAttrib->skINT );
            prev = true;
            nothin = false;
         }
         if( lpAttrib->skWIS != 0 )
         {
            if( prev ){
               reqText += ", ";
            }                
            reqText += format( _STR( 7283, uLearner->GetLang() ), lpAttrib->skWIS );
            prev = true;
            nothin = false;
         }
         if( lpAttrib->skSTR != 0 )
         {
            if( prev ){
               reqText += ", ";
            }                
            reqText += format( _STR( 7279, uLearner->GetLang() ), lpAttrib->skSTR );
            prev = true;
            nothin = false;
         }
         if( lpAttrib->skEND != 0 )
         {
            if( prev ){
               reqText += ", ";
            }                
            reqText += format( _STR( 7280, uLearner->GetLang() ), lpAttrib->skEND );
            prev = true;
            nothin = false;
         }
         if( lpAttrib->skAGI != 0 )
         {
            if( prev ){
               reqText += ", ";
            }                
            reqText += format( _STR( 7281, uLearner->GetLang() ), lpAttrib->skAGI );
            prev = true;
            nothin = false;
         }

         if( lpAttrib->tlskSkillRequired.NbObjects() != 0 )
         {
            if( prev )
            {
               reqText += _STR( 7284, uLearner->GetLang() );
            }
            nothin = false;

            /*
            if( lpAttrib->tlskSkillRequired.NbObjects() > 1 )
            {
               reqText += _STR( 7288, uLearner->GetLang() );
            }
            else
            {
               reqText += _STR( 7289, uLearner->GetLang() );
            }
            */

            lpAttrib->tlskSkillRequired.ToHead();
            if( lpAttrib->tlskSkillRequired.QueryNext() )
            {
               LPUSER_SKILL lpRequired = lpAttrib->tlskSkillRequired.Object();
               LPSPELL_STRUCT lpSpell = GetSpell( lpRequired->GetSkillID() );
               if( lpSpell != NULL )
               {
                  reqText += lpSpell->GetName( uLearner->GetLang() );   
               }
               else
               {
                  //Look si c un skill
                  CString strMsg;
                  LPSKILL lpUserSkill = Skills::GetSkill( lpRequired->GetSkillID() );
                  if(lpUserSkill)
                     strMsg.Format("%u %s",lpRequired->GetTrueSkillPnts(),lpUserSkill->GetName( uLearner->GetLang()));
                  else if(lpRequired->GetSkillID() == __SKILL_ATTACK)
                     strMsg.Format("%u %s",lpRequired->GetTrueSkillPnts(),_STR( 449, uLearner->GetLang()));
                  else if(lpRequired->GetSkillID() == __SKILL_DODGE)
                     strMsg.Format("%u %s",lpRequired->GetTrueSkillPnts(),_STR( 450, uLearner->GetLang()));
                  reqText += strMsg; 
               }

               while( lpAttrib->tlskSkillRequired.QueryNext() )
               {
                  reqText += ", ";
                  lpRequired = lpAttrib->tlskSkillRequired.Object();
                  LPSPELL_STRUCT lpSpell = GetSpell( lpRequired->GetSkillID() );
                  if( lpSpell != NULL )
                  {
                     reqText += lpSpell->GetName( uLearner->GetLang() );   
                  }
                  else
                  {
                     //Look si c un skill
                     CString strMsg;
                     LPSKILL lpUserSkill = Skills::GetSkill( lpRequired->GetSkillID() );
                     if(lpUserSkill)
                        strMsg.Format("%u %s",lpRequired->GetTrueSkillPnts(),lpUserSkill->GetName( uLearner->GetLang()));
                     else if(lpRequired->GetSkillID() == __SKILL_ATTACK)
                        strMsg.Format("%u %s",lpRequired->GetTrueSkillPnts(),_STR( 449, uLearner->GetLang()));
                     else if(lpRequired->GetSkillID() == __SKILL_DODGE)
                        strMsg.Format("%u %s",lpRequired->GetTrueSkillPnts(),_STR( 450, uLearner->GetLang()));
                     reqText += strMsg; 
                  }
               }
            }
         }
         if( nothin )
         {
            reqText += _STR( 7286, uLearner->GetLang() );
         }
         else
         {
            reqText += ".";
         }

         if(lpAttrib->skLevel <= (int)uLearner->GetLevel())
         {
            if(lpAttrib->skAGI <= uLearner->GetTrueAGI() && 
               lpAttrib->skSTR <= uLearner->GetTrueSTR() &&
               lpAttrib->skEND <= uLearner->GetTrueEND() &&
               lpAttrib->skINT <= uLearner->GetTrueINT() &&
               lpAttrib->skWIS <= uLearner->GetTrueWIS()/* &&
               lpAttrib->skWIL <= uLearner->GetWIL() &&
               lpAttrib->skLCK <= uLearner->GetLCK()*/)
            {

               BOOL boOK = TRUE;
               LPUSER_SKILL lpRequired;
               LPUSER_SKILL lpUserSkill;
               lpAttrib->tlskSkillRequired.Lock();
               lpAttrib->tlskSkillRequired.ToHead();
               while( lpAttrib->tlskSkillRequired.QueryNext() && boOK )
               {
                  lpRequired = lpAttrib->tlskSkillRequired.Object();

                  lpUserSkill = uLearner->GetSkill( lpRequired->GetSkillID() );
                  if(lpUserSkill)
                  {
                     //si il la, on valide si c un SKILL on valide quil a le bon nombre de points
                     LPSPELL_STRUCT lpSpell = GetSpell( lpRequired->GetSkillID() );
                     if( lpSpell == NULL )
                     {
                        //C'est un SKILL... a t il le bon nombre de point...
                        if(lpUserSkill->GetTrueSkillPnts() < lpRequired->GetTrueSkillPnts())
                           boOK = FALSE;
                     }
                  }
                  else if( !lpUserSkill )
                  {
                     //on valide si c attack ou dodge...
                     if(lpRequired->GetSkillID() == __SKILL_ATTACK)
                     {
                        if(uLearner->GetTrueATTACK() < lpRequired->GetTrueSkillPnts())
                           boOK = FALSE;
                     }
                     else if(lpRequired->GetSkillID() == __SKILL_DODGE)
                     {
                        if(uLearner->GetTrueDODGE() < lpRequired->GetTrueSkillPnts())
                           boOK = FALSE;
                     }
                     else
                     {
                        //ni attack ou dodge on fail
                        boOK = FALSE;
                     }
                  }
               }
               lpAttrib->tlskSkillRequired.Unlock();

               return boOK;
            }
         }
      }
   }
   return FALSE;
}
/******************************************************************************/
DWORD GetNextGlobalEffectID()
/******************************************************************************/
{
    static DWORD globalEffectID = 1;
    InterlockedIncrement( (long*)&globalEffectID );
    return globalEffectID;
}