/******************************************************************************
Modify for vs2008 (26/04/2009)
Add En prison validation, zone arena restriction runestone block flood by Nightmare (28/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "Potions.h"
#include "../TFC Server.h"
#include "../RPMaster.h"
extern CTFCServerApp theApp;

/******************************************************************************/
Potions::Potions()
/******************************************************************************/
{}
/******************************************************************************/
Potions::~Potions()
/******************************************************************************/
{}
/******************************************************************************/
void Potions::OnInitialise(UNIT_FUNC_PROTOTYPE)
/******************************************************************************/
{
	ObjectStructure::OnInitialise( UNIT_FUNC_PARAM );
	self->SetFlag(__FLAG_CHARGES, magic.charges);
}
/******************************************************************************/
// Uses a potion
void Potions::OnUse(UNIT_FUNC_PROTOTYPE)
/******************************************************************************/
{
    int nEnPrison    = medium->ViewFlag( __FLAG_NMS_EN_PRISON );
    int nUnderPlayer = medium->GetUnderBlockMap();
	BOOL boBrakeItem = FALSE;
    LPOBJECT_SPELL lpSpell;

    int nOldCharges = self->ViewFlag( __FLAG_CHARGES );
    int nCharges = nOldCharges;    

	DWORD &itemUsed = *(DWORD *)( valueOUT );

    TRACE( "\r\nCharges %u", nCharges );

	// Search for attack spells
	tlSpells.ToHead();
	while( tlSpells.QueryNext() )
	{
		lpSpell = tlSpells.Object();
		
		// If there is a spell to trigger.
		if( lpSpell->wHook == MSG_OnUse )
		{
			itemUsed = TRUE;
			if( nCharges != 0 )
			{
            bool boChangeCharge = true;

            if(nEnPrison)
            {
               CString strMessage;
               strMessage.Format(_STR( 15146, medium->GetLang() ));
               medium->SendSystemMessage(strMessage);
            }
            else
            {
               // A bloquer si pas en zone combat....
               //NMS Items
               //__arena_prisme     43189
               //__arena_stone      43190
               //__arena_critiques  43191

               //Found if item are in Arena Items..
               BOOL bArenaFound = FALSE;
               for(int i=0;i<theApp.m_aArenaItems.GetSize();i++)
               {
                  if(self->GetStaticReference() == theApp.m_aArenaItems[i].uiID)
                  {
                     bArenaFound = TRUE;
                     i = theApp.m_aArenaItems.GetSize()+1;
                  }

               }
               
               //CV: IMP:mettre les objets arene en list dans le control panel...
               if((bArenaFound) && 
                   nUnderPlayer != __FULL_PVP_CANNOT_REALLY_DIE_DROP_ORROB && 
                   nUnderPlayer != __FULL_PVP_CANNOT_REALLY_DIE_DROP_ORROB_CAST_SPELL &&
                   nUnderPlayer != __ARENAGAME_FULL_PVP &&
                   nUnderPlayer != __ARENAGAME_BT_FULL_PVP &&
                   nUnderPlayer != __ARENAGAME_RT_FULL_PVP 
                   )
               {
                  CString strMessage;
                  strMessage.Format(_STR( 15147, medium->GetLang() ));
                  medium->SendSystemMessage(strMessage);
               }
               else
               {
                  bool bCanExecute = true;
                  if(self->GetStaticReference() == 41840) //Runestone...
                  {
                      time_t tCurTime  =  time(NULL);
                      time_t tLastTime =  medium->ViewFlag(__FLAG_NMS_RUNESTONE_TIME);
                      if(tCurTime - tLastTime > 5)
                      {
                         medium->SetFlag(__FLAG_NMS_RUNESTONE_TIME,tCurTime);
                      }
                      else
                      {
                         CString strMessage;
                         strMessage.Format(_STR( 15148, medium->GetLang() ));
                         medium->SendSystemMessage(strMessage);
                         bCanExecute = false;
                      }
                  }

                  //NMS == 45542
                  if(self->GetStaticReference() == theApp.dwEventsXPTradeItemID) //recompense Event en XP
                  {
                     Character *ThisCharacter = static_cast<Character *>( medium );

                     RPMaster::RPEchangerRPDirect(ThisCharacter->GetPlayer());
                  }


                  //CV: valide si c<est un parcho xp ou orr et que le systeme ets actif ou pas...
                  //valide si le systeme est a off pour ne pas prendre d<items...
                  BOOL bXPFound = FALSE;
                  BOOL bORFound = FALSE;
                  for(int i=0;i<theApp.m_aXPItems.GetSize();i++)
                  {
                     if(self->GetStaticReference() == theApp.m_aXPItems[i].uiID)
                     {
                        bXPFound = TRUE;
                        i = theApp.m_aXPItems.GetSize()+1;
                     }
                  }
                  for(int i=0;i<theApp.m_aORItems.GetSize();i++)
                  {
                     if(self->GetStaticReference() == theApp.m_aORItems[i].uiID)
                     {
                        bORFound = TRUE;
                        i = theApp.m_aORItems.GetSize()+1;
                     }
                  }

                  if(theApp.dwManageScrollXP == 0)
                  {
                     if(bXPFound || bORFound)
                     {
                         boChangeCharge = false;
                         bCanExecute = false;

                         CString strMessage;
                         strMessage.Format(_STR( 15322, medium->GetLang() ));
                         medium->SendSystemMessage(strMessage);
                     }
                  }

                  //valide si ye deja en trein utiliser un parcho et veux en utiliser un autres...
                  if(bXPFound && bCanExecute)
                  {
                     Character *ThisCharacter = static_cast<Character *>( medium );

                     int iXPManageTime = medium->ViewFlag(__FLAG_SCROLL_XP_MANAGEMENT);
                     int iXPTimeStamp  = medium->ViewFlag(__FLAG_SCROLL_XP_TIMESTAMP);

                     if(iXPTimeStamp !=0)
                     {
                        boChangeCharge = false;
                        bCanExecute = false;

                        CString strMessage;
                        strMessage.Format(_STR( 15323, medium->GetLang() ));
                        medium->SendSystemMessage(strMessage);
                     }

                     if(bCanExecute)
                     {
                        _LOG_SPECIAL_ITEMS
                           LOG_ALWAYS,
                           "Player %s (Account: %s) -->USE XP Scroll Item ID  %d ",
                           (LPCTSTR)ThisCharacter->GetTrueName(),
                           (LPCTSTR)ThisCharacter->GetPlayer()->GetFullAccountName(),
                           self->GetStaticReference()
                           LOG_
                     }
                  }

                  if(bORFound && bCanExecute)
                  {
                     Character *ThisCharacter = static_cast<Character *>( medium );

                     int iORManageTime = medium->ViewFlag(__FLAG_SCROLL_OR_MANAGEMENT);
                     int iORTimeStamp  = medium->ViewFlag(__FLAG_SCROLL_OR_TIMESTAMP);

                     if(iORTimeStamp !=0)
                     {
                        boChangeCharge = false;
                        bCanExecute = false;

                        CString strMessage;
                        strMessage.Format(_STR( 15510, medium->GetLang() ));
                        medium->SendSystemMessage(strMessage);
                     }

                     if(bCanExecute)
                     {
                        _LOG_SPECIAL_ITEMS
                           LOG_ALWAYS,
                           "Player %s (Account: %s) -->USE OR Scroll Item ID  %d ",
                           (LPCTSTR)ThisCharacter->GetTrueName(),
                           (LPCTSTR)ThisCharacter->GetPlayer()->GetFullAccountName(),
                           self->GetStaticReference()
                           LOG_
                     }
                  }

                 

 
  
 
                  if(bCanExecute)
                  {
                     WorldPos wlPos = medium->GetWL();
                     SpellMessageHandler::ActivateSpell( lpSpell->wSpellID, medium, self, target, wlPos );

                     /*
                     FILE *pft = NULL;
                     fopen_s(&pft,"C:\\!!!!!invi2.txt","a+t");
                     fprintf(pft,"Self == %x\n",self);
                     fprintf(pft,"medium == %x\n",medium);
                     fprintf(pft,"target == %x\n",target);
                     fprintf(pft,"\n\n");
                     fclose(pft);
                     */

                     if(medium != target)
                     {
                        Character *ThisCharacter = static_cast<Character *>( medium );
                        ThisCharacter->StartAutoCombat(Character::Attack( Character::Attack::spell, lpSpell->wSpellID ),target);
                     }
                  }
               }
            }
            
            // If charges aren't unlimited.
            if( nOldCharges > 0 && boChangeCharge)
            {
               nCharges--;
               if( nCharges <= 0 )
               {
                  boBrakeItem = TRUE;
               }
            }
         }
         else
         {
            OnNoMoreShots( medium, NULL, medium, NULL, NULL );
            boBrakeItem = TRUE;
         }
      }
   }
   
   if( nOldCharges != nCharges && nOldCharges > 0 )
   {
      self->SetFlag( __FLAG_CHARGES, nCharges );
   }
   
   if( boBrakeItem )
   {
      self->SetMark( MARK_DELETION );
   }
   
}
/******************************************************************************/
ObjectStructure *Potions::CreateObject( void )
/******************************************************************************/
{
	return new Potions;
}