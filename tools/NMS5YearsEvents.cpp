/* Note to self: 
 *   This is possibly the reason for the Deadlocks:
 *     CPlayerManager::GetCharacterRessourceByID();
 */

#include "stdafx.h"
#include "TFC Server.h"
#include "TFC_MAIN.h"
#include "NMS5YearsEvents.h"
#include "IntlText.h"
#include "Unit.h"
#include "PlayerManager.h"
#include "QuestFlagsListing.h"
#include "RegKeyHandler.h"

#include "T4CLog.h"
#include "DeadlockDetector.h"
#ifdef _WIN32
#include <mmsystem.h>
#endif


extern CTFCServerApp theApp;


DWORD            NMS5YearsEvents::m_dwNextItemTime;
int              NMS5YearsEvents::m_iLastPosUse;
WorldPos         NMS5YearsEvents::m_wlPosition[50];


static CLock     g_5yearEventsLock;




//////////////////////////////////////////////////////////////////////////////////////////
// Creates the class
//////////////////////////////////////////////////////////////////////////////////////////
void NMS5YearsEvents::Create()
{
   m_wlPosition[10].X = 1577;  m_wlPosition[10].Y = 1203;  m_wlPosition[10].world = 0;
   m_wlPosition[22].X = 1643;  m_wlPosition[22].Y = 1063;  m_wlPosition[22].world = 0;
   m_wlPosition[32].X = 1928;  m_wlPosition[32].Y = 1102;  m_wlPosition[32].world = 0;
   m_wlPosition[ 4].X = 1797;  m_wlPosition[ 4].Y = 747;   m_wlPosition[ 4].world = 0;
   m_wlPosition[23].X = 2181;  m_wlPosition[23].Y = 628;   m_wlPosition[23].world = 0;
   m_wlPosition[49].X = 2196;  m_wlPosition[49].Y = 1219;  m_wlPosition[49].world = 0;
   m_wlPosition[24].X = 2333;  m_wlPosition[24].Y = 1110;  m_wlPosition[24].world = 0;
   m_wlPosition[42].X = 2273;  m_wlPosition[42].Y = 835;   m_wlPosition[42].world = 0;
   m_wlPosition[45].X = 2570;  m_wlPosition[45].Y = 1154;  m_wlPosition[45].world = 0;
   m_wlPosition[ 3].X = 2572;  m_wlPosition[ 3].Y = 892;   m_wlPosition[ 3].world = 0;
   m_wlPosition[33].X = 2826;  m_wlPosition[33].Y = 680;   m_wlPosition[33].world = 0;
   m_wlPosition[46].X = 2340;  m_wlPosition[46].Y = 621;   m_wlPosition[46].world = 0;
   m_wlPosition[21].X = 2750;  m_wlPosition[21].Y = 1314;  m_wlPosition[21].world = 0;
   m_wlPosition[11].X = 2851;  m_wlPosition[11].Y = 1305;  m_wlPosition[11].world = 0;
   m_wlPosition[43].X = 2540;  m_wlPosition[43].Y = 527;   m_wlPosition[43].world = 0;
   m_wlPosition[14].X = 2697;  m_wlPosition[14].Y = 432;   m_wlPosition[14].world = 0;
   m_wlPosition[34].X = 2841;  m_wlPosition[34].Y = 284;   m_wlPosition[34].world = 0;
   m_wlPosition[12].X = 2456;  m_wlPosition[12].Y = 178;   m_wlPosition[12].world = 0;
   m_wlPosition[25].X = 1913;  m_wlPosition[25].Y = 254;   m_wlPosition[25].world = 0;
   m_wlPosition[ 9].X = 1981;  m_wlPosition[ 9].Y = 117;   m_wlPosition[ 9].world = 0;
   m_wlPosition[31].X = 1544;  m_wlPosition[31].Y = 192;   m_wlPosition[31].world = 0;
   m_wlPosition[20].X = 2834;  m_wlPosition[20].Y = 138;   m_wlPosition[20].world = 0;
   m_wlPosition[ 2].X = 2904;  m_wlPosition[ 2].Y = 1198;  m_wlPosition[ 2].world = 0;
   m_wlPosition[30].X = 1775;  m_wlPosition[30].Y = 1319;  m_wlPosition[30].world = 0;
   m_wlPosition[39].X = 1698;  m_wlPosition[39].Y = 1140;  m_wlPosition[39].world = 0;
   m_wlPosition[44].X = 156;   m_wlPosition[44].Y = 844;   m_wlPosition[44].world = 2;
   m_wlPosition[ 8].X = 332;   m_wlPosition[ 8].Y = 1086;  m_wlPosition[ 8].world = 2;
   m_wlPosition[48].X = 650;   m_wlPosition[48].Y = 873;   m_wlPosition[48].world = 2;
   m_wlPosition[35].X = 2198;  m_wlPosition[35].Y = 2931;  m_wlPosition[35].world = 1;
   m_wlPosition[13].X = 531;   m_wlPosition[13].Y = 1288;  m_wlPosition[13].world = 2;
   m_wlPosition[29].X = 252;   m_wlPosition[29].Y = 1371;  m_wlPosition[29].world = 2;
   m_wlPosition[19].X = 302;   m_wlPosition[19].Y = 1360;  m_wlPosition[19].world = 2;
   m_wlPosition[ 0].X = 744;   m_wlPosition[ 0].Y = 88;    m_wlPosition[ 0].world = 1;
   m_wlPosition[28].X = 730;   m_wlPosition[28].Y = 223;   m_wlPosition[28].world = 1;
   m_wlPosition[ 5].X = 583;   m_wlPosition[ 5].Y = 559;   m_wlPosition[ 5].world = 2;
   m_wlPosition[38].X = 251;   m_wlPosition[38].Y = 744;   m_wlPosition[38].world = 2;
   m_wlPosition[37].X = 411;   m_wlPosition[37].Y = 324;   m_wlPosition[37].world = 2;
   m_wlPosition[40].X = 185;   m_wlPosition[40].Y = 176;   m_wlPosition[40].world = 1;
   m_wlPosition[ 7].X = 159;   m_wlPosition[ 7].Y = 369;   m_wlPosition[ 7].world = 1;
   m_wlPosition[27].X = 47;    m_wlPosition[27].Y = 184;   m_wlPosition[27].world = 1;
   m_wlPosition[47].X = 577;   m_wlPosition[47].Y = 236;   m_wlPosition[47].world = 1;
   m_wlPosition[16].X = 1610;  m_wlPosition[16].Y = 2987;  m_wlPosition[16].world = 1;
   m_wlPosition[ 1].X = 2589;  m_wlPosition[ 1].Y = 1615;  m_wlPosition[ 1].world = 0;
   m_wlPosition[18].X = 1050;  m_wlPosition[18].Y = 1629;  m_wlPosition[18].world = 1;
   m_wlPosition[41].X = 517;   m_wlPosition[41].Y = 2166;  m_wlPosition[41].world = 2;
   m_wlPosition[36].X = 301;   m_wlPosition[36].Y = 2673;  m_wlPosition[36].world = 2;
   m_wlPosition[ 6].X = 272;   m_wlPosition[ 6].Y = 2940;  m_wlPosition[ 6].world = 2;
   m_wlPosition[26].X = 871;   m_wlPosition[26].Y = 2497;  m_wlPosition[26].world = 2;
   m_wlPosition[15].X = 658;   m_wlPosition[15].Y = 2120;  m_wlPosition[15].world = 2;
   m_wlPosition[17].X = 1178;  m_wlPosition[17].Y = 168;   m_wlPosition[17].world = 1;



   m_dwNextItemTime = 0;
   m_iLastPosUse    = 0;

   //read last pos from registry
   RegKeyHandler regKey; 
   if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
   {
      m_iLastPosUse = static_cast< DWORD >( regKey.GetProfileInt( "NMS5YearLastPos", 0 ) );
   }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Destroys skills
// 
//////////////////////////////////////////////////////////////////////////////////////////
void NMS5YearsEvents::Destroy( void )
{
  

}


void NMS5YearsEvents::Manage5YearEvents()
{

   if(theApp.m_dwNMS5YearItemnPod == 0)
      return ;

   CString strTmp;
   CAutoLock autoArena1Lock( &g_5yearEventsLock );

   DWORD dwCurTime = timeGetTime();
   if(dwCurTime > m_dwNextItemTime)
   {
      //process top summon ITEM to new position...
      _LOG_WORLD 
         LOG_MISC_1, 
         "****NMS 5 Year Events POD Item at position %d,%d,%d",m_wlPosition[m_iLastPosUse].X,m_wlPosition[m_iLastPosUse].Y,m_wlPosition[m_iLastPosUse].world
         LOG_


      DWORD dwID = Unit::GetIDFromName( "__NM_Coffre Anniversaire", U_NPC, TRUE );

      if( dwID != 0 )
      {
         WorldPos wlPos = m_wlPosition[m_iLastPosUse];
         WorldMap *wlWorld = TFCMAIN::GetWorld( wlPos.world );			
         if( wlWorld != NULL )
         {
            if( wlWorld->IsValidPosition( wlPos ) )
            {
               Creatures *lpMonster = new Creatures;
               if( lpMonster->Create( U_NPC, dwID ) )
               {
                  lpMonster->SetDestination( wlPos );
                  lpMonster->Do( wandering,"SUMMON MONSTER $ AT $,$,$" );
                  lpMonster->SetWL( wlPos );
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
         }
      }
    


      m_iLastPosUse++;
      if(m_iLastPosUse >= 50)
         m_iLastPosUse = 0; //restart to 0

      RegKeyHandler regKey; 
      if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
      {
         regKey.WriteProfileInt( "NMS5YearLastPos", m_iLastPosUse ); 
      }



      //4h == 60*60*4 == 14400
      //8h == 60*60*8 == 28800

      int iNextTime = rand()%14400;  // random 0 to 4 hour
      iNextTime+= 14400;             // add 4 hour constant to random between 4 and 8 hour
      iNextTime*=1000;               // put in milisec


      //int iNextTime = rand()%120;  // random 0 to 2 min
      //iNextTime+= 120;             // add 2 minute constant to random between 4 and 8 hour
      //iNextTime*=1000;               // put in milisec

      m_dwNextItemTime = dwCurTime+iNextTime;
   }

   return;
}
