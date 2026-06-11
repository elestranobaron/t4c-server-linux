/* Note to self: 
 *   This is possibly the reason for the Deadlocks:
 *     CPlayerManager::GetCharacterRessourceByID();
 */

#include "stdafx.h"
#include "TFC Server.h"
#include "TFC_MAIN.h"
#include "EventsMaster.h"
#include "IntlText.h"
#include "Unit.h"
#include "PlayerManager.h"
#include "QuestFlagsListing.h"
#include "RegKeyHandler.h"
#include "EVENTS_key.h"
#include "NPCMacroScriptLng.h"
#include "NPC thread.h"
#include "CustomBuild.h"

#include "T4CLog.h"
#include "DeadlockDetector.h"
#ifdef _WIN32
#include <mmsystem.h>
#endif


unsigned int  get_dword_E();
unsigned short get_word_E();
BYTE get_byte_E();
double get_double_E();
int get_long_E();
short get_short_E();
char* get_string_E();

extern CTFCServerApp theApp;
extern TFC_MAIN *TFCServer;


CPtrArray EventsMaster::c_aEvents;

BYTE *g_pDataTmpE = NULL;
char g_chLigneE[2048];

static CLock     g_EventsMasterLock;




//////////////////////////////////////////////////////////////////////////////////////////
// Creates the class
//////////////////////////////////////////////////////////////////////////////////////////
void EventsMaster::Create()
{
   #ifndef BUILD_NMS_CUSTOM_NPC
      return;
   #endif

   char strTmp[512];

   CString strFileP;
   strFileP = TFCMAIN::GetHomeDir();
#ifdef _WIN32
   strFileP += "WDA\\NMS_Events.dat";
#else
   strFileP += "WDA/NMS_Events.dat";
#endif

   FILE *pf = NULL;
   fopen_s(&pf,strFileP,"rb");
   if(!pf)
   {
      _LOG_DEBUG
         LOG_CRIT_ERRORS,
         "NO Events file found %s",
         strFileP
         LOG_

         printf( "\r\nNO Events file found %s.",strFileP);
      return;
   }

   fseek(pf,0,SEEK_END);
   int dwTaille = ftell(pf);
   fseek(pf,0,SEEK_SET);
   // Alloue la memoire pour contenir le fichier

   BYTE *pData = NULL;

   pData = new BYTE[dwTaille];
   // lit le contenue du fichier
   fread(pData,dwTaille,1,pf);
   fclose(pf);

   for (UINT i=0; i<dwTaille; i++)
      pData[i] ^= EVENTS_key[i%3418];

   g_pDataTmpE = pData;

   int uiNbrEvents = get_dword_E();
   if(uiNbrEvents >0)
   {
      for(int i=0;i<uiNbrEvents;i++)
      {
         sEvents *pEventData = new sEvents;

         pEventData->chEnable = get_byte_E(); 
         pEventData->ushID    = get_word_E();

         //le NOM
         sprintf_s(strTmp,512,"%s",get_string_E());
         pEventData->iNameSize = strlen(strTmp);
         if(pEventData->iNameSize >0)
         {
            pEventData->pstrName = new char[strlen(strTmp)+1];
            strcpy_s(pEventData->pstrName,strlen(strTmp)+1,strTmp);
         }
         else
            pEventData->pstrName = NULL;
         //Le start Message
         sprintf_s(strTmp,512,"%s",get_string_E());
         pEventData->iStartMsg = strlen(strTmp);
         if(pEventData->iStartMsg >0)
         {
            pEventData->pstrStartMsg = new char[strlen(strTmp)+1];
            strcpy_s(pEventData->pstrStartMsg,strlen(strTmp)+1,strTmp);
         }
         else
            pEventData->pstrStartMsg = NULL;
         //Le stop Message
         sprintf_s(strTmp,512,"%s",get_string_E());
         pEventData->iStopMsg = strlen(strTmp);
         if(pEventData->iStopMsg >0)
         {
            pEventData->pstrStopMsg = new char[strlen(strTmp)+1];
            strcpy_s(pEventData->pstrStopMsg,strlen(strTmp)+1,strTmp);
         }
         else
            pEventData->pstrStopMsg = NULL;

         pEventData->iFlagID             = get_long_E();
         pEventData->iFlagStopValue      = get_long_E();
         pEventData->iLevelMin           = get_long_E();
         pEventData->iLevelMax           = get_long_E();

         //list des summon monster
         pEventData->ushNbrSummonMonster = get_word_E();
         if(pEventData->ushNbrSummonMonster >0)
         {
            pEventData->pMonsterList = new sSummonRequestM[pEventData->ushNbrSummonMonster];
            for(int n=0;n<pEventData->ushNbrSummonMonster;n++)
            {
               sprintf_s(strTmp,512,"%s",get_string_E());
               pEventData->pMonsterList[n].iSize = strlen(strTmp);
               if(pEventData->pMonsterList[n].iSize >0)
               {
                  pEventData->pMonsterList[n].pstrSN = new char[strlen(strTmp)+1];
                  strcpy_s(pEventData->pMonsterList[n].pstrSN,strlen(strTmp)+1,strTmp);
               }
               else
                  pEventData->pMonsterList[n].pstrSN = NULL;

               pEventData->pMonsterList[n].ushX   = get_word_E();
               pEventData->pMonsterList[n].ushY   = get_word_E();
               pEventData->pMonsterList[n].ushW   = get_word_E();
               pEventData->pMonsterList[n].ushQty = get_word_E();
            }
         }
         else
         {
            pEventData->pMonsterList = NULL;
         }

         //List des summon items
         pEventData->ushNbrSummonItem = get_word_E();
         if(pEventData->ushNbrSummonItem >0)
         {
            pEventData->pItemList = new sSummonRequestI[pEventData->ushNbrSummonItem];
            for(int n=0;n<pEventData->ushNbrSummonItem;n++)
            {
               pEventData->pItemList[n].uiID   = get_long_E();
               pEventData->pItemList[n].ushX   = get_word_E();
               pEventData->pItemList[n].ushY   = get_word_E();
               pEventData->pItemList[n].ushW   = get_word_E();
               pEventData->pItemList[n].ushQty = get_word_E();
            }
         }
         else
         {
            pEventData->pItemList = NULL;
         }

         c_aEvents.SetAtGrow(i, pEventData);

      }
   }
   printf( "\n  Found %d Event(s)",c_aEvents.GetSize() );

   if(pData)
      delete []pData;
   pData = NULL;


}


//////////////////////////////////////////////////////////////////////////////////////////
// Destroys skills
// 
//////////////////////////////////////////////////////////////////////////////////////////
void EventsMaster::Destroy( void )
{
   sEvents *lpEvent;
   int i;

   for(i = 0; i < c_aEvents.GetSize(); i++)
   {
      lpEvent = (sEvents *)c_aEvents.GetAt(i);
      if(lpEvent)
      {
         if(lpEvent->pstrName)
            delete []lpEvent->pstrName;
         lpEvent->pstrName = NULL;

         if(lpEvent->pstrStartMsg)
            delete []lpEvent->pstrStartMsg;
         lpEvent->pstrStartMsg = NULL;

         if(lpEvent->pstrStopMsg)
            delete []lpEvent->pstrStopMsg;
         lpEvent->pstrStopMsg = NULL;

         if(lpEvent->pMonsterList)
         {
            for(int m=0; m<lpEvent->ushNbrSummonMonster; m++)
            {
               if(lpEvent->pMonsterList[m].pstrSN)
                  delete []lpEvent->pMonsterList[m].pstrSN;
               lpEvent->pMonsterList[m].pstrSN = NULL;
            }
            delete []lpEvent->pMonsterList;
         }
         lpEvent->pMonsterList = NULL;

         if(lpEvent->pItemList)
            delete []lpEvent->pItemList;
         lpEvent->pItemList = NULL;

         delete lpEvent;
         lpEvent = NULL;
      }
   }
}

int EventsMaster::GetNbrEvent()
{
   return c_aEvents.GetSize();
}

int EventsMaster::GetEventFlagByIndex(int iIdx)
{
   if(iIdx <0 || iIdx >= GetNbrEvent())
      return 0;

   sEvents *lpEvent;
   lpEvent = (sEvents *)c_aEvents.GetAt(iIdx);
   return lpEvent->iFlagID;
}

void EventsMaster::SendEventList(Character *pUser)
{
   int iNbrE = GetNbrEvent();
   TFCPacket sending;
   sending << (RQ_SIZE)RQ_GetEventsList;
   sending << (long)iNbrE;
   sEvents *lpEvent;
   

   for(int i=0;i<iNbrE;i++)
   {
      lpEvent = (sEvents *)c_aEvents.GetAt(i);
      CString strName;
      strName.Format("%s",lpEvent->pstrName);
      sending << strName;
      sending << (long)lpEvent->iFlagID;
      sending << (long)lpEvent->iFlagStopValue;
      sending << (long)CheckGlobalFlag( lpEvent->iFlagID );
   }
   pUser->SendPlayerMessage(sending);
}


void EventsMaster::ManageEvents()
{
   #ifndef BUILD_NMS_CUSTOM_NPC
      return;
   #endif


   // 1: IOn  stop tous les EVENT a stopper...
   sEvents *lpEvent;
   for(int i=0;i<GetNbrEvent();i++)
   {
      lpEvent = (sEvents *)c_aEvents.GetAt(i);
      long lFlagValue = CheckGlobalFlag( lpEvent->iFlagID );
      if(lFlagValue >= lpEvent->iFlagStopValue)
      {
         //Kill tous les mob
         for(int m=0;m<lpEvent->ushNbrSummonMonster;m++)
         {
            DWORD dwID = Unit::GetIDFromName( lpEvent->pMonsterList[m].pstrSN, U_NPC, TRUE );
            NPCMain::GetInstance().KillAllID(dwID);
         }

         //Kill tous les NPC...
         for(int m=0;m<lpEvent->ushNbrSummonItem;m++)
         {
            KillObjectID(lpEvent->pItemList[m].uiID,lpEvent->pItemList[m].ushX,lpEvent->pItemList[m].ushY,lpEvent->pItemList[m].ushW);
         }

         //stop l'event
         GiveGlobalFlag( lpEvent->iFlagID,0);

         WorldPos wlPos = { 0, 0, 0 };
         if(strlen(lpEvent->pstrStopMsg) > 1)
            Broadcast::BCServerMessage( wlPos, 0, lpEvent->pstrStopMsg,NULL,CL_BLUE_LIGHT,true,lpEvent->iLevelMin,lpEvent->iLevelMax);


         _LOG_EVENTS
            LOG_ALWAYS,
            "End Events ID %d",lpEvent->ushID
            LOG_

      }
   }

   //2 on look si on dois starter de new events
   for(int i=0;i<GetNbrEvent();i++)
   {
      lpEvent = (sEvents *)c_aEvents.GetAt(i);

      long lFlagValue = CheckGlobalFlag( lpEvent->iFlagID );
      if(lFlagValue == 1)
      {
         //On start l evenement

         //Summon tous les MPB / NPC
         for(int m=0;m<lpEvent->ushNbrSummonMonster;m++)
         {
            SummonMonsterE(lpEvent->ushID,
               lpEvent->pMonsterList[m].pstrSN,
               lpEvent->pMonsterList[m].ushQty,
               lpEvent->pMonsterList[m].ushX,
               lpEvent->pMonsterList[m].ushY,
               lpEvent->pMonsterList[m].ushW);
         }

         //Summon tous les Items au sol
         for(int m=0;m<lpEvent->ushNbrSummonItem;m++)
         {
            SummonItemE(lpEvent->ushID,
               lpEvent->pItemList[m].uiID,
               lpEvent->pItemList[m].ushQty,
               lpEvent->pItemList[m].ushX,
               lpEvent->pItemList[m].ushY,
               lpEvent->pItemList[m].ushW);
         }

         WorldPos wlPos = { 0, 0, 0 };
         if(strlen(lpEvent->pstrStartMsg) > 1)
            Broadcast::BCServerMessage( wlPos, 0, lpEvent->pstrStartMsg,NULL,CL_BLUE_LIGHT,true,lpEvent->iLevelMin,lpEvent->iLevelMax);

         //set le flag a 2 pour dire que c<est en cours
         GiveGlobalFlag( lpEvent->iFlagID,2);
         _LOG_EVENTS
            LOG_ALWAYS,
            "Start Events ID %d",lpEvent->ushID
            LOG_
      }
   }

   return;
}

void EventsMaster::SummonMonsterE(int iEventID, CString strMID, int iQty, int iX, int iY, int iW)
{
   DWORD dwID = Unit::GetIDFromName( strMID, U_NPC, TRUE );
   if(dwID == 0)
   {
      _LOG_EVENTS
         LOG_ALWAYS,
         "ERROR on Event ID %d, Unable to summon monster ID Events ID %s",iEventID,strMID
         LOG_
   }
   else
   {
      int iRayon = iQty;
      if(iQty>100)
         iRayon = iQty/5;
      else if(iQty>10)
         iRayon = iQty/2;
      if(iRayon <=0)
         iRayon = 1;


      WorldPos wlPos = { iX, iY, iW };
      WorldMap *wlWorld = TFCMAIN::GetWorld( wlPos.world );	

      if( wlPos.X != 0 && wlPos.Y != 0 && wlWorld != NULL)
      {
         WorldPos wlSPos;
         wlSPos.world = wlPos.world;
         for (int j = 0; j < iQty; j++)
         {
            wlSPos.X = wlPos.X + (rand()%iRayon) - iRayon/2;
            wlSPos.Y = wlPos.Y + (rand()%iRayon) - iRayon/2;
            if( wlWorld->IsValidPosition( wlPos ) )
            {
               Creatures *lpMonster = new Creatures;
               if( lpMonster->Create( U_NPC, dwID ) )
               {
                  lpMonster->SetDestination( wlSPos );
                  lpMonster->Do( wandering ,"SummonMonsterE");
                  lpMonster->SetWL( wlSPos );
                  if( !wlWorld->SummonMonster( lpMonster, TRUE ) )
                  {
                     lpMonster->DeleteUnit();
                  }
               }
               else
               {
                  lpMonster->DeleteUnit();
               }
            }
            else
            {
               //Nothing on avertie pas pour rien que la pos ets invalide,...
            }
         }
      }
      else
      {
         _LOG_EVENTS
            LOG_ALWAYS,
            "ERROR on Event ID %d, Unable to summon monster ID %s at position %d,%d,%d",iEventID,strMID,iX,iY,iW
            LOG_
      }
   }
}

void EventsMaster::SummonItemE(int iEventID, DWORD dwID, int iQty, int iX, int iY, int iW)
{
   if(dwID == 0)
   {
      _LOG_EVENTS
         LOG_ALWAYS,
         "ERROR on Event ID %d, Unable to summon item ID %d",iEventID,dwID
         LOG_
   }
   else
   {
      BOOL bError = TRUE;

      int iRayon = iQty;
      if(iQty>100)
         iRayon = iQty/5;
      else if(iQty>10)
         iRayon = iQty/2;
      if(iRayon <=0)
         iRayon = 1;


      WorldPos wlPos = { iX, iY, iW };
      WorldMap *wlWorld = TFCMAIN::GetWorld( wlPos.world );	

      if( wlPos.X != 0 && wlPos.Y != 0 && wlWorld != NULL)
      {
         WorldPos wlSPos;
         wlSPos.world = wlPos.world;
         for (int j = 0; j < iQty; j++)
         {
            bError = TRUE;

            wlSPos.X = wlPos.X + (rand()%iRayon) - iRayon/2;
            wlSPos.Y = wlPos.Y + (rand()%iRayon) - iRayon/2;

            Objects *lpItem = new Objects;
            if( lpItem->Create( U_OBJECT, dwID ) )
            {  
               _item *item = NULL;
               // Get the item structure.
               lpItem->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &item );

               if( item->canSummon)
               {                
                  WorldMap *world = TFCMAIN::GetWorld( wlPos.world );
                  if( world != NULL && world->IsValidPosition( wlSPos ) )
                  {
                     world->deposit_unit( wlSPos, lpItem );
                     lpItem->BroadcastPopup( wlSPos, true );
                     bError = FALSE;
                  }
                  else
                     lpItem->DeleteUnit();
               }
               else
                  lpItem->DeleteUnit();
            }
            else
               lpItem->DeleteUnit();

            if(bError)
            {
               _LOG_EVENTS
                  LOG_ALWAYS,
                  "ERROR on Event ID %d, Unable to summon item ID %d at position %d,%d,%d",iEventID,dwID,iX,iY,iW
                  LOG_
            }
         }
      }
      else
      {
         _LOG_EVENTS
            LOG_ALWAYS,
            "ERROR on Event ID %d, Unable to summon item ID %d at position %d,%d,%d",iEventID,dwID,iX,iY,iW
            LOG_
      }
   }
}

void EventsMaster::KillObjectID(DWORD dwIDItem, int iX, int iY, int iW)
{
   WorldPos wlPos = { iX, iY, iW };
   WorldMap *world = TFCMAIN::GetWorld( iW);
   if(world)
   {
      DWORD dwList[500];
      WorldPos wlID[500];
      int iXS = iX-50;
      if(iXS < 0)
         iXS = 0;
      int iXE = iX+50;
      if(iXE >3071)
         iXE = 3071;
      int iYS = iY-50;
      if(iYS < 0)
         iYS = 0;
      int iYE = iY+50;
      if(iYE >3071)
         iYE = 3071;

      DWORD dwNbItem = world->GetZoneObjectsUnits(iXS,iYS,iXE,iYE,500,dwList,wlID);

      DWORD dwID = 0;
      WorldPos wlPos = { 0, 0, 0 };
      Unit *lpuGet;

      for(int ii=0;ii<dwNbItem;ii++)
      {
         wlPos.X  = 	wlID[ii].X;			
         wlPos.Y  = 	wlID[ii].Y;
         dwID     =  dwList[ii];

         lpuGet = world->FindNearUnit( wlPos, dwID );

         if( lpuGet && lpuGet->GetType() == U_OBJECT && lpuGet->GetStaticReference() == dwIDItem)
         {
            lpuGet->VaporizeUnit(false);
         }
      }
   }
}







unsigned int get_dword_E()
{
   unsigned int val;

   val=*(unsigned int *)g_pDataTmpE;	g_pDataTmpE+=4;
   return val;
}

unsigned short get_word_E()
{
   unsigned short val;

   val=*(unsigned short *)g_pDataTmpE;	g_pDataTmpE+=2;
   return val;
}

BYTE get_byte_E()
{
   unsigned char val;

   val=*g_pDataTmpE;	g_pDataTmpE+=1;
   return val;
}

double get_double_E()
{
   double val;

   val=*(double *)g_pDataTmpE;	g_pDataTmpE+=8;
   return val;
}

int get_long_E()
{
   int val;

   val=*(int *)g_pDataTmpE;	g_pDataTmpE+=4;
   return val;
}

short get_short_E()
{
   short val;

   val=*(short *)g_pDataTmpE;	g_pDataTmpE+=2;
   return val;
}

char* get_string_E()
{
   int lg,i;

   lg=*(int *)g_pDataTmpE;	g_pDataTmpE+=4;
   for(i=0; i<lg; i++)
      g_chLigneE[i]=*g_pDataTmpE++;

   g_chLigneE[i]=0;

   return (char *)g_chLigneE;
}
