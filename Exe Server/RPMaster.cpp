/* Note to self: 
 *   This is possibly the reason for the Deadlocks:
 *     CPlayerManager::GetCharacterRessourceByID();
 */

#include "stdafx.h"
#include "TFC Server.h"
#include "TFC_MAIN.h"
#include "RPMaster.h"
#include "IntlText.h"
#include "Unit.h"
#include "PlayerManager.h"
#include "GMMsgMaster.h"

#include "T4CLog.h"
#include "DeadlockDetector.h"

extern CTFCServerApp theApp;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CPtrArray RPMaster::c_aInterRP;
CPtrArray RPMaster::c_aInviteRP;
int       RPMaster::c_InterRPIndex;

static CLock     g_ALockRP;





//////////////////////////////////////////////////////////////////////////////////////////
// Creates the class
//////////////////////////////////////////////////////////////////////////////////////////
void RPMaster::Create( void )
{
   c_InterRPIndex = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Destroys skills
// 
//////////////////////////////////////////////////////////////////////////////////////////
void RPMaster::Destroy( void )
{
}

int RPMaster::CreateNewRP(Players *pPlayer,CString strMessage)
{
   CAutoLock autoRPLock( &g_ALockRP );

   CString strMsgTmp;

   //Find if player already in RP Interaction
   if(pPlayer->self->ViewFlag(__FLAG_INTERACTION_RP) == 0)  //pas activer
   {
      strMsgTmp.Format(_STR(15397, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
      return 0;
   }
   if(pPlayer->self->ViewFlag(__FLAG_INTERACTION_RP) == 2) //bloquer GM
   {
      strMsgTmp.Format(_STR(15398, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
      return 0;
   }

   char bHaveRP = 0;
   int haveRPID = 0;
   RPGetIsOnRP(pPlayer->self->GetID(),bHaveRP,haveRPID);

   if(bHaveRP == 1)
   {
      //deja proprietaire dun message
      strMsgTmp.Format(_STR(15396, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
      return 0;
   }
   if(bHaveRP == 2)
   {
      //a deja membre dun message
      strMsgTmp.Format(_STR(15396, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
      return 0;
   }

   sIntercationRP *pNewRP = new sIntercationRP;
   pNewRP->strRPSujet           = strMessage.GetBuffer(0);
   pNewRP->pPlayer              = pPlayer;
   pNewRP->iMessageID           = c_InterRPIndex;
   pNewRP->pPlayerWaitingAnswer = NULL;
   pNewRP->iWaitingCnt          = 0;
   pNewRP->iSignaledNOTRP       = 0;
   


   //on se rajoute au groupe...
   sRPPl *pNewPl = new sRPPl;
   pNewPl->pPlayer = pPlayer;
   int iPosPL = pNewRP->aPlayerList.GetSize();
   if(iPosPL <0)
      iPosPL = 0;
   pNewRP->aPlayerList.SetAtGrow(iPosPL, pNewPl );

   int iPos = c_aInterRP.GetSize();
   if(iPos <0)
      iPos = 0;
   c_aInterRP.SetAtGrow(iPos, pNewRP );

   _LOG_INTERRP
      LOG_DEBUG_LVL1,
      "Player ID:%d %s (%s) CREATE RP %s",
      pPlayer->self->GetID(),
      pPlayer->self->GetTrueName(),
      pPlayer->GetFullAccountName(),
      strMessage.GetBuffer(0)
      LOG_

   pPlayer->self->NMModeRPPhaseID(c_InterRPIndex);

   c_InterRPIndex++;
   if(c_InterRPIndex >0xFFFF)
      c_InterRPIndex = 0;

   CString strCC;
   strCC = "Phase Roleplay";
   strMsgTmp.Format(_STR(15399, pPlayer->self->GetLang()),pPlayer->self->GetTrueName());

   ChatterChannels &cChatter = CPlayerManager::GetChatter();
   cChatter.TalkSystem( "Système", (char*)strCC.GetBuffer(0), (char*)strMsgTmp.GetBuffer(0) );

   strMsgTmp.Format(_STR(15427, pPlayer->self->GetLang()));
   pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);

   RPBroadcastInfo(pPlayer);

   return 0;
}

int RPMaster::RPBroadcastInfo(Players *pPlayer)
{
   CAutoLock autoRPLock( &g_ALockRP );

   sIntercationRP *lpRP;
   sRPPl          *lpPL;

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_RP_BroadCastRP;

   int iXPlevel = pPlayer->self->GetRP_XPLevel();
   USHORT ushNbrRp = c_aInterRP.GetSize();

   int xpInThisLevel = theApp.g_dwRPXPTable[iXPlevel]- pPlayer->self->GetRP_XP();
   int xpToNext      = theApp.g_dwRPXPTableToLevel[iXPlevel];
   int iPourcentTmp  = (xpToNext-xpInThisLevel)*100/xpToNext;


   sending << (long)iPourcentTmp;
   sending << (long)iXPlevel;
   sending << (long)0;

   sending << (short)ushNbrRp;

   if(ushNbrRp <=0 )
   {
      pPlayer->self->SendPlayerMessage(sending);
      return 0 ;
   }

   char bHaveRP = 0;
   int haveRPID = 0;
   RPGetIsOnRP(pPlayer->self->GetID(),bHaveRP,haveRPID);
   for(int i=0;i<c_aInterRP.GetSize();i++)
   {
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(i);
      sending << (long)lpRP->iMessageID;      
      sending << lpRP->pPlayer->self->GetTrueName();      
      sending << lpRP->strRPSujet;      
   }
   sending << (char)bHaveRP;     
   if(bHaveRP)
   {
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(haveRPID);

      sending << lpRP->strRPSujet;     
      USHORT ushNbrPL = lpRP->aPlayerList.GetSize();
      sending << (short)ushNbrPL;

      for(int i=0;i<lpRP->aPlayerList.GetSize();i++)
      {
         lpPL = (sRPPl *)lpRP->aPlayerList.GetAt(i);
         sending << (long)lpPL->pPlayer->self->GetID(); 
         sending << lpPL->pPlayer->self->GetTrueName();   
      }
   }
   pPlayer->self->SendPlayerMessage(sending);

   return 0;
}

int RPMaster::RPInteractionTerminate(Players *pPlayer)
{
   CAutoLock autoRPLock( &g_ALockRP );

   CString strMsgTmp;
   sIntercationRP *lpRP;
   sRPPl          *lpPL;
   //trouve si il est proprietaire de son RP...
   char bHaveRP = 0;
   int haveRPID = 0;
   RPGetIsOnRP(pPlayer->self->GetID(),bHaveRP,haveRPID);
   if(bHaveRP == 1 && haveRPID >=0) //proprietaire d<un RP TERMINER
   {
      pPlayer->self->NMModeRPPhaseID(-1);
      strMsgTmp.Format(_STR(15400, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);


      //delete tous les user de ce RP
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(haveRPID);
      for(int i = 0; i < lpRP->aPlayerList.GetSize(); i++)
      {
         lpPL = (sRPPl *)lpRP->aPlayerList.GetAt(i);
         delete lpPL;
         lpPL = NULL;
         lpRP->aPlayerList.RemoveAt(i);
         i--;
      }

      lpRP = (sIntercationRP *)c_aInterRP.GetAt(haveRPID);
      delete lpRP;
      lpRP = NULL;
      c_aInterRP.RemoveAt(haveRPID);


      
      RPBroadcastInfo(pPlayer);

      _LOG_INTERRP
         LOG_DEBUG_LVL1,
         "Player ID:%d %s (%s) TERMINATE RP",
         pPlayer->self->GetID(),
         pPlayer->self->GetTrueName(),
         pPlayer->GetFullAccountName()
         LOG_
   }
   else if(bHaveRP == 2 && haveRPID >=0) //membre d'un rp QUITTER
   {
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(haveRPID);
      for(UINT pl=0;pl < lpRP->aPlayerList.GetSize();pl++)
      {
         lpPL = (sRPPl *)lpRP->aPlayerList.GetAt(pl);
         if(lpPL->pPlayer == pPlayer)
         {
            strMsgTmp.Format(_STR(15402, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);

            delete lpPL;
            lpPL = NULL;
            lpRP->aPlayerList.RemoveAt(pl);
            pl = lpRP->aPlayerList.GetSize(); //sort de la boucle...
         }
      }
      pPlayer->self->NMModeRPPhaseID(-1);
      RPBroadcastInfo(pPlayer);

      //send message to RP Creteur
      strMsgTmp.Format(_STR(15412, lpRP->pPlayer->self->GetLang()),pPlayer->self->GetTrueName());
      lpRP->pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
      RPBroadcastInfo(lpRP->pPlayer);
     

      _LOG_INTERRP
         LOG_DEBUG_LVL1,
         "Player ID:%d %s (%s) LEAVE RP",
         pPlayer->self->GetID(),
         pPlayer->self->GetTrueName(),
         pPlayer->GetFullAccountName()
         LOG_
   }
   else //pas supposer
   {
      strMsgTmp.Format(_STR(15401, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
   }

   return 0;
}

int RPMaster::RPRejoindreRP(Players *pPlayer,int iRPID)
{
   CAutoLock autoRPLock( &g_ALockRP );

   CString strMsgTmp;
   if(pPlayer->self->ViewFlag(__FLAG_INTERACTION_RP) == 2) //bloquer GM
   {
      strMsgTmp.Format(_STR(15403, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
      return 0;
   }

   sIntercationRP *lpRP;

   //trouve si il est proprietaire de son RP...
   char bHaveRP  = 0;
   int haveRPIDT = 0;
   RPGetIsOnRP(pPlayer->self->GetID(),bHaveRP,haveRPIDT);
   if(bHaveRP)
   {
      strMsgTmp.Format(_STR(15396, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
      return 0;
   }


   int haveRPIDFind = -1;
   for(int i=0;i<c_aInterRP.GetSize();i++)
   {
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(i);
      if(iRPID == lpRP->iMessageID)
      {
         haveRPIDFind = i;
         i = c_aInterRP.GetSize();
      }
   }

   if(haveRPIDFind == -1)//le message existe plus
   {
      strMsgTmp.Format(_STR(15413, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
      return 0;
   }

   lpRP = (sIntercationRP *)c_aInterRP.GetAt(haveRPIDFind);
   if(lpRP->pPlayerWaitingAnswer != NULL) //deja une autre atente
   {
      strMsgTmp.Format(_STR(15404, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
      return 0;
   }

   //on send une demande la personne maitre de ce RP...
   if(lpRP->pPlayer)
   {
      lpRP->pPlayerWaitingAnswer = pPlayer;
      lpRP->iWaitingCnt = 0;

      //Send request to this player...
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_RP_RejoindreRP;
      sending << pPlayer->self->GetTrueName();   
      lpRP->pPlayer->self->SendPlayerMessage(sending);

      strMsgTmp.Format(_STR(15432, lpRP->pPlayer->self->GetLang()),pPlayer->self->GetTrueName());
      lpRP->pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);

      strMsgTmp.Format(_STR(15431, pPlayer->self->GetLang()),lpRP->pPlayer->self->GetTrueName());
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
   }
   else
   {
      strMsgTmp.Format(_STR(15437, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
   }

   return 0;
}

int RPMaster::RPRejoindreRPResult(Players *pPlayer,int iRPAnswer)
{
   CAutoLock autoRPLock( &g_ALockRP );
   CString strMsgTmp;

   sIntercationRP *lpRP;

   char bHaveRP = 0;
   int haveRPID = 0;
   RPGetIsOnRP(pPlayer->self->GetID(),bHaveRP,haveRPID);
   if(bHaveRP == 1 && haveRPID >=0)
   {
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(haveRPID);
      if( lpRP->pPlayerWaitingAnswer != NULL)
      {
         //recherche le player qui a fait la demande
         if(lpRP->pPlayerWaitingAnswer)
         {
            char bHaveRP2 = 0;
            int haveRPID2 = 0;
            RPGetIsOnRP(lpRP->pPlayerWaitingAnswer->self->GetID(),bHaveRP2,haveRPID2);
            if(bHaveRP2 ==0) //sassure quil ne fait pas partie dun autre phase RP
            {
               if(iRPAnswer == 0)
               {
                  //reponse negative...
                  strMsgTmp.Format(_STR(15405, lpRP->pPlayerWaitingAnswer->self->GetLang()));
                  lpRP->pPlayerWaitingAnswer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);

                  strMsgTmp.Format(_STR(15407, pPlayer->self->GetLang()), lpRP->pPlayerWaitingAnswer->self->GetTrueName());
                  pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);

                  _LOG_INTERRP
                     LOG_DEBUG_LVL1,
                     "Player ID:%d %s (%s) NOT ACCEPTE Player ID:%d %s (%s)",
                     pPlayer->self->GetID(),
                     pPlayer->self->GetTrueName(),
                     pPlayer->GetFullAccountName(),
                     lpRP->pPlayerWaitingAnswer->self->GetID(),
                     lpRP->pPlayerWaitingAnswer->self->GetTrueName(),
                     lpRP->pPlayerWaitingAnswer->GetFullAccountName()
                     LOG_
               }
               else
               {
                  sRPPl *pNewPl = new sRPPl;
                  pNewPl->pPlayer = lpRP->pPlayerWaitingAnswer;
                  int iPosPL = lpRP->aPlayerList.GetSize();
                  if(iPosPL <0)
                     iPosPL = 0;
                  lpRP->aPlayerList.SetAtGrow(iPosPL, pNewPl );
                  lpRP->pPlayerWaitingAnswer->self->NMModeRPPhaseID(lpRP->iMessageID);

                  RPBroadcastInfo(lpRP->pPlayerWaitingAnswer);
                  RPBroadcastInfo(pPlayer);

                  strMsgTmp.Format(_STR(15406, pPlayer->self->GetLang()), pPlayer->self->GetTrueName());
                  lpRP->pPlayerWaitingAnswer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);

                  strMsgTmp.Format(_STR(15408, pPlayer->self->GetLang()), lpRP->pPlayerWaitingAnswer->self->GetTrueName());
                  pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);

                  _LOG_INTERRP
                     LOG_DEBUG_LVL1,
                     "Player ID:%d %s (%s) ACCEPTE Player ID:%d %s (%s)",
                     pPlayer->self->GetID(),
                     pPlayer->self->GetTrueName(),
                     pPlayer->GetFullAccountName(),
                     lpRP->pPlayerWaitingAnswer->self->GetID(),
                     lpRP->pPlayerWaitingAnswer->self->GetTrueName(),
                     lpRP->pPlayerWaitingAnswer->GetFullAccountName()
                     LOG_
               }
            }
         }
         else
         {
            strMsgTmp.Format(_STR(15442, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
         }
      }
      else
      {
         strMsgTmp.Format(_STR(15442, pPlayer->self->GetLang()));
         pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
      }

      //reset lattente...
      lpRP->pPlayerWaitingAnswer = NULL;
      lpRP->iWaitingCnt          = 0;
   }

   return 0;
}

int RPMaster::RPInteractionExpluser(Players *pPlayer,int iPlID)
{
   CAutoLock autoRPLock( &g_ALockRP );

   CString strMsgTmp;
   if(iPlID == pPlayer->self->GetID())
   {
      strMsgTmp.Format(_STR(15421, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
      return 0 ;
   }

   //trouve si il est proprietaire de son RP...
   sIntercationRP *lpRP;
   sRPPl          *lpPL;

   char bHaveRP = 0;
   int haveRPID = 0;
   RPGetIsOnRP(pPlayer->self->GetID(),bHaveRP,haveRPID);

   if(bHaveRP == 1) //Deja si on est un proprietaire dun RP...
   {
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(haveRPID);
      for(UINT pl=0;pl<lpRP->aPlayerList.GetSize();pl++)
      {
         lpPL = (sRPPl *)lpRP->aPlayerList.GetAt(pl);
         if(lpPL->pPlayer->self->GetID() == iPlID)
         {
            if(lpPL->pPlayer)
            {
               strMsgTmp.Format(_STR(15415, lpPL->pPlayer->self->GetLang()));
               lpPL->pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);

               strMsgTmp.Format(_STR(15416, pPlayer->self->GetLang()),lpPL->pPlayer->self->GetTrueName());
               pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);

               lpPL->pPlayer->self->NMModeRPPhaseID(-1);

               _LOG_INTERRP
                  LOG_DEBUG_LVL1,
                  "Player ID:%d %s (%s) a EXPULSER de son RP  Player ID:%d %s (%s)",
                  pPlayer->self->GetID(),
                  pPlayer->self->GetTrueName(),
                  pPlayer->GetFullAccountName(),
                  lpPL->pPlayer->self->GetID(),
                  lpPL->pPlayer->self->GetTrueName(),
                  lpPL->pPlayer->GetFullAccountName()
                  LOG_

               delete lpPL;
               lpPL = NULL;
               lpRP->aPlayerList.RemoveAt(pl);
               pl = lpRP->aPlayerList.GetSize(); //sort de la boucle...

               RPBroadcastInfo(pPlayer);
            }
            else
            {
               strMsgTmp.Format(_STR(15439, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
            }
         }
      }
   }
   else //pas supposer
   {
      strMsgTmp.Format(_STR(15428, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
   }

   return 0;
}

int RPMaster::RPInviteRP(Players *pPlayer,int iInvitedID)
{
   CAutoLock autoRPLock( &g_ALockRP );
   CString strMsgTmp;

   sInviteRP      *lpInv;

   char bHaveRP = 0;
   int haveRPID = 0;
   RPGetIsOnRP(pPlayer->self->GetID(),bHaveRP,haveRPID);

   if(bHaveRP != 1)
   {
      strMsgTmp.Format(_STR(15414, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
      return 0; //ce joueur est deja dans une autre phase pas suppose arriver mais on empeche...
   }

   Players *pInvitedUser     = CPlayerManager::GetCharacterRessourceByID(iInvitedID);//PM
   if(pInvitedUser)
   {
      //verifie que cet utilisateur nes pas deja inviter...
      for(int i=0;i<c_aInviteRP.GetSize();i++)
      {
         lpInv = (sInviteRP *)c_aInviteRP.GetAt(i);
         if(lpInv->pPlayerInvited->self->GetID() == pInvitedUser->self->GetID())
         {
            strMsgTmp.Format(_STR(15434, pPlayer->self->GetLang()),pInvitedUser->self->GetTrueName());
            pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
            CPlayerManager::FreePlayerResource(pInvitedUser);
            return 0;
         }
      }

      //Oki on ajoute a la liste des inviter...
      sInviteRP *pNewInv = NULL;
      pNewInv = new sInviteRP;
      pNewInv->pPlayerInvite  = pPlayer;
      pNewInv->pPlayerInvited = pInvitedUser;

      int iPosInv = c_aInviteRP.GetSize();
      if(iPosInv <0)
         iPosInv = 0;
      c_aInviteRP.SetAtGrow(iPosInv, pNewInv );



      TFCPacket sending;
      sending << (RQ_SIZE)RQ_RP_InviteRP;
      sending << pPlayer->self->GetTrueName();
      pInvitedUser->self->SendPlayerMessage( sending );


      pPlayer->self->SendInfoMessage( _STR( 15069, pPlayer->self->GetLang() ) ,CL_ORANGE);

      CPlayerManager::FreePlayerResource(pInvitedUser);
   }
   else
   {
      strMsgTmp.Format(_STR(15440, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
   }

   return 0;
}

int RPMaster::RPInviteRPResult(Players *pPlayer,int iRPAnswer)
{
   CAutoLock autoRPLock( &g_ALockRP );

   CString strMsgTmp;

   //on chercvhe le RPO de ce user...

   sIntercationRP *lpRP;
   sInviteRP      *lpInv;

   char bHaveRP = 0;
   int haveRPID = 0;
   RPGetIsOnRP(pPlayer->self->GetID(),bHaveRP,haveRPID);

   if(bHaveRP)
   {
      return 0; //ce joueur est deja dans une autre phase pas suppose arriver mais on empeche...
   }

   // trouve dans la liste des inviter si vous avez vraiment inviter ce joueur...
   Players *pInvitedPL = NULL;;
   bool bInvited = false;
   for(int a=0;a<c_aInviteRP.GetSize();a++)
   {
      lpInv = (sInviteRP *)c_aInviteRP.GetAt(a);
      if(lpInv->pPlayerInvited == pPlayer)
      {
         pInvitedPL = lpInv->pPlayerInvite;
         bInvited   = true;

         delete lpInv;
         lpInv = NULL;
         c_aInviteRP.RemoveAt(a);
         a = c_aInviteRP.GetSize(); //sort de la boucle...
      }
   }

   //regarde que cet invite a bien un RP...

   if(bInvited)
   {
      for(int i=0;i<c_aInterRP.GetSize();i++)
      {
         lpRP = (sIntercationRP *)c_aInterRP.GetAt(i);
         if(lpRP->pPlayer == pInvitedPL)
         {
            //oki on a le RP du gard qui nous a inviter...
            if(lpRP->pPlayer)
            {
               //c<est bien ce RP que nous cherchons...
               if(iRPAnswer == 0) //il na aps accepter
               {
                  strMsgTmp.Format(_STR(15418, lpRP->pPlayer->self->GetLang()),pPlayer->self->GetTrueName());
                  lpRP->pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);

                  strMsgTmp.Format(_STR(15430, pPlayer->self->GetLang()),lpRP->pPlayer->self->GetTrueName());
                  pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);

                  return 0;
               }
               else
               {
                  //il a accepter on ajoute a la liste des user
                  strMsgTmp.Format(_STR(15406, pPlayer->self->GetLang()), lpRP->pPlayer->self->GetTrueName());
                  pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
                  pPlayer->self->NMModeRPPhaseID(lpRP->iMessageID);

                  sRPPl *pNewPl = new sRPPl;
                  pNewPl->pPlayer = pPlayer;
                  int iPosPL = lpRP->aPlayerList.GetSize();
                  if(iPosPL <0)
                     iPosPL = 0;
                  lpRP->aPlayerList.SetAtGrow(iPosPL, pNewPl );

                  RPBroadcastInfo(lpRP->pPlayer);
                  RPBroadcastInfo(pPlayer);

                  strMsgTmp.Format(_STR(15419, lpRP->pPlayer->self->GetLang()), pPlayer->self->GetTrueName());
                  lpRP->pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);

                  _LOG_INTERRP
                     LOG_DEBUG_LVL1,
                     "Player ID:%d %s (%s) ACCEPTE INVITATION from Player ID:%d %s (%s)",
                     pPlayer->self->GetID(),
                     pPlayer->self->GetTrueName(),
                     pPlayer->GetFullAccountName(),
                     lpRP->pPlayer->self->GetID(),
                     lpRP->pPlayer->self->GetTrueName(),
                     lpRP->pPlayer->GetFullAccountName()
                     LOG_
                  return 0;

               }
            }
            else
            {
               strMsgTmp.Format(_STR(15442, pPlayer->self->GetLang()));
               pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
            }
         }
      }
   }
   return 0;
}

int RPMaster::RPSignalerRP(Players *pPlayer,int iRPID)
{
   CAutoLock autoRPLock( &g_ALockRP );
   CString strMsgTmp;

   sIntercationRP *lpRP;

   int haveRPIDFind = -1;
   for(int i=0;i<c_aInterRP.GetSize();i++)
   {
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(i);
      if(iRPID == lpRP->iMessageID)
      {
         haveRPIDFind = i;
         if(lpRP->iSignaledNOTRP > 0)
         {
            strMsgTmp.Format(_STR(15435, pPlayer->self->GetLang()));
            pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
            return 0 ;
         }
         i = c_aInterRP.GetSize();
      }
   }

   if(haveRPIDFind == -1)//le message existe plus
   {
      strMsgTmp.Format(_STR(15413, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
      return 0;
   }

   lpRP = (sIntercationRP *)c_aInterRP.GetAt(haveRPIDFind);

   if(lpRP->pPlayer)
   {
      lpRP->iSignaledNOTRP++;
      CString strMessage;
      strMessage.Format(_STR(15422, pPlayer->self->GetLang()),pPlayer->self->GetID(),
         pPlayer->self->GetTrueName(),
         pPlayer->GetFullAccountName(),
         lpRP->pPlayer->self->GetID(),
         lpRP->pPlayer->self->GetTrueName(),
         lpRP->pPlayer->GetFullAccountName());

      CString strTitre;
      strTitre = "RP Integration";
      GMMsgMaster::PostGMSystemMessage(strTitre, strMessage);

      CPlayerManager::SendMessagetoAllGOD(_STR(15438, pPlayer->self->GetLang()));
      strMsgTmp.Format(_STR(15423, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
   }
   else
   {
      strMsgTmp.Format(_STR(15441, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
   }

   return 0;
}


int RPMaster::RPEchangerRPDirect(Players *pPlayer)
{
   CAutoLock autoRPLock( &g_ALockRP );

   CString strMsgTmp;
   //on calcule le nbr de XP que le gars aura...
   int iLevel = pPlayer->self->GetLevel();
   if(iLevel >= MAX_LEVEL_CAN_HAVE)
   {
      strMsgTmp.Format(_STR(15425, pPlayer->self->GetLang()));
      pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
      return 0;
   }

   if(iLevel < 1)
      iLevel = 1;

   __int64  iNbrXpToNextLevel = Character::sm_n64XPchart[iLevel]- Character::sm_n64XPchart[iLevel-1];

   int iPCMax = 30;
   int iPCMin =  5;

   int iPCD = iPCMax-iPCMin;

   double dPCXPlevel = (double)((double)iLevel*(double)iPCD)/(double)MAX_LEVEL_CAN_HAVE;
   dPCXPlevel = iPCMax- dPCXPlevel;
   __int64  iGainXP = (__int64)((dPCXPlevel*iNbrXpToNextLevel)/100.00);


   char buf[256];
   _i64toa_s( iGainXP, buf,256, 10 );

   pPlayer->self->SetXP(pPlayer->self->GetXP()+iGainXP,true);

   _LOG_EVENTS
      LOG_DEBUG_LVL1,
      "Player ID:%d %s (%s) Use Items for XP. Gain %.02f%% XP (%s XPPoints) ",
      pPlayer->self->GetID(),
      pPlayer->self->GetTrueName(),
      pPlayer->GetFullAccountName(),
      dPCXPlevel,
      buf
      LOG_


   strMsgTmp.Format(_STR(15509, pPlayer->self->GetLang()),buf);
   pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
   RPBroadcastInfo(pPlayer);
   return 0;
}


int RPMaster::RPSendRPList(Players *pPlayer)
{
   CAutoLock autoRPLock( &g_ALockRP );

   sIntercationRP *lpRP;
   CString strMessage;

   strMessage.Format("MsgID - Player");
   pPlayer->self->SendInfoMessage(strMessage);
   for(int i=0;i<c_aInterRP.GetSize();i++)
   {
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(i);
      strMessage.Format("%05d - %s",lpRP->iMessageID,lpRP->pPlayer->self->GetTrueName());
      pPlayer->self->SendInfoMessage(strMessage);
   }

   return 0;
}

BOOL RPMaster::RPTerminateRP(int iMessageID)
{
   CAutoLock autoRPLock( &g_ALockRP );
   CString strMsgTmp;

   sIntercationRP *lpRP;
   sRPPl          *lpPL;

   for(int r=0;r<c_aInterRP.GetSize();r++)
   {
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(r);
      if(lpRP->iMessageID == iMessageID)
      {
         _LOG_INTERRP
            LOG_DEBUG_LVL1,
            "RP Message ID[%d] TERMINATED BY GM",
            iMessageID
            LOG_

            for(int i = 0; i < lpRP->aPlayerList.GetSize(); i++)
            {
               lpPL = (sRPPl *)lpRP->aPlayerList.GetAt(i);
               if(lpPL->pPlayer)
               {
                  strMsgTmp.Format(_STR(15436, lpPL->pPlayer->self->GetLang()));
                  lpPL->pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
                  lpPL->pPlayer->self->NMModeRPPhaseID(-1);
                  RPBroadcastInfo(lpPL->pPlayer);
               }
               delete lpPL;
               lpPL = NULL;
               lpRP->aPlayerList.RemoveAt(i);
               i--;
            }
            
            delete lpRP;
            lpRP = NULL;
            c_aInterRP.RemoveAt(r);
            return TRUE;
      }
   }
   return FALSE;
}




BOOL RPMaster::RpExist(int iRPID)
{
   CAutoLock autoRPLock( &g_ALockRP );
   sIntercationRP *lpRP;

   for(int r=0;r<c_aInterRP.GetSize();r++)
   {
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(r);
      if(lpRP->iMessageID == iRPID)
         return TRUE;
   }
   return FALSE;
}

BOOL RPMaster::RpExistAndCreateur(int iRPID, int iPlID)
{
   CAutoLock autoRPLock( &g_ALockRP );

   sIntercationRP *lpRP;
   for(int r=0;r<c_aInterRP.GetSize();r++)
   {
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(r);
      if(lpRP->iMessageID == iRPID)
      {
         if(lpRP->pPlayer->self->GetID() == iPlID)
            return TRUE;
         else
            return FALSE;
      }
   }
   return FALSE;
}

int RPMaster::RPInteractionTerminateLogOff(Players *pPlayer)
{
   CAutoLock autoRPLock( &g_ALockRP );
   CString strMsgTmp;

   //trouve si il est proprietaire de son RP...
   sIntercationRP *lpRP;

   char bHaveRP = 0;
   int haveRPID = 0;
   RPGetIsOnRP(pPlayer->self->GetID(),bHaveRP,haveRPID);

   if(bHaveRP == 1 && haveRPID >=0) //proprietaire d<un RP TERMINER
   {
      pPlayer->self->NMModeRPPhaseID(-1);
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(haveRPID);
      delete lpRP;
      lpRP = NULL;
      c_aInterRP.RemoveAt(haveRPID);

      _LOG_INTERRP
         LOG_DEBUG_LVL1,
         "Player ID:%d %s (%s) TERMINATE RP by logoff",
         pPlayer->self->GetID(),
         pPlayer->self->GetTrueName(),
         pPlayer->GetFullAccountName()
         LOG_
   }

   return 0;
}

int RPMaster::RPInteractionQuitterLogOff(Players *pPlayer)
{
   CAutoLock autoRPLock( &g_ALockRP );
   CString strMsgTmp;

   //trouve si il est proprietaire de son RP...

   sIntercationRP *lpRP;
   sRPPl          *lpPL;

   char bHaveRP = 0;
   int haveRPID = 0;
   RPGetIsOnRP(pPlayer->self->GetID(),bHaveRP,haveRPID);

   if(bHaveRP == 2 && haveRPID >=0) //membre d'un rp QUITTER
   {
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(haveRPID);
      for(UINT pl=0;pl < lpRP->aPlayerList.GetSize();pl++)
      {
         lpPL = (sRPPl *)lpRP->aPlayerList.GetAt(pl);
         if(lpPL->pPlayer->self->GetID() == pPlayer->self->GetID())
         {
            delete lpPL;
            lpPL = NULL;
            lpRP->aPlayerList.RemoveAt(pl);
            pl = lpRP->aPlayerList.GetSize();
         }
      }
      pPlayer->self->NMModeRPPhaseID(-1);

      //send message to RP Creteur
      if(lpRP->pPlayer)
      {
         strMsgTmp.Format(_STR(15412, lpRP->pPlayer->self->GetLang()),pPlayer->self->GetTrueName());
         lpRP->pPlayer->self->SendSystemMessage(strMsgTmp,CL_ORANGE);
         RPBroadcastInfo(lpRP->pPlayer);
      }

      _LOG_INTERRP
         LOG_DEBUG_LVL1,
         "Player ID:%d %s (%s) LEAVE RP by logoff",
         pPlayer->self->GetID(),
         pPlayer->self->GetTrueName(),
         pPlayer->GetFullAccountName()
         LOG_
   }

   return 0;
}



int RPMaster::RPGetIsOnRP(int iplID,char &chIsRP, int &iRPIndex)
{
   chIsRP = 0;
   iRPIndex = 0;

   sIntercationRP *lpRP;
   sRPPl          *lpPL;
   int i;
   for(i = 0; i < c_aInterRP.GetSize(); i++)
   {
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(i);
      if(lpRP->pPlayer->self->GetID() == iplID)
      {
         chIsRP = 1;
         iRPIndex = i;
         return 0;
      }
      else
      {
         for(int j=0;j<lpRP->aPlayerList.GetSize();j++)
         {
            lpPL = (sRPPl *)lpRP->aPlayerList.GetAt(j);
            if(lpPL->pPlayer->self->GetID() == iplID)
            {
               chIsRP = 2;
               iRPIndex = i;
               return 0;
            }
         }
      }
   }

   return 0;
}

BOOL RPMaster::IsInRPRange(int iP1X,int iP1Y,int iP2X,int iP2Y)
{
   if( abs( iP1X - iP2X ) <= theApp.m_dwLocalTalkRange && abs( iP1Y - iP2Y ) <= theApp.m_dwLocalTalkRange )
      return TRUE;

   return FALSE;
}

int RPMaster::CalculatePlayerPoints(int n,Players *pPlayer,int &iRule1,int &iRule2,int &iRule3,int &iRule4)
{
   sIntercationRP *lpRP;
   sRPPl          *lpPL;

   int iRule4Tmp = 0;
   int iRet = 0;
   if(pPlayer->self->ViewFlag(__FLAG_INTERACTION_RP) == 2)
   {
      iRet = 2; //flush player reason 2
   }
   else if(pPlayer->self->GetNMModeRPPhaseCntNOTTalk() <5 ) //si le player na pas parle depuis 5 coup de clock, on ne le process meme plus
   {
      //Scan les joueur a proximite...
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(n);
      for(int p=0;p<lpRP->aPlayerList.GetSize();p++)
      {
         lpPL = (sRPPl *)lpRP->aPlayerList.GetAt(p);
         if(lpPL->pPlayer->self->GetID() != pPlayer->self->GetID())  //on ne se compte pas...
         {
            if(lpPL->pPlayer)
            {
               if(IsInRPRange(pPlayer->self->GetWL().X,pPlayer->self->GetWL().Y,lpPL->pPlayer->self->GetWL().X,lpPL->pPlayer->self->GetWL().Y) &&
                  lpPL->pPlayer->self->GetNMModeRPPhaseCntNOTTalk() <5 ) //on ne compte pas les player qui non pas parkler depuis plus ed 5 coup...
               {
                  iRule1++;
                  if(lpPL->pPlayer->self->GetNMModeRPPhaseCntTalk() ==0)
                  {
                     iRule4Tmp++; //RULE 4
                  }
               }
            }
         }
      }

      if(iRule1 > 0)
         iRule2 = pPlayer->self->GetRP_XPLevel()/10; //RULE 2


      if(pPlayer->self->GetNMModeRPPhaseCntTalk() > 0 && iRule1 >0)
         iRule3 = 1; //RULE 3
      else if(pPlayer->self->GetNMModeRPPhaseCntTalk() < 1 && iRule1 >0)
         iRule4++; //RULE 3

      if(iRule1 >10)
         iRule1 = 10; //maximum of 10 points...
      if(iRule4Tmp>10)
         iRule4Tmp = 10;

      iRule4+=iRule4Tmp;

   }

   return 0;
}







int RPMaster::ManageInteractionRP()
{
   CAutoLock autoRPLock( &g_ALockRP );

   sIntercationRP *lpRP;
   sRPPl          *lpPL;
   CString strMsgTmp;

   for(UINT rp=0;rp< c_aInterRP.GetSize();)
   {
      lpRP = (sIntercationRP *)c_aInterRP.GetAt(rp);
      if(lpRP->pPlayerWaitingAnswer != NULL)
      {
         lpRP->iWaitingCnt++;
         if(lpRP->iWaitingCnt >1)
         {
            lpRP->iWaitingCnt = 0;
            lpRP->pPlayerWaitingAnswer = NULL;
         }
      }

      //first on look si membre est toujours en jeux
      if(lpRP->pPlayer)
      {
         //on passe tous les joueur du groupe... (le membre sera inclu car il fait partie de son groupe...
         //on valide pour le createur les regles de points...
         //1: +1 point par autre joueur inclus dans la phase et à proximité. OK (dans un range par defaut du serveur de X case (comme les local message settuper dans le serveur)
         //2: +X/10 où X est le niveau du joueur et arrondi à un entier (donc 0-9 == 0 10 a 19 == 1 20 a 29 == 2, etc etc) et si le point (1:) donne au moins 1 point
         //3: +1 si on a dit au moins une phrase en locale et en .rp durant la dernière minute et si le point (1:) donne au moins 1 point
         //4: -1 par joueur du groupe present dans la zone qui ne cause pas (oki good idea)
         //5: A TROUVER
         for(UINT pl=0; pl<lpRP->aPlayerList.GetSize();pl++)
         {
            lpPL = (sRPPl *)lpRP->aPlayerList.GetAt(pl);

            int iFlushThisPlayer        = 0;
            int iNbrAddPtsT             = 0;
            int iNbrAddPtsPjZone        = 0;
            int iNbrAddPtsRPLevel       = 0;
            int iNbrAddTalkRP           = 0;
            int iNbrRemPtsPjZoneNotTalk = 0;


            if(lpPL->pPlayer)
            {
               iFlushThisPlayer = CalculatePlayerPoints(rp,lpPL->pPlayer,iNbrAddPtsPjZone,iNbrAddPtsRPLevel,iNbrAddTalkRP,iNbrRemPtsPjZoneNotTalk);
               if(iFlushThisPlayer == 0)
               {
                  iNbrAddPtsT = iNbrAddPtsPjZone+iNbrAddPtsRPLevel+iNbrAddTalkRP-iNbrRemPtsPjZoneNotTalk;

                  int iLevelBefore = lpPL->pPlayer->self->GetRP_XPLevel();
                  lpPL->pPlayer->self->SetRP_XP(lpPL->pPlayer->self->GetRP_XP()+iNbrAddPtsT);
                  lpPL->pPlayer->self->SetFlag(__FLAG_POINTS_RP_XP_EVENTS,lpPL->pPlayer->self->ViewFlag(__FLAG_POINTS_RP_XP_EVENTS)+iNbrAddPtsT);
                  lpPL->pPlayer->self->SetFlag(__FLAG_POINTS_RP_XP_EVENTS_TOTAL,lpPL->pPlayer->self->ViewFlag(__FLAG_POINTS_RP_XP_EVENTS_TOTAL)+iNbrAddPtsT);
                  int iLevel = lpPL->pPlayer->self->GetRP_XPLevel();
                  if(iLevel >=10 && lpPL->pPlayer->self->ViewFlag(__FLAG_INTERACTION_RP) == 0)
                  {
                     lpPL->pPlayer->self->SetFlag(__FLAG_INTERACTION_RP,1);

                     CString strCC;
                     strCC = "Phase Roleplay";
                     strMsgTmp.Format(_STR(15411, lpPL->pPlayer->self->GetLang()),lpPL->pPlayer->self->GetTrueName());
                     ChatterChannels &cChatter = CPlayerManager::GetChatter();
                     cChatter.TalkSystem( "Système", (char*)strCC.GetBuffer(0), (char*)strMsgTmp.GetBuffer(0) );

                     strMsgTmp.Format(_STR(15409, lpPL->pPlayer->self->GetLang()));
                     lpPL->pPlayer->self->SendSystemMessage( strMsgTmp, CL_ORANGE );

                     _LOG_INTERRP
                        LOG_DEBUG_LVL1,
                        "Player ID:%d %s (%s)  Now Level %d and can CREATE RP Phase.",
                        lpPL->pPlayer->self->GetID(),
                        lpPL->pPlayer->self->GetTrueName(),
                        lpPL->pPlayer->GetFullAccountName(),
                        iLevel
                        LOG_
                  }
                  if(iLevelBefore == 6 && iLevel == 5 && lpPL->pPlayer->self->ViewFlag(__FLAG_INTERACTION_RP) == 1)
                  {
                     lpPL->pPlayer->self->SetFlag(__FLAG_INTERACTION_RP,0);

                     CString strCC;
                     strCC = "Phase Roleplay";

                     strMsgTmp.Format(_STR(15420, lpPL->pPlayer->self->GetLang()),lpPL->pPlayer->self->GetTrueName());
                     ChatterChannels &cChatter = CPlayerManager::GetChatter();
                     cChatter.TalkSystem( "Système", (char*)strCC.GetBuffer(0), (char*)strMsgTmp.GetBuffer(0) );


                     strMsgTmp.Format(_STR(15410, lpPL->pPlayer->self->GetLang()));
                     lpPL->pPlayer->self->SendSystemMessage( strMsgTmp, CL_ORANGE );

                     _LOG_INTERRP
                        LOG_DEBUG_LVL1,
                        "Player ID:%d %s (%s)  Now Level %d and CANT CREATE RP Phase.",
                        lpPL->pPlayer->self->GetID(),
                        lpPL->pPlayer->self->GetTrueName(),
                        lpPL->pPlayer->GetFullAccountName(),
                        iLevel
                        LOG_
                  }

                  _LOG_INTERRP
                     LOG_DEBUG_LVL1,
                     "Player ID:%d %s (%s)  Receive %d RP Points. (R1=+%d) (R2=+%d) (R3=+%d) (R4=-%d)",
                     lpPL->pPlayer->self->GetID(),
                     lpPL->pPlayer->self->GetTrueName(),
                     lpPL->pPlayer->GetFullAccountName(),
                     iNbrAddPtsT,
                     iNbrAddPtsPjZone,iNbrAddPtsRPLevel,iNbrAddTalkRP,iNbrRemPtsPjZoneNotTalk
                     LOG_

               }
            }


            if(iFlushThisPlayer == 2)//PJ PJ bloquer par un GM
            {
               _LOG_INTERRP
                  LOG_DEBUG_LVL1,
                  "Player ID:%d %s have RP Flag BLOCKEY by GM... is out of RP",
                  lpPL->pPlayer->self->GetID(),
                  lpPL->pPlayer->self->GetTrueName()
                  LOG_
            }

            if(iFlushThisPlayer > 0) 
            {
               delete lpPL;
               lpPL = NULL;
               lpRP->aPlayerList.RemoveAt(pl);
               pl--;
            }
         }


         for(UINT pl=0;pl<lpRP->aPlayerList.GetSize();pl++)
         {
            lpPL = (sRPPl *)lpRP->aPlayerList.GetAt(pl);
            if(lpPL->pPlayer)
            {
               if(lpPL->pPlayer->self->GetNMModeRPPhaseCntTalk() == 0) //si ya pas parler on step le counter de not talk
                  lpPL->pPlayer->self->StpeNMModeRPPhaseCntNOTTalk();

               lpPL->pPlayer->self->ResetNMModeRPPhaseCntTalk();
            }
         }
         rp++;
      }
   }

   //manage toutes les attentes
   sInviteRP      *lpInv;
   for(int a=0;a<c_aInviteRP.GetSize();a++)
   {
      lpInv = (sInviteRP *)c_aInviteRP.GetAt(a);
      lpInv->iCntTimeout++;
      if(lpInv->iCntTimeout > 1)
      {
         delete lpInv;
         lpInv = NULL;
         c_aInviteRP.RemoveAt(a);
         a--;
      }
   }

   return 0;
}
