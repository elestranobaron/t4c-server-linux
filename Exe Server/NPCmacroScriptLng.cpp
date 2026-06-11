/******************************************************************************
Modify for vs2008 (01/05/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC_MAIN.h"
#include "TFC Server.h"
#include "IntlText.h"
#include "format.h"
#include "RegKeyHandler.h"
#include "PlayerManager.h"
#include "NPCMAcroscriptLng.h"
#include "QuestFlagsListing.h"

extern CTFCServerApp theApp;

#ifdef EXPORT
	#undef EXPORT
#endif

/******************************************************************************/
#define EXPORT __declspec( dllexport )

/******************************************************************************/
struct FlagList
{
	DWORD flagID;
	DWORD flagValue;
};

/******************************************************************************/
extern CTFCServerApp theApp;
extern Random rnd;
list< FlagList > flagList;

/******************************************************************************/
// Converts a character string to numbers, strips commas
int EXPORT NPCatoi( const char *szString ) // The string.
/******************************************************************************/
{
    int nNumber = 0;
    
    while( isdigit( *szString ) || *szString == ',' )
	{
        if( *szString != ',' )
		{
            nNumber *= 10;
            nNumber += *szString - '0';
        }
        szString++;
    }
    return nNumber;
}
/******************************************************************************/
// Gives an item to a player.
BOOL EXPORT __GiveItem( 
 Unit *self,
 Unit *target,
 WORD wItemID,  // The item ID.
 BOOL boEcho,    // TRUE if backpack should be displayed.
 bool boGiveAbsolute,
 bool boBackpackUpdate, // True if backpack should be updated.
 bool bLog
)
/******************************************************************************/
{
   // Only deal with characters.
   if( target->GetType() != U_PC )
   {
      return FALSE;
   }

   BOOL boGiven = FALSE;
   Objects *new_obj = new Objects;
   Character *lpChar = static_cast< Character * >( target );

   if(new_obj->Create(U_OBJECT, wItemID))
   {
      bool boOverweight = ( new_obj->GetWeight() + target->GetWeight() < target->GetMaxWeight() ? false : true );

      // If the target can hold this item or the item is in give absolute.
      if( !boOverweight || boGiveAbsolute )
      {

         // Avoid deletion of new_obj
         new_obj->CreateVirtualUnit();

         // Add the object to the bacpkack.
         lpChar->AddToBackpack( new_obj );

         if( boBackpackUpdate )
         {
            TFCPacket sending;
            sending << (RQ_SIZE)RQ_ViewBackpack2;
            if( boEcho )
            {
               sending << (char)1;
            }
            else
            {
               sending << (char)0;
            }
            sending << (long)target->GetID();
            target->PacketBackpack( sending );
            target->SendPlayerMessage( sending );

            sending.Destroy();
            if( target->GetType() == U_PC )
            {
               Character *ch = static_cast< Character * >( target );
               ch->packet_equiped( sending );
               ch->SendPlayerMessage( sending );
            }
         }
         boGiven = TRUE;


            

         char lpszID[ 256 ];
         Unit::GetNameFromID( new_obj->GetStaticReference(), lpszID, U_OBJECT );
         if(bLog)
         {
            _LOG_NPCS
               LOG_MISC_1,
               "NPC %s gave item %s ( ID %s ) to player %s.",
               self->GetName( _DEFAULT_LNG ),
               new_obj->GetName( _DEFAULT_LNG ),
               lpszID,
               target->GetName( _DEFAULT_LNG )
               LOG_
         }

         if( boOverweight )
         {
            // Make function fail for overweight even though we gave the item.
            boGiven = false;
         }

         // Delete new_obj's virtual unit.
         new_obj->DeleteUnit();

      }
      else
      {            
         target->SendSystemMessage( _STR( 37, target->GetLang() ) );

         new_obj->DeleteUnit();
      }
   }
   else
   {
      new_obj->DeleteUnit();
   };

   return boGiven;
}
/******************************************************************************/
// Returns the Unit object of an item in the player's backpack or equipment.
LPVOID __declspec( dllexport) __GetItemHandle(
 Unit *lpuUnit, // The unit holding the backpack or equip.
 int nItem      // The item base reference ID.
)
/******************************************************************************/
{
	TemplateList <Unit> *bp = lpuUnit->GetBackpack();	
    Unit *uReturn = NULL;

	if(bp)
	{
		bp->Lock();
        bp->ToHead();		
		while(bp->QueryNext() && uReturn == NULL)
		{
			// Check for the static reference, not the appearance
			if(bp->Object()->GetStaticReference() == (WORD)nItem )
			{
                uReturn = bp->Object();
			}
        }
        bp->Unlock();
	}

    // If item wasn't found in backpack.
    if( uReturn == NULL )
	{
        // If unit is a PC
        if( lpuUnit->GetType() == U_PC )
		{
            int nI;
            // Search equipped items
            Character *lpuChar = static_cast< Character * >( lpuUnit );
            Unit **lpuEquip = lpuChar->GetEquipment( );
            for( nI = 0; nI < EQUIP_POSITIONS; nI++ )
			{
                if( lpuEquip[ nI ] != NULL )
				{
                    if( lpuEquip[ nI ]->GetStaticReference() == (WORD)nItem )
					{
                        uReturn = lpuEquip[ nI ];
                    }
                }
            }
        }
    }
    
    return uReturn;
}
/******************************************************************************/
// Checks to see how many items a PC has
int EXPORT __CheckItem(
 Unit *target,
 WORD item // the ID of the item to check for
)
/******************************************************************************/
{
	TemplateList <Unit> *bp = target->GetBackpack();
	
	int nCount = 0;
	if(bp)
	{
        bp->Lock();
		bp->ToHead();		
		while(bp->QueryNext())
		{
            Objects *obj = static_cast< Objects * >( bp->Object() );
			// Check for the static reference, not the appearance
			if( obj->GetStaticReference() == item )
			{
				nCount += obj->GetQty();
			}
        }
        bp->Unlock();
	}

	return nCount;
}
/******************************************************************************/
// Makes NPCs able to teach skills to player!
BOOL EXPORT __TeachSkill(
 Unit *lpuWho, // Unit which should learn, must be a U_PC
 DWORD dwSkill, // Skill to learn.
 WORD wInitialStrength, // Initial strength to give to player
 DWORD dwGoldCost
)
/******************************************************************************/
{
	if( lpuWho->GetType() == U_PC )
	{
		Character *lpch = static_cast< Character *>( lpuWho );
        // If this is a skill
        if( dwSkill < SPELL_ID_OFFSET )
		{
            LPSKILL lpSkill = Skills::GetSkill( dwSkill );

            if( lpSkill != NULL )
			{
                DWORD dwGold = lpch->GetGold();

                if( dwGold >= static_cast< DWORD >( dwGoldCost ) )
				{
    		        CString dummy;
                    // If user can learn skill
	    	        if( Skills::IsSkillLearnable( dwSkill, lpuWho, dummy ) )
					{
                        CString errMsg;
                        // Learn skill.
			            if( lpch->LearnSkill( dwSkill, wInitialStrength, true, errMsg ) )
						{

                            _LOG_PC
                                LOG_MISC_1,
                                "Player %s has been taught skill %s ( %u ).", 
                                lpuWho->GetName( _DEFAULT_LNG ), 
                                lpSkill->GetName( _DEFAULT_LNG ),
                                dwSkill
                            LOG_

                            dwGold -= dwGoldCost;
                            lpch->SetGold( dwGold );
				            return TRUE;
			            }
		            }
                }
				else
				{
                    lpch->SendSystemMessage( _STR( 38, lpch->GetLang() ) );
                }
            }
        }
        // If this is a spell.
        else
		{
		    LPSPELL_STRUCT lpSpell = SpellMessageHandler::GetSpell( (WORD)dwSkill );

            if( lpSpell != NULL )
			{
                DWORD dwGold = lpch->GetGold();

                if( dwGold >= dwGoldCost )
				{
                    CString dummy;
    		        // If user can learn skill
	    	        if( SpellMessageHandler::IsSpellLearnable( (WORD)dwSkill, lpuWho, dummy ) )
					{
		    	        CString errMsg;
                        // Learn skill.
			            if( lpch->LearnSkill( dwSkill, wInitialStrength, true, errMsg ) )
						{
                            _LOG_PC
                                LOG_MISC_1,
                                "Player %s has been taught spell %s ( %u ).", 
                                lpuWho->GetName( _DEFAULT_LNG ), 
                                lpSpell->GetName( _DEFAULT_LNG ),
                                dwSkill
                            LOG_

                            dwGold -= dwGoldCost;
                            lpch->SetGold( dwGold );
				            return TRUE;
			            }
		            }
                }
				else
				{
                    lpch->SendSystemMessage( _STR( 39, lpch->GetLang() ) );
                }
            }
        }
	}
	return FALSE;
}
/******************************************************************************/
BOOL EXPORT __TeachProfession
/******************************************************************************/
(
 Unit *lpuWho,          // Unit which should learn, must be a U_PC
 DWORD dwID,            // FormuleID.
 DWORD dwGoldCost
)
{

	if( lpuWho->GetType() == U_PC )
   {

		Character *lpch = static_cast< Character *>( lpuWho );
     
      if(  Professions::IsFormuleExist(dwID))
      {
         DWORD dwGold = lpch->GetGold();
         
         if( dwGold >= static_cast< DWORD >( dwGoldCost ) )
         {
            if( Professions::IsFormuleLearnable(dwID,lpch) == 1)
            {
               CString errMsg;
               // Learn skill.
               if( lpch->LearnProfessionFormule( dwID, dwGoldCost, true, errMsg ) )
               {
                  
                  _LOG_PC
                     LOG_MISC_1,
                     "Player %s has been learn formule %s ( %u ).", 
                     lpuWho->GetName( _DEFAULT_LNG ), 
                     Professions::GetFormuleName(dwID),
                     dwID
                     LOG_
                     
                  dwGold -= dwGoldCost;
                  lpch->SetGold( dwGold );
                  return TRUE;
               }
            }
         }
         else
         {
            lpch->SendSystemMessage( _STR( 15019, lpch->GetLang() ) );
         }
      }
   }
   
   return FALSE;
}
/******************************************************************************/
// Allows a NPC to train a user's skill
BOOL EXPORT __TrainSkill(
 Unit *lpuTarget, // 
 WORD wSkillID, // 
 WORD wTrainPnts, // 
 DWORD dwCost,
 WORD wMax
)
/******************************************************************************/
{
    dwCost *= wTrainPnts;

	if( lpuTarget->GetType() == U_PC )
	{
        // Get the character structure of the user
		Character *lpch = static_cast< Character *>( lpuTarget );

        DWORD dwGold = lpch->GetGold();

        if( dwGold >= dwCost )
		{
		    if( wSkillID == __SKILL_DODGE )
			{
			    DWORD dwFlag = lpch->GetTrueDODGE();

                // If skill points goes over the max.
                if( dwFlag + wTrainPnts > wMax )
				{
                    if( dwFlag < wMax )
					{
                        wTrainPnts = wMax - (WORD)dwFlag;
                    }
					else
					{
                        wTrainPnts = 0;
                    }
                }
                if( wTrainPnts != 0 )
				{
    			    // If player successfully spent its skill points
	    		    if( lpch->UseSkillPnts( wTrainPnts ) )
					{
		    		    DWORD dwBefore = lpch->GetTrueDODGE();
                        
                        dwGold -= dwCost;
                        lpch->SetGold( dwGold );                        
                        dwFlag += (DWORD)wTrainPnts;
				        lpch->SetDODGE( (WORD)dwFlag );

                        _LOG_PC
                            LOG_MISC_1,
                            "Player %s trained %u skill points in dodge skill %u->%u.", 
                            lpuTarget->GetName( _DEFAULT_LNG ),
                            wTrainPnts,
                            dwBefore,
                            lpch->GetTrueDODGE()
                        LOG_

				        return TRUE;
			        }
                }
		    }
			else if( wSkillID == __SKILL_ATTACK )
			{
			    DWORD dwFlag = lpch->GetTrueATTACK();

                // If skill points goes over the max.
                if( dwFlag + wTrainPnts > wMax )
				{
                    if( dwFlag < wMax )
					{
                        wTrainPnts = wMax - (WORD)dwFlag;
                    }
					else
					{
                        wTrainPnts = 0;
                    }
                }
                if( wTrainPnts != 0 )
				{
			        if( lpch->UseSkillPnts( wTrainPnts ) )
					{
                        DWORD dwBefore = lpch->GetTrueATTACK();

                        dwGold -= dwCost;
                        lpch->SetGold( dwGold );
				        dwFlag += (DWORD)wTrainPnts;
				        lpch->SetATTACK( (WORD)dwFlag );

                        _LOG_PC
                            LOG_MISC_1,
                            "Player %s trained %u skill points in attack skill %u->%u.", 
                            lpuTarget->GetName( _DEFAULT_LNG ),
                            wTrainPnts,
                            dwBefore,
                            lpch->GetTrueATTACK()
                        LOG_

				        return TRUE;
                    }
                }
            }
			else
			{
                // Fetch the current user skill points.
                LPUSER_SKILL lpusSkill = lpch->GetSkill( wSkillID );
                if( lpusSkill != NULL )
				{
                    // Save the skill points before training.
                    DWORD dwSkillBefore = lpusSkill->GetTrueSkillPnts();

                    // If skill was successfully trained.
                    if( lpch->TrainSkillPnt( wSkillID, wTrainPnts, wMax ) )
					{
                        // Get skill structure and the new user skill structure.
                        LPSKILL lpSkill = Skills::GetSkill( wSkillID );
                        lpusSkill = lpch->GetSkill( wSkillID );

                        // If both structure exist
                        if( lpSkill != NULL && lpusSkill != NULL )
						{
                            // Log skill training.
                            _LOG_PC
                                LOG_MISC_1,
                                "Player %s trained %u skill points in skill %s(ID %u) %u->%u.", 
                                lpuTarget->GetName( _DEFAULT_LNG ),
                                wTrainPnts,
                                lpSkill->GetName( _DEFAULT_LNG ),
                                wSkillID,
                                dwSkillBefore,
                                lpusSkill->GetTrueSkillPnts()
                            LOG_

                            // Deduct gold.
                            dwGold -= dwCost;
                            lpch->SetGold( dwGold );
                        }
	    		        return TRUE;
                    }
                }
		    }
        }
		else
		{
            _TELL_PLAYER( lpch, 20 );
        }
	}
	return FALSE;
}
/******************************************************************************/
// Returns TRUE if cmd1 and cmd2 are in msg and in order of appearance.
BOOL EXPORT _ORDER(
 CString msg,  // message.
 LPCTSTR cmd1, // First keyword.
 LPCTSTR cmd2, // Second keyword.
 LPCTSTR cmd3, // Third keyword.
 LPCTSTR cmd4, // Fourth keyword.
 LPCTSTR cmd5  // Fifth keyword.
)
/******************************************************************************/
{
	int i = msg.Find( cmd1 );
	int j;
	
	if( i != -1 )
	{
		if( !cmd3 )
		{
			if( i < msg.Find( cmd2 ))
			{
				return TRUE;
			}			
		}
		else if( !cmd4 )
		{
			j = msg.Find( cmd2 );
			if( i < j )
			{
				i = msg.Find( cmd3 );
				if( j < i )
				{
					return TRUE;
				}
			}		
		}
		else if( !cmd5 )
		{
			j = msg.Find( cmd2 );
			if( i < j )
			{
				i = msg.Find( cmd3 );
				if( j < i )
				{
					j = msg.Find( cmd4 );
					if( i < j )
					{
						return TRUE;
					}
				}
			}
		}
		else
		{
			j = msg.Find( cmd2 );
			if( i < j )
			{
				i = msg.Find( cmd3 );
				if( j < i )
				{
					j = msg.Find( cmd4 );
					if( i < j )
					{
						i = msg.Find( cmd5 );
						if( j < i )
						{
							return TRUE;
						}
					}
				}
			}
		}	
	}
	return FALSE;
}
/******************************************************************************/
//  Teleports a user.
void EXPORT TeleportFunc(int x, int y, int world, Unit *target)
/******************************************************************************/
{
    WorldPos wlPos = { x, y, world };
    target->Teleport( wlPos, 0 );
}
/******************************************************************************/
// Breaks a conversation.
void EXPORT BreakFunc( Unit *npc, Unit *target )
/******************************************************************************/
{
   if( npc != NULL )
   {
      npc->SetTarget( npc->GetBond() );

      if( !npc->IsPrivateTalk() || npc->GetBond() == NULL)
      {
         npc->Do( wandering ,"BreakFunc");
         WorldPos dest = { -1,-1,-1 };
         npc->SetDestination( dest );
      }
      
   }
	if(target != NULL)
   {
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_BreakConversation;
      target->SendPlayerMessage( sending );
      target->SetTarget(NULL);//NMNMNM??? Add le remove target to NPC...
   }
}
/******************************************************************************/
//  Takes an item from the backpack
void EXPORT TakeItemFunc( UINT itemID, Unit *target )
/******************************************************************************/
{
    TemplateList <Unit> *bp = target->GetBackpack();
    
    if( bp == NULL )
	{
        return;
    }
    
    // Find the object in the backpack.
    bp->Lock();
    bp->ToHead();
    while( bp->QueryNext() )
	{
        Objects *obj = static_cast< Objects * >( bp->Object() );
        if( obj->GetStaticReference() == itemID)
		{
            // Remove an item from the stack
            obj->Remove();
            if( obj->GetQty() == 0 )
			{
                bp->Object()->DeleteUnit();
                bp->Remove();
            }
            
            break;
        }
    }
    bp->Unlock();
}
/******************************************************************************/
//  Takes an item.
void EXPORT TakeItemHandleFunc( Unit *&handle, Unit *target)
/******************************************************************************/
{
    TemplateList <Unit> *bp = target->GetBackpack();
    
    if( bp == NULL )
	{
        return;
    }
    
    bp->Lock(); 
    bp->ToHead();
    while( bp->QueryNext() )
	{
        Objects *obj = static_cast< Objects * >( bp->Object() );
        if( obj == static_cast< Objects * >( handle ) )
		{
            obj->Remove();
            if( obj->GetQty() == 0 )
			{
                bp->Object()->DeleteUnit();
                bp->Remove();
                handle = NULL;
            }
        }
    }
    bp->Unlock();
}
/******************************************************************************/
//  Give gold.
void EXPORT GiveGoldFunc( int amount, Unit *target, bool echo )
/******************************************************************************/
{
    int gold = target->GetGold();
    gold += amount;
    target->SetGold( gold, echo );
}
/******************************************************************************/
//  Takes gold.
void EXPORT TakeGoldFunc( int amount, Unit *target, bool echo )
/******************************************************************************/
{
    int gold = target->GetGold();
    gold -= amount;
    gold = gold < 0 ? 0 : gold;
    target->SetGold( gold, echo );
}
/******************************************************************************/
//  Gives XP.
void EXPORT GiveXPFunc( int amount, Unit *npc, Unit *target )
/******************************************************************************/
{
   if( -amount > target->GetXP() )
   {
      target->SetXP( 0 );
   }
   else
   {
      target->SetXP( target->GetXP() + amount );
   }

    _LOG_NPCS
        LOG_MISC_1,
        "NPC %s gave %u xp to player %s.",
        npc->GetName(_DEFAULT_LNG),
        amount,
        target->GetName(_DEFAULT_LNG)
    LOG_
};
/******************************************************************************/
//  Summons a monster.
void EXPORT SummonFunc( LPCTSTR mobID, int absx, int absy, int world, Unit *npc, Unit *target )
/******************************************************************************/
{
    WorldMap *wlWorld = TFCMAIN::GetWorld( world );
    if( wlWorld == NULL )
	{
		return;
	}

    Creatures *lpuCreature = new Creatures;
    if( !lpuCreature->Create( U_NPC, Unit::GetIDFromName( mobID, U_NPC ) ) )
	{
		lpuCreature->DeleteUnit();
		return;
	}
    WorldPos wlPos = { absx, absy, world };
    wlPos = wlWorld->FindValidSpot( wlPos, 5 );
    if( wlPos.X == -1 || wlPos.Y == -1 )
	{
		lpuCreature->DeleteUnit();
		return;
	}
    lpuCreature->SetWL( wlPos );

    if( !wlWorld->SummonMonster( lpuCreature, TRUE ) )
	{
		lpuCreature->DeleteUnit();
		return;
	}

	_LOG_NPCS
		LOG_MISC_1,
		"NPC %s summoned creature %s ( ID %s ) at ( %u, %u, %u )",
		npc->GetName(_DEFAULT_LNG),
		lpuCreature->GetName(_DEFAULT_LNG),
		mobID,
		wlPos.X,
		wlPos.Y,
		world
	LOG_
}
/******************************************************************************/
//  Logs withdrawing from bank.
void EXPORT LogGoldWithdrawFunc( int currentGold, int goldWithdrew, LPCTSTR bankName, Character *lpChar)
/******************************************************************************/
{
    if( lpChar != NULL )
	{
        Players *lpPlayer = lpChar->GetPlayer();
        if( lpPlayer != NULL )
		{
            _LOG_PC
                LOG_MISC_1,
                "Player %s (%s) withdrew %u gold (now has %u gold) from %s.",
                (LPCTSTR)lpChar->GetTrueName(),
                (LPCTSTR)lpPlayer->GetFullAccountName(),
                goldWithdrew,
                currentGold,
                bankName
            LOG_  
        }
    }
}
/******************************************************************************/
//  Logs depositing
void EXPORT LogGoldDepositFunc( int currentGold, int goldDeposited, LPCTSTR bankName, Character *lpChar)
/******************************************************************************/
{
    if( lpChar != NULL )
	{
        Players *lpPlayer = lpChar->GetPlayer();
        if( lpPlayer != NULL )
		{
            _LOG_PC
                LOG_MISC_1,
                "Player %s (%s) deposited %u gold (now has %u gold) in %s.",
                (LPCTSTR)lpChar->GetTrueName(),
                (LPCTSTR)lpPlayer->GetFullAccountName(),
                goldDeposited,
                currentGold,
                bankName
            LOG_
        }
    }
}
/******************************************************************************/
//  Heals the player.
void EXPORT HealPlayerFunc( int hitPnts, Unit *target )
/******************************************************************************/
{
    DWORD maxHitPoints = target->GetMaxHP();
    DWORD hitPoints = target->GetHP();
    hitPoints += hitPnts;
    hitPoints  = hitPoints > maxHitPoints ? maxHitPoints : hitPoints;
    target->SetHP( hitPoints, true );
}
/******************************************************************************/
//  Shouts a message.
void EXPORT ShoutFunc( LPCTSTR msg, Unit *npc, Unit *target)
/******************************************************************************/
{
    if( npc->IsPrivateTalk() )
	{
        CString csMsg( msg );
        if( target != NULL )
		{
            target->SendPrivateMessage( csMsg, npc );
        }
		else
		{
            npc->Talk( csMsg );
        }
    }
	else
	{
        CString csMsg( msg );;
        npc->Talk( (LPCTSTR)csMsg );
    }
}
/******************************************************************************/
//  Adds a skill to a teaching list.
void EXPORT AddTeachSkillFunc( int skillID, DWORD skillCost, DWORD skillPnts,
	BOOL &boCantLearn, TemplateList< _SKILLITEM > &tlSkillList, Unit *target)
/******************************************************************************/
{
	if( skillID < SPELL_ID_OFFSET )
	{
		LPSKILL lpSkill = Skills::GetSkill( skillID );
		if( lpSkill != NULL )
		{
			_SKILLITEM *NewItem = new _SKILLITEM;
			LPUSER_SKILL lpUserSkill = target->GetSkill( skillID );
			NewItem->cCanHave = Skills::IsSkillLearnable( skillID, target, NewItem->reqDesc );
			if( lpUserSkill ) NewItem->cCanHave = FALSE;
			NewItem->wCurPnts = 0;
			NewItem->dwSkillCost = skillCost;
			NewItem->csSkillName = lpSkill->GetName( target->GetLang() );
			NewItem->wID = skillID;
            NewItem->dwIcon = skillID;
            NewItem->wMaxLearnPnts = skillPnts;
			tlSkillList.AddToTail(NewItem);
		}
	}
	else
	{
		LPSPELL_STRUCT lpSpell = SpellMessageHandler::GetSpell( skillID );
		if( lpSpell != NULL ) /* If spell exists */
		{
			_SKILLITEM *NewItem = new _SKILLITEM;
			LPUSER_SKILL lpUserSkill = target->GetSkill( skillID );
			NewItem->cCanHave = SpellMessageHandler::IsSpellLearnable( skillID, target, NewItem->reqDesc );
			if( lpUserSkill != NULL )
			{
				NewItem->cCanHave = FALSE;
			}
			NewItem->wCurPnts = 0;
			NewItem->dwSkillCost = skillCost;
			NewItem->csSkillName = lpSpell->GetName( target->GetLang() );
            NewItem->wMaxLearnPnts = skillPnts;
            NewItem->dwIcon = lpSpell->dwIcon;
			NewItem->wID = skillID;
			tlSkillList.AddToTail(NewItem);
		}
	}
}

/******************************************************************************/
void EXPORT AddTeachFormuleFunc( int FormuleID, DWORD FormuleCost, BOOL &boCantLearn,
								 TemplateList< _FORMULEITEM > &tlFormuleList, Unit *target)
/******************************************************************************/
{
   if(Professions::IsFormuleExist(FormuleID))
   {
      _FORMULEITEM *NewItem = new _FORMULEITEM;
      
      NewItem->chCanLearn   = Professions::IsFormuleLearnable(FormuleID,target);
      NewItem->chProfession = Professions::GetFormulesProfession(FormuleID);
      NewItem->ushID        = FormuleID;
      NewItem->dwCost       = FormuleCost;
      NewItem->ushSkill     = Professions::GetFormuleSkill(FormuleID);
      NewItem->strName.Format("%s",Professions::GetFormuleName(FormuleID));
      
      tlFormuleList.AddToTail(NewItem);
   }
}
/******************************************************************************/
//  Adds a skill to a training list.
void EXPORT AddTrainSkillFunc( int skillId, int maxPnts, DWORD skillCost,
 BOOL &boCantLearn, TemplateList< _SKILLITEM > &tlSkillList, Unit *target)
/******************************************************************************/
{
	LPUSER_SKILL lpUserSkill = target->GetSkill( skillId );
	LPSKILL lpSkill = Skills::GetSkill( skillId );
	if( lpSkill != NULL || ( skillId == __SKILL_DODGE || skillId == __SKILL_ATTACK ) ) /* If skill exists */
	{
		_SKILLITEM *NewItem = new _SKILLITEM;
		if( skillId == __SKILL_DODGE )
		{
			NewItem->csSkillName = _STR( 450, target->GetLang() );
			NewItem->cCanHave = TRUE;
			NewItem->wCurPnts = target->GetTrueDODGE();
		}
		else if( skillId == __SKILL_ATTACK )
		{
			NewItem->csSkillName = _STR( 449, target->GetLang() );
			NewItem->cCanHave = TRUE;
			NewItem->wCurPnts = target->GetTrueATTACK();
		}
		else
		{
			if( lpUserSkill )
			{
				NewItem->cCanHave = TRUE;
				NewItem->wCurPnts = lpUserSkill->GetTrueSkillPnts();
			}
			else
			{
				NewItem->cCanHave = FALSE;
				NewItem->wCurPnts = 0;
			}
			NewItem->csSkillName = lpSkill->GetName( target->GetLang() );
		}
		NewItem->dwSkillCost = skillCost;
		NewItem->wID = skillId;
		NewItem->wMaxLearnPnts = maxPnts;
		if( NewItem->cCanHave )
		{
			boCantLearn = FALSE;
		}
		tlSkillList.AddToTail(NewItem);
	}
}
/******************************************************************************/
//  Sends the list of train items to the target.
void EXPORT SendTrainSkillListFunc( BOOL boCantLearn,
	TemplateList< _SKILLITEM > &tlSkillList, Unit *target)
/******************************************************************************/
{
	tlSkillList.ToHead(); TFCPacket sending;
	sending << (RQ_SIZE)RQ_SendTrainSkillList;
	sending << (short)target->GetSkillPoints();
	sending << (short)tlSkillList.NbObjects();
	_SKILLITEM *lpSkillItem;
	tlSkillList.ToHead();
	while(tlSkillList.QueryNext())
	{
		lpSkillItem = tlSkillList.Object();
		sending << (char)(lpSkillItem->cCanHave);
		sending << (short)lpSkillItem->wID;
		sending << (short)lpSkillItem->wCurPnts;
		sending << (short)lpSkillItem->wMaxLearnPnts;
		sending << (long)lpSkillItem->dwSkillCost;
		TRACE("\r\nSending skill %s", lpSkillItem->csSkillName );
		sending << (CString)lpSkillItem->csSkillName;
		tlSkillList.DeleteAbsolute();
	}
	target->SendPlayerMessage( sending );
}
/******************************************************************************/
//  Sends a list of skills to teach to the target.
void EXPORT SendTeachSkillListFunc(
 BOOL boCantLearn,
 TemplateList< _SKILLITEM > &tlSkillList,
 Unit *target
)
/******************************************************************************/
{
    tlSkillList.ToHead(); TFCPacket sending;
    sending << (RQ_SIZE)RQ_SendTeachSkillList;
    sending << (short)target->GetSkillPoints();
    sending << (short)tlSkillList.NbObjects();
    _SKILLITEM *lpSkillItem;
    tlSkillList.ToHead();
    while( tlSkillList.QueryNext() )
	{
        lpSkillItem = tlSkillList.Object();
        sending << (char)(lpSkillItem->cCanHave);
        sending << (short)lpSkillItem->wID;
        sending << (long)lpSkillItem->dwSkillCost;
        sending << (CString)lpSkillItem->csSkillName;
        sending << (CString)lpSkillItem->reqDesc;
        sending << (long)lpSkillItem->wMaxLearnPnts;
        sending << (long)lpSkillItem->dwIcon;
        tlSkillList.DeleteAbsolute();
    }
    target->SendPlayerMessage( sending );
}

//  Sends a list of skills to teach to the target.
/******************************************************************************/
void EXPORT SendTeachFormuleListFunc( BOOL boCantLearn, TemplateList< _FORMULEITEM > &tlFormuleList, Unit *target)
/******************************************************************************/
{
   Players *TargetPlayer = static_cast< Players * >(target->GetPlayer());
   if(TargetPlayer)
   {
      if(theApp.dwProfessionSystemEnable == 0 || (theApp.dwProfessionSystemEnable == 2 && !TargetPlayer->IsGod()))
      {
          target->SendInfoMessage( _STR( 15152 , target->GetLang() ),0x0020FF);
         return;
      }
   }
   
   tlFormuleList.ToHead(); TFCPacket sending;
   sending << (RQ_SIZE)RQ_NM_SendTeachFormuleList;
   sending << (short)target->ViewFlag(__FLAG_PROF_APOTICAIRE);
   sending << (short)target->ViewFlag(__FLAG_PROF_BIJOUTIER);
   sending << (short)target->ViewFlag(__FLAG_PROF_COUTURIER);
   sending << (short)target->ViewFlag(__FLAG_PROF_ARMURIER);
   sending << (short)target->ViewFlag(__FLAG_PROF_FORGERON);
   sending << (short)target->ViewFlag(__FLAG_PROF_EBENISTE);
   sending << (long)target->GetGold();
   sending << (short)tlFormuleList.NbObjects();
   _FORMULEITEM *lpFormuleItem;
   tlFormuleList.ToHead();
   while( tlFormuleList.QueryNext() )
   {
      lpFormuleItem = tlFormuleList.Object();
      
      sending << (char)(lpFormuleItem->chCanLearn);
      sending << (char)(lpFormuleItem->chProfession);
      sending << (short)lpFormuleItem->ushID;
      sending << (short)lpFormuleItem->ushSkill;
      sending << (long)lpFormuleItem->dwCost;
      sending << (CString)lpFormuleItem->strName;
      tlFormuleList.DeleteAbsolute();
   }
   target->SendPlayerMessage( sending );
}

/******************************************************************************/
void EXPORT SendAHListFunc(Unit *target)
/******************************************************************************/
{
   //Add the query to AH List...
   if(theApp.dwAHSystemEnable == 0)
   {
      target->SendInfoMessage( _STR( 15121 , target->GetLang() ),0x0020FF);
   }
   else
   {
      theApp.AddAHRequest(NULL,NULL,NULL,AH_REQ_GET_LIST,target->GetID(),1,0,0,0,0,0,"","","",0);
   }
}

/******************************************************************************/
//  Adds an item to buy in an item list.
void EXPORT AddBuyItemFunc(
 DWORD price,
 DWORD id,
 TemplateList< _OBJECTITEM > &tlItemList,
 Character *lpChar
)
/******************************************************************************/
{
    if( lpChar == NULL ){
        return;
    }

    Objects *newObj = new Objects;
    if( newObj->Create( U_OBJECT, id ) )
    {
       _OBJECTITEM *NewObject = new _OBJECTITEM;
       NewObject->dwPrice = price;
       NewObject->wAppearance = newObj->GetAppearance();
       NewObject->dwID = id;
       NewObject->csItemName = newObj->GetName( lpChar->GetLang() );        
       if( lpChar->CanEquip( newObj, NULL, FALSE, &NewObject->reqDesc ) )
       {
          NewObject->bCanEquip = 1;
       }
       else
       {
          NewObject->bCanEquip = 0;
       }
       tlItemList.AddToTail(NewObject);
    }
    newObj->DeleteUnit();
}
/******************************************************************************/
//  Adds an item to sell to a list of items.
void EXPORT AddSellItemFunc(
 DWORD itemType,
 DWORD lowPriceRange,
 DWORD hiPriceRange,
 TemplateList< _OBJECTITEM > &tlItemList,
 Character *lpChar
)
/******************************************************************************/
{
   if( lpChar == NULL )
   {
       return;
   }
    
   TemplateList <Unit> *lpBackpack = lpChar->GetBackpack();\
	if( lpBackpack )
	{
		// Search the backpack
		lpBackpack->Lock();
		lpBackpack->ToHead();
		while( lpBackpack->QueryNext() )
		{
         Objects *lpUnit = static_cast< Objects * >( lpBackpack->Object() );

         // Only process objects
         if( lpUnit->GetType() != U_OBJECT )
         {
            continue;
         }

         // Gets the item structure
         _item *lpItem = NULL;
         lpUnit->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItem );

         if( lpItem == NULL )
         {
            continue;
         }

         if( lpItem->sell_type != itemType )
         {
            continue;
         }
         // If NPC can buy the item

         DWORD dwCostCurrent = lpItem->costFixed;
         if(dwCostCurrent == 0)
            dwCostCurrent = (DWORD)lpItem->costFormula.GetBoost( lpChar );


			if( dwCostCurrent >= lowPriceRange && dwCostCurrent < hiPriceRange )
			{
				_OBJECTITEM *NewObject = new _OBJECTITEM;
				NewObject->dwPrice = dwCostCurrent;
				NewObject->wAppearance = lpUnit->GetAppearance();
				NewObject->dwID = lpUnit->GetID();
				NewObject->csItemName = lpUnit->GetName( lpChar->GetLang() );
                NewObject->dwQty = lpUnit->GetQty();
				tlItemList.AddToTail(NewObject);
			}
		}
		lpBackpack->Unlock();
	}
}
/******************************************************************************/
//  Sends a list of items to buy from a target.
void EXPORT SendBuyItemListFunc(
 TemplateList< _OBJECTITEM > &tlItemList,
 Unit *target
)
/******************************************************************************/
{
    tlItemList.ToHead(); 
    TFCPacket sending;
    
    sending << (RQ_SIZE)RQ_SendBuyItemList;
	 sending << (long)target->GetGold();
	 sending << (short)tlItemList.NbObjects();
	 
    tlItemList.ToHead();
	 while(tlItemList.QueryNext())
	 {
        sending << (short)tlItemList.Object()->dwID;
        sending << (short)tlItemList.Object()->wAppearance;
        sending << (long)tlItemList.Object()->dwPrice;
        sending << (char)tlItemList.Object()->bCanEquip;
        sending << (CString)tlItemList.Object()->csItemName;
        sending << (CString)tlItemList.Object()->reqDesc;
	 	
        tlItemList.DeleteAbsolute();
    } 
    
    target->SendPlayerMessage(sending);	
}

/******************************************************************************/
//  Sends a list of items to buy from a target.
void EXPORT SendPointsItemListFunc(
                                TemplateList< _OBJECTITEM > &tlItemList,
                                Unit *target
                                )
                                /******************************************************************************/
{
   tlItemList.ToHead(); 
   TFCPacket sending;

   sending << (RQ_SIZE)RQ_SendPointsItemList;
   sending << (long)target->ViewFlag(__FLAG_POINTS_RP_XP_EVENTS);
   sending << (short)tlItemList.NbObjects();

   tlItemList.ToHead();
   while(tlItemList.QueryNext())
   {
      sending << (short)tlItemList.Object()->dwID;
      sending << (short)tlItemList.Object()->wAppearance;
      sending << (long)tlItemList.Object()->dwPrice;
      sending << (char)tlItemList.Object()->bCanEquip;
      sending << (CString)tlItemList.Object()->csItemName;
      sending << (CString)tlItemList.Object()->reqDesc;

      tlItemList.DeleteAbsolute();
   } 

   target->SendPlayerMessage(sending);	
}


/******************************************************************************/
//  Sends a list of items to sell to the target.
void EXPORT SendSellItemListFunc(
 CString &output,
 LPCTSTR text,
 TemplateList< _OBJECTITEM > &tlItemList,
 Unit *target
)
/******************************************************************************/
{
    if( tlItemList.NbObjects() == 0 )
	{
        output = text;
    }
	else
	{
        tlItemList.ToHead(); TFCPacket sending;
        sending.Destroy();
        sending << (RQ_SIZE)RQ_SendSellItemList;
        sending << (long)target->GetGold();
        sending << (short)tlItemList.NbObjects();
        
        tlItemList.ToHead();
        while(tlItemList.QueryNext())
		{
            sending << (long)tlItemList.Object()->dwID;
            sending << (short)tlItemList.Object()->wAppearance;
            sending << (long)tlItemList.Object()->dwPrice;
            sending << (long)tlItemList.Object()->dwQty;
            sending << (CString)tlItemList.Object()->csItemName;

            tlItemList.DeleteAbsolute();
        } 
        target->SendPlayerMessage(sending);
    }
}
/******************************************************************************/
//  Makes an NPC buy an item from the player.
bool EXPORT NPC_BUYFunc(
 Unit *npc, // 
 Unit *target, // 
 WORD ItemType, // 
 DWORD LowPriceRange, // 
 DWORD HiPriceRange, // 
 int   &MoneyData,
 LPSHOP_DATA shop
)
/******************************************************************************/
{
   TemplateList <Unit> *lpBackpack = target->GetBackpack();
   if( lpBackpack == NULL )
   {
      return false;
   }

   // Search the backpack
   lpBackpack->Lock();
   lpBackpack->ToHead();    
   while( lpBackpack->QueryNext() )
   {
      Objects *lpUnit = static_cast< Objects * >( lpBackpack->Object() );
      // If unit was not found or unit is not an item
      if( lpUnit->GetID() != shop->ID || lpUnit->GetType() != U_OBJECT )
      {
         continue;
      }

      // Get the item's structure.
      _item *lpItem = NULL;
      lpUnit->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItem );

      if( lpItem == NULL )
      {
         break;
      }

      // If this is not the kind of items we sell
      if( lpItem->sell_type != ItemType )
      {
         break;
      }
      // If NPC cannot buy the item
      DWORD dwCostCurrent = lpItem->costFixed;
      if(dwCostCurrent == 0)
         dwCostCurrent = (DWORD)lpItem->costFormula.GetBoost( npc,target );

      if( !( dwCostCurrent >= LowPriceRange && dwCostCurrent < HiPriceRange ) )
      {
         break;
      }
      if( shop->wQuantity > lpUnit->GetQty() )
      {
         shop->wQuantity = lpUnit->GetQty();
      }

      // Give gold to user.
      int nGold = target->GetGold();
      nGold += dwCostCurrent * shop->wQuantity;

      if(lpItem->dwBuySellFlagID > 0)
      {
         //on decremnente le flag...

         DWORD dwFlagValue = CheckGlobalFlag(lpItem->dwBuySellFlagID);
         int iQty = dwFlagValue-shop->wQuantity;
         if(iQty < 0)
            iQty = 0;

         GiveGlobalFlag(lpItem->dwBuySellFlagID,iQty);
      }

      // Log transaction.
      _LOG_NPCS
         LOG_MISC_1,
            "NPC %s bought %u item %s ( ID %u ) from player %s for %u gold (now at %u gold).",
            (LPCTSTR)npc->GetName(target->GetLang()),
            shop->wQuantity,
            (LPCTSTR)lpUnit->GetName(target->GetLang()),
            lpUnit->GetStaticReference(),
            (LPCTSTR)target->GetName(target->GetLang()),
            dwCostCurrent,
            nGold
         LOG_

         // Remove the unit from the stack
         lpUnit->Remove( shop->wQuantity );
      if( lpUnit->GetQty() == 0 )
      {
         lpUnit->DeleteUnit();
         lpBackpack->Remove();    
      }

      target->SetGold( nGold );

      MoneyData = 3;

      lpBackpack->Unlock();
      // Item found and sold.
      return true;
   }
   lpBackpack->Unlock();
   // Item not found.
   return false;
}
/******************************************************************************/
// This function allows a NPC to shout a message.
void EXPORT ChatterShoutFunc(
 LPCTSTR msg,   // The text to shout
 Unit *npc,     // The NPC
 Unit *target   // The current NPC target.
)
/******************************************************************************/
{
    // Create a shout packet
    TFCPacket sending;
    
    sending << (RQ_SIZE)RQ_SendChatterMessage;
    sending << CPlayerManager::GetChatter().GetMainChannel();
    sending << npc->GetName(_DEFAULT_LNG);
    sending << (const char *)msg;
    
    Broadcast::BCast( npc->GetWL(), 0, sending );                

}
/******************************************************************************/
// This function allows a NPC to shout a message.
void EXPORT NewChatterShoutFunc(
 string Channel,
 LPCTSTR msg,
 Unit *npc,
 Unit *target
)
/******************************************************************************/
{
	TFCPacket sending;

	sending << (RQ_SIZE)RQ_SendChatterMessage;
	sending << Channel;
	sending << npc->GetName(_DEFAULT_LNG);
	sending << (const char *)msg;

	Broadcast::BCast( npc->GetWL(), 0, sending);

    _LOG_SHOUTS
        LOG_MISC_1,
        "NPC %s said in channel %s: %s",
		(LPCTSTR)npc->GetName(_DEFAULT_LNG),
        Channel.c_str(),
        msg
    LOG_
}
/******************************************************************************/
//  Allows a NPC to send out system messages, either globally or privatly.
void EXPORT SysMsgFunc(
 LPCTSTR msg,       // The message to send.
 Unit *npc,         // The NPC
 Unit *target,      // The current NPC target.
 bool privateMsg    // Wether the message is global or private.
)
/******************************************************************************/
{
    if( privateMsg )
	{
        target->SendSystemMessage( msg );
    }
	else
	{
        WorldPos wlPos = {0,0,0};
        Broadcast::BCServerMessage( wlPos, 0,
            msg    
        );
    }
}
/******************************************************************************/
bool IsCharacterManglingDisabled()
{
    return _stricmp( theApp.sGeneral.csLang, "t4c_kor.elng" ) == 0;
}
/******************************************************************************/
//  Returns the character equivalent.
BYTE GetMsgChar( BYTE ch ) // The TChar
/******************************************************************************/
{   
    static bool disableCharacterMangling = IsCharacterManglingDisabled();

	if ( disableCharacterMangling )	//DC
	{
		return ch;									//For korean
	}
	else											//DC
	{
		// Check if the character is an accent character.
		switch( ch )
		{
			case 192: //'À':
			case 193: //'Á':
			case 194: //'Â':
			case 195: //'Ã':
			case 196: //'Ä':
			case 197: //'Å':
			case 198: //'Æ':
				return 'A';
				break;
			case 199: //'Ç':
				return 'C';
				break;
			case 200: //'È':
			case 201: //'É':
			case 202: //'Ê':
			case 203: //'Ë':
				return 'E';
				break;
			case 204: //'Ì':
			case 205: //'Í':
			case 206: //'Î':
			case 207: //'Ï':
				return 'I';
				break;
			case 209: //'Ñ':
				return 'N';
				break;
			case 210: //'Ò':
			case 211: //'Ó':
			case 212: //'Ô':
			case 213: //'Õ':
			case 214: //'Ö':
				return 'O';
				break;
			case 217: //'Ù':
			case 218: //'Ú':
			case 219: //'Û':
			case 220: //'Ü':
				return 'U';
				break;
			case 221: //'Ý':
				return 'Y';
				break;
			case 224: //'à':
			case 225: //'á':
			case 226: //'â':
			case 227: //'ã':
			case 228: //'ä':
			case 229: //'å':
			case 230: //'æ':
				return 'a';
				break;
			case 231: //'ç':
				return 'c';
				break;
			case 232: //'è':
			case 233: //'é':
			case 234: //'ê':
			case 235: //'ë':
				return 'e';
				break;
			case 236: //'ì':
			case 237: //'í':
			case 238: //'î':
			case 239: //'ï':
				return 'i';
				break;
			case 240:
				return 'o';
				break;
			case 241: //'ñ':
				return 'n';
				break;
			case 242: //'ò':
			case 243: //'ó':
			case 244: //'ô':
			case 245: //'õ':
			case 246: //'ö':
				return 'o';
				break;
			case 249: //'ù':
			case 250: //'ú':
			case 251: //'û':
			case 252: //'ü':
				return 'u';
				break;
			case 253: //'ý':
			case 255: //'ÿ':
				return 'y';
				break;
			};
			return ch;
	}
}
/******************************************************************************/
// Determines wether the given keyword was found in the message
bool EXPORT NPCKeyWord(
 const CString &msg,    // 
 const char *kw         // 
)
/******************************************************************************/
{
    // Add a space in front of the keyword
    CString keyWord;
    
    if( kw[ 0 ] != ' ' )
	{
        keyWord += ' ';
    }
    
    keyWord += kw;

    // Convert all characters in the keyword.
    int i;
    for( i = 0; i < keyWord.GetLength(); i++ )
	{
        keyWord.SetAt( i, toupper( GetMsgChar( keyWord.GetAt( i ) ) ) );
    }

    if( msg.Find( keyWord ) != -1 )
	{
        return true;
    }
    keyWord.SetAt( 0, '-' );

    return( msg.Find( keyWord ) != -1 );
}
/******************************************************************************/
//  Formats incoming messages for parsing.
void EXPORT NPCFormatMsg(
 CString &msg, // 
 CString &parammsg
)
/******************************************************************************/
{        
    msg.TrimLeft();
    msg.TrimRight();

    // Convert all characters in the msg.
    int i;
    for( i = 0; i < msg.GetLength(); i++ )
	{
        msg.SetAt( i, GetMsgChar( msg.GetAt( i ) ) );
    }

    // Insert a space at the beginning and at the end.
    msg.Insert( 0, " " );
    msg += " ";

    // Create the parammsg from the current state of the msg
    parammsg = msg;
    parammsg.TrimRight();
    parammsg += ".";

    // Convert the msg to uppercase.
    msg.MakeUpper();
}
/******************************************************************************/
//  Gets the parameters from a parameter keyword template
bool EXPORT NPCGetParameters(
 const char *cmd,       // The parameter template
 CString &parammsg,     // The parameter message
 CString *lpcsParams    // The found parameters (must be able to hold the quantity of
                        // parameters specified in the parameter template).
)
/******************************************************************************/
{
    // Add a space in front of the keyword
    CString keyWord( " " );
    keyWord += cmd;

    // Convert all characters in the keyword.
    int i;
    for( i = 0; i < keyWord.GetLength(); i++ )
	{
        keyWord.SetAt( i, GetMsgChar( keyWord.GetAt( i ) ) );
    }
        
    return SysopCmd::GetParametersForNPC( keyWord, parammsg, lpcsParams ) == TRUE ? true : false;
}
/******************************************************************************/
// Makes the NPC cast a spell on the desired target.
void EXPORT __CastSpell(
 DWORD dwSpellID,   // The spell to cast.
 Unit *caster,      // The caster.
 Unit *target       // The spell's target.
)
/******************************************************************************/
{
    // Activate the spell!
    SpellMessageHandler::ActivateSpell(
		dwSpellID,			
		caster,
		NULL,
		target,
		target->GetWL()
	);
}
/******************************************************************************/
// Remorts a player
void EXPORT RemortTo(
 Unit *self,
 DWORD x,       // Teleport X, Y, World positions
 DWORD y,       // 
 DWORD world    // 
)
/******************************************************************************/
{
    if( self == NULL )
	{
        return;
    }
    if( self->GetType() != U_PC )
	{
        return;
    }
    
    Character *ch = static_cast< Character * >( self );

    // FLAG WIPE
    int iFlagResetChar      = ch->ViewFlag( __FLAG_RESET_BOUST_EQUIP_POS);
    int FlagBankOfWindhowl  = ch->ViewFlag( __FLAG_BANK_OF_WINDHOWL );
    int numberOfRemorts = ch->ViewFlag( __FLAG_NUMBER_OF_REMORTS );
    int FlagUserAlignment  = ch->ViewFlag( __QUEST_FIXED_ALIGNMENT );
    int UserKnowsAboutDopplganger = ch->ViewFlag( __FLAG_ADDON_USER_KNOWS_ABOUT_DOPPELGANGER );

    //Backup les profession
    int iProf01 = ch->ViewFlag( __FLAG_PROF_APOTICAIRE);
    int iProf02 = ch->ViewFlag( __FLAG_PROF_BIJOUTIER );
    int iProf03 = ch->ViewFlag( __FLAG_PROF_COUTURIER );
    int iProf04 = ch->ViewFlag( __FLAG_PROF_ARMURIER  );
    int iProf05 = ch->ViewFlag( __FLAG_PROF_FORGERON  );
    int iProf06 = ch->ViewFlag( __FLAG_PROF_EBENISTE  );
    if(theApp.dwResetProfessionRemort)
    {
       iProf01 = 0;
       iProf02 = 0;
       iProf03 = 0;
       iProf04 = 0;
       iProf05 = 0;
       iProf06 = 0;
    }


    int iBankIF1 = ch->ViewFlag( __FLAG_NMSBANK_TYPE_CDOMPTE         );
    int iBankIF2 = ch->ViewFlag( __FLAG_NMSBANK_OR_EN_BANK           );
    int iBankIF3 = ch->ViewFlag( __FLAG_NMSBANK_OR_MINIMUM_WEEKS     );
    int iBankIF4 = ch->ViewFlag( __FLAG_NMSBANK_MAX_OR               );
    int iBankIF5 = ch->ViewFlag( __FLAG_NMSBANK_MIN_OR               );
    int iBankIF6 = ch->ViewFlag( __FLAG_NMSBANK_MAX_EMPRUNT          );
    int iBankIF7 = ch->ViewFlag( __FLAG_NMSBANK_OR_EMPRUNT_NBR_WEEKS );
    int iBankIF8 = ch->ViewFlag( __FLAG_NMSBANK_NEXT_INTERET_TIME    );

    int iInterRP     = ch->ViewFlag( __FLAG_INTERACTION_RP    );
    int iPLNewChest  = ch->ViewFlag( __FLAG_PLAYER_USE_NEW_CHEST    );
    int iPLEventsPts = ch->ViewFlag( __FLAG_POINTS_RP_XP_EVENTS    );
    int iPLEventsPtsT= ch->ViewFlag( __FLAG_POINTS_RP_XP_EVENTS_TOTAL    );

    // V2 - Open the registry and find flags to save from destruction
    RegKeyHandler regKey;
    regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY );

	flagList.clear();
	TFormat format;

	int i = 1;
	DWORD flags = static_cast< DWORD >( regKey.GetProfileInt( "OracleFlag1", 0 ) );
	while( flags != 0 )
	{
		FlagList flag;
		flag.flagID = flags;
		flag.flagValue = ch->ViewFlag( flags );
		flagList.push_back( flag );

		_LOG_NPCS
			LOG_MISC_1,
			"NPC The Oracle stored player %s's flag ( %u = %u ).",
			(LPCTSTR)ch->GetTrueName(),
			flag.flagID,
			flag.flagValue
		LOG_

		i++;
		flags = static_cast< DWORD >( regKey.GetProfileInt( format( "OracleFlag%u", i ), 0 ) );
	}

    ch->DestroyFlags();
    if ( theApp.dwCustomStartupSanctuaryOnOff) 
    { 
       WorldPos WLSanc;
       WLSanc.X	 = theApp.dwCustomStartupSanctuaryX;
       WLSanc.Y	 = theApp.dwCustomStartupSanctuaryY;
       WLSanc.world = theApp.dwCustomStartupSanctuaryW;

       // Test if its a valid position.
       WorldMap *wlWorld = TFCMAIN::GetWorld( WLSanc.world );
       // If world exist and the position is valid, use custom sanctuary position else use the defaut
       if( wlWorld != NULL && wlWorld->IsValidPosition( WLSanc ) )
       {
          DWORD dwStartSanctuary = ( ( (DWORD)( (WORD)WLSanc.X ) << 20 ) + ( (DWORD)( (WORD)WLSanc.Y ) << 8 ) + (DWORD)( (BYTE)WLSanc.world ) );
          ch->SetFlag( __FLAG_DEATH_LOCATION, dwStartSanctuary);
       }
    }

    numberOfRemorts++;

    // Restore flags that are to be kept.
	 ch->SetFlag( __FLAG_ADDON_USER_KNOWS_ABOUT_DOPPELGANGER, UserKnowsAboutDopplganger );
    ch->SetFlag( __FLAG_BANK_OF_WINDHOWL, FlagBankOfWindhowl );
    ch->SetFlag( __FLAG_NUMBER_OF_REMORTS, numberOfRemorts );
    ch->SetFlag( __FLAG_USER_ALIGNMENT, FlagUserAlignment );
    ch->SetFlag( __QUEST_FIXED_ALIGNMENT, FlagUserAlignment );
    ch->SetFlag( __FLAG_REMORT_POINTS, 8 + 2 * numberOfRemorts );


    ch->SetFlag( __FLAG_NMSBANK_TYPE_CDOMPTE         ,iBankIF1);
    ch->SetFlag( __FLAG_NMSBANK_OR_EN_BANK           ,iBankIF2);
    ch->SetFlag( __FLAG_NMSBANK_OR_MINIMUM_WEEKS     ,iBankIF3);
    ch->SetFlag( __FLAG_NMSBANK_MAX_OR               ,iBankIF4);
    ch->SetFlag( __FLAG_NMSBANK_MIN_OR               ,iBankIF5);
    ch->SetFlag( __FLAG_NMSBANK_MAX_EMPRUNT          ,iBankIF6);
    ch->SetFlag( __FLAG_NMSBANK_OR_EMPRUNT_NBR_WEEKS ,iBankIF7);
    ch->SetFlag( __FLAG_NMSBANK_NEXT_INTERET_TIME    ,iBankIF8);

    ch->SetFlag( __FLAG_RESET_BOUST_EQUIP_POS,iFlagResetChar);

    ch->SetFlag( __FLAG_INTERACTION_RP      ,iInterRP);
    ch->SetFlag( __FLAG_PLAYER_USE_NEW_CHEST,iPLNewChest);
    ch->SetFlag( __FLAG_POINTS_RP_XP_EVENTS ,iPLEventsPts);
    ch->SetFlag( __FLAG_POINTS_RP_XP_EVENTS_TOTAL ,iPLEventsPtsT);

    //Recopie les profession
    ch->SetFlag( __FLAG_PROF_APOTICAIRE, iProf01); 
    ch->SetFlag( __FLAG_PROF_BIJOUTIER , iProf02); 
    ch->SetFlag( __FLAG_PROF_COUTURIER , iProf03); 
    ch->SetFlag( __FLAG_PROF_ARMURIER  , iProf04); 
    ch->SetFlag( __FLAG_PROF_FORGERON  , iProf05); 
    ch->SetFlag( __FLAG_PROF_EBENISTE  , iProf06); 

	// V2 - Retore those flags saved from destruction
	list< FlagList >::iterator j;
   	for( j = flagList.begin(); j != flagList.end(); j++ )
	{
		ch->SetFlag ( (*j).flagID , (*j).flagValue );
		_LOG_NPCS
			LOG_MISC_1,
			"NPC The Oracle restored player %s's flag ( %u = %u ).",
			(LPCTSTR)ch->GetTrueName(),
			(*j).flagID,
			(*j).flagValue
		LOG_
	}

    //////////////////////////////
    // REMOVAL OF ALL ITEMS
    for( i = 0; i < EQUIP_POSITIONS; i++ )
	{
        ch->unequip_object( i );
    }
    Unit **equip = ch->GetEquipment();
    for( i = 0; i < EQUIP_POSITIONS; i++ )
	{
        // Flush all items that couldn't be unequipped.
        equip[ i ] = NULL;
    }

    // Scroll the backpack for "quest" items
    TemplateList< Unit > *backpack = ch->GetBackpack();
    if( backpack != NULL )
	{
        backpack->Lock();
        backpack->ToHead();
        while( backpack->QueryNext() )
		{
            Objects *obj = static_cast< Objects * >( backpack->Object() );

            if( obj->GetStaticReference() == __OBJ_GEM_OF_DESTINY  || obj->GetStaticReference() == theApp.m_dwMinionGemID)
            {
               continue;
            }

			_item *itemStructure = NULL;

			// Get the item structure.
			obj->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &itemStructure );

			if( itemStructure == NULL )
			{
                continue;
            }
            // If the object should be junked.
            if( itemStructure->dwDropFlags & JUNK_AT_SERAPH )
			{
                // Destroy it.
                backpack->Remove();
                obj->DeleteUnit();
            }            
        }
        backpack->Unlock();
    }

    //Remove AllBoust
    ch->ClearAllSkillsAndSpells();
    ch->RemoveAllBoost();

    //////////////////////////////
    // STAT RESET
    ch->SetAirResist  ( 100 );
    ch->SetWaterResist( 100 );
    ch->SetFireResist ( 100 );
    ch->SetEarthResist( 100 );
    ch->SetLightResist( 5000 );
    ch->SetDarkResist ( 100 );
    
    ch->SetAirPower  ( 100 );
    ch->SetWaterPower( 100 );
    ch->SetFirePower ( 100 );
    ch->SetEarthPower( 100 );
    ch->SetLightPower( 100 );
    ch->SetDarkPower ( 100 );

	ch->SetINT( 20 + 5 * numberOfRemorts );
	ch->SetEND( 20 + 5 * numberOfRemorts );
	ch->SetSTR( 20 + 5 * numberOfRemorts );
	ch->SetAGI( 20 + 5 * numberOfRemorts );
	ch->SetWIS( 20 + 5 * numberOfRemorts );
	ch->SetLCK( 100 );
	ch->SetATTACK( 15 );
	ch->SetDODGE( 15 );    

    ch->SetMaxHP( rnd.roll( dice( 2, 5 ) ) + 48 + ch->GetEND() );
    ch->SetHP( ch->GetMaxHP(), false );

    ch->SetMaxMana( 10 + ch->GetTrueINT() * 2 / 3 + ch->GetTrueWIS() / 3 + rnd( 0, 5 ) );
    ch->SetMana( ch->GetMaxMana() );    

    ch->SetLevel( 1 );
    ch->SetXP( 0 );
    

    // NEW POWERS
    WorldPos wlPos = { 0, 0, 0 };
    // Cast the permanent remort spell.
    SpellMessageHandler::ActivateSpell( 10696, ch, NULL, NULL, wlPos );

    // TELEPORT TO REMORT AREA
   DWORD itemId;
   #ifdef BUILD_NMS_CUSTOM_NPC //ok
       if( FlagUserAlignment >= 0 )
       {
          if(numberOfRemorts >=6)
            itemId = 44181;
          else
            itemId = 42631;
       }
       else
       {
          if(numberOfRemorts >=6)
            itemId = 44180;
          else
            itemId = 42630;
       }
   #else
       if( FlagUserAlignment >= 0 )
         itemId = __OBJ_REMORT_WHITE_WINGS;
      else
         itemId = __OBJ_REMORT_BLACK_WINGS;
   #endif

    // Create seraph wings.
    Objects *obj = new Objects;
    if( obj->Create( U_OBJECT, itemId ) )
	{
        DWORD itemGlobalId = obj->GetID();

        // Add the wings to the backpack.
        ch->AddToBackpack( obj );
        
        // And equip them
        ch->equip_object( itemGlobalId );
    }

    TeleportFunc( x, y, world, ch );

    TFCPacket sending;
    sending << (RQ_SIZE)RQ_Remort;
    ch->SendPlayerMessage( sending );
}

/******************************************************************************/
// Remorts directly a player
void EXPORT RemortDirectTo(Unit *self,DWORD x,DWORD y,DWORD world, BOOL bEvil, DWORD uiX)
/******************************************************************************/
{
   if( self == NULL )
   {
      return;
   }
   if( self->GetType() != U_PC )
   {
      return;
   }
    
    Character *ch = static_cast< Character * >( self );

    /////////////////////////////
    // FLAG WIPE
    //NMNMNM FLAG_RESTORE_BANK
    int iFlagResetChar         = ch->ViewFlag( __FLAG_RESET_BOUST_EQUIP_POS);
    int FlagBankOfWindhowl     = ch->ViewFlag( __FLAG_BANK_OF_WINDHOWL );

    int iBankIF1 = ch->ViewFlag( __FLAG_NMSBANK_TYPE_CDOMPTE         );
    int iBankIF2 = ch->ViewFlag( __FLAG_NMSBANK_OR_EN_BANK           );
    int iBankIF3 = ch->ViewFlag( __FLAG_NMSBANK_OR_MINIMUM_WEEKS     );
    int iBankIF4 = ch->ViewFlag( __FLAG_NMSBANK_MAX_OR               );
    int iBankIF5 = ch->ViewFlag( __FLAG_NMSBANK_MIN_OR               );
    int iBankIF6 = ch->ViewFlag( __FLAG_NMSBANK_MAX_EMPRUNT          );
    int iBankIF7 = ch->ViewFlag( __FLAG_NMSBANK_OR_EMPRUNT_NBR_WEEKS );
    int iBankIF8 = ch->ViewFlag( __FLAG_NMSBANK_NEXT_INTERET_TIME    );

    int iInterRP     = ch->ViewFlag( __FLAG_INTERACTION_RP    );
    int iPLNewChest  = ch->ViewFlag( __FLAG_PLAYER_USE_NEW_CHEST    );
    int iPLEventsPts = ch->ViewFlag( __FLAG_POINTS_RP_XP_EVENTS    );
    int iPLEventsPtsT= ch->ViewFlag( __FLAG_POINTS_RP_XP_EVENTS_TOTAL    );

    //Backup les profession
    int iProf01 = ch->ViewFlag( __FLAG_PROF_APOTICAIRE);
    int iProf02 = ch->ViewFlag( __FLAG_PROF_BIJOUTIER );
    int iProf03 = ch->ViewFlag( __FLAG_PROF_COUTURIER );
    int iProf04 = ch->ViewFlag( __FLAG_PROF_ARMURIER  );
    int iProf05 = ch->ViewFlag( __FLAG_PROF_FORGERON  );
    int iProf06 = ch->ViewFlag( __FLAG_PROF_EBENISTE  );
    if(theApp.dwResetProfessionRemort)
    {
       iProf01 = 0;
       iProf02 = 0;
       iProf03 = 0;
       iProf04 = 0;
       iProf05 = 0;
       iProf06 = 0;
    }


	
	int UserKnowsAboutDopplganger = ch->ViewFlag( __FLAG_ADDON_USER_KNOWS_ABOUT_DOPPELGANGER );

    int numberOfRemorts        = uiX;
    int FlagUserAlignment      = 1;
    if(bEvil)
       FlagUserAlignment = -1;


    // V2 - Open the registry and find flags to save from destruction
    RegKeyHandler regKey;
    regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY );
    
    flagList.clear();
    TFormat format;

    int i = 1;
    DWORD flags = static_cast< DWORD >( regKey.GetProfileInt( "OracleFlag1", 0 ) );
    while( flags != 0 )
    {
       FlagList flag;
       flag.flagID = flags;
       flag.flagValue = ch->ViewFlag( flags );
       flagList.push_back( flag );
       
       _LOG_NPCS
          LOG_MISC_1,
          "NPC The Oracle stored player %s's flag ( %u = %u ).",
          (LPCTSTR)ch->GetTrueName(),
          flag.flagID,
          flag.flagValue
          LOG_
          
          i++;
       flags = static_cast< DWORD >( regKey.GetProfileInt( format( "OracleFlag%u", i ), 0 ) );
    }
    ch->DestroyFlags();
    if ( theApp.dwCustomStartupSanctuaryOnOff) 
    { 
       WorldPos WLSanc;
       WLSanc.X	 = theApp.dwCustomStartupSanctuaryX;
       WLSanc.Y	 = theApp.dwCustomStartupSanctuaryY;
       WLSanc.world = theApp.dwCustomStartupSanctuaryW;

       // Test if its a valid position.
       WorldMap *wlWorld = TFCMAIN::GetWorld( WLSanc.world );
       // If world exist and the position is valid, use custom sanctuary position else use the defaut
       if( wlWorld != NULL && wlWorld->IsValidPosition( WLSanc ) )
       {
          DWORD dwStartSanctuary = ( ( (DWORD)( (WORD)WLSanc.X ) << 20 ) + ( (DWORD)( (WORD)WLSanc.Y ) << 8 ) + (DWORD)( (BYTE)WLSanc.world ) );
          ch->SetFlag( __FLAG_DEATH_LOCATION, dwStartSanctuary);
       }
    }

    // Restore flags that are to be kept.
 	 ch->SetFlag( __FLAG_BANK_OF_WINDHOWL, FlagBankOfWindhowl + (uiX*200000));
    ch->SetFlag( __FLAG_NUMBER_OF_REMORTS, numberOfRemorts );
    ch->SetFlag( __FLAG_USER_ALIGNMENT, FlagUserAlignment );
    ch->SetFlag( __QUEST_FIXED_ALIGNMENT, FlagUserAlignment );
    ch->SetFlag( __FLAG_REMORT_POINTS, 8 + 2 * numberOfRemorts );

    ch->SetFlag( __FLAG_NMSBANK_TYPE_CDOMPTE         ,iBankIF1);
    ch->SetFlag( __FLAG_NMSBANK_OR_EN_BANK           ,iBankIF2);
    ch->SetFlag( __FLAG_NMSBANK_OR_MINIMUM_WEEKS     ,iBankIF3);
    ch->SetFlag( __FLAG_NMSBANK_MAX_OR               ,iBankIF4);
    ch->SetFlag( __FLAG_NMSBANK_MIN_OR               ,iBankIF5);
    ch->SetFlag( __FLAG_NMSBANK_MAX_EMPRUNT          ,iBankIF6);
    ch->SetFlag( __FLAG_NMSBANK_OR_EMPRUNT_NBR_WEEKS ,iBankIF7);
    ch->SetFlag( __FLAG_NMSBANK_NEXT_INTERET_TIME    ,iBankIF8);

    ch->SetFlag( __FLAG_RESET_BOUST_EQUIP_POS,iFlagResetChar);

    ch->SetFlag( __FLAG_INTERACTION_RP      ,iInterRP);
    ch->SetFlag( __FLAG_PLAYER_USE_NEW_CHEST,iPLNewChest);
    ch->SetFlag( __FLAG_POINTS_RP_XP_EVENTS ,iPLEventsPts);
    ch->SetFlag( __FLAG_POINTS_RP_XP_EVENTS_TOTAL ,iPLEventsPtsT);

    //Recopie les profession
    ch->SetFlag( __FLAG_PROF_APOTICAIRE, iProf01); 
    ch->SetFlag( __FLAG_PROF_BIJOUTIER , iProf02); 
    ch->SetFlag( __FLAG_PROF_COUTURIER , iProf03); 
    ch->SetFlag( __FLAG_PROF_ARMURIER  , iProf04); 
    ch->SetFlag( __FLAG_PROF_FORGERON  , iProf05); 
    ch->SetFlag( __FLAG_PROF_EBENISTE  , iProf06); 
	
	// Restore flags that are to be kept.
	ch->SetFlag( __FLAG_ADDON_USER_KNOWS_ABOUT_DOPPELGANGER, UserKnowsAboutDopplganger );
    

	// V2 - Retore those flags saved from destruction
    list< FlagList >::iterator j;
    for( j = flagList.begin(); j != flagList.end(); j++ )
    {
       ch->SetFlag ( (*j).flagID , (*j).flagValue );
       _LOG_NPCS
          LOG_MISC_1,
          "NPC The Oracle restored player %s's flag ( %u = %u ).",
          (LPCTSTR)ch->GetTrueName(),
          (*j).flagID,
          (*j).flagValue
          LOG_
    }

    //////////////////////////////
    // REMOVAL OF ALL ITEMS
    for( i = 0; i < EQUIP_POSITIONS; i++ )
    {
        ch->unequip_object( i );
    }
    Unit **equip = ch->GetEquipment();
    for( i = 0; i < EQUIP_POSITIONS; i++ )
    {
        equip[ i ] = NULL;
    }

    // Scroll the backpack for "quest" items
    TemplateList< Unit > *backpack = ch->GetBackpack();
    if( backpack != NULL ){
        backpack->Lock();
        backpack->ToHead();
        while( backpack->QueryNext() ){
            Objects *obj = static_cast< Objects * >( backpack->Object() );

            if( obj->GetStaticReference() == __OBJ_GEM_OF_DESTINY  || obj->GetStaticReference() == theApp.m_dwMinionGemID)
            {
                continue;
            }

			_item *itemStructure = NULL;

			// Get the item structure.
			obj->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &itemStructure );

			if( itemStructure == NULL ){
                continue;
            }
            // If the object should be junked.
            if( itemStructure->dwDropFlags & JUNK_AT_SERAPH ){
                // Destroy it.
                backpack->Remove();
                obj->DeleteUnit();
            }            
        }
        backpack->Unlock();
    }

    //Remove AllBoust
    ch->ClearAllSkillsAndSpells();
    ch->RemoveAllBoost();

    //////////////////////////////
    // STAT RESET
    ch->SetAirResist  ( 100 );
    ch->SetWaterResist( 100 );
    ch->SetFireResist ( 100 );
    ch->SetEarthResist( 100 );
    ch->SetLightResist( 5000 );
    ch->SetDarkResist ( 100 );
    
    ch->SetAirPower  ( 100 );
    ch->SetWaterPower( 100 );
    ch->SetFirePower ( 100 );
    ch->SetEarthPower( 100 );
    ch->SetLightPower( 100 );
    ch->SetDarkPower ( 100 );
    
    ch->SetINT( 20 + 5 * numberOfRemorts );
    ch->SetEND( 20 + 5 * numberOfRemorts );
    ch->SetSTR( 20 + 5 * numberOfRemorts );
    ch->SetAGI( 20 + 5 * numberOfRemorts );
    ch->SetWIS( 20 + 5 * numberOfRemorts );
    ch->SetATTACK( 15 );
    ch->SetDODGE( 15 );    
    
    ch->SetMaxHP( rnd.roll( dice( 2, 5 ) ) + 48 + ch->GetEND() );
    ch->SetHP( ch->GetMaxHP(), false );
    
    ch->SetMaxMana( 10 + ch->GetTrueINT() * 2 / 3 + ch->GetTrueWIS() / 3 + rnd( 0, 5 ) );
    ch->SetMana( ch->GetMaxMana() ,FALSE);    
    
    ch->SetLevel( 1 );
    ch->SetXP( 0 );
    

    //////////////////////////////////
    // NEW POWERS
    WorldPos wlPos = { 0, 0, 0 };


    // Remove The dechu Aura...
    SpellMessageHandler::ActivateSpell( 11369, ch, NULL, NULL, wlPos );
    //cast aura Seraph
    SpellMessageHandler::ActivateSpell( 10696, ch, NULL, NULL, wlPos );

    /////////////////////////////////////
    // TELEPORT TO REMORT AREA

    DWORD itemId;
    if( FlagUserAlignment >= 0 )
    {
       if(numberOfRemorts >=6)
          itemId = 44181;
       else
          itemId = 42631;
    }
    else
    {
       if(numberOfRemorts >=6)
          itemId = 44180;
       else
          itemId = 42630;
    }

    // Create seraph wings.
    Objects *obj = new Objects;
    if( obj->Create( U_OBJECT, itemId ) )
    {
        DWORD itemGlobalId = obj->GetID();

        // Add the wings to the backpack.
        ch->AddToBackpack( obj );
        
        // And equip them
        ch->equip_object( itemGlobalId );
    }

    TeleportFunc( x, y, world, ch );

    TFCPacket sending;
    sending << (RQ_SIZE)RQ_Remort;
    ch->SendPlayerMessage( sending );
}

/******************************************************************************/
// Remorts directly a player
void EXPORT RerollDirect(Unit *self,DWORD x,DWORD y,DWORD world)
/******************************************************************************/
{
   int iErr = theApp.NMSGOLD_Reroll(self,x, y, world );
   if(iErr == 0)
   { 
      self->SendInfoMessage( _STR( 15312 , self->GetLang() ),CL_YELLOW);
      Character *ch = static_cast< Character * >( self );
      ch->GetPlayer()->SaveAccount();
      ch->GetPlayer()->ForceSave(); //reroll by NPC
   }
   else
      self->SendInfoMessage( _STR( 15334 , self->GetLang() ),CL_RED);
}

/******************************************************************************/
// PRE-Remorts a player to dechu
void EXPORT RemortDechu1To
/******************************************************************************/
(
 Unit *self,
 DWORD x,       // Teleport X, Y, World positions
 DWORD y,       // 
 DWORD world    // 
)
{
    if( self == NULL )
    {
        return;
    }
    if( self->GetType() != U_PC )
    {
        return;
    }
    
    Character *ch = static_cast< Character * >( self );

    ch->SetFlag(__FLAG_DEATH_LOCATION,-1462345213);

    // Scroll the backpack for "quest" items
    TemplateList< Unit > *backpack = ch->GetBackpack();
    if( backpack != NULL )
    {
        backpack->Lock();
        backpack->ToHead();
        while( backpack->QueryNext() )
        {
            Objects *obj = static_cast< Objects * >( backpack->Object() );

            if( obj->GetStaticReference() == __OBJ_SCROLL_OF_LIGHTHAVEN ||
                obj->GetStaticReference() == __OBJ_SCROLL_OF_WINDHOWL   || 
                obj->GetStaticReference() == __OBJ_SCROLL_OF_SILVERSKY  ||
                obj->GetStaticReference() == __OBJ_SCROLL_OF_STONECREST ||
                obj->GetStaticReference() == __OBJ_SCROLL_OF_SANCTUARY  ||
                obj->GetStaticReference() == __OBJ_SCROLL_OF_RECALL     ||
                obj->GetStaticReference() == __OBJ_RUNED_STONE_TABLET   ||
                
                obj->GetStaticReference() == 42725                      ||  //scroll of nieve
                obj->GetStaticReference() == 41888                          //scroll of redwall
                )
            {
                // Destroy it.
                backpack->Remove();
                obj->DeleteUnit();
            }
        }
        backpack->Unlock();
    }
    

    ch->ClearAllTeleportSpells();
    TeleportFunc( x, y, world, ch );
}



/******************************************************************************/
void EXPORT RemortDechu2To
/******************************************************************************/
// Remorts a player
(
 Unit *self,
 DWORD x,       // Teleport X, Y, World positions
 DWORD y,       // 
 DWORD world    // 
)
{
    if( self == NULL )
    {
        return;
    }
    if( self->GetType() != U_PC )
    {
        return;
    }
    
    Character *ch = static_cast< Character * >( self );

    /////////////////////////////
    // FLAG WIPE
    int iFlagResetChar      = ch->ViewFlag( __FLAG_RESET_BOUST_EQUIP_POS);
    int FlagBankOfWindhowl  = ch->ViewFlag( __FLAG_BANK_OF_WINDHOWL );
    int numberOfRemorts     = ch->ViewFlag( __FLAG_NUMBER_OF_REMORTS );
    int FlagUserAlignment   = ch->ViewFlag( __QUEST_FIXED_ALIGNMENT );
    int UserKnowsAboutDopplganger = ch->ViewFlag( __FLAG_ADDON_USER_KNOWS_ABOUT_DOPPELGANGER );

    //Backup les profession
    int iProf01 = ch->ViewFlag( __FLAG_PROF_APOTICAIRE);
    int iProf02 = ch->ViewFlag( __FLAG_PROF_BIJOUTIER );
    int iProf03 = ch->ViewFlag( __FLAG_PROF_COUTURIER );
    int iProf04 = ch->ViewFlag( __FLAG_PROF_ARMURIER  );
    int iProf05 = ch->ViewFlag( __FLAG_PROF_FORGERON  );
    int iProf06 = ch->ViewFlag( __FLAG_PROF_EBENISTE  );
    if(theApp.dwResetProfessionRemort)
    {
       iProf01 = 0;
       iProf02 = 0;
       iProf03 = 0;
       iProf04 = 0;
       iProf05 = 0;
       iProf06 = 0;
    }


    int iBankIF1 = ch->ViewFlag( __FLAG_NMSBANK_TYPE_CDOMPTE         );
    int iBankIF2 = ch->ViewFlag( __FLAG_NMSBANK_OR_EN_BANK           );
    int iBankIF3 = ch->ViewFlag( __FLAG_NMSBANK_OR_MINIMUM_WEEKS     );
    int iBankIF4 = ch->ViewFlag( __FLAG_NMSBANK_MAX_OR               );
    int iBankIF5 = ch->ViewFlag( __FLAG_NMSBANK_MIN_OR               );
    int iBankIF6 = ch->ViewFlag( __FLAG_NMSBANK_MAX_EMPRUNT          );
    int iBankIF7 = ch->ViewFlag( __FLAG_NMSBANK_OR_EMPRUNT_NBR_WEEKS );
    int iBankIF8 = ch->ViewFlag( __FLAG_NMSBANK_NEXT_INTERET_TIME    );

    int iInterRP     = ch->ViewFlag( __FLAG_INTERACTION_RP    );
    int iPLNewChest  = ch->ViewFlag( __FLAG_PLAYER_USE_NEW_CHEST    );
    int iPLEventsPts = ch->ViewFlag( __FLAG_POINTS_RP_XP_EVENTS    );
    int iPLEventsPtsT= ch->ViewFlag( __FLAG_POINTS_RP_XP_EVENTS_TOTAL    );

    // V2 - Open the registry and find flags to save from destruction
    RegKeyHandler regKey;
    regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY );

    flagList.clear();
    TFormat format;

    int i = 1;
    DWORD flags = static_cast< DWORD >( regKey.GetProfileInt( "OracleFlag1", 0 ) );
    while( flags != 0 )
    {
       FlagList flag;
       flag.flagID = flags;
       flag.flagValue = ch->ViewFlag( flags );
       flagList.push_back( flag );

       _LOG_NPCS
          LOG_MISC_1,
          "NPC The Oracle stored player %s's flag ( %u = %u ).",
          (LPCTSTR)ch->GetTrueName(),
          flag.flagID,
          flag.flagValue
          LOG_

          i++;
       flags = static_cast< DWORD >( regKey.GetProfileInt( format( "OracleFlag%u", i ), 0 ) );
    }

    ch->DestroyFlags();
    if ( theApp.dwCustomStartupSanctuaryOnOff) 
    { 
       WorldPos WLSanc;
       WLSanc.X	 = theApp.dwCustomStartupSanctuaryX;
       WLSanc.Y	 = theApp.dwCustomStartupSanctuaryY;
       WLSanc.world = theApp.dwCustomStartupSanctuaryW;

       // Test if its a valid position.
       WorldMap *wlWorld = TFCMAIN::GetWorld( WLSanc.world );
       // If world exist and the position is valid, use custom sanctuary position else use the defaut
       if( wlWorld != NULL && wlWorld->IsValidPosition( WLSanc ) )
       {
          DWORD dwStartSanctuary = ( ( (DWORD)( (WORD)WLSanc.X ) << 20 ) + ( (DWORD)( (WORD)WLSanc.Y ) << 8 ) + (DWORD)( (BYTE)WLSanc.world ) );
          ch->SetFlag( __FLAG_DEATH_LOCATION, dwStartSanctuary);
       }
    }

    // Restore flags that are to be kept.
    ch->SetFlag( __FLAG_ADDON_USER_KNOWS_ABOUT_DOPPELGANGER, UserKnowsAboutDopplganger );
    ch->SetFlag( __FLAG_BANK_OF_WINDHOWL, FlagBankOfWindhowl );
    ch->SetFlag( __FLAG_NUMBER_OF_REMORTS, 0 );
    ch->SetFlag( __FLAG_USER_ALIGNMENT, FlagUserAlignment );
    ch->SetFlag( __QUEST_FIXED_ALIGNMENT, FlagUserAlignment );
    ch->SetFlag( __FLAG_REMORT_POINTS, 0 );

    ch->SetFlag( __FLAG_NMSBANK_TYPE_CDOMPTE         ,iBankIF1);
    ch->SetFlag( __FLAG_NMSBANK_OR_EN_BANK           ,iBankIF2);
    ch->SetFlag( __FLAG_NMSBANK_OR_MINIMUM_WEEKS     ,iBankIF3);
    ch->SetFlag( __FLAG_NMSBANK_MAX_OR               ,iBankIF4);
    ch->SetFlag( __FLAG_NMSBANK_MIN_OR               ,iBankIF5);
    ch->SetFlag( __FLAG_NMSBANK_MAX_EMPRUNT          ,iBankIF6);
    ch->SetFlag( __FLAG_NMSBANK_OR_EMPRUNT_NBR_WEEKS ,iBankIF7);
    ch->SetFlag( __FLAG_NMSBANK_NEXT_INTERET_TIME    ,iBankIF8);

    ch->SetFlag( __FLAG_RESET_BOUST_EQUIP_POS,iFlagResetChar);

    ch->SetFlag( __FLAG_INTERACTION_RP      ,iInterRP);
    ch->SetFlag( __FLAG_PLAYER_USE_NEW_CHEST,iPLNewChest);
    ch->SetFlag( __FLAG_POINTS_RP_XP_EVENTS ,iPLEventsPts);
    ch->SetFlag( __FLAG_POINTS_RP_XP_EVENTS_TOTAL ,iPLEventsPtsT);

    //Recopie les profession
    ch->SetFlag( __FLAG_PROF_APOTICAIRE, iProf01); 
    ch->SetFlag( __FLAG_PROF_BIJOUTIER , iProf02); 
    ch->SetFlag( __FLAG_PROF_COUTURIER , iProf03); 
    ch->SetFlag( __FLAG_PROF_ARMURIER  , iProf04); 
    ch->SetFlag( __FLAG_PROF_FORGERON  , iProf05); 
    ch->SetFlag( __FLAG_PROF_EBENISTE  , iProf06); 

    // V2 - Retore those flags saved from destruction
    list< FlagList >::iterator j;
    for( j = flagList.begin(); j != flagList.end(); j++ )
    {
       ch->SetFlag ( (*j).flagID , (*j).flagValue );
       _LOG_NPCS
          LOG_MISC_1,
          "NPC The Oracle restored player %s's flag ( %u = %u ).",
          (LPCTSTR)ch->GetTrueName(),
          (*j).flagID,
          (*j).flagValue
          LOG_
    }


    /*
    if(numberOfRemorts == 0)
       numberOfRemorts = 2;
    else
       numberOfRemorts++;
    if(numberOfRemorts > ACK_MAXREMORTS)
       numberOfRemorts = ACK_MAXREMORTS;
    */
    numberOfRemorts = 6; //fixeda 6 now...

    ch->SetFlag( __FLAG_NMS_DECHU, numberOfRemorts); //est rendu dechu...
    ch->SetFlag( __FLAG_REMORT_POINTS, 8 + 2 * numberOfRemorts ); 
    
    //////////////////////////////
    // REMOVAL OF ALL ITEMS
    for( i = 0; i < EQUIP_POSITIONS; i++ )
    {
        ch->unequip_object( i );
    }
    
    Unit **equip = ch->GetEquipment();
    for( i = 0; i < EQUIP_POSITIONS; i++ )
    {
        // Flush all items that couldn't be unequipped.
        equip[ i ] = NULL;
    }

    // Scroll the backpack for "quest" items
    TemplateList< Unit > *backpack = ch->GetBackpack();
    if( backpack != NULL )
    {
        backpack->Lock();
        backpack->ToHead();
        while( backpack->QueryNext() )
        {
            Objects *obj = static_cast< Objects * >( backpack->Object() );

            if( obj->GetStaticReference() == __OBJ_GEM_OF_DESTINY  || obj->GetStaticReference() == theApp.m_dwMinionGemID)
            {
                continue;
            }

			_item *itemStructure = NULL;

			// Get the item structure.
			obj->SendUnitMessage(MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &itemStructure );

			if( itemStructure == NULL ){
                continue;
            }
            // If the object should be junked.
            if( itemStructure->dwDropFlags & JUNK_AT_SERAPH )
            {
                // Destroy it.
                backpack->Remove();
                obj->DeleteUnit();
            }            
        }
        backpack->Unlock();
    }

    
    //////////////////////////////
    // STAT RESET
    ch->SetAirResist  ( 100 );
    ch->SetWaterResist( 100 );
    ch->SetFireResist ( 100 );
    ch->SetEarthResist( 100 );
    ch->SetLightResist( 5000 );
    ch->SetDarkResist ( 100 );
    
    ch->SetAirPower  ( 100 );
    ch->SetWaterPower( 100 );
    ch->SetFirePower ( 100 );
    ch->SetEarthPower( 100 );
    ch->SetLightPower( 100 );
    ch->SetDarkPower ( 100 );

	ch->SetINT( 50 );
	ch->SetEND( 50 );
	ch->SetSTR( 50 );
	ch->SetAGI( 50 );
	ch->SetWIS( 50 );
	ch->SetATTACK( 15 );
	ch->SetDODGE( 15 );    

    ch->SetMaxHP( rnd.roll( dice( 2, 5 ) ) + 90 + ch->GetEND() );
    ch->SetHP( ch->GetMaxHP(), false );

    ch->SetMaxMana( 10 + ch->GetTrueINT() * 2 / 3 + ch->GetTrueWIS() / 3 + rnd( 0, 5 ) );
    ch->SetMana( ch->GetMaxMana() ,FALSE);    

    ch->ClearAllSkillsAndSpells();
    ch->SetLevel( 1 );
    ch->SetXP( 0 );
    ch->SetKarma(0);

    //////////////////////////////////
    // NEW POWERS
    WorldPos wlPos = { 0, 0, 0 };
    // Cast the permanent remort spell.
    SpellMessageHandler::ActivateSpell( 10414, ch, NULL, NULL, wlPos ); //remortToDechu

    /////////////////////////////////////
    // TELEPORT TO REMORT AREA

    DWORD itemId;
    itemId = 44177;

    // Create seraph wings.
    Objects *obj = new Objects;
    if( obj->Create( U_OBJECT, itemId ) )
    {
        DWORD itemGlobalId = obj->GetID();

        // Add the wings to the backpack.
        ch->AddToBackpack( obj );
        
        // And equip them
        ch->equip_object( itemGlobalId );
    }

    TeleportFunc( x, y, world, ch );

    TFCPacket sending;
    sending << (RQ_SIZE)RQ_Remort;
    ch->SendPlayerMessage( sending );
}




/******************************************************************************/
// Makes the NPC check for a clear path to the target
bool EXPORT __IsBlocked( 
 Unit *self,		// The NPC
 Unit *target		// The Player
)
/******************************************************************************/
{
	if( (self != NULL) && (target != NULL))
	{
		WorldPos tempPos;
		WorldMap *wlWorld = TFCMAIN::GetWorld( self->GetWL().world );
		Unit *lpCollisionUnit = NULL;

		if (wlWorld->GetCollisionPos( self->GetWL(), target->GetWL(), &tempPos, &lpCollisionUnit, false, false )) 
		{
			return TRUE;
		}

		return FALSE;
	} 
	else 
	{
		return FALSE;
	}
}