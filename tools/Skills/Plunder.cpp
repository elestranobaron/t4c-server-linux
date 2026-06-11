
// Plunder.cpp: implementation of the Plunder class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Plunder.h"
#include "../GameDefs.h"
#include "../intltext.h"
#include "../ObjectListing.h"
#include "../t4clog.h"
#include "../TFC Server.h"

extern CTFCServerApp theApp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Plunder::Plunder()
{
   s_saAttrib.skLevel = 17;
   s_saAttrib.skAGI = 50;
   s_saAttrib.skSTR = 0;
   s_saAttrib.skEND = 0;
   s_saAttrib.skINT = 0;
   s_saAttrib.skWIS = 0;
   s_saAttrib.skWIL = 0;
   s_saAttrib.skLCK = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
void Plunder::Destroy( void )
//////////////////////////////////////////////////////////////////////////////////////////
//  Destroys peek.
// 
//////////////////////////////////////////////////////////////////////////////////////////
{
}

LPSKILLPNTFUNC Plunder::lpOnAddPnts = NULL;

#define PLUNDER_SKILL   lpusUserSkill->GetSkillPnts( self )

//////////////////////////////////////////////////////////////////////////////////////////
int Plunder::Func
//////////////////////////////////////////////////////////////////////////////////////////
// Critical strike main function
// 
(
 DWORD dwReason,			// Hook which was used to call the skill.
 Unit *self,				// Unit using the skill.
 Unit *medium,				// Unused.
 Unit *target,				// Target of attack.
 void *valueIN,			// Unused.
 void *valueOUT,			// Unused.
 LPUSER_SKILL lpusUserSkill // Current skill strength of the user.
 )
 // Return: int, SKILL_* return parameter.
 //////////////////////////////////////////////////////////////////////////////////////////
{    
   const int PlunberRange = 3;
   CString strPilledItems;
   
   // Requires both target and self to be present.
   if( !target || !self ) 
      return SKILL_NO_FEEDBACK;
   
   // Skill only works on OTHER characters.
   if( self->GetType() != U_PC || target->GetType() != U_PC || self == target )
   {
      self->SendSystemMessage( _STR( 23, self->GetLang() ) );        
      return SKILL_NO_FEEDBACK;
   }

   if(target->ViewFlag(__FLAG_NMS_PLAYER_CAN_PLUNDER) <=0)
   {
      //self->SendSystemMessage( _STR( 15008 , self->GetLang() ) );        
      self->SendInfoMessage( _STR( 15008 , self->GetLang() ) ,0x000080);
      return SKILL_NO_FEEDBACK;
   }
   
   
   Character *lpChar = static_cast< Character * >( self );
   
   target->Lock();
   self->Lock();
   
   const int HidingBonus = 0;
   
   // Calculate the range between the two players.
   int nXdiff = abs( self->GetWL().X - target->GetWL().X );
   int nYdiff = abs( self->GetWL().Y - target->GetWL().Y );
   int nRange = ::sqrt( double(nXdiff * nXdiff + nYdiff * nYdiff) );
   
   
   
   if( self->GetType() == U_PC )
   {
      Character *ch = static_cast< Character * >( self );
      ch->StopAutoCombat();
   }
   
   if( nRange <= PlunberRange  )
   {
      // If robbed was successful.
      int iSuccess = rnd( 0, 10000 );

      int iLevelInfluence = 0;
      int dwlevelrange = self->GetLevel() - target->GetLevel();

      if( dwlevelrange >= 100 )
         iLevelInfluence = 15;
      else if( dwlevelrange >= 50 )
         iLevelInfluence = 10;		
      else if( dwlevelrange > 25 )
         iLevelInfluence = 5;
      else if (dwlevelrange >= -25 && dwlevelrange <= 25 )
         iLevelInfluence = 0;						
      else if( dwlevelrange <= -100 )
         iLevelInfluence = -15;
      else if( dwlevelrange <= -50 )
         iLevelInfluence = -10;
      else if( dwlevelrange < -25 )
         iLevelInfluence = -5;



      
      if(  iSuccess <= (PLUNDER_SKILL*(25+iLevelInfluence)))
      {
         BOOL boFound     = FALSE;
         BOOL boCannotGet = FALSE;
         
         // Find the object in the target's backpack.
         TemplateList< Unit > *lpBackpack      = target->GetBackpack();
         TemplateList< Unit > *lpThiefBackpack = self->GetBackpack();
         
         // If both target and the thieve's backpack exist.
         if( lpBackpack && lpThiefBackpack )
         {
            MultiLock( lpBackpack, lpThiefBackpack );
            
            //en premier on construit une liste des item pouvant etre voler...
            //
            TemplateList< Unit > InvItemID;
            
            lpBackpack->ToHead();
            while( lpBackpack->QueryNext())
            {
               Objects *lpItem = static_cast< Objects * >( lpBackpack->Object() );
               _item *lpItemStructure = NULL;
               lpItem->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItemStructure );
               
               if( !( lpItemStructure->dwDropFlags & CANNOT_DROP_ITEM ) && !( lpItemStructure->dwDropFlags & CANNOT_ROB_ITEM ) )
               {
                  InvItemID.AddToTail(lpItem);
               }
            }
            
            DWORD dwIDSpoiled = 0;
            if(InvItemID.NbObjects() >0)
            {
               DWORD dwRndItem   = rnd(0,InvItemID.NbObjects());
               int dwCnt = 0;
               InvItemID.ToHead();
               for(dwCnt=0;dwCnt<dwRndItem;dwCnt++)
               {
                  InvItemID.QueryNext();
               }

               Objects *lpItem = static_cast< Objects * >( InvItemID.Object() );
               dwIDSpoiled = lpItem->GetID();
            }
            else
            {
               dwIDSpoiled = 0xFFFFFFFF;
            }


            lpBackpack->ToHead();
            while( lpBackpack->QueryNext() && !boFound && dwIDSpoiled != 0xFFFFFFFF)
            {
               Objects *lpItem = static_cast< Objects * >( lpBackpack->Object() );
               
               // If the object was found.
               if( lpItem->GetID() == (DWORD)dwIDSpoiled )
               {
                  // Get the item's structure.
                  _item *lpItemStructure = NULL;
                  lpItem->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItemStructure );
                  
                  // If the object can be dropped, robbed
                  if( !( lpItemStructure->dwDropFlags & CANNOT_DROP_ITEM ) && !( lpItemStructure->dwDropFlags & CANNOT_ROB_ITEM ) )
                  {
                     // If the item is not too heavy
                     if( lpItemStructure->size + self->GetWeight() < self->GetMaxWeight() )
                     {                                                                   
                        // Special case for gold.                                
                        if( lpItem->GetStaticReference() == __OBJ_GOLD )
                        {
                           int nGoldRobbed = target->GetGold()/10+ rnd(0,(target->GetGold()/5));
                           
                           // If gold was robbed
                           if( nGoldRobbed > 0 )
                           {
                              DWORD dwTargetGold = target->GetGold();
                              // If the amount stolen is smaller than the total amount on the target.
                              if( nGoldRobbed < dwTargetGold )
                              {
                                 // Substract from the total amount.
                                 dwTargetGold -= nGoldRobbed;
                              }
                              else
                              {
                                 // Gold robbed equals the total target's gold.
                                 nGoldRobbed = static_cast< int >( dwTargetGold );
                                 dwTargetGold = 0;                                                                                        
                              }
                              
                              // Update target gold
                              target->SetGold( dwTargetGold );
                              
                              // Update robber's gold
                              DWORD dwRobberGold = self->GetGold();
                              dwRobberGold += nGoldRobbed;
                              
                              
                              if( self->GetType() == U_PC && target->GetType() == U_PC )
                              {
                                 Character *selfCh     = static_cast< Character * >( self );
                                 Character *targetCh   = static_cast< Character * >( target );
                                 Players *selfPlayer   = selfCh->GetPlayer();
                                 Players *targetPlayer = selfCh->GetPlayer();
                                 
                                 _LOG_ITEMS
                                    LOG_MISC_1,
                                    "Player %s (Id %u, Acct %s, Lvl %u, Pos %u, %u, %u) Pulled %u gold from player %s (Id %u, Acct %s, Lvl %u, Pos %u, %u, %u) (new amount=%u gold).",
                                    (LPCTSTR)self->GetName( _DEFAULT_LNG ),
                                    self->GetID(),
                                    (LPCTSTR)selfPlayer->GetFullAccountName(),
                                    self->GetLevel(),
                                    self->GetWL().X,
                                    self->GetWL().Y,
                                    self->GetWL().world,
                                    nGoldRobbed,
                                    (LPCTSTR)target->GetName( _DEFAULT_LNG ),
                                    target->GetID(),
                                    (LPCTSTR)targetPlayer->GetFullAccountName(),
                                    target->GetLevel(),
                                    target->GetWL().X,
                                    target->GetWL().Y,
                                    target->GetWL().world,
                                    dwTargetGold
                                    LOG_
                              }
                              
                              self->SetGold( dwRobberGold );
                              strPilledItems.Format("%d %s",nGoldRobbed,_STR( 33 , self->GetLang() ));
                           }
                           else
                           {
                              // Tell the robber. impossible de mettre main sur piece or...
                              //self->SendSystemMessage( _STR( 2778, self->GetLang() ) );
                              self->SendInfoMessage( _STR( 2778, self->GetLang() ), 0x000080 );
                              boCannotGet = TRUE;
                           }                                   
                        }
                        else
                        {
                           TFCPacket sending;
                           
                           // Create a copy of the item.
                           Objects *itemCopy = new Objects;
                           if( !itemCopy->Create( U_OBJECT, lpItem->GetStaticReference() ) )
                           {
                              itemCopy->DeleteUnit();
                              itemCopy = NULL;
                           }

                           
                           if( itemCopy != NULL )
                           {
                              DWORD remove = 1;
                              if(lpItem->GetQty() >5)
                                 remove = lpItem->GetQty()/5;

                              itemCopy->SetQty(remove);

                             
                              if( self->GetType() == U_PC && target->GetType() == U_PC )
                              {
                                 Character *selfCh = static_cast< Character * >( self );
                                 Character *targetCh = static_cast< Character * >( target );
                                 
                                 Players *selfPlayer = selfCh->GetPlayer();
                                 Players *targetPlayer = selfCh->GetPlayer();
                                 
                                 _LOG_ITEMS
                                    LOG_MISC_1,
                                    "Player %s (Id %u, Acct %s, Lvl %u, Pos %u, %u, %u) plumbed item %s from player %s (Id %u, Acct %s, Lvl %u, Pos %u, %u, %u).",
                                    (LPCTSTR)self->GetName( _DEFAULT_LNG ),
                                    self->GetID(),
                                    (LPCTSTR)selfPlayer->GetFullAccountName(),
                                    self->GetLevel(),
                                    self->GetWL().X,
                                    self->GetWL().Y,
                                    self->GetWL().world,
                                    (LPCTSTR)lpItem->GetName( _DEFAULT_LNG ),
                                    (LPCTSTR)target->GetName( _DEFAULT_LNG ),
                                    target->GetID(),
                                    (LPCTSTR)targetPlayer->GetFullAccountName(),
                                    target->GetLevel(),
                                    target->GetWL().X,
                                    target->GetWL().Y,
                                    target->GetWL().world
                                    LOG_
                              }

                              DWORD oldQty = lpItem->GetQty();
                              lpItem->Remove( remove );
                              if( lpItem->GetQty() == 0 )
                              {                                        
                                 // Remove the object from the target's backpack.
                                 lpBackpack->Remove();
                                 
                                 // Reset its quantity
                                 lpItem->SetQty( oldQty );
                                 
                                 // Destroy the copy and use this item
                                 // as the 'copy'.
                                 itemCopy->DeleteUnit();
                                 itemCopy = lpItem;
                              }
                              
                              strPilledItems.Format("%d %s",itemCopy->GetQty(),itemCopy->GetName(self->GetLang()));

                              
                              // Add the item copy to the thief's backpack.
                              lpChar->AddToBackpack( itemCopy );
                              // Update the thieve's personnal backpack.
                              sending << (RQ_SIZE)RQ_ViewBackpack2;
                              sending << (char)1;
                              sending << (long)target->GetID();		    		    
                              self->PacketBackpack( sending );
                              // Send it to the peeking player.
                              self->SendPlayerMessage( sending );
                              
                              // Update the victim's backpack (for opened backpacks).
                              sending.Destroy();
                              sending << (RQ_SIZE)RQ_ViewBackpack2;
                              sending << (char)0;
                              sending << (long)target->GetID();		    		    
                              target->PacketBackpack( sending );
                              // Send it to the peeking player.
                              target->SendPlayerMessage( sending );

                              self->SetFlag(__FLAG_NMS_PILLER_SKILL,self->ViewFlag(__FLAG_NMS_PILLER_SKILL)+1);
                           }
                        }
                     }
                     else
                     {
                        boCannotGet = TRUE;
                        // Send a too much weight message.
                        self->SendSystemMessage( _STR( 17, self->GetLang() ) );
                     }
                   }
                   else
                   {
                      boCannotGet = TRUE;
                      // Send a strange force message.
                      self->SendSystemMessage( _STR( 29, self->GetLang() ) );
                   }
                   boFound = TRUE;
               }
            }
            if(dwIDSpoiled == 0xFFFFFFFF)
            {
               target->SetFlag(__FLAG_NMS_PLAYER_CAN_PLUNDER,0);
               self->SendInfoMessage( _STR( 15008 , self->GetLang() ) ,0x000080);
            }
            lpBackpack->Unlock();
            lpThiefBackpack->Unlock();
         }
         if( boFound )
         {
            if( !boCannotGet )
            {
               // Send success message.
               //self->SendSystemMessage( _STR( 15009, self->GetLang() ) );
               //self->SendSystemMessage(strPilledItems);

               self->SendInfoMessage( _STR( 15009, self->GetLang() ),0x000080 );
               self->SendInfoMessage(strPilledItems,0xFFFFFF);

               if(theApp.m_dwPVPSyetem2Actif == 1) //PVP SYSTEM
               {
                  int iNbrTime = self->ViewFlag(__FLAG_NMS_PVP_ROB_PLUNDER_CNT)+1;
                  if(iNbrTime >=10)
                  {
                     //add 1 point de crime and reset compteur
                     self->SetFlag(__FLAG_NMS_PVP_ROB_PLUNDER_CNT,0);
                     int iCrime = self->GetCrime();
                     if(iCrime <9)
                        self->SetCrime(iCrime+1);
                  }
                  else
                     self->SetFlag(__FLAG_NMS_PVP_ROB_PLUNDER_CNT,iNbrTime);
               }

               //target->SendSystemMessage( _STR( 15010, self->GetLang() ) );
               target->SendInfoMessage( _STR( 15010, self->GetLang() ),0x000080 );
               target->SetFlag(__FLAG_NMS_PLAYER_CAN_PLUNDER,target->ViewFlag(__FLAG_NMS_PLAYER_CAN_PLUNDER)-1);
            }
            else
            {
               target->SetFlag(__FLAG_NMS_PLAYER_CAN_PLUNDER,target->ViewFlag(__FLAG_NMS_PLAYER_CAN_PLUNDER)-1);
            }
         }
         else
         {
            self->SendSystemMessage( _STR( 28, self->GetLang() ) );
            target->SetFlag(__FLAG_NMS_PLAYER_CAN_PLUNDER,target->ViewFlag(__FLAG_NMS_PLAYER_CAN_PLUNDER)-1);
         }
      }
      else
      {
         //self->SendSystemMessage( _STR( 15011, self->GetLang() ) );
         self->SendInfoMessage( _STR( 15011, self->GetLang() ),0x000080 );
         target->SetFlag(__FLAG_NMS_PLAYER_CAN_PLUNDER,target->ViewFlag(__FLAG_NMS_PLAYER_CAN_PLUNDER)-1);
      }
   }
   else
   {
      self->SendSystemMessage( _STR( 24, self->GetLang() ) );
   }

   self->Unlock();
   target->Unlock();
   return SKILL_NO_FEEDBACK;
}