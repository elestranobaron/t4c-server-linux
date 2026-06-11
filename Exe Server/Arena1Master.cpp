/* Note to self: 
 *   This is possibly the reason for the Deadlocks:
 *     CPlayerManager::GetCharacterRessourceByID();
 */

#include "stdafx.h"
#include "TFC Server.h"
#include "TFC_MAIN.h"
#include "Arena1Master.h"
#include "IntlText.h"
#include "Unit.h"
#include "PlayerManager.h"
#include "QuestFlagsListing.h"

#include "T4CLog.h"
#include "DeadlockDetector.h"
#ifdef _WIN32
#include <mmsystem.h>
#endif




#define ARENA1_END_NONE           0
#define ARENA1_END_TIMEOUT        1
#define ARENA1_END_COMPLETED      2
#define ARENA1_END_NO_PLAYER      3
#define ARENA1_END_BALANCE_PLAYER 4


extern CTFCServerApp theApp;

int              Arena1Master::m_iNbrArena;
sArenaFlagGame  *Arena1Master::m_pArenaList = NULL;



static CLock     g_Arena1Lock;




//////////////////////////////////////////////////////////////////////////////////////////
// Creates the class
//////////////////////////////////////////////////////////////////////////////////////////
void Arena1Master::Create(int iNbrArena)
{
   m_iNbrArena = iNbrArena;
   if(m_iNbrArena <=0)
      return;

   m_pArenaList = new sArenaFlagGame[m_iNbrArena];
   for(int i=0;i<m_iNbrArena;i++)
   {
      m_pArenaList[i].c_AreneStatus = 0;
      m_pArenaList[i].c_iLevelMin   = 0;
      m_pArenaList[i].c_iLevelMax   = 0;
      m_pArenaList[i].c_dwStartTime = 0;
      m_pArenaList[i].c_PointBTeam  = 0;
      m_pArenaList[i].c_PointRTeam  = 0;
      m_pArenaList[i].c_FlagBInHome = 1;
      m_pArenaList[i].c_FlagRInHome = 1;
      m_pArenaList[i].c_iLastNbrSec = 0;
      m_pArenaList[i].c_iLastNbrMin = 0;

      m_pArenaList[i].c_itArene = theApp.CombatArenaLocationList1.begin();
      for(int it=0;it<i;it++)
         m_pArenaList[i].c_itArene++;

      
      m_pArenaList[i].c_ArenaCfg.iNbrMinPlayer         = (*m_pArenaList[i].c_itArene).iSettingsMinPlayer;
      m_pArenaList[i].c_ArenaCfg.iNbrMaxPlayer         = (*m_pArenaList[i].c_itArene).iSettingsMaxPlayer; 
      m_pArenaList[i].c_ArenaCfg.iNbrMaxPoint          = (*m_pArenaList[i].c_itArene).iSettingsMaxPoint;  
      m_pArenaList[i].c_ArenaCfg.iNbrMaxMinute         = (*m_pArenaList[i].c_itArene).iSettingsMinuteMax;
      m_pArenaList[i].c_ArenaCfg.iNbrStartWaitTimeSec  = (*m_pArenaList[i].c_itArene).iSettingsWaitStartSec;
      m_pArenaList[i].c_ArenaCfg.iNbrPlayWaitTimeSec   = m_pArenaList[i].c_ArenaCfg.iNbrMaxMinute*60; 
      m_pArenaList[i].c_ArenaCfg.iNbrPlayWaitDeathSec  = (*m_pArenaList[i].c_itArene).iSettingsWaitDeathSec; 
      m_pArenaList[i].c_ArenaCfg.iNbrPlayWaitDeathMSec = m_pArenaList[i].c_ArenaCfg.iNbrPlayWaitDeathSec*60; 
      m_pArenaList[i].c_ArenaCfg.iNbrPlayTakeItemSec   = (*m_pArenaList[i].c_itArene).iSettingsTakeItemSec; 
   }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Destroys skills
// 
//////////////////////////////////////////////////////////////////////////////////////////
void Arena1Master::Destroy( void )
{
   if(m_iNbrArena <=0)
      return;

   delete []m_pArenaList;
   m_pArenaList = NULL;

}


void Arena1Master::ManageArena()
{
   CString strTmp;
   CAutoLock autoArena1Lock( &g_Arena1Lock );


   for(int a=0;a<m_iNbrArena;a++)
   {
      if(m_pArenaList[a].c_AreneStatus == 0)
      {
         //Do nothing pas de starter...
         m_pArenaList[a].c_iLastNbrSec = 0;
         m_pArenaList[a].c_iLastNbrMin = 0;
      }
      else if(m_pArenaList[a].c_AreneStatus == 1)
      {
         //Do nothing pas de starter...
         //en mode attente... on update le timer a tous les player deja isncrit...
         DWORD dwElapsed = timeGetTime()-m_pArenaList[a].c_dwStartTime;

         int iNbrSecC = dwElapsed/1000;
         int iNbrSecT = m_pArenaList[a].c_ArenaCfg.iNbrStartWaitTimeSec; 
         int iNbrSecR = iNbrSecT-iNbrSecC;    //10 min - nbr temps ecouler
         short shNbrMin  = iNbrSecR/60;
         short shNbrSec  = iNbrSecR-(shNbrMin*60);
         m_pArenaList[a].c_iLastNbrSec = shNbrSec;
         m_pArenaList[a].c_iLastNbrMin = shNbrMin;

         sArenaPlayerList *lpAR;
         for(int i=0;i<m_pArenaList[a].c_aPlayerList.GetSize();i++)
         {
            lpAR = (sArenaPlayerList *)m_pArenaList[a].c_aPlayerList.GetAt(i);
            TFCPacket sending;
            sending << (RQ_SIZE)RQ_ARENA1_UpdateTimeBS;
            sending << (long)a;
            sending << (char)1;
            sending << (long)ARENE1_TYPE;
            sending << (short)shNbrMin;
            sending << (short)shNbrSec;
            lpAR->pPlayer->self->SendPlayerMessage(sending);

            //Reset Arena Points
            lpAR->pPlayer->self->SetArenaPVP(1.00);
            lpAR->pPlayer->self->ResetArenaINACTIFStart();
            lpAR->pPlayer->self->ResetArenaDUREE();
            lpAR->pPlayer->self->SetArenaPOINTS(0);
         }

         if(iNbrSecC >  m_pArenaList[a].c_ArenaCfg.iNbrStartWaitTimeSec)
         {
            //////////////////////////////////////////////////////
            //0 valide si ay assez de joueur...
            //////////////////////////////////////////////////////
            if(m_pArenaList[a].c_aPlayerList.GetSize() < m_pArenaList[a].c_ArenaCfg.iNbrMinPlayer)
            {
               sArenaPlayerList *lpAR;
               while(m_pArenaList[a].c_aPlayerList.GetSize())
               {
                  lpAR = (sArenaPlayerList *)m_pArenaList[a].c_aPlayerList.GetAt(0);
                  TFCPacket sending;
                  sending << (RQ_SIZE)RQ_ARENA1_UpdateTimeBS;
                  sending << (long)a;
                  sending << (char)2;
                  sending << (long)ARENE1_TYPE;
                  sending << (short)0;
                  sending << (short)0;
                  sending << (short)0;
                  sending << (short)0;
                  lpAR->pPlayer->self->SendPlayerMessage(sending);
                  strTmp.Format(_STR(15485, lpAR->pPlayer->self->GetLang()),(*m_pArenaList[a].c_itArene).strZOneName);
                  lpAR->pPlayer->self->SendInfoMessage(strTmp,CL_YELLOW);
                  ResetPlayerArene(lpAR->pPlayer,a);

                  delete lpAR;
                  lpAR = NULL;
                  m_pArenaList[a].c_aPlayerList.RemoveAt(0);
               }
               m_pArenaList[a].c_AreneStatus = 0;
               m_pArenaList[a].c_iLevelMin   = 0;
               m_pArenaList[a].c_iLevelMax   = 0;
               m_pArenaList[a].c_dwStartTime = 0;
               m_pArenaList[a].c_PointBTeam  = 0;
               m_pArenaList[a].c_PointRTeam  = 0;

               return;
            }



            //////////////////////////////////////////////////////
            //1- On cree les equipe virtuel...
            //////////////////////////////////////////////////////
            int iNbrPlayer = m_pArenaList[a].c_aPlayerList.GetSize();
            int *pNumbers = new int[m_pArenaList[a].c_ArenaCfg.iNbrMaxPlayer];  
            int i, n, tmp;   

            srand(time(NULL));   
            // Initialize the array  
            for(i = 0;i < iNbrPlayer;++i)    
               pNumbers[i] = i;   
            // Shuffle the array  
            for(i = 0;i < iNbrPlayer;++i)  
            {
               n = rand() % iNbrPlayer;   
               tmp = pNumbers[n];    
               pNumbers[n] = pNumbers[i];    
               pNumbers[i] = tmp;  
            } 

            int iNbrPlTeam = iNbrPlayer/2;

            int p=0;
            int iPlB = 0;
            Group *pGroubB = NULL;
            for(p=0;p<iNbrPlTeam;p++)
            {
               lpAR = (sArenaPlayerList *)m_pArenaList[a].c_aPlayerList.GetAt(pNumbers[p]);
               lpAR->pPlayer->self->SetArenaTeam(1);
               if(iPlB == 0) //Create real group
               {
                  if(lpAR->pPlayer->self->GetGroup()!= NULL)
                  {
                     lpAR->pPlayer->self->GetGroup()->Dismiss( lpAR->pPlayer->self ); 
                     lpAR->pPlayer->self->SetGroup(NULL);
                  }
                  pGroubB = Group::CreateGroupArene(lpAR->pPlayer->self);
               }
               else//Add member to group
               {
                  if(lpAR->pPlayer->self->GetGroup()!= NULL)
                  {
                     lpAR->pPlayer->self->GetGroup()->Dismiss( lpAR->pPlayer->self ); 
                     lpAR->pPlayer->self->SetGroup(NULL);
                  }
                  pGroubB->AddGroupPlayerArene(lpAR->pPlayer->self);
               }
               iPlB++;
            }


            int iPlR = 0;
            Group *pGroubR = NULL;
            for(p=iNbrPlTeam;p<m_pArenaList[a].c_aPlayerList.GetSize();p++)
            {
               lpAR = (sArenaPlayerList *)m_pArenaList[a].c_aPlayerList.GetAt(pNumbers[p]);
               lpAR->pPlayer->self->SetArenaTeam(2);
               if(iPlR == 0) //Create real group
                  pGroubR = Group::CreateGroupArene(lpAR->pPlayer->self);
               else//Add member to group
                  pGroubR->AddGroupPlayerArene(lpAR->pPlayer->self);
               iPlR++;
            }
            if(pNumbers)
               delete []pNumbers;
            pNumbers = NULL;

            //////////////////////////////////////////////////////
            //2- Clean la map de tout ses objets
            //////////////////////////////////////////////////////
            WorldMap *world = TFCMAIN::GetWorld( (*m_pArenaList[a].c_itArene).wlTopLeft.world);
            if(world)
            {
               DWORD dwList[500];
               WorldPos wlID[500];
               DWORD dwNbItem = world->GetZoneObjectsUnits((*m_pArenaList[a].c_itArene).wlTopLeft.X,(*m_pArenaList[a].c_itArene).wlTopLeft.Y,
                                                           (*m_pArenaList[a].c_itArene).wlBottomRight.X,(*m_pArenaList[a].c_itArene).wlBottomRight.Y,500,dwList,wlID);

               DWORD dwID = 0;
               WorldPos wlPos = { 0, 0, 0 };
               Unit *lpuGet;

               for(int ii=0;ii<dwNbItem;ii++)
               {
                  wlPos.X  = 	wlID[ii].X;			
                  wlPos.Y  = 	wlID[ii].Y;
                  dwID     =  dwList[ii];

                  lpuGet = world->FindNearUnit( wlPos, dwID );
                  if( lpuGet && lpuGet->GetType() == U_OBJECT )
                  {
                     lpuGet->VaporizeUnit(false);
                  }
               }
            }

            //////////////////////////////////////////////////////
            //3- Summon le drapeau du jeux...
            //////////////////////////////////////////////////////
            SummonFlagRnd(a);

            //////////////////////////////////////////////////////
            //4- Summon le drapeau du jeux...
            //////////////////////////////////////////////////////
            CString strPlTmp,strEquipB,strEquipR;
            int iR = 0;
            int iB = 0;
            sArenaPlayerList *lpAR;
            for(int i=0;i<m_pArenaList[a].c_aPlayerList.GetSize();i++)
            {
               lpAR = (sArenaPlayerList *)m_pArenaList[a].c_aPlayerList.GetAt(i);
               strPlTmp = lpAR->pPlayer->self->GetTrueName();
               if(lpAR->pPlayer->self->GetArenaTeam() == 1)
               {
                  if(iB > 0)
                     strEquipB += ", ";
                  strEquipB += strPlTmp;
                  iB++;
               }
               else if(lpAR->pPlayer->self->GetArenaTeam() == 2)
               {
                  if(iR > 0)
                     strEquipR += ", ";
                  strEquipR += strPlTmp;
                  iR++;
               }
            }


            strTmp.Format(_STR(15492, IntlText::GetDefaultLng()),strEquipB,strEquipR);
            CString strCC;
            strCC = "PVP";
            ChatterChannels &cChatter = CPlayerManager::GetChatter();
            cChatter.TalkSystem( "Système", (char*)strCC.GetBuffer(0), (char*)strTmp.GetBuffer(0) );

            //////////////////////////////////////////////////////
            //5- On FAKE group les gens de la meme equipe et TP les pj dans leur zone respective
            //////////////////////////////////////////////////////
            sArenaPlayerList *lpPL;
            for(p=0;p<m_pArenaList[a].c_aPlayerList.GetSize();p++)
            {
               lpPL = (sArenaPlayerList *)m_pArenaList[a].c_aPlayerList.GetAt(p);
               SendPlayerFakeGroup(lpPL->pPlayer,a);

               lpPL->wlOldLoc = lpPL->pPlayer->self->GetWL();
               //teleport le user a la bonne position
               if(lpPL->pPlayer->self->GetArenaTeam() == 1)
                  lpPL->pPlayer->self->Teleport( (*m_pArenaList[a].c_itArene).wlRecallBlueStart, 1 ,TRUE);	
               else if(lpPL->pPlayer->self->GetArenaTeam() == 2)
                  lpPL->pPlayer->self->Teleport( (*m_pArenaList[a].c_itArene).wlRecallRedStart, 1 ,TRUE);	
               lpPL->pPlayer->self->DealExhaust(0,0,1000);
               
               strTmp.Format(_STR(15478, lpPL->pPlayer->self->GetLang()),(*m_pArenaList[a].c_itArene).strZOneName,m_pArenaList[a].c_ArenaCfg.iNbrMaxPoint,m_pArenaList[a].c_ArenaCfg.iNbrMaxMinute);
               lpPL->pPlayer->self->SendInfoMessage(strTmp,CL_YELLOW);
            }

            _LOG_ARENA
               LOG_ALWAYS,
               "ARENE1 ID %d: START Game Now",a
               LOG_
            
            m_pArenaList[a].c_dwStartTime = timeGetTime();
            m_pArenaList[a].c_AreneStatus = 2;
         }
      }
      else if(m_pArenaList[a].c_AreneStatus == 2)
      {
         UINT uiEndGame = ARENA1_END_NONE;
         int iNbrPlayerB = 0;
         int iNbrPlayerR = 0;
         DWORD dwElapsed = timeGetTime()-m_pArenaList[a].c_dwStartTime;
         int iNbrSecC = dwElapsed/1000;
         int iNbrSecT = m_pArenaList[a].c_ArenaCfg.iNbrPlayWaitTimeSec; 
         int iNbrSecR = iNbrSecT-iNbrSecC;    //10 min - nbr temps ecouler
         short shNbrMin  = iNbrSecR/60;
         short shNbrSec  = iNbrSecR-(shNbrMin*60);
         m_pArenaList[a].c_iLastNbrSec = shNbrSec;
         m_pArenaList[a].c_iLastNbrMin = shNbrMin;

         sArenaPlayerList *lpAR;
         for(int i=0;i<m_pArenaList[a].c_aPlayerList.GetSize();i++)
         {
            lpAR = (sArenaPlayerList *)m_pArenaList[a].c_aPlayerList.GetAt(i);
            if(lpAR->pPlayer->self->GetArenaTeam() == 1)
               iNbrPlayerB++;
            else if(lpAR->pPlayer->self->GetArenaTeam() == 2)
               iNbrPlayerR++;

            if(lpAR->pPlayer->self->ViewFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG) ==0)
               lpAR->pPlayer->self->AddArenaINACTIF();
            lpAR->pPlayer->self->AddArenaDUREE();
         }

         //valide le take item...
         BOOL bFlagTaked = FALSE;
         sArenaTakeItemList *lpIT;
         for(int i=0;i<m_pArenaList[a].c_aTakeFlagList1.GetSize();i++)
         {
            lpIT = (sArenaTakeItemList *)m_pArenaList[a].c_aTakeFlagList1.GetAt(i);
            lpIT->uiDecompte--;
            if(lpIT->uiDecompte == 0)
            {
               //prend l'item de l'ARENE
               //on vaporize cet items et set le flag a ce user pour dire quil a pris le flag...
               lpIT->pItemUnit->VaporizeUnit(false);
               lpIT->pPlayer->self->SetFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG,1);
               if(lpIT->pPlayer->self->GetArenaTeam() == 1) //Set blue tag
                  lpIT->pPlayer->self->SetFlag(__FLAG_NMS_TAG_DISPLAY_OVER_HEAD,4);
               else if(lpIT->pPlayer->self->GetArenaTeam() == 2) //Set red tag
                  lpIT->pPlayer->self->SetFlag(__FLAG_NMS_TAG_DISPLAY_OVER_HEAD,4);

               TFCPacket sending;
               lpIT->pPlayer->self->PacketPuppetInfo( sending );
               Broadcast::BCast( lpIT->pPlayer->self->GetWL(), _DEFAULT_RANGE, sending, lpIT->pPlayer->self->GetInvisibleQuery() );

               CString strTmp;
               if(lpIT->pPlayer->self->GetArenaTeam() == 1)
               {
                  strTmp.Format(_STR( 15486, lpIT->pPlayer->self->GetLang() ),lpIT->pPlayer->self->GetTrueName());
                  SendMessageToAll(strTmp,CL_BLUE_2,a);
               }
               else
               {
                  strTmp.Format(_STR( 15487, lpIT->pPlayer->self->GetLang() ),lpIT->pPlayer->self->GetTrueName());
                  SendMessageToAll(strTmp,CL_RED,a);
               }

               //Prend drapeau
               lpIT->pPlayer->self->AddArenaPOINTS(10,"Take Flag");//(NM:Regle 3)
               lpIT->pPlayer->self->ResetArenaINACTIF();

               bFlagTaked = TRUE;
               SendFlagTakeStatus(lpIT->pPlayer,2,m_pArenaList[a].c_ArenaCfg.iNbrPlayTakeItemSec,a,true);

               delete lpIT;
               lpIT = NULL;
               m_pArenaList[a].c_aTakeFlagList1.RemoveAt(i);
               i = m_pArenaList[a].c_aTakeFlagList1.GetSize(); //sort de la boucle...
            }
            else
            {
               SendFlagTakeStatus(lpIT->pPlayer,0,lpIT->uiDecompte,a);
            }
         }

         if(bFlagTaked)
         {
            //remove all flag waiting
            while(m_pArenaList[a].c_aTakeFlagList1.GetSize())
            {
               lpIT = (sArenaTakeItemList *)m_pArenaList[a].c_aTakeFlagList1.GetAt(0);
               SendFlagTakeStatus(lpIT->pPlayer,1,m_pArenaList[a].c_ArenaCfg.iNbrPlayTakeItemSec,a,true);
               delete lpIT;
               lpIT = NULL;
               m_pArenaList[a].c_aTakeFlagList1.RemoveAt(0);
            }
         }




         //si fin de partie plus de TEMPS...
         if(iNbrSecC > m_pArenaList[a].c_ArenaCfg.iNbrPlayWaitTimeSec)
         {
            // on defait le groupe,
            // on tp les user a leur position de recall
            // On call un display des stat a tous les joueur present dans le RP
            // on reset les a user au pour setter plus en arene
            // on reset cet arene...

            uiEndGame = ARENA1_END_TIMEOUT;

         }
         else if(m_pArenaList[a].c_PointBTeam >= m_pArenaList[a].c_ArenaCfg.iNbrMaxPoint|| m_pArenaList[a].c_PointRTeam >= m_pArenaList[a].c_ArenaCfg.iNbrMaxPoint)
         {
            //Nous avons des gagnant.Create..
            uiEndGame = ARENA1_END_COMPLETED;
         }
         else
         {
            //le jeux se passe on fais rien...
            //trouve le nombre de player par equipe... et valide si le jeux ets toujours OK
            //regle 1: difference de joueur > 2 END
            //regle 2: une des 2 equipe a 0 joueur END
            if(m_pArenaList[a].c_ArenaCfg.iNbrMinPlayer > 1)
            {
               if(iNbrPlayerB == 0 || iNbrPlayerR == 0)
               {
                  uiEndGame = ARENA1_END_NO_PLAYER;
               }
            }
            else
            {
               if(iNbrPlayerB == 0 && iNbrPlayerR == 0)
               {
                  uiEndGame = ARENA1_END_NO_PLAYER;
               }
            }
            if(iNbrPlayerB > iNbrPlayerR && iNbrPlayerB-iNbrPlayerR >2)
            {
               uiEndGame = ARENA1_END_BALANCE_PLAYER;
            }
            if(iNbrPlayerR > iNbrPlayerB && iNbrPlayerR-iNbrPlayerB >2)
            {
               uiEndGame = ARENA1_END_BALANCE_PLAYER;
            }

         }

         if(uiEndGame == ARENA1_END_NONE)
         {
            sArenaPlayerList *lpAR;
            for(int i=0;i<m_pArenaList[a].c_aPlayerList.GetSize();i++)
            {
               lpAR = (sArenaPlayerList *)m_pArenaList[a].c_aPlayerList.GetAt(i);
               if(lpAR->pPlayer->self->GetArenaTeam() == 1)
                  iNbrPlayerB++;
               else if(lpAR->pPlayer->self->GetArenaTeam() == 2)
                  iNbrPlayerR++;
               TFCPacket sending;
               sending << (RQ_SIZE)RQ_ARENA1_UpdateTimeBS;
               sending << (long)a;
               sending << (char)2;
               sending << (long)ARENE1_TYPE;
               sending << (short)shNbrMin;
               sending << (short)shNbrSec;
               sending << (char)m_pArenaList[a].c_PointBTeam; // score team bleue
               sending << (char)m_pArenaList[a].c_PointRTeam; // score team red
               lpAR->pPlayer->self->SendPlayerMessage(sending);
            }
         }
         else
         {
            //Cumul des point par joueur et donne les point equipe...


            if(uiEndGame == ARENA1_END_COMPLETED || uiEndGame == ARENA1_END_TIMEOUT)
            {
               CString strPlTmp,strEquipB,strEquipR;
               int iR = 0;
               int iB = 0;
               sArenaPlayerList *lpAR;
               for(int i=0;i<m_pArenaList[a].c_aPlayerList.GetSize();i++)
               {
                  lpAR = (sArenaPlayerList *)m_pArenaList[a].c_aPlayerList.GetAt(i);
                  strPlTmp = lpAR->pPlayer->self->GetTrueName();
                  if(lpAR->pPlayer->self->GetArenaTeam() == 1)
                  {
                     if(m_pArenaList[a].c_PointBTeam > m_pArenaList[a].c_PointRTeam)
                        lpAR->pPlayer->self->AddArenaPOINTS(30,"Team WIN"); //(NM:Regle 9)
                     lpAR->pPlayer->self->AddArenaPOINTS(10*m_pArenaList[a].c_PointBTeam,"Team Score bonus"); //(NM:Regle 8)

                     if(iB > 0)
                        strEquipB += ", ";
                     strEquipB += strPlTmp;
                     iB++;
                  }
                  else if(lpAR->pPlayer->self->GetArenaTeam() == 2)
                  {
                     if(m_pArenaList[a].c_PointRTeam > m_pArenaList[a].c_PointBTeam)
                        lpAR->pPlayer->self->AddArenaPOINTS(30,"Team WIN"); //(NM:Regle 9)
                     lpAR->pPlayer->self->AddArenaPOINTS(10*m_pArenaList[a].c_PointRTeam,"Team Score bonus"); //(NM:Regle 8)

                     if(iR > 0)
                        strEquipR += ", ";
                        strEquipR += strPlTmp;
                     iR++;
                  }

                  //cumule les point actif / inactif
                  lpAR->pPlayer->self->AddArenaPOINTS(-1*lpAR->pPlayer->self->GetArenaINACTIF(),"Inactif points"); //(NM:Regle 7)
                  lpAR->pPlayer->self->AddArenaPOINTS(1*(lpAR->pPlayer->self->GetArenaDUREE()-lpAR->pPlayer->self->GetArenaINACTIF()),"Actif points"); //(NM:Regle 6)
               }

               //calcule les vrai point des joueurs selon penalty ou anti afk ou hack
               int iNbrT = 0;
               int iNbrI = 0;
               for(int i=0;i<m_pArenaList[a].c_aPlayerList.GetSize();i++)
               {
                  lpAR = (sArenaPlayerList *)m_pArenaList[a].c_aPlayerList.GetAt(i);
                  if(lpAR->pPlayer->self->GetArenaINACTIF() > (int)(lpAR->pPlayer->self->GetArenaDUREE()*75.00/100.00))
                  {
                     iNbrI++;
                     lpAR->pPlayer->self->ResetArenaPOINTS("Inactif > 75 pourcent we reset points");
                  }
                  iNbrT++;
               }

               for(int i=0;i<m_pArenaList[a].c_aPlayerList.GetSize();i++)
               {
                  lpAR = (sArenaPlayerList *)m_pArenaList[a].c_aPlayerList.GetAt(i);
                  if(iNbrI > iNbrT/2)
                  {
                     lpAR->pPlayer->self->ResetArenaPOINTS("More than 50 pourcent of player Inactif we reset points");
                  }
                  if(lpAR->pPlayer->self->GetArenaPOINTS() > 0)
                  {
                     lpAR->pPlayer->self->SetFlag(__FLAG_POINTS_RP_XP_EVENTS      ,lpAR->pPlayer->self->ViewFlag(__FLAG_POINTS_RP_XP_EVENTS)      +lpAR->pPlayer->self->GetArenaPOINTS());
                     lpAR->pPlayer->self->SetFlag(__FLAG_POINTS_RP_XP_EVENTS_TOTAL,lpAR->pPlayer->self->ViewFlag(__FLAG_POINTS_RP_XP_EVENTS_TOTAL)+lpAR->pPlayer->self->GetArenaPOINTS());
                  }
                  else
                  {
                     lpAR->pPlayer->self->SetArenaPOINTS(0);
                  }
                  SendArenaPlayStat(lpAR->pPlayer,true,a);
               }


               if(m_pArenaList[a].c_PointBTeam > m_pArenaList[a].c_PointRTeam)
               {
                  strTmp.Format(_STR(15483, IntlText::GetDefaultLng()),strEquipB,strEquipR,m_pArenaList[a].c_PointBTeam,m_pArenaList[a].c_PointRTeam);
               }
               else if(m_pArenaList[a].c_PointRTeam > m_pArenaList[a].c_PointBTeam)
               {
                  strTmp.Format(_STR(15484, IntlText::GetDefaultLng()),strEquipR,strEquipB,m_pArenaList[a].c_PointRTeam,m_pArenaList[a].c_PointBTeam);
               }
               else if(m_pArenaList[a].c_PointRTeam == m_pArenaList[a].c_PointBTeam)
               {
                  strTmp.Format(_STR(15481, IntlText::GetDefaultLng()),strEquipB,strEquipR,m_pArenaList[a].c_PointBTeam,m_pArenaList[a].c_PointRTeam);
               }
               
               CString strCC;
               strCC = "PVP";
               ChatterChannels &cChatter = CPlayerManager::GetChatter();
               cChatter.TalkSystem( "Système", (char*)strCC.GetBuffer(0), (char*)strTmp.GetBuffer(0) );
            }



            sArenaPlayerList *lpAR;
            while(m_pArenaList[a].c_aPlayerList.GetSize())
            {
               lpAR = (sArenaPlayerList *)m_pArenaList[a].c_aPlayerList.GetAt(0);
               lpAR->pPlayer->self->Teleport(lpAR->wlOldLoc,1,TRUE);


               TFCPacket sending;
               sending << (RQ_SIZE)RQ_ARENA1_UpdateTimeBS;
               sending << (long)a;
               sending << (char)2;
               sending << (long)ARENE1_TYPE;
               sending << (short)0;
               sending << (short)0;
               sending << (char)0;
               sending << (char)0;
               lpAR->pPlayer->self->SendPlayerMessage(sending);
               
               //remove 
               if( lpAR->pPlayer->self->GetGroup() != NULL )
               {
                  lpAR->pPlayer->self->GetGroup()->DismissArene( lpAR->pPlayer->self ); //Dismiss normal car pas en arene...
                  lpAR->pPlayer->self->SetGroup(NULL);
               }
              
               if(uiEndGame == ARENA1_END_TIMEOUT)
               {
                  strTmp.Format(_STR(15482, lpAR->pPlayer->self->GetLang()),(*m_pArenaList[a].c_itArene).strZOneName);
               }
               else if(uiEndGame == ARENA1_END_COMPLETED)
               {
                  strTmp.Format(_STR(15482, lpAR->pPlayer->self->GetLang()),(*m_pArenaList[a].c_itArene).strZOneName);
               }
               else if(uiEndGame == ARENA1_END_NO_PLAYER)
               {
                  strTmp.Format(_STR(15479, lpAR->pPlayer->self->GetLang()),(*m_pArenaList[a].c_itArene).strZOneName);
               }
               else if(uiEndGame == ARENA1_END_BALANCE_PLAYER)
               {
                  strTmp.Format(_STR(15490, lpAR->pPlayer->self->GetLang()),(*m_pArenaList[a].c_itArene).strZOneName);
               }
               
               lpAR->pPlayer->self->SendInfoMessage(strTmp,CL_YELLOW);
               ResetPlayerArene(lpAR->pPlayer,a);

               delete lpAR;
               lpAR = NULL;
               m_pArenaList[a].c_aPlayerList.RemoveAt(0);
            }

            if(uiEndGame == ARENA1_END_TIMEOUT)
            {
               _LOG_ARENA
                  LOG_ALWAYS,
                  "ARENE1 ID %d: TIMEOUT blue/red %d/%d",a,
                  m_pArenaList[a].c_PointBTeam,m_pArenaList[a].c_PointRTeam
                  LOG_
            }
            else if(uiEndGame == ARENA1_END_COMPLETED)
            {
               _LOG_ARENA
                  LOG_ALWAYS,
                  "ARENE1 ID %d: ENDED blue/red %d/%d",a,
                  m_pArenaList[a].c_PointBTeam,m_pArenaList[a].c_PointRTeam
                  LOG_
            }
            else if(uiEndGame == ARENA1_END_NO_PLAYER)
            {
               _LOG_ARENA
                  LOG_ALWAYS,
                  "ARENE1 ID %d: NO Player left on a team",a
                  LOG_
            }
            else if(uiEndGame == ARENA1_END_BALANCE_PLAYER)
            {
               _LOG_ARENA
                  LOG_ALWAYS,
                  "ARENE1 ID %d: Number of team player > 2",a
                  LOG_
            }
            
            
            m_pArenaList[a].c_AreneStatus = 0;
            m_pArenaList[a].c_iLevelMin   = 0;
            m_pArenaList[a].c_iLevelMax   = 0;
            m_pArenaList[a].c_dwStartTime = 0;
            m_pArenaList[a].c_PointBTeam  = 0;
            m_pArenaList[a].c_PointRTeam  = 0;
         }
      }
   }
   return;
}

void Arena1Master::IncreasePoint(Players *pPlayer, int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return;

   CAutoLock autoArena1Lock( &g_Arena1Lock );
   
   if(pPlayer->self->GetArenaTeam() == 1)
      m_pArenaList[iAreneID].c_PointBTeam++;
   else if(pPlayer->self->GetArenaTeam() == 2)
      m_pArenaList[iAreneID].c_PointRTeam++;
}

void Arena1Master::AddPlayer(Players *pPlayer, int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return;

   CAutoLock autoArena1Lock( &g_Arena1Lock );

   CString strTmp;

   if(pPlayer->self->ViewFlag(__FLAG_PLAYER_ARENE_BLOCK_VALUE_FLAG) >=3)
   {
      pPlayer->self->SendInfoMessage( _STR( 15495 , pPlayer->self->GetLang() ),CL_RED);
      return;
   }

   //Look the arena1 current status...
   if(m_pArenaList[iAreneID].c_AreneStatus == 0)
   {
      if(pPlayer->self->GetLevel() < 25)
      {
         pPlayer->self->SendInfoMessage( _STR( 15497 , pPlayer->self->GetLang() ),CL_RED);
         return;
      }

      //si larene nes pas definie on empeche de starter une arene...
      if(m_iNbrArena<1)
      {
         pPlayer->self->SendInfoMessage( _STR( 15480 , pPlayer->self->GetLang() ),CL_RED);
         return;
      }

      //pas de game en cours...  on cree new game attente pour un demarrege bientot...

      //valide que le joueur ne faiot pas partue d<une arena...
      if(pPlayer->self->GetArenaID() >0)
      {
         pPlayer->self->SendInfoMessage( _STR( 15473 , pPlayer->self->GetLang() ),CL_RED);
         return ;
      }

      //Valide que le player vien pas d'en lancer une...
      if(pPlayer->self->GetArenaLastStart() > timeGetTime())
      {
         pPlayer->self->SendInfoMessage( _STR( 15494 , pPlayer->self->GetLang() ),CL_RED);
         return ;
      }

      //si le joueur fait partie d'un groupe on dois le flusher du groupe...
      if( pPlayer->self && pPlayer->self->GetGroup() != NULL )
      {
         pPlayer->self->GetGroup()->Dismiss( pPlayer->self ); //Dismiss normal car pas en arene...
         pPlayer->self->SetGroup(NULL);
      }

      ResetPlayerArene(pPlayer,iAreneID);
      pPlayer->self->SetArenaType(ARENE1_TYPE);
      pPlayer->self->SetArenaID(iAreneID+1);
      pPlayer->self->SetArenaLastStart(timeGetTime()+60000); //il ne pourra pas recreer d arene avanty 1 minutes
      

      sArenaPlayerList *pNewPL = new sArenaPlayerList();
      pNewPL->pPlayer    = pPlayer;
      int iPos = m_pArenaList[iAreneID].c_aPlayerList.GetSize();
      if(iPos <0) iPos = 0;
      m_pArenaList[iAreneID].c_aPlayerList.SetAtGrow(iPos, pNewPL );


      int iNiveauMax = pPlayer->self->GetLevel()+25;
      int iNiveauMin = pPlayer->self->GetLevel()-25;
      int iNiveauOff = 0;
      if(iNiveauMax > MAX_LEVEL_CAN_HAVE)
      {
         iNiveauOff = iNiveauMax-MAX_LEVEL_CAN_HAVE;
         iNiveauMax = MAX_LEVEL_CAN_HAVE;
         iNiveauMin -=iNiveauOff;
      }
      else if(iNiveauMin < 25)
      {
         iNiveauOff = 25 -iNiveauMin;
         iNiveauMax += iNiveauOff;
         iNiveauMin = 25;
      }

      m_pArenaList[iAreneID].c_iLevelMin = iNiveauMin;
      m_pArenaList[iAreneID].c_iLevelMax = iNiveauMax;


      m_pArenaList[iAreneID].c_dwStartTime = timeGetTime();


      _LOG_ARENA
         LOG_ALWAYS,
         "ARENE1 ID %d: Player ID:%d %s (%s) CREATE Arene level range %d - %d",iAreneID,
         pPlayer->self->GetID(),
         pPlayer->self->GetTrueName(),
         pPlayer->GetFullAccountName(),
         m_pArenaList[iAreneID].c_iLevelMin,m_pArenaList[iAreneID].c_iLevelMax
         LOG_

      strTmp.Format(_STR(15472, pPlayer->self->GetLang()),(*m_pArenaList[iAreneID].c_itArene).strZOneName ,iNiveauMin ,iNiveauMax);
      CString strCC;
      strCC = "PVP";
      ChatterChannels &cChatter = CPlayerManager::GetChatter();
      cChatter.TalkSystem( "Système", (char*)strCC.GetBuffer(0), (char*)strTmp.GetBuffer(0) );

      m_pArenaList[iAreneID].c_AreneStatus = 1;

      SendArenaList(NULL,iAreneID); //creation de larene
   }
   else if(m_pArenaList[iAreneID].c_AreneStatus == 1)
   {
      //valide que le joueur ne faiot pas partue d<une arena...
      if(pPlayer->self->GetArenaID() >0)
      {
         pPlayer->self->SendInfoMessage( _STR( 15473 , pPlayer->self->GetLang() ),CL_RED);
         return;
      }
      //si nombre maximum de joueur atteing
      if(m_pArenaList[iAreneID].c_aPlayerList.GetSize()>m_pArenaList[iAreneID].c_ArenaCfg.iNbrMaxPlayer)
      {
         pPlayer->self->SendInfoMessage( _STR( 15475 , pPlayer->self->GetLang() ),CL_RED);
         return;
      }

      if(pPlayer->self->GetLevel() < m_pArenaList[iAreneID].c_iLevelMin || pPlayer->self->GetLevel() > m_pArenaList[iAreneID].c_iLevelMax)
      {
         pPlayer->self->SendInfoMessage( _STR( 15491 , pPlayer->self->GetLang() ),CL_RED);
         return;
      }

      //si le joueur fait partie d'un groupe on dois le flusher du groupe...
      if( pPlayer->self && pPlayer->self->GetGroup() != NULL )
      {
         pPlayer->self->GetGroup()->Dismiss( pPlayer->self );//Dismiss normal car pas en arene...
         pPlayer->self->SetGroup(NULL);
      }
      ResetPlayerArene(pPlayer,iAreneID);
      pPlayer->self->SetArenaType(ARENE1_TYPE);
      pPlayer->self->SetArenaID(iAreneID+1);

      sArenaPlayerList *pNewPL = new sArenaPlayerList();
      pNewPL->pPlayer    = pPlayer;
      int iPos = m_pArenaList[iAreneID].c_aPlayerList.GetSize();
      if(iPos <0) iPos = 0;
      m_pArenaList[iAreneID].c_aPlayerList.SetAtGrow(iPos, pNewPL );

      _LOG_ARENA
         LOG_ALWAYS,
         "ARENE1 ID %d: Player ID:%d %s (%s) JOIN ARENA (waiting) level range %d - %d",iAreneID,
         pPlayer->self->GetID(),
         pPlayer->self->GetTrueName(),
         pPlayer->GetFullAccountName(),
         m_pArenaList[iAreneID].c_iLevelMin,m_pArenaList[iAreneID].c_iLevelMax
         LOG_

      SendArenaList(NULL,iAreneID); //Ajout dun personne dans larene
   }
   else if(m_pArenaList[iAreneID].c_AreneStatus == 2)
   {
      //une partie est en cours... pour linstant on accepte pas de nouveau joueur...
      //eventuellement on pourrais ajouter ce user a la game direct...
   }
}
 
void Arena1Master::RemovePlayer(Players *pPlayer,bool bUpdateList, int iAreneID,BOOL bPenalty)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return;

   CAutoLock autoArena1Lock( &g_Arena1Lock );

   //aucune arene de creer
   if(m_pArenaList[iAreneID].c_AreneStatus == 0)
   {
      pPlayer->self->SendInfoMessage( _STR( 15476 , pPlayer->self->GetLang() ),CL_RED);
      return;
   }

   //valide que le joueur ne faiot pas partue d<une arena...
   if(pPlayer->self->GetArenaID()==0)
   {
      pPlayer->self->SendInfoMessage( _STR( 15477 , pPlayer->self->GetLang() ),CL_RED);
      return;
   }

   //on le quitte de cette arene...
   if(m_pArenaList[iAreneID].c_AreneStatus == 1)
   {
      //on ne fait que le quitter de la liste...

      sArenaPlayerList *lpPL;
      for(UINT i=0;i<m_pArenaList[iAreneID].c_aPlayerList.GetSize();i++)
      {
         lpPL = (sArenaPlayerList *)m_pArenaList[iAreneID].c_aPlayerList.GetAt(i);
         if(pPlayer->self->GetID() == lpPL->pPlayer->self->GetID())
         {
            TFCPacket sending;
            sending << (RQ_SIZE)RQ_ARENA1_UpdateTimeBS;
            sending << (long)iAreneID;
            sending << (char)1;
            sending << (long)ARENE1_TYPE;
            sending << (short)0;
            sending << (short)0;
            lpPL->pPlayer->self->SendPlayerMessage(sending);

            delete lpPL;
            lpPL = NULL;
            m_pArenaList[iAreneID].c_aPlayerList.RemoveAt(i);
            i = m_pArenaList[iAreneID].c_aPlayerList.GetSize(); //sort de la boucle...
         }
      }
      ResetPlayerArene(pPlayer,iAreneID);

      _LOG_ARENA
         LOG_ALWAYS,
         "ARENE1 ID %d: Player ID:%d %s (%s) LEAVE Arene in waiting mode",iAreneID,
         pPlayer->self->GetID(),
         pPlayer->self->GetTrueName(),
         pPlayer->GetFullAccountName()
         LOG_

      if(m_pArenaList[iAreneID].c_aPlayerList.GetSize() <=0)
      {
         _LOG_ARENA
            LOG_ALWAYS,
            "ARENE1 ID %d: END of Arene, No player left on waiting mode",iAreneID
            LOG_

         m_pArenaList[iAreneID].c_AreneStatus = 0;
         m_pArenaList[iAreneID].c_iLevelMin   = 0;
         m_pArenaList[iAreneID].c_iLevelMax   = 0;
         m_pArenaList[iAreneID].c_dwStartTime = 0;
         m_pArenaList[iAreneID].c_PointBTeam  = 0;
         m_pArenaList[iAreneID].c_PointRTeam  = 0;
      }

      SendArenaList(NULL,iAreneID); //suppression dune personne de larene
   }
   else if(m_pArenaList[iAreneID].c_AreneStatus == 2)
   {
      //Quitte la partie en cours
      int iTeamID = 0;
      sArenaPlayerList *lpPL;
      for(UINT i=0;i<m_pArenaList[iAreneID].c_aPlayerList.GetSize();i++)
      {
         lpPL = (sArenaPlayerList *)m_pArenaList[iAreneID].c_aPlayerList.GetAt(i);
         if(pPlayer->self->GetID() == lpPL->pPlayer->self->GetID())
         {
            if(pPlayer->self->ViewFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG)!= 0)
            {
               //on dois remettre le drapeau en jeu...
               SummonFlag(iAreneID,pPlayer->self->GetWL().X,pPlayer->self->GetWL().Y,pPlayer->self->GetWL().world);
            }
            ResetPlayerArene(pPlayer,iAreneID);
            if( pPlayer->self && pPlayer->self->GetGroup() != NULL )
            {
               pPlayer->self->GetGroup()->DismissArene( pPlayer->self ); //Dismiss ARENE car on ne dois pas detruire group si leader quitte
               pPlayer->self->SetGroup(NULL);

            }
            pPlayer->self->Teleport(lpPL->wlOldLoc,1,TRUE);

            if(bPenalty)
            {
               int iCurTime = time(NULL);
               pPlayer->self->SetFlag(__FLAG_PLAYER_ARENE_BLOCK_VALUE_FLAG,pPlayer->self->ViewFlag(__FLAG_PLAYER_ARENE_BLOCK_VALUE_FLAG)+1);
               pPlayer->self->SetFlag(__FLAG_PLAYER_ARENE_BLOCK_TIME_FLAG,iCurTime);
            }
            
            

            iTeamID = lpPL->pPlayer->self->GetArenaTeam();

            TFCPacket sendingBS;
            sendingBS << (RQ_SIZE)RQ_ARENA1_UpdateTimeBS;
            sendingBS << (long)iAreneID;
            sendingBS << (char)2;
            sendingBS << (long)ARENE1_TYPE;
            sendingBS << (short)0;
            sendingBS << (short)0;
            sendingBS << (short)0;
            sendingBS << (short)0;
            lpPL->pPlayer->self->SendPlayerMessage(sendingBS);

            delete lpPL;
            lpPL = NULL;
            m_pArenaList[iAreneID].c_aPlayerList.RemoveAt(i);
            i = m_pArenaList[iAreneID].c_aPlayerList.GetSize(); //sort de la boucle...
         }
      }
      

      _LOG_ARENA
         LOG_ALWAYS,
         "ARENE1 ID %d: Player ID:%d %s (%s) LEAVE Arene in playing mode",iAreneID,
         pPlayer->self->GetID(),
         pPlayer->self->GetTrueName(),
         pPlayer->GetFullAccountName()
         LOG_

      for(int p=0;p<m_pArenaList[iAreneID].c_aPlayerList.GetSize();p++)
      {
         lpPL = (sArenaPlayerList *)m_pArenaList[iAreneID].c_aPlayerList.GetAt(p);
         if(lpPL->pPlayer->self->GetArenaTeam() == iTeamID)
            SendPlayerFakeGroup(lpPL->pPlayer,iAreneID);
      }
   }
}

void Arena1Master::SendArenaList(Players *pPlayer, int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return;

   CAutoLock autoArena1Lock( &g_Arena1Lock );

   if(m_pArenaList[iAreneID].c_AreneStatus == 0)
   {
      //personne en wait quand pas starter
   }
   else if(m_pArenaList[iAreneID].c_AreneStatus == 1)
   {
      sArenaPlayerList *lpAR;
      for(int i=0;i<m_pArenaList[iAreneID].c_aPlayerList.GetSize();i++)
      {
         lpAR = (sArenaPlayerList *)m_pArenaList[iAreneID].c_aPlayerList.GetAt(i);
         SendArenaWaitingList(lpAR->pPlayer,iAreneID);
      }
   }
   else if(m_pArenaList[iAreneID].c_AreneStatus ==2)
   {
     SendArenaPlayStat(pPlayer,false,iAreneID);
   }
}

void Arena1Master::SendArenaPlayStat(Players *pPlayer,bool bForceOffline, int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return;

   CAutoLock autoArena1Lock( &g_Arena1Lock );
   if(m_pArenaList[iAreneID].c_AreneStatus ==2)
   {
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_ARENA1_UpdatePlayStat;

      char chOnList        = 0;

      DWORD dwElapsed = timeGetTime()-m_pArenaList[iAreneID].c_dwStartTime;
      int iNbrSecC = dwElapsed/1000;
      int iNbrSecT = m_pArenaList[iAreneID].c_ArenaCfg.iNbrPlayWaitTimeSec; 
      int iNbrSecR = iNbrSecT-iNbrSecC;    //10 min - nbr temps ecouler
      short shNbrMin  = iNbrSecR/60;
      short shNbrSec  = iNbrSecR-(shNbrMin*60);
      if(pPlayer->self->GetArenaID()==iAreneID+1)
         chOnList = 1;
      if(bForceOffline)
         chOnList = 0;

      sending << (long)iAreneID;
      sending << (long)ARENE1_TYPE;
      sending << (short)shNbrMin;
      sending << (short)shNbrSec;
      sending << (char)chOnList;
      sending << (short)m_pArenaList[iAreneID].c_iLevelMin;
      sending << (short)m_pArenaList[iAreneID].c_iLevelMax;
      sending << (short)m_pArenaList[iAreneID].c_PointBTeam;
      sending << (short)m_pArenaList[iAreneID].c_PointRTeam;



      USHORT ushNbrPl = m_pArenaList[iAreneID].c_aPlayerList.GetSize();
      sending << (short)ushNbrPl;
      if(ushNbrPl <=0 )
      {
         pPlayer->self->SendPlayerMessage(sending);
         return;
      } 

      sArenaPlayerList *lpPL;
      for(int i=0;i<m_pArenaList[iAreneID].c_aPlayerList.GetSize();i++)
      {
         lpPL = (sArenaPlayerList *)m_pArenaList[iAreneID].c_aPlayerList.GetAt(i);
         sending << (long)lpPL->pPlayer->self->GetID();
         sending << lpPL->pPlayer->self->GetTrueName();
         sending << (short)lpPL->pPlayer->self->GetArenaTeam();
         sending << (short)lpPL->pPlayer->self->GetArenaFlag();
         sending << (short)lpPL->pPlayer->self->GetArenaKill();
         sending << (short)lpPL->pPlayer->self->GetArenaDead();
         if(lpPL->pPlayer->self->GetArenaPOINTS() >0)
            sending << (short)lpPL->pPlayer->self->GetArenaPOINTS();
         else
            sending << (short)0;
      }

      pPlayer->self->SendPlayerMessage(sending);

      return ;
   }
}

void Arena1Master::SendArenaWaitingList(Players *pPlayer, int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return;

   if(m_pArenaList[iAreneID].c_AreneStatus != 1)
      return; //larene est meme aps en wating
   
   if(pPlayer->self->GetArenaID()!=iAreneID+1)
      return; //le player est meem pas dans alrene...


   TFCPacket sending;
   sending << (RQ_SIZE)RQ_ARENA1_GetWaitPlayerList;

   sending << (long)iAreneID;
   sending << (long)ARENE1_TYPE;
   sending << (short)m_pArenaList[iAreneID].c_iLastNbrMin;
   sending << (short)m_pArenaList[iAreneID].c_iLastNbrSec;
   sending << (short)m_pArenaList[iAreneID].c_iLevelMin;
   sending << (short)m_pArenaList[iAreneID].c_iLevelMax;


   USHORT ushNbrPl = m_pArenaList[iAreneID].c_aPlayerList.GetSize();
   sending << (short)ushNbrPl;
   if(ushNbrPl <=0 )
   {
      pPlayer->self->SendPlayerMessage(sending);
      return;
   }

   sArenaPlayerList *lpPL;
   for(int i=0;i<m_pArenaList[iAreneID].c_aPlayerList.GetSize();i++)
   {
      lpPL = (sArenaPlayerList *)m_pArenaList[iAreneID].c_aPlayerList.GetAt(i);
      sending << (long)lpPL->pPlayer->self->GetID();
      sending << lpPL->pPlayer->self->GetTrueName();      
   }
   
   pPlayer->self->SendPlayerMessage(sending);

   return ;
}


void Arena1Master::SendPlayerFakeGroup(Players *pPlayer, int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return;

   short shNbrGroup1      = 0;
   short shNbrGroup2      = 0;
   short shPlayerIDGroup  = 0;
   short shPlayerCntGroup = 0;

   sArenaPlayerList *lpPL;
   for(int i=0;i<m_pArenaList[iAreneID].c_aPlayerList.GetSize();i++)
   {
      lpPL = (sArenaPlayerList *)m_pArenaList[iAreneID].c_aPlayerList.GetAt(i);
      if(lpPL->pPlayer->self->GetID()== pPlayer->self->GetID())
         shPlayerIDGroup = lpPL->pPlayer->self->GetArenaTeam();
      if(lpPL->pPlayer->self->GetArenaTeam() == 1)
         shNbrGroup1++;
      else
         shNbrGroup2++;
   }
   if(shPlayerIDGroup == 1)
      shPlayerCntGroup = shNbrGroup1;
   else
      shPlayerCntGroup = shNbrGroup2;


   TFCPacket sending;
   sending << (RQ_SIZE)RQ_UpdateGroupMembers;
   sending << (char)1;
   sending << (short)shPlayerCntGroup;

   
   for(int i=0;i<m_pArenaList[iAreneID].c_aPlayerList.GetSize();i++)
   {
      lpPL = (sArenaPlayerList *)m_pArenaList[iAreneID].c_aPlayerList.GetAt(i);
      if(lpPL->pPlayer->self->GetArenaTeam() == shPlayerIDGroup)
      {
         sending << (long)lpPL->pPlayer->self->GetID();    
         sending << (short)lpPL->pPlayer->self->GetLevel();
         
         if( lpPL->pPlayer->self->GetMaxHP() == 0 )// Avoid a divide-by-0
            sending << (short)0;
         else
            sending << (short)( lpPL->pPlayer->self->GetHP() * 100 / lpPL->pPlayer->self->GetMaxHP() ); // Send the HP percentage.
         sending << (char)0;
         sending << lpPL->pPlayer->self->GetTrueName();
      }
   }
   pPlayer->self->SendPlayerMessage( sending );
}

void Arena1Master::ResetPlayerArene(Players *pPlayer, int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return;

   RemTakeList(pPlayer,iAreneID);
   pPlayer->self->SetArenaType(DEFAULT_TYPE);
   pPlayer->self->SetArenaID(0);
   pPlayer->self->SetArenaTeam(0);
   pPlayer->self->SetArenaKill(0);
   pPlayer->self->SetArenaDead(0);
   pPlayer->self->SetArenaFlag(0);
   pPlayer->self->SetFlag(__FLAG_PLAYER_ARENE_HAVE_FLAG,0);
   pPlayer->self->SetFlag(__FLAG_NMS_TAG_DISPLAY_OVER_HEAD,0);
}


void Arena1Master::SummonFlagRnd(int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return;

   int iPos = rand() % 5;   
   switch(iPos)
   {
      case 0: SummonFlag(iAreneID,(*m_pArenaList[iAreneID].c_itArene).wlItemPod1.X,(*m_pArenaList[iAreneID].c_itArene).wlItemPod1.Y,(*m_pArenaList[iAreneID].c_itArene).wlItemPod1.world); break;
      case 1: SummonFlag(iAreneID,(*m_pArenaList[iAreneID].c_itArene).wlItemPod2.X,(*m_pArenaList[iAreneID].c_itArene).wlItemPod2.Y,(*m_pArenaList[iAreneID].c_itArene).wlItemPod2.world); break;
      case 2: SummonFlag(iAreneID,(*m_pArenaList[iAreneID].c_itArene).wlItemPod3.X,(*m_pArenaList[iAreneID].c_itArene).wlItemPod3.Y,(*m_pArenaList[iAreneID].c_itArene).wlItemPod3.world); break;
      case 3: SummonFlag(iAreneID,(*m_pArenaList[iAreneID].c_itArene).wlItemPod4.X,(*m_pArenaList[iAreneID].c_itArene).wlItemPod4.Y,(*m_pArenaList[iAreneID].c_itArene).wlItemPod4.world); break;
      case 4: SummonFlag(iAreneID,(*m_pArenaList[iAreneID].c_itArene).wlItemPod5.X,(*m_pArenaList[iAreneID].c_itArene).wlItemPod5.Y,(*m_pArenaList[iAreneID].c_itArene).wlItemPod5.world); break;
   }
}


void Arena1Master::SummonFlag(int iAreneID,int iX, int iY, int iW)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return;

   CAutoLock autoArena1Lock( &g_Arena1Lock );

   DWORD dwID = (*m_pArenaList[iAreneID].c_itArene).iItemID1;
   WorldPos wlPos = { iX,iY,iW};

   Objects *lpItem = new Objects;
   if( lpItem->Create( U_OBJECT, dwID ) )
   {  
      _item *item = NULL;
      lpItem->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &item );
             
      WorldMap *world = TFCMAIN::GetWorld( wlPos.world );
      if( world != NULL && world->IsValidPosition( wlPos ) )
      {
         world->deposit_unit( wlPos, lpItem );
         lpItem->BroadcastPopup( wlPos, true );
      }
      else
      {
         _LOG_ARENA
            LOG_ALWAYS,
            "ARENE1 ID %d: Unable to summon flag... invalid position",iAreneID
            LOG_
         lpItem->DeleteUnit();
      }

   }
   else
   {
      _LOG_ARENA
         LOG_ALWAYS,
         "ARENE1 ID %d: Unable to summon flag... invalid item",iAreneID+1
         LOG_
   }
}

void Arena1Master::SendMessageToAll(CString strMessage,DWORD dwColor, int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return;

   CAutoLock autoArena1Lock( &g_Arena1Lock );
   sArenaPlayerList *lpPL;
   for(int i=0;i<m_pArenaList[iAreneID].c_aPlayerList.GetSize();i++)
   {
      lpPL = (sArenaPlayerList *)m_pArenaList[iAreneID].c_aPlayerList.GetAt(i);
      lpPL->pPlayer->self->SendSystemMessage(strMessage,dwColor);
   }
}

void Arena1Master::SendMessageToTeam(CString strPlayerName,int iTeamID,CString strMessage, int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return;

   CAutoLock autoArena1Lock( &g_Arena1Lock );
   sArenaPlayerList *lpPL;
   for(int i=0;i<m_pArenaList[iAreneID].c_aPlayerList.GetSize();i++)
   {
      lpPL = (sArenaPlayerList *)m_pArenaList[iAreneID].c_aPlayerList.GetAt(i);
      if(lpPL->pPlayer->self->GetArenaTeam() == iTeamID)
      {
         CString strMsg;
         strMsg.Format("{%s}: %s",strPlayerName,strMessage);
         lpPL->pPlayer->self->SendSystemMessage(strMsg,CL_BLUE_LIGHT);
      }
   }
}


void Arena1Master::AddTakeList(Players *pPlayer,Unit *pItemUnit, int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return;

   CAutoLock autoArena1Lock( &g_Arena1Lock );

   RemTakeList(pPlayer,iAreneID);
   
   sArenaTakeItemList *pNewItem = new sArenaTakeItemList();
   pNewItem->pPlayer    = pPlayer;
   pNewItem->pItemUnit  = pItemUnit;
   pNewItem->uiDecompte = m_pArenaList[iAreneID].c_ArenaCfg.iNbrPlayTakeItemSec; 
   int iPos = m_pArenaList[iAreneID].c_aTakeFlagList1.GetSize();
   if(iPos <0) iPos = 0;
   m_pArenaList[iAreneID].c_aTakeFlagList1.SetAtGrow(iPos, pNewItem );

   SendFlagTakeStatus(pPlayer,0,pNewItem->uiDecompte,iAreneID);
}

void Arena1Master::RemTakeList(Players *pPlayer, int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return;

   CAutoLock autoArena1Lock( &g_Arena1Lock );

   sArenaTakeItemList *pItemL;
   for(int i=0;i<m_pArenaList[iAreneID].c_aTakeFlagList1.GetSize();i++)
   {
      pItemL = (sArenaTakeItemList *)m_pArenaList[iAreneID].c_aTakeFlagList1.GetAt(i);
      if(pItemL->pPlayer->self->GetID() == pPlayer->self->GetID())
      {
         SendFlagTakeStatus(pItemL->pPlayer,1,m_pArenaList[iAreneID].c_ArenaCfg.iNbrPlayTakeItemSec,iAreneID);
         delete pItemL;
         pItemL = NULL;
         m_pArenaList[iAreneID].c_aTakeFlagList1.RemoveAt(i);
         i = m_pArenaList[iAreneID].c_aTakeFlagList1.GetSize(); //sort de la boucle...
      }
   }
}

void Arena1Master::SendFlagTakeStatus(Players *pPlayer, char chCode, char chDecompte, int iAreneID,bool bForceZero)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return;

   int iPC = 0;
   if(!bForceZero)
   {
      if(chDecompte >0)
         chDecompte--;
      iPC = ((m_pArenaList[iAreneID].c_ArenaCfg.iNbrPlayTakeItemSec-chDecompte)*100)/m_pArenaList[iAreneID].c_ArenaCfg.iNbrPlayTakeItemSec;
   }
   TFCPacket sending;
   sending << (RQ_SIZE)RQ_ARENA_SendTakeStatus;
   sending << (char)iPC;       //% completion,...
   sending << (char)chCode;    //status    Success
   pPlayer->self->SendPlayerMessage(sending);
}

int Arena1Master::GetNumberOfArene()
{
   return m_iNbrArena;
}

CString Arena1Master::GetAreneName(int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return "";
   return (*m_pArenaList[iAreneID].c_itArene).strZOneName;
}

int Arena1Master::GetSummonItemID(int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return 0;
   return (*m_pArenaList[iAreneID].c_itArene).iItemID1;
}

WorldPos Arena1Master::GetRecallDeathTeam(int iTeamID,int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
   {
      WorldPos wlTmp = {0,0,0};
      return wlTmp;
   }
   if(iTeamID == 1)
      return (*m_pArenaList[iAreneID].c_itArene).wlRecallBlueDead;
   else
      return (*m_pArenaList[iAreneID].c_itArene).wlRecallRedDead;
}

int  Arena1Master::GetDeathWaitTimeMS(int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return 0;
   return m_pArenaList[iAreneID].c_ArenaCfg.iNbrPlayWaitDeathMSec;
}
int  Arena1Master::GetDeathWaitTimeS(int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return 0;
   return m_pArenaList[iAreneID].c_ArenaCfg.iNbrPlayWaitDeathSec;
}

int  Arena1Master::GetMaxMinutes(int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return 0;
   return m_pArenaList[iAreneID].c_ArenaCfg.iNbrMaxMinute;
}

int  Arena1Master::GetMaxPoints(int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return 0;
   return m_pArenaList[iAreneID].c_ArenaCfg.iNbrMaxPoint;
}

int  Arena1Master::GetArenaStatus(int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return 0;
   return m_pArenaList[iAreneID].c_AreneStatus;
}

int  Arena1Master::GetArenaMinLevel(int iAreneID)
{
   return m_pArenaList[iAreneID].c_iLevelMin;
}

int  Arena1Master::GetArenaMaxLevel(int iAreneID)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return 0;
   return m_pArenaList[iAreneID].c_iLevelMax;
}

int  Arena1Master::GetArenaWaitTimeGen(int iAreneID,int &iNbrSec,int &iNbrMin)
{
   if(iAreneID <0 || iAreneID >= m_iNbrArena)
      return 0;
   iNbrSec = m_pArenaList[iAreneID].c_iLastNbrSec;
   iNbrMin = m_pArenaList[iAreneID].c_iLastNbrMin;
   return 0;
}
