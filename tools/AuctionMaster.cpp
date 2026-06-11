#include "stdafx.h"
#include "TFC Server.h"
#include "TFC_MAIN.h"
#include "AuctionMaster.h"
#include "IntlText.h"
#include "Unit.h"
#include "DynObjManager.h"
#include "PlayerManager.h"

#include "T4CLog.h"
#include "DeadlockDetector.h"
#include "CustomBuild.h"

extern CTFCServerApp theApp;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define ADD_QUERY	{ \
	LPSQL_REQUEST lpSql = new SQL_REQUEST;\
	lpSql->csQuery = strQuery;\
	lptlSQLRequests->AddToTail( lpSql );\
}


CPtrArray AuctionMaster::c_aAuctionSold;
CPtrArray AuctionMaster::c_aAuctionGive;
bool      AuctionMaster::bAuctionChanged;

static cODBCMage ODBCAuction;//Load Direct et save Threader avec MANUAL commit
static CLock     g_ALockAH;



//////////////////////////////////////////////////////////////////////////////////////////
// Creates the class
//////////////////////////////////////////////////////////////////////////////////////////
void AuctionMaster::Create( void )
{
   ODBCAuction.Connect( USERS_DSN, USERS_USER, USERS_PWD );
   ODBCAuction.ConnectOption( SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF );


   ODBCAuction.Lock();

   //load la liste des objets en ventes...
   CString csQuery;
   #ifdef BUILD_NMS_CUSTOM_NPC
      csQuery.Format( "SELECT IndexID, OwnerID, VendeurName, ObjType, ObjName, Qty, EquipPos, MadeBy, BuyItNow, MinimumBid, TimeEnter, TimeMax, CurrentBid, BidOwnerID, Charge, BuyItNowNMS FROM AuctionSold");
   #else
      csQuery.Format( "SELECT IndexID, OwnerID, VendeurName, ObjType, ObjName, Qty, EquipPos, MadeBy, BuyItNow, MinimumBid, TimeEnter, TimeMax, CurrentBid, BidOwnerID, Charge FROM AuctionSold");
   #endif
   ODBCAuction.SendRequest( (LPCTSTR)csQuery );

   int iCnt  = 0;
   while( ODBCAuction.Fetch())
   {
      sAuctionMasterSold *pNewSold = new sAuctionMasterSold();

		char lpszVendeurName [50];
      char lpszObjType     [50];
      char lpszObjName     [255];
      char lpszMadeBy      [50];

      memset(lpszVendeurName,0x00,50);
      memset(lpszObjType    ,0x00,50);
      memset(lpszObjName    ,0x00,255);
      memset(lpszMadeBy     ,0x00,50);

      ODBCAuction.GetDWORD  ( 1,	&pNewSold->dwIndex);
      ODBCAuction.GetDWORD  ( 2,	&pNewSold->OwnerID);
      ODBCAuction.GetString ( 3,	lpszVendeurName,50);
      ODBCAuction.GetString ( 4,	lpszObjType,50);
      ODBCAuction.GetString ( 5,	lpszObjName,255);
      ODBCAuction.GetDWORD  ( 6,	&pNewSold->Qty);
      ODBCAuction.GetDWORD  ( 7,	&pNewSold->EquipPos);
      ODBCAuction.GetString ( 8,	lpszMadeBy,50);
      ODBCAuction.GetDWORD  ( 9,	&pNewSold->BuyItNow);
      ODBCAuction.GetDWORD  (10,	&pNewSold->MinimumBid);
      ODBCAuction.GetDWORD  (11,	&pNewSold->TimeEnter);
      ODBCAuction.GetDWORD  (12,	&pNewSold->TimeMax);
      ODBCAuction.GetDWORD  (13,	&pNewSold->CurrentBid);
      ODBCAuction.GetDWORD  (14,	&pNewSold->BidOwnerID);
      ODBCAuction.GetSDWORD (15,	&pNewSold->lCharge);
      #ifdef BUILD_NMS_CUSTOM_NPC
         ODBCAuction.GetDWORD (16,	&pNewSold->BuyItNowNMS);
      #else
         pNewSold->BuyItNowNMS = 0;
      #endif

      pNewSold->VendeurName.Format("%s",lpszVendeurName);
      pNewSold->ObjType    .Format("%s",lpszObjType);
      pNewSold->ObjName    .Format("%s",lpszObjName);
      pNewSold->MadeBy     .Format("%s",lpszMadeBy);
      pNewSold->ObjName.Replace("''","'");

      if(pNewSold->lCharge == 0)
         pNewSold->lCharge = -1;

		c_aAuctionSold.SetAtGrow(iCnt, pNewSold );
      iCnt++;
	}

   ODBCAuction.Cancel();

   //load la liste des objets /or a remettre
   #ifdef BUILD_NMS_CUSTOM_NPC
      csQuery.Format( "SELECT IndexID, AuctionStatus, OwnerID, ObjType, ObjName, Qty, MadeBy, Gold, Charge, NMSEcu  FROM AuctionGive");
   #else
      csQuery.Format( "SELECT IndexID, AuctionStatus, OwnerID, ObjType, ObjName, Qty, MadeBy, Gold, Charge  FROM AuctionGive");
   #endif
   
   ODBCAuction.SendRequest( (LPCTSTR)csQuery );
   
   iCnt  = 0;
   while( ODBCAuction.Fetch())
   {
      sAuctionMasterGive *pNewGive = new sAuctionMasterGive();

      char lpszObjType     [50];
      char lpszObjName     [255];
      char lpszMadeBy      [50];
      
      memset(lpszObjType    ,0x00,50);
      memset(lpszObjName    ,0x00,255);
      memset(lpszMadeBy     ,0x00,50);


      ODBCAuction.GetDWORD  ( 1,	&pNewGive->dwIndex);
      ODBCAuction.GetDWORD  ( 2,	&pNewGive->AuctionStatus);
      ODBCAuction.GetDWORD  ( 3,	&pNewGive->OwnerID);
      ODBCAuction.GetString ( 4,	lpszObjType,50);
      ODBCAuction.GetString ( 5,	lpszObjName,255);
      ODBCAuction.GetDWORD  ( 6,	&pNewGive->Qty);
      ODBCAuction.GetString ( 7,	lpszMadeBy,50);
      ODBCAuction.GetDWORD  ( 8,	&pNewGive->Gold);
      ODBCAuction.GetSDWORD ( 9,	&pNewGive->lCharge);
      #ifdef BUILD_NMS_CUSTOM_NPC
         ODBCAuction.GetDWORD (10,	&pNewGive->NMSEcu);
      #else
         pNewGive->NMSEcu = 0;
      #endif

      pNewGive->ObjType    .Format("%s",lpszObjType);
      pNewGive->ObjName    .Format("%s",lpszObjName);
      pNewGive->MadeBy     .Format("%s",lpszMadeBy);

      pNewGive->ObjName.Replace("''","'");

      if(pNewGive->lCharge == 0)
         pNewGive->lCharge = -1;
      c_aAuctionGive.SetAtGrow(iCnt, pNewGive );
      iCnt++;
   }
   
   ODBCAuction.Cancel();
	ODBCAuction.Unlock();

   bAuctionChanged = false;

   printf( "\n  Found %d Item(s) in Sold list",c_aAuctionSold.GetSize() );
   printf( "\n  Found %d Item(s) in Give list",c_aAuctionGive.GetSize() );
}


//////////////////////////////////////////////////////////////////////////////////////////
void AuctionMaster::Destroy( void )
{
	sAuctionMasterSold   *lpS;
   sAuctionMasterGive   *lpG;
	int i;
	
   for(i = 0; i < c_aAuctionSold.GetSize(); i++)
   {
      lpS = (sAuctionMasterSold *)c_aAuctionSold.GetAt(i);
      if(lpS)
      {
         delete lpS;
         lpS = NULL;
      }
   }

   for(i = 0; i < c_aAuctionGive.GetSize(); i++)
   {
      lpG = (sAuctionMasterGive *)c_aAuctionGive.GetAt(i);
      if(lpG)
      {
         delete lpG;
         lpG = NULL;
      }
   }
 

   ODBCAuction.Disconnect( );
}

void AuctionMaster::SaveAll(bool bForce) {
	CAutoLock autoAHLock( &g_ALockAH );

   if(bForce || bAuctionChanged)
   {
      TemplateList< SQL_REQUEST > *lptlSQLRequests = new TemplateList< SQL_REQUEST >;

      _DeleteAll(lptlSQLRequests);

      for(int i = 0; i < c_aAuctionSold.GetSize(); i++) {
         sAuctionMasterSold *auction = (sAuctionMasterSold *)c_aAuctionSold.GetAt(i);
         _SaveAuction(auction, lptlSQLRequests);
      }

      for(int i = 0; i < c_aAuctionGive.GetSize(); i++) {
         sAuctionMasterGive *give_list = (sAuctionMasterGive *)c_aAuctionGive.GetAt(i);
         _SaveGiveList(give_list, lptlSQLRequests);
      }

      ODBCAuction.SendBatchRequest( lptlSQLRequests, NULL, NULL, "ODBCAuction" );   

      bAuctionChanged = false;
   }

	
}

void AuctionMaster::_DeleteAll(TemplateList< SQL_REQUEST > *lptlSQLRequests) {
	CString strQuery;

	strQuery.Format( "DELETE FROM AuctionSold");
	ADD_QUERY;
	strQuery.Format( "DELETE FROM AuctionGive");
	ADD_QUERY;
}

void AuctionMaster::_SaveAuction(sAuctionMasterSold* auction, TemplateList< SQL_REQUEST > *lptlSQLRequests) {
	CString strQuery, seller_name(auction->VendeurName), obj_type(auction->ObjType), 
		obj_name(auction->ObjName), made_by(auction->MadeBy);

	seller_name.Replace("'","''");
	obj_type.Replace("'","''");
	obj_name.Replace("'","''");
	made_by.Replace("'","''");

   #ifdef BUILD_NMS_CUSTOM_NPC
      strQuery.Format(
         "INSERT INTO AuctionSold"
         "(IndexID,OwnerID,VendeurName,ObjType,ObjName,Qty,EquipPos,MadeBy,BuyItNow,MinimumBid,TimeEnter,TimeMax,CurrentBid,BidOwnerID,Charge,BuyItNowNMS) "
         "VALUES (%d,%d,'%s','%s','%s',%d,%d,'%s',%d,%d,%d,%d,%d,%d,%d,%d)",
         auction->dwIndex,
         auction->OwnerID,
         seller_name,
         obj_type,
         obj_name,
         auction->Qty,
         auction->EquipPos,
         made_by,
         auction->BuyItNow,
         auction->MinimumBid,
         auction->TimeEnter,
         auction->TimeMax,
         auction->CurrentBid,
         auction->BidOwnerID,
         auction->lCharge,
         auction->BuyItNowNMS
         );
   #else
      strQuery.Format(
         "INSERT INTO AuctionSold"
         "(IndexID,OwnerID,VendeurName,ObjType,ObjName,Qty,EquipPos,MadeBy,BuyItNow,MinimumBid,TimeEnter,TimeMax,CurrentBid,BidOwnerID,Charge) "
         "VALUES (%d,%d,'%s','%s','%s',%d,%d,'%s',%d,%d,%d,%d,%d,%d,%d)",
         auction->dwIndex,
         auction->OwnerID,
         seller_name,
         obj_type,
         obj_name,
         auction->Qty,
         auction->EquipPos,
         made_by,
         auction->BuyItNow,
         auction->MinimumBid,
         auction->TimeEnter,
         auction->TimeMax,
         auction->CurrentBid,
         auction->BidOwnerID,
         auction->lCharge
         );
   #endif
	ADD_QUERY;
}

void AuctionMaster::_SaveGiveList(sAuctionMasterGive* give_list, TemplateList< SQL_REQUEST > *lptlSQLRequests) {
	CString strQuery, obj_type(give_list->ObjType), obj_name(give_list->ObjName), made_by(give_list->MadeBy);

	obj_type.Replace("'","''");
	obj_name.Replace("'","''");
	made_by.Replace("'","''");

   #ifdef BUILD_NMS_CUSTOM_NPC
      strQuery.Format(
         "INSERT INTO AuctionGive"
         "(IndexID,AuctionStatus,OwnerID,ObjType,ObjName,Qty,MadeBy,Gold,Charge,NMSEcu) "
         "VALUES (%d,%d,%d,'%s','%s',%d,'%s',%d,%d,%d)",
         give_list->dwIndex,
         give_list->AuctionStatus,
         give_list->OwnerID,
         obj_type,
         obj_name,
         give_list->Qty,
         made_by,
         give_list->Gold,
         give_list->lCharge,
         give_list->NMSEcu
         );
   #else
      strQuery.Format(
         "INSERT INTO AuctionGive"
         "(IndexID,AuctionStatus,OwnerID,ObjType,ObjName,Qty,MadeBy,Gold,Charge) "
         "VALUES (%d,%d,%d,'%s','%s',%d,'%s',%d,%d)",
         give_list->dwIndex,
         give_list->AuctionStatus,
         give_list->OwnerID,
         obj_type,
         obj_name,
         give_list->Qty,
         made_by,
         give_list->Gold,
         give_list->lCharge
         );
   #endif
	ADD_QUERY;
}


int AuctionMaster::GetNbrSold() { return c_aAuctionSold.GetSize(); }
int AuctionMaster::GetNbrGive() { return c_aAuctionGive.GetSize(); }

bool AuctionMaster::IsCanAddItems(Character *pUser) {
   CAutoLock autoAHLock( &g_ALockAH );
   int iNbrItemSold = 0;
   for(int i = 0; i < c_aAuctionSold.GetSize(); i++) {
      sAuctionMasterSold *lpS = (sAuctionMasterSold *)c_aAuctionSold.GetAt(i);
      if(lpS->OwnerID == pUser->GetID()) {
         iNbrItemSold++;
         if(iNbrItemSold >= theApp.dwAHSystemMaxSold) {
            return false; //User cant sold item he already have get the maximum number of items...
         }
      }
   }
   return true;
}

bool AuctionMaster::IsAuctionChanged()
{
   CAutoLock autoAHLock( &g_ALockAH );
   return bAuctionChanged;
}


int AuctionMaster::AddSoldItem(Players *pPlayer, CString strObjType, CString strObjName, DWORD dwQty, DWORD dwEquipPos,
                               CString strMadeBy,DWORD dwBuyNow,DWORD dwBuyNowNMS, DWORD dwBid, DWORD dwTimeMax, long lCharge)
{
   CAutoLock autoAHLock( &g_ALockAH );

   time_t tCurTime =  time(NULL);

   if(dwTimeMax < 1)
      dwTimeMax = 1;
   else if(dwTimeMax >5)
      dwTimeMax = 5;

   //now fill all sold data info...
   sAuctionMasterSold *lpS = new sAuctionMasterSold();

   lpS->dwIndex        = GetValidSoldID();
   lpS->OwnerID        = pPlayer->self->GetID();
   lpS->VendeurName    = pPlayer->self->GetTrueName();
   lpS->ObjType        = strObjType;
   lpS->ObjName        = strObjName;
   lpS->Qty            = dwQty;
   lpS->EquipPos       = dwEquipPos;
   lpS->MadeBy         = strMadeBy;
   lpS->BuyItNow       = dwBuyNow;
   lpS->BuyItNowNMS    = dwBuyNowNMS;
   lpS->MinimumBid     = dwBid;
   lpS->TimeEnter      = tCurTime;
   lpS->TimeMax        = tCurTime + (dwTimeMax*86400);//now + XDays...
   lpS->CurrentBid     = 0;
   lpS->BidOwnerID     = 0;
   lpS->lCharge        = lCharge;

   c_aAuctionSold.Add(lpS);

   CString strMessage;
   if(dwBuyNow >0 && dwBid >0) 
   {
      if(dwBuyNowNMS > 0)
         strMessage.Format(_STR( 15503, pPlayer->self->GetLang() ),dwQty,strObjName,dwBuyNow,dwBuyNowNMS,dwBid,dwTimeMax);
      else
         strMessage.Format(_STR( 15127, pPlayer->self->GetLang() ),dwQty,strObjName,dwBuyNow,dwBid,dwTimeMax);
   } 
   else if(dwBuyNow >0 || dwBuyNowNMS > 0) 
   {
      if(dwBuyNowNMS > 0)
         strMessage.Format(_STR( 15504, pPlayer->self->GetLang() ),dwQty,strObjName,dwBuyNow,dwBuyNowNMS,dwTimeMax);
      else
         strMessage.Format(_STR( 15128, pPlayer->self->GetLang() ),dwQty,strObjName,dwBuyNow,dwTimeMax);
   } 
   else 
   {
        strMessage.Format(_STR( 15129, pPlayer->self->GetLang() ),dwQty,strObjName,dwBid,dwTimeMax);
   }
   
   theApp.AddAHRequest(NULL,NULL,NULL,AH_REQ_GET_LIST,pPlayer->self->GetID(),0,0,0,0,0,0,"","","",0);

   pPlayer->self->SendInfoMessage(strMessage,0x0080FF);

   _LOG_AH
      LOG_MISC_1,
      "Player %s [%s] Put in AuctionHouse %u item %s Charge( %d ) ID( %s ) Buyout: %d  Buyout ecu: %d   Bid min: %d",
      pPlayer->self->GetTrueName(),
      pPlayer->GetFullAccountName(),
      dwQty,
      strObjName,
      lCharge,
      strObjType,dwBuyNow,dwBuyNowNMS,dwBid
   LOG_

   bAuctionChanged = true; //Add Sold Item

   return 0;
}

int AuctionMaster::_GetAuctionArrayPosition(DWORD auction_id) {
	for(int i = 0; i < c_aAuctionSold.GetSize(); i++) {
		sAuctionMasterSold   *lpS = (sAuctionMasterSold *)c_aAuctionSold.GetAt(i);
		if(lpS->dwIndex == auction_id) {
			return i;
		}
	}
	return -1;
}
sAuctionMasterSold* AuctionMaster::GetAuction(DWORD auction_id) {
	int position = _GetAuctionArrayPosition(auction_id);
	if (position == -1) {
		return NULL;
	}
	return (sAuctionMasterSold *)c_aAuctionSold.GetAt(position);
}
void AuctionMaster::_RemoveAuction(DWORD auction_id) {
	int position = _GetAuctionArrayPosition(auction_id);
	if (position == -1) return;
	c_aAuctionSold.RemoveAt(position);
}

int AuctionMaster::BuySoldItem(Character *pUser, DWORD dwIndex, DWORD dwBuy,DWORD dwPrix, DWORD dwPrixNMS, DWORD dwTS) 
{
   CAutoLock autoAHLock( &g_ALockAH );

   sAuctionMasterSold   *lpS = GetAuction(dwIndex);

   if(lpS != NULL) 
   {
      if(lpS->TimeEnter == dwTS) 
      {
         if(dwBuy == 0) 
         {
            if(lpS->MinimumBid > 0)
            {
               /** This is a bid **/
               if(lpS->OwnerID != pUser->GetID()) 
               {
                  if(dwPrix > lpS->CurrentBid && dwPrix >= lpS->MinimumBid)
                  { 
                     //User has enough gold to bid?
                     if(pUser->GetGold() >dwPrix) 
                     {
                        //Take the gold
                        pUser->SetGold(pUser->GetGold()-dwPrix,FALSE);

                        //If there was already a (lower) bid, give that gold back.
                        if(lpS->CurrentBid >0) 
                        {
                           sAuctionMasterGive   *lpG = new sAuctionMasterGive;
                           lpG->dwIndex       = GetValidGiveID();
                           lpG->AuctionStatus = AUCTION_STATUS_OVERBID;
                           lpG->OwnerID       = lpS->BidOwnerID;
                           lpG->ObjType       = lpS->ObjType;
                           lpG->ObjName       = lpS->ObjName;
                           lpG->Qty           = lpS->Qty;
                           lpG->MadeBy        = "";
                           lpG->Gold          = lpS->CurrentBid;
                           lpG->lCharge       = lpS->lCharge;
                           lpG->NMSEcu        = 0;

                           c_aAuctionGive.Add(lpG);
                        }

                        //Register the new bid!
                        lpS->CurrentBid = dwPrix;
                        lpS->BidOwnerID = pUser->GetID();

                        //Notify user
                        CString strTmp;
                        strTmp.Format(_STR( 15136, pUser->GetLang() ),lpS->CurrentBid,lpS->Qty,lpS->ObjName);
                        pUser->SendInfoMessage(strTmp,0x0080FF);

                        //Refresh user's AH listing.
                        theApp.AddAHRequest(NULL,NULL,NULL,AH_REQ_GET_LIST,pUser->GetID(),0,0,0,0,0,0,"","","",0);
                        bAuctionChanged = true; //Buy SoldItem Buy 

                        _LOG_AH
                           LOG_MISC_1,
                           "Player %s bid %d gold in AuctionHouse for item %s ID( %s )",
                           pUser->GetTrueName(),dwPrix,
                           lpS->ObjName,lpS->ObjType
                           LOG_
                     } 
                     else 
                     {
                        pUser->SendInfoMessage(_STR( 15144, pUser->GetLang() ),0x0080FF);   
                     }
                  } 
                  else 
                  {
                     pUser->SendInfoMessage(_STR( 15135, pUser->GetLang() ),0x0080FF);   
                  }
               } 
               else 
               {
                  pUser->SendInfoMessage(_STR( 15134, pUser->GetLang() ),0x0080FF);   
               }
            }
            else 
            {
               //pUser->SendInfoMessage(_STR( 15134, pUser->GetLang() ),0x0080FF);   
            }
         } 
         else 
         {
            /** Buy Now! GOLD**/
            if(lpS->BuyItNow > 0 && dwPrix > 0)
            {
               if(lpS->OwnerID != pUser->GetID()) 
               {
                  //User has enough gold ?
                  if(pUser->GetGold() >=lpS->BuyItNow)
                  { 
                     //Take the gold
                     pUser->SetGold(pUser->GetGold()-lpS->BuyItNow,FALSE);

                     //If there was already a bid, give that gold back.
                     if(lpS->CurrentBid >0) 
                     {
                        sAuctionMasterGive   *lpG = new sAuctionMasterGive;
                        lpG->dwIndex       = GetValidGiveID();
                        lpG->AuctionStatus = AUCTION_STATUS_OVERBID;
                        lpG->OwnerID       = lpS->BidOwnerID;
                        lpG->ObjType       = lpS->ObjType;
                        lpG->ObjName       = lpS->ObjName;
                        lpG->Qty           = lpS->Qty;
                        lpG->MadeBy        = "";
                        lpG->Gold          = lpS->CurrentBid;
                        lpG->lCharge       = lpS->lCharge;
                        lpG->NMSEcu        = 0;

                        c_aAuctionGive.Add(lpG);
                     }

                     //Put the seller's gold on the Give List
                     sAuctionMasterGive   *lpGO = new sAuctionMasterGive;
                     lpGO->dwIndex       = GetValidGiveID();
                     lpGO->AuctionStatus = AUCTION_STATUS_SOLD_NOW;
                     lpGO->OwnerID       = lpS->OwnerID;
                     lpGO->ObjType       = lpS->ObjType;
                     lpGO->ObjName       = lpS->ObjName;
                     lpGO->Qty           = lpS->Qty;
                     lpGO->MadeBy        = "";
                     lpGO->Gold          = lpS->BuyItNow;
                     lpGO->lCharge       = lpS->lCharge;
                     lpGO->NMSEcu        = 0;

                     c_aAuctionGive.Add(lpGO);

                     //Put the buyer's item on the Give List
                     sAuctionMasterGive   *lpGI = new sAuctionMasterGive;
                     lpGI->dwIndex       = GetValidGiveID();
                     lpGI->AuctionStatus = AUCTION_STATUS_SOLD_NOW;
                     lpGI->OwnerID       = pUser->GetID();
                     lpGI->ObjType       = lpS->ObjType;
                     lpGI->ObjName       = lpS->ObjName;
                     lpGI->Qty           = lpS->Qty;
                     lpGI->MadeBy        = lpS->MadeBy;
                     lpGI->Gold          = 0;
                     lpGI->lCharge       = lpS->lCharge;
                     lpGI->NMSEcu        = 0;

                     c_aAuctionGive.Add(lpGI);


                     DWORD dwPrixTmp = lpS->BuyItNow;

                     // this Auction is over
                     _RemoveAuction(dwIndex);
                     delete lpS;

                     //Notify user
                     CString strTmp;
                     strTmp.Format(_STR( 15138 , pUser->GetLang() ),lpGI->Qty,lpGI->ObjName,dwPrixTmp);
                     pUser->SendInfoMessage(strTmp,0x0080FF);

                     //Refresh user's AH listing.
                     theApp.AddAHRequest(NULL,NULL,NULL,AH_REQ_GET_LIST,pUser->GetID(),0,0,0,0,0,0,"","","",0);

                     bAuctionChanged = true; //Buy SoldItem Buy Now
                     _LOG_AH
                        LOG_MISC_1,
                        "Player %s [%s] buyout in AuctionHouse item %s ID( %s ) for %d golds",
                        pUser->GetTrueName(),
                        pUser->GetPlayer()->GetFullAccountName(),
                        lpGI->ObjName,lpGI->ObjType,dwPrixTmp
                        LOG_
                  } 
                  else 
                  {
                     pUser->SendInfoMessage(_STR( 15137, pUser->GetLang() ),0x0080FF);   
                  }
               } 
               else 
               {
                  pUser->SendInfoMessage(_STR( 15134, pUser->GetLang() ),0x0080FF);   
               }
            }
            /** Buy Now! ECU**/
            else if(lpS->BuyItNowNMS > 0 && dwPrixNMS > 0)
            {
               if(lpS->OwnerID != pUser->GetID()) 
               {
                  //User has enough gold ?
                  if(pUser->GetPlayer()->GetNMSGold() >=lpS->BuyItNowNMS)
                  { 
                     //Take the NMSGold
                     pUser->GetPlayer()->SetNMSGold(pUser->GetPlayer()->GetNMSGold() - lpS->BuyItNowNMS);
                     pUser->GetPlayer()->SaveAccount();
                     
                     //If there was already a bid, give that gold back.
                     if(lpS->CurrentBid >0) 
                     {
                        sAuctionMasterGive   *lpG = new sAuctionMasterGive;
                        lpG->dwIndex       = GetValidGiveID();
                        lpG->AuctionStatus = AUCTION_STATUS_OVERBID;
                        lpG->OwnerID       = lpS->BidOwnerID;
                        lpG->ObjType       = lpS->ObjType;
                        lpG->ObjName       = lpS->ObjName;
                        lpG->Qty           = lpS->Qty;
                        lpG->MadeBy        = "";
                        lpG->Gold          = lpS->CurrentBid;
                        lpG->lCharge       = lpS->lCharge;
                        lpG->NMSEcu        = 0;

                        c_aAuctionGive.Add(lpG);
                     }

                     //Put the seller's NMSGold on the Give List
                     sAuctionMasterGive   *lpGO = new sAuctionMasterGive;
                     lpGO->dwIndex       = GetValidGiveID();
                     lpGO->AuctionStatus = AUCTION_STATUS_SOLD_NOW;
                     lpGO->OwnerID       = lpS->OwnerID;
                     lpGO->ObjType       = lpS->ObjType;
                     lpGO->ObjName       = lpS->ObjName;
                     lpGO->Qty           = lpS->Qty;
                     lpGO->MadeBy        = "";
                     lpGO->Gold          = 0;
                     lpGO->lCharge       = lpS->lCharge;
                     lpGO->NMSEcu        = lpS->BuyItNowNMS;

                     c_aAuctionGive.Add(lpGO);

                     //Put the buyer's item on the Give List
                     sAuctionMasterGive   *lpGI = new sAuctionMasterGive;
                     lpGI->dwIndex       = GetValidGiveID();
                     lpGI->AuctionStatus = AUCTION_STATUS_SOLD_NOW;
                     lpGI->OwnerID       = pUser->GetID();
                     lpGI->ObjType       = lpS->ObjType;
                     lpGI->ObjName       = lpS->ObjName;
                     lpGI->Qty           = lpS->Qty;
                     lpGI->MadeBy        = lpS->MadeBy;
                     lpGI->Gold          = 0;
                     lpGI->lCharge       = lpS->lCharge;
                     lpGI->NMSEcu        = 0;

                     c_aAuctionGive.Add(lpGI);


                     DWORD dwPrixTmp = lpS->BuyItNowNMS;

                     // this Auction is over
                     _RemoveAuction(dwIndex);
                     delete lpS;

                     //Notify user
                     CString strTmp;
                     strTmp.Format(_STR( 15506 , pUser->GetLang() ),lpGI->Qty,lpGI->ObjName,dwPrixTmp);
                     pUser->SendInfoMessage(strTmp,0x0080FF);

                     //Refresh user's AH listing.
                     theApp.AddAHRequest(NULL,NULL,NULL,AH_REQ_GET_LIST,pUser->GetID(),0,0,0,0,0,0,"","","",0);

                     bAuctionChanged = true; //Buy SoldItem Buy Now
                     _LOG_AH
                        LOG_MISC_1,
                        "Player %s [%s] buyout in AuctionHouse item %s ID( %s ) for %d ecu(s)",
                        pUser->GetTrueName(),
                        pUser->GetPlayer()->GetFullAccountName(),
                        lpGI->ObjName,lpGI->ObjType,dwPrixTmp
                        LOG_
                  } 
                  else 
                  {
                     pUser->SendInfoMessage(_STR( 15507, pUser->GetLang() ),0x0080FF);   
                  }
               } 
               else 
               {
                  pUser->SendInfoMessage(_STR( 15134, pUser->GetLang() ),0x0080FF);   
               }
            }
            else 
            {
               pUser->SendInfoMessage(_STR( 14999, pUser->GetLang() ),0x0080FF);   
            }
         }
      } 
      else 
      {
         pUser->SendInfoMessage(_STR( 15231, pUser->GetLang() ),0x0080FF);
      }
   } 
   else 
   {
      pUser->SendInfoMessage(_STR( 15131, pUser->GetLang() ),0x0080FF);
   }

   return 0;
}

int  AuctionMaster::CancelSoldItem(Character *pUser, DWORD dwIndex, DWORD dwTS) {
   CAutoLock autoAHLock( &g_ALockAH );
   //Get the auction
   sAuctionMasterSold   *lpS = GetAuction(dwIndex);

   if(lpS != NULL) {
      if(lpS->OwnerID == pUser->GetID()) {
         if(lpS->TimeEnter == dwTS) {
            if(lpS->CurrentBid >0) {
               //Someone bidded on that item, give their gold back.
               sAuctionMasterGive   *lpGB = new sAuctionMasterGive;
               lpGB->dwIndex       = GetValidGiveID();
               lpGB->AuctionStatus = AUCTION_STATUS_CANCEL;
               lpGB->OwnerID       = lpS->BidOwnerID;
               lpGB->ObjType       = lpS->ObjType;
               lpGB->ObjName       = lpS->ObjName;
               lpGB->Qty           = lpS->Qty;
               lpGB->MadeBy        = "";
               lpGB->Gold          = lpS->CurrentBid;
               lpGB->lCharge       = lpS->lCharge;
               lpGB->NMSEcu        = 0;

               
               c_aAuctionGive.Add(lpGB);
            }

            //Return the item to the user
            sAuctionMasterGive   *lpGI = new sAuctionMasterGive;
            lpGI->dwIndex       = GetValidGiveID();
            lpGI->AuctionStatus = AUCTION_STATUS_CANCEL;
            lpGI->OwnerID       = lpS->OwnerID;
            lpGI->ObjType       = lpS->ObjType;
            lpGI->ObjName       = lpS->ObjName;
            lpGI->Qty           = lpS->Qty;
            lpGI->MadeBy        = lpS->MadeBy;
            lpGI->Gold          = 0;
            lpGI->lCharge       = lpS->lCharge;
            lpGI->NMSEcu        = 0;

            c_aAuctionGive.Add(lpGI);

            //This auction is over
				_RemoveAuction(dwIndex);
            delete lpS;

            //Notify user
						CString strTmp;
            strTmp.Format(_STR( 15133, pUser->GetLang() ),lpGI->Qty,lpGI->ObjName);
            pUser->SendInfoMessage(strTmp,0x0080FF);

						//Refresh user's AH listing
            theApp.AddAHRequest(NULL,NULL,NULL,AH_REQ_GET_LIST,pUser->GetID(),0,0,0,0,0,0,"","","",0);
            bAuctionChanged = true; //CancelSoldItem
         } else {
            pUser->SendInfoMessage(_STR( 15232, pUser->GetLang() ),0x0080FF);
         }
      } else {
         pUser->SendInfoMessage(_STR( 15132, pUser->GetLang() ),0x0080FF);
      }
   } else {
      pUser->SendInfoMessage(_STR( 15131, pUser->GetLang() ),0x0080FF);
   }
   return 0;
}

int  AuctionMaster::InfoSoldItem(Character *pUser, DWORD dwIndex) {
   CAutoLock autoAHLock( &g_ALockAH );
   //On dois trouver l<item dans la liste des itema  vendre...
   sAuctionMasterSold   *lpS = GetAuction(dwIndex);

   if(lpS != NULL) {
      //Si mod cette fonction pas oublier de mod celle dans QueryIteminfo...
      //TAG_ITEM_INFO
      TFCPacket sending;
      sending << (RQ_SIZE)RQ_QueryItemInfo;

      DWORD dwID = Unit::GetIDFromName( lpS->ObjType, U_OBJECT, TRUE );
      Objects *lpItem = NULL;
      bool bCreated = false;
      if(dwID != 0) {
         lpItem = new Objects;
         if( lpItem->Create( U_OBJECT, dwID ) ) {
            bCreated = true;
         }
      }

      if (lpItem && bCreated) {
         if (lpItem->GetType() == U_OBJECT) {
            sending << (char)0;
            sending << lpItem->GetName(pUser->GetLang());
            if(theApp.dwDisableItemInfo == 0) { //ItemInfo is enabled...
               _item *item;
               lpItem->SendUnitMessage(MSG_OnGetUnitStructure,pUser,0,0,0,&item);
               sending << (short)item->appearance;
               sending << (char)item->cRadiance;
               sending << (short)item->armor.AC;
               sending << (short)item->armor.Dod;
               sending << (short)item->armor.End;
               sending << (long)(item->weapon.cDamage.GetMinBoost(pUser));
               sending << (long)(item->weapon.cDamage.GetMaxBoost(pUser));
               sending << (short)item->weapon.Att;
               sending << (short)item->weapon.Str;
               sending << (short)item->weapon.Agi;
               sending << (short)item->magic.nMinWis;
               sending << (short)item->magic.nMinInt;
               WORD plInt = pUser->GetTrueINT();
               WORD plWis = pUser->GetTrueWIS();
               sending << (short)item->tlBoosts.NbObjects();
               item->tlBoosts.ToHead();
               while (item->tlBoosts.QueryNext()) {
                  LPOBJECT_BOOST lpBoost = item->tlBoosts.Object();
                  if (lpBoost->wStat > 10000 /* SkillBoostOffset */) {
                     switch(lpBoost->wStat) {
                     case 10001: // Stun Blow
                        sending << (char)27;
                        break;
                     case 10002: // Powerful Blow
                        sending << (char)28;
                        break;
                     case 10004: // First Aid
                        sending << (char)29;
                        break;
                     case 10008: // Parry
                        sending << (char)30;
                        break;
                     case 10009: // Meditate
                        sending << (char)31;
                        break;
                     case 10011: // Dodge
                        sending << (char)32;
                        break;
                     case 10012: // Attack
                        sending << (char)33;
                        break;
                     case 10014: // Hide
                        sending << (char)34;
                        break;
                     case 10015: // Rob
                        sending << (char)35;
                        break;
                     case 10016: // Sneak
                        sending << (char)36;
                        break;
                     case 10017: // Search
                        sending << (char)37;
                        break;
                     case 10026: // Picklock
                        sending << (char)38;
                        break;
                     case 10027: // Armor Penetration
                        sending << (char)39;
                        break;
                     case 10028: // Peek
                        sending << (char)40;
                        break;
                     case 10029: // Rapid Healing
                        sending << (char)41;
                        break;
                     case 10035: // Archery
                        sending << (char)42;
                        break;
                     case 10036: // Dual Weapons
                        sending << (char)43;
                        break;
                     case 10037: // plunder
                        sending << (char)44;
                        break;
                     default: // Invalid or unused skill
                        sending << (char)0;
                     }
                  } else {
                     sending << (char)lpBoost->wStat;
                  }
                  if ((lpBoost->nMinWIS <= plWis) && (lpBoost->nMinINT <= plInt)) {
                     sending << (long)(lpBoost->bfBoost.GetMinBoost(pUser));
                     sending << (long)(lpBoost->bfBoost.GetMaxBoost(pUser));
                  } else {
                     sending << (long)0;
                     sending << (long)0;
                  }
               }
            } else { //Blank Window
               sending << (short)0;
               sending << (char)0;
               sending << (short)0;
               sending << (short)0;
               sending << (short)0;
               sending << (long)0;
               sending << (long)0;
               sending << (short)0;
               sending << (short)0;
               sending << (short)0;
               sending << (short)0;
               sending << (short)0;
               sending << (short)0;
            }
         } else {
            sending << (char)2;
         }
         pUser->SendPlayerMessage(sending);
      }
      if(lpItem) {
         lpItem->DeleteUnit();
      }
      lpItem = NULL;
   } else {
      pUser->SendInfoMessage(_STR( 15131, pUser->GetLang() ),0x0080FF);
   }
   return 0;
}


int  AuctionMaster::AddMoneyGiveList(Character *pUser, int iMontant) {
   CAutoLock autoAHLock( &g_ALockAH );

   if(theApp.dwAHSystemEnable == 0) {
      return 0;
   }

   sAuctionMasterGive   *lpGO = new sAuctionMasterGive;
   lpGO->dwIndex       = GetValidGiveID();
   lpGO->AuctionStatus = AUCTION_STATUS_BANK_GIVE;
   lpGO->OwnerID       = pUser->GetID();
   lpGO->ObjType       = "None";
   lpGO->ObjName       = "None";
   lpGO->Qty           = 0;
   lpGO->MadeBy        = "";
   lpGO->Gold          = iMontant;
   lpGO->lCharge       = 1;
   lpGO->NMSEcu        = 0;
   
   c_aAuctionGive.Add(lpGO);

   bAuctionChanged = true; //AddMoneyGiveList
   return 0;
}

int  AuctionMaster::ForceExpireAllItems(Character *pUser) {
   CAutoLock autoAHLock( &g_ALockAH );

	 //Set the expiration time somewhere in the past and let
	 //the maintenance thread take care of it.
   for(int i = 0; i < c_aAuctionSold.GetSize(); i++) {
      sAuctionMasterSold *lpS = (sAuctionMasterSold *)c_aAuctionSold.GetAt(i);
      lpS->TimeMax = 0;
   }
   pUser->SendInfoMessage("ALL Auction items will expire now....");

   _LOG_AH
      LOG_MISC_1,
      "Player %s [%d]Force a EXPIRE ALL",
      pUser->GetTrueName(),
      pUser->GetID()
   LOG_

   bAuctionChanged = true; //ForceExpireAllItems
   return 0;
}


int  AuctionMaster::ManageSoldList() {
   CAutoLock autoAHLock( &g_ALockAH );
   time_t tCurTime =  time(NULL);

   for(int i = 0; i < c_aAuctionSold.GetSize(); i++) {
      sAuctionMasterSold *lpS = (sAuctionMasterSold *)c_aAuctionSold.GetAt(i);
      if(lpS->TimeMax < tCurTime) 
      {
         //On est expirer...
         //on dois valider si on a vendu ou pas...
         if(lpS->CurrentBid ==0) 
         {
            //on est expirer et ce n<etais pas une vente au enchere ou personne a miser...
            //on dois virer ce sold et redonner l<items au vendeur...

            sAuctionMasterGive   *lpG = new sAuctionMasterGive;
            lpG->dwIndex       = GetValidGiveID();
            lpG->AuctionStatus = AUCTION_STATUS_EXPIRED;
            lpG->OwnerID       = lpS->OwnerID;
            lpG->ObjType       = lpS->ObjType;
            lpG->ObjName       = lpS->ObjName;
            lpG->Qty           = lpS->Qty;
            lpG->MadeBy        = lpS->MadeBy;
            lpG->Gold          = 0;
            lpG->lCharge       = lpS->lCharge;
            lpG->NMSEcu        = 0;
            
            c_aAuctionGive.Add(lpG);

            DWORD dwIndex = lpS->dwIndex;
            delete lpS;
            c_aAuctionSold.RemoveAt(i);
            i--;
         } 
         else 
         {
            //il est expirer et a un acheteur....
            //on dois donner or au vendeur et or a lacheteuyr...

            sAuctionMasterGive   *lpGI = new sAuctionMasterGive;
            lpGI->dwIndex       = GetValidGiveID();
            lpGI->AuctionStatus = AUCTION_STATUS_SOLD_BID;
            lpGI->OwnerID       = lpS->BidOwnerID;
            lpGI->ObjType       = lpS->ObjType;
            lpGI->ObjName       = lpS->ObjName;
            lpGI->Qty           = lpS->Qty;
            lpGI->MadeBy        = lpS->MadeBy;
            lpGI->Gold          = 0;
            lpGI->lCharge       = lpS->lCharge;
            lpGI->NMSEcu        = 0;
            
            c_aAuctionGive.Add(lpGI);

            sAuctionMasterGive   *lpGO = new sAuctionMasterGive;
            lpGO->dwIndex       = GetValidGiveID();
            lpGO->AuctionStatus = AUCTION_STATUS_SOLD_BID;
            lpGO->OwnerID       = lpS->OwnerID;
            lpGO->ObjType       = lpS->ObjType;
            lpGO->ObjName       = lpS->ObjName;
            lpGO->Qty           = lpS->Qty;
            lpGO->MadeBy        = "";
            lpGO->Gold          = lpS->CurrentBid;
            lpGO->lCharge       = lpS->lCharge;
            lpGO->NMSEcu        = 0;
            
            c_aAuctionGive.Add(lpGO);


            DWORD dwIndex = lpS->dwIndex;
            delete lpS;
            c_aAuctionSold.RemoveAt(i);
            i--;
         }
         bAuctionChanged = true; // ManageSoldList
      }
   }
   return 0;
}

int  AuctionMaster::ManageGiveList() {
   CAutoLock autoAHLock( &g_ALockAH );

   for(int i = 0; i < c_aAuctionGive.GetSize(); i++) 
   {
      sAuctionMasterGive *lpG = (sAuctionMasterGive *)c_aAuctionGive.GetAt(i);
      bool bItemProcessed = false;

      Players * pPlayer = CPlayerManager::GetCharacterRessourceByID(lpG->OwnerID); //PM
      if(pPlayer) 
      {
         if(pPlayer->in_game) 
         {
            CString strMessage;

            if(lpG->AuctionStatus == AUCTION_STATUS_SOLD_NOW) 
            {
               if(lpG->Gold > 0) 
               {
                  //le vendeur recoit son OR
                  pPlayer->self->SetGold(pPlayer->self->GetGold()+lpG->Gold);
                  strMessage.Format(_STR( 15139, pPlayer->self->GetLang() ),lpG->Qty,lpG->ObjName,lpG->Gold);
                  bItemProcessed = true;
                  _LOG_AH
                     LOG_MISC_1,
                     "Player %s [%s] a received from AuctionHouse(AuctionBuy) %u gold",
                     pPlayer->self->GetTrueName(),
                     pPlayer->GetFullAccountName(),
                     lpG->Gold
                  LOG_
               } 
               else if(lpG->NMSEcu > 0) 
               {
                  //le vendeur recoit ses ECU NMS
                  pPlayer->SetNMSGold(pPlayer->GetNMSGold()+lpG->NMSEcu);
                  pPlayer->SaveAccount();

                  strMessage.Format(_STR( 15508, pPlayer->self->GetLang() ),lpG->Qty,lpG->ObjName,lpG->NMSEcu);
                  bItemProcessed = true;
                  _LOG_AH
                     LOG_MISC_1,
                     "Player %s [%s] a received from AuctionHouse(AuctionBuy) %u ecu",
                     pPlayer->self->GetTrueName(),
                     pPlayer->GetFullAccountName(),
                     lpG->NMSEcu
                     LOG_
               } 
               else 
               {
                  bool bOK = false;
                  //l'acheteur recoit son Item...
                  DWORD dwID = Unit::GetIDFromName( lpG->ObjType, U_OBJECT, TRUE );
                  if( dwID != 0 ) 
                  {
                     Objects *lpItem = new Objects;
                     if( lpItem->Create( U_OBJECT, dwID ) ) 
                     {
                        lpItem->SetQty( lpG->Qty );

                        //on se set pas la charge des charge a -1 car lors de l ajout de la charge nous avons mis -1
                        //a cette valeur pour permettre de ne pas resetter tout AH pour rien... NM
                        if(lpG->lCharge != -1) 
                           lpItem->SetFlag( __FLAG_CHARGES, lpG->lCharge);
                      
                        // If the item is not unique, set its charge to 1 since
                        // it wasn't saved to avoid cluttering the database.
                        if( !lpItem->IsUnique() )
                        {
                           lpItem->SetFlag( __FLAG_CHARGES, 1 );
                        }

                        if(lpG->MadeBy != "") 
                        {
                           lpItem->SetCreatedBy(lpG->MadeBy.GetBuffer(0));
                        }

                        if(pPlayer->self->PutItemToChest(lpItem) == 0) 
                        {
                           bOK            = true;
                           bItemProcessed = true;
                           strMessage.Format(_STR( 15141, pPlayer->self->GetLang() ),lpG->Qty,lpG->ObjName);
                           

                           _LOG_AH
                              LOG_MISC_1,
                              "Player %s [%s] received from AuctionHouse(AuctionBuy) %u item %s Charge( %d ) ID( %s )",
                              pPlayer->self->GetTrueName(),
                              pPlayer->GetFullAccountName(),
                              lpG->Qty ,
                              lpG->ObjName,
                              lpG->lCharge,
                              lpG->ObjType
                           LOG_

                        } 
                        else 
                        {
                           strMessage.Format(_STR( 15370, pPlayer->self->GetLang() ),lpG->Qty,lpG->ObjName);
                           lpItem->DeleteUnit();

                           _LOG_AH
                              LOG_MISC_1,
                                 "***AuctionHouse Error Player %s [%s] received from AuctionHouse(AuctionBuy) %u item %s ID( %s )",
                                 pPlayer->self->GetTrueName(),
                                 pPlayer->GetFullAccountName(),
                                 lpG->Qty ,
                                 lpG->ObjName,
                                 lpG->ObjType
                              LOG_
                        }
                     } 
                     else 
                     {
                        lpItem->DeleteUnit();
                     }
                  }

                  if(!bOK) 
                  {
                     //send un erreur au LOG impossible de creer objets...
                     _LOG_AH
                        LOG_MISC_1,
                        "***AuctionHouse Error Player %s [%s] not received from AuctionHouse(AuctionBuy) %u item %s ID( %s )",
                        pPlayer->self->GetTrueName(),
                        pPlayer->GetFullAccountName(),
                        lpG->Qty ,
                        lpG->ObjName,
                        lpG->ObjType
                     LOG_
            
                  }
               }
            }
            else if(lpG->AuctionStatus == AUCTION_STATUS_SOLD_BID) 
            {
               if(lpG->Gold > 0) 
               {
                  //le vendeur recoit son OR
                  pPlayer->self->SetGold(pPlayer->self->GetGold()+lpG->Gold);
                  strMessage.Format(_STR( 15139, pPlayer->self->GetLang() ),lpG->Qty,lpG->ObjName,lpG->Gold);
                  bItemProcessed = true;
                  _LOG_AH
                     LOG_MISC_1,
                     "Player %s [%s] received from AuctionHouse(AuctionBid) %u gold",
                     pPlayer->self->GetTrueName(),
                     pPlayer->GetFullAccountName(),
                     lpG->Gold
                  LOG_
               } 
               else 
               {
                  bool bOK = false;
                  //l'acheteur recoit son Item...
                  DWORD dwID = Unit::GetIDFromName( lpG->ObjType, U_OBJECT, TRUE );
                  if( dwID != 0 ) 
                  {
                     Objects *lpItem = new Objects;
                     if( lpItem->Create( U_OBJECT, dwID ) ) 
                     {
                        lpItem->SetQty( lpG->Qty );

                        //on se set pas la charge des charge a -1 car lors de l ajout de la charge nous avons mis -1
                        //a cette valeur pour permettre de ne pas resetter tout AH pour rien... NM
                        if(lpG->lCharge != -1) 
                           lpItem->SetFlag( __FLAG_CHARGES, lpG->lCharge);

                        // If the item is not unique, set its charge to 1 since
                        // it wasn't saved to avoid cluttering the database.
                        if( !lpItem->IsUnique() )
                        {
                           lpItem->SetFlag( __FLAG_CHARGES, 1 );
                        }

                        if(lpG->MadeBy != "") 
                        {
                           lpItem->SetCreatedBy(lpG->MadeBy.GetBuffer(0));
                        }
                        
                        if(pPlayer->self->PutItemToChest(lpItem) == 0) 
                        {
                           bOK = true;
                           bItemProcessed = true;
                           strMessage.Format(_STR( 15141, pPlayer->self->GetLang() ),lpG->Qty,lpG->ObjName);
                           
                           _LOG_AH
                              LOG_MISC_1,
                                 "Player %s [%s] a received from AuctionHouse(AuctionBid) %u item %s ID( %s )",
                                 pPlayer->self->GetTrueName(),
                                 pPlayer->GetFullAccountName(),
                                 lpG->Qty ,
                                 lpG->ObjName,
                                 lpG->ObjType
                              LOG_
                        } 
                        else 
                        {
                           strMessage.Format(_STR( 15370, pPlayer->self->GetLang() ),lpG->Qty,lpG->ObjName);
                           lpItem->DeleteUnit();
                           _LOG_AH
                              LOG_MISC_1,
                                 "***AuctionHouse Error Player %s [%s] received from AuctionHouse(AuctionBid) %u item %s ID( %s )",
                                 pPlayer->self->GetTrueName(),
                                 pPlayer->GetFullAccountName(),
                                 lpG->Qty ,
                                 lpG->ObjName,
                                 lpG->ObjType
                              LOG_
                        }
                     } 
                     else
                     {
                        lpItem->DeleteUnit();
                     }
                  }
                  
                  if(!bOK) 
                  {
                     //send un erreur au LOG impossible de creer objets...
                     _LOG_AH
                        LOG_MISC_1,
                        "***AuctionHouse Error Player %s [%s] not received from AuctionHouse(AuctionBid) %u item %s ID( %s )",
                        pPlayer->self->GetTrueName(),
                        pPlayer->GetFullAccountName(),
                        lpG->Qty ,
                        lpG->ObjName,
                        lpG->ObjType
                        LOG_
                        
                  }
               }

            } 
            else if(lpG->AuctionStatus == AUCTION_STATUS_CANCEL) 
            {
               bool bOK = false;
               if(lpG->Gold > 0) 
               {
                  //le vendeur recoit son OR
                  pPlayer->self->SetGold(pPlayer->self->GetGold()+lpG->Gold);
                  strMessage.Format(_STR( 15140, pPlayer->self->GetLang() ),lpG->Qty,lpG->ObjName,lpG->Gold);
                  bItemProcessed = true;
                  _LOG_AH
                     LOG_MISC_1,
                     "Player %s [%s] received from AuctionHouse(AuctionCancel) %u gold",
                     pPlayer->self->GetTrueName(),
                     pPlayer->GetFullAccountName(),
                     lpG->Gold
                     LOG_
               } 
               else 
               {
                  //l'acheteur recoit son Item...
                  DWORD dwID = Unit::GetIDFromName( lpG->ObjType, U_OBJECT, TRUE );
                  if( dwID != 0 ) 
                  {
                     Objects *lpItem = new Objects;
                     if( lpItem->Create( U_OBJECT, dwID ) ) 
                     {
                        lpItem->SetQty( lpG->Qty );

                        //on se set pas la charge des charge a -1 car lors de l ajout de la charge nous avons mis -1
                        //a cette valeur pour permettre de ne pas resetter tout AH pour rien... NM
                        if(lpG->lCharge != -1) 
                           lpItem->SetFlag( __FLAG_CHARGES, lpG->lCharge);

                        // If the item is not unique, set its charge to 1 since
                        // it wasn't saved to avoid cluttering the database.
                        if( !lpItem->IsUnique() )
                        {
                           lpItem->SetFlag( __FLAG_CHARGES, 1 );
                        }

                        if(lpG->MadeBy != "") 
                        {
                           lpItem->SetCreatedBy(lpG->MadeBy.GetBuffer(0));
                        }

                        if(pPlayer->self->PutItemToChest(lpItem) == 0) 
                        {
                           bOK = true;
                           bItemProcessed = true;
                           strMessage.Format(_STR( 15142, pPlayer->self->GetLang() ),lpG->Qty,lpG->ObjName);
                        
                           _LOG_AH
                              LOG_MISC_1,
                              "Player %s [%s] a received from AuctionHouse (AuctionCancel) %u item %s ID( %s )",
                              pPlayer->self->GetTrueName(),
                              pPlayer->GetFullAccountName(),
                              lpG->Qty ,
                              lpG->ObjName,
                              lpG->ObjType
                              LOG_
                           
                        } 
                        else 
                        {
                           strMessage.Format(_STR( 15370, pPlayer->self->GetLang() ),lpG->Qty,lpG->ObjName);
                           lpItem->DeleteUnit();
                           _LOG_AH
                              LOG_MISC_1,
                              "***AuctionHouse Error(Unable to put in chest) Player %s [%s] not received from AuctionHouse(AuctionCancel) %u item %s ID( %s )",
                              pPlayer->self->GetTrueName(),
                              pPlayer->GetFullAccountName(),
                              lpG->Qty ,
                              lpG->ObjName,
                              lpG->ObjType
                           LOG_
                        }
                     } 
                     else 
                     {
                        lpItem->DeleteUnit();
                     }
                  }
               
                  if(!bOK)
                  {
                     //send un erreur au LOG impossible de creer objets...
                     _LOG_AH
                        LOG_MISC_1,
                        "***AuctionHouse Error Player %s [%s] not received from AuctionHouse(AuctionCancel) %u item %s ID( %s )",
                        pPlayer->self->GetTrueName(),
                        pPlayer->GetFullAccountName(),
                        lpG->Qty ,
                        lpG->ObjName,
                        lpG->ObjType
                        LOG_
                  }
               }

            }
            else if(lpG->AuctionStatus == AUCTION_STATUS_OVERBID) 
            {
               //le vendeur recoit son OR
               pPlayer->self->SetGold(pPlayer->self->GetGold()+lpG->Gold);
               strMessage.Format(_STR( 15140, pPlayer->self->GetLang() ),lpG->Qty,lpG->ObjName,lpG->Gold);
               bItemProcessed = true;
               _LOG_AH
                  LOG_MISC_1,
                  "Player %s [%s] received from AuctionHouse(AuctionOverBid) %u gold",
                  pPlayer->self->GetTrueName(),
                  pPlayer->GetFullAccountName(),
                  lpG->Gold
               LOG_

            } 
            else if(lpG->AuctionStatus == AUCTION_STATUS_EXPIRED) 
            {
               bool bOK = false;
               //l'acheteur recoit son Item...
               DWORD dwID = Unit::GetIDFromName( lpG->ObjType, U_OBJECT, TRUE );
               if( dwID != 0 ) 
               {
                  Objects *lpItem = new Objects;
                  if( lpItem->Create( U_OBJECT, dwID ) ) 
                  {
                     lpItem->SetQty( lpG->Qty );

                     //on se set pas la charge des charge a -1 car lors de l ajout de la charge nous avons mis -1
                     //a cette valeur pour permettre de ne pas resetter tout AH pour rien... NM
                     if(lpG->lCharge != -1) 
                        lpItem->SetFlag( __FLAG_CHARGES, lpG->lCharge);

                     // If the item is not unique, set its charge to 1 since
                     // it wasn't saved to avoid cluttering the database.
                     if( !lpItem->IsUnique() )
                     {
                        lpItem->SetFlag( __FLAG_CHARGES, 1 );
                     }

                     if(lpG->MadeBy != "") 
                     {
                        lpItem->SetCreatedBy(lpG->MadeBy.GetBuffer(0));
                     }
                     
                     if(pPlayer->self->PutItemToChest(lpItem) == 0) 
                     {
                        bOK = true;
                        bItemProcessed = true;
                        strMessage.Format(_STR( 15143 , pPlayer->self->GetLang() ),lpG->Qty,lpG->ObjName);

                        _LOG_AH
                           LOG_MISC_1,
                           "Player %s [%s] received from AuctionHouse (AuctionExpired) %u item %s ID( %s )",
                           pPlayer->self->GetTrueName(),
                           pPlayer->GetFullAccountName(),
                           lpG->Qty ,
                           lpG->ObjName,
                           lpG->ObjType
                        LOG_
                     } 
                     else 
                     {
                        strMessage.Format(_STR( 15370, pPlayer->self->GetLang() ),lpG->Qty,lpG->ObjName);
                        lpItem->DeleteUnit();
                        _LOG_AH
                           LOG_MISC_1,
                              "***AuctionHouse Error(Unable to put in chest) Player %s [%s] received from AuctionHouse (AuctionExpired) %u item %s ID( %s )",
                              pPlayer->self->GetTrueName(),
                              pPlayer->GetFullAccountName(),
                              lpG->Qty ,
                              lpG->ObjName,
                              lpG->ObjType
                           LOG_
                     }
                  } 
                  else 
                  {
                     lpItem->DeleteUnit();
                  }
               }
               
               if(!bOK) 
               {
                  //send un erreur au LOG impossible de creer objets...
                  _LOG_AH
                     LOG_MISC_1,
                     "***AuctionHouse Error Player %s [%s] not received from AuctionHouse(AuctionExpired) %u item %s ID( %s )",
                     pPlayer->self->GetTrueName(),
                     pPlayer->GetFullAccountName(),
                     lpG->Qty ,
                     lpG->ObjName,
                     lpG->ObjType
                     LOG_
                     
               }
            }
            else if(lpG->AuctionStatus == AUCTION_STATUS_BANK_GIVE) 
            {
               //le joueur recoit les interet,...
               pPlayer->self->SetGold(pPlayer->self->GetGold()+lpG->Gold);
               strMessage.Format(_STR( 15150, pPlayer->self->GetLang() ),lpG->Gold);
               bItemProcessed = true;
               _LOG_AH
                  LOG_MISC_1,
                  "Player %s [%s] a received from AuctionHouseBank(Interet) %u gold",
                  pPlayer->self->GetTrueName(),
                  pPlayer->GetFullAccountName(),
                  lpG->Gold
               LOG_
            }

            pPlayer->self->SendInfoMessage(strMessage);

            if(bItemProcessed)
            {
               bAuctionChanged = true; // ManageGiveList

               delete lpG;
               c_aAuctionGive.RemoveAt(i);
               i--;
            }
            
         }
         CPlayerManager::FreePlayerResource(pPlayer);
      }
   }

   return 0;
}

int  AuctionMaster::SendAHList(Character *pUser,DWORD dwIsShowDialog) {
   CAutoLock autoAHLock( &g_ALockAH );

   CString strNom;
   CString strNotes;
   int     iNbr = 0;
   time_t tCurTime =  time(NULL);

   iNbr = GetNbrSold();
   
   TFCPacket sending;
   sending << (RQ_SIZE)RQ_NM_GetAHList;
   sending << (long)pUser->GetID();
   sending << (long)dwIsShowDialog;
   sending << (long)iNbr;
   
   sAuctionMasterSold   *pItem;
   for(int i=0;i<iNbr;i++) {
      pItem =  GetAHSoldItem(i);
      if(pItem) {
         //cb reste de temps... en secondes...
         
         sending << (long)pItem->dwIndex;
         sending << (long)pItem->OwnerID;
         sending << (CString)pItem->ObjName;
         sending << (long)pItem->Qty;
         sending << (long)(pItem->TimeMax-tCurTime);
         sending << (long)pItem->BuyItNow;
         sending << (long)pItem->BuyItNowNMS;
         sending << (long)pItem->MinimumBid;
         sending << (long)pItem->CurrentBid;
         sending << (long)pItem->BidOwnerID;
         sending << (long)pItem->EquipPos;
         sending << (long)pItem->TimeEnter;
      } else {
         // pas supposer arriver mais on envoie un item bidon pour pas fraire crasher client
         CString strVide = " ";
         int iBidon = 0;
         sending << (long)iBidon;
         sending << (CString)strVide;
         sending << (long)iBidon;
         sending << (long)iBidon;
         sending << (long)iBidon;
         sending << (long)iBidon;
         sending << (long)iBidon;
         sending << (long)iBidon;
         sending << (long)iBidon;
         sending << (long)iBidon;
         sending << (long)iBidon;
      }
   }
   
   if(pUser) pUser->SendPlayerMessage(sending);

   return 0;
}


sAuctionMasterSold   * AuctionMaster::GetAHSoldItem(int iIndex) {
   if(iIndex >=0 && iIndex < c_aAuctionSold.GetSize()) {
      sAuctionMasterSold   *lpS = (sAuctionMasterSold *)c_aAuctionSold.GetAt(iIndex);
      return lpS;
   }
   return NULL;
}

/**
 * Returns the highest ID (dwIndex) from the Auctions List
 */
DWORD AuctionMaster::_HighestAuctionID() {
	DWORD highest_id = 0;
	for(int i = 0; i < c_aAuctionSold.GetSize(); i++) {
		sAuctionMasterSold   *auction = (sAuctionMasterSold *)c_aAuctionSold.GetAt(i);
		if(auction->dwIndex > highest_id) {
			highest_id = auction->dwIndex;
		}
	}
	return highest_id;
}

/**
 * Returns the highest ID (dwIndex) from the Give List
 */
DWORD AuctionMaster::_HighestGiveListID() {
	DWORD highest_id = 0;
	for(int i = 0; i < c_aAuctionGive.GetSize(); i++) {
		sAuctionMasterGive *give = (sAuctionMasterGive*)c_aAuctionGive.GetAt(i);
		if(give->dwIndex > highest_id) {
			highest_id = give->dwIndex;
		}
	}
	return highest_id;
}

/**
 * Return an always incrementing number
 */
DWORD AuctionMaster::GetValidSoldID() {
	static DWORD dwIndex = 0;

	if (dwIndex == 0) { //Initialize
		dwIndex = _HighestAuctionID();
	}
	return ++dwIndex;
}

/**
 * Return an always incrementing number
 */
DWORD AuctionMaster::GetValidGiveID() {
	static DWORD dwIndex = 0;

	if (dwIndex == 0) {
		dwIndex = _HighestGiveListID();
	}
	return ++dwIndex;
}