// AuctionMaster.h: interface for the Professions class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUCTIONMASTER_H__D02F3B81_5542_11D1_BD7A_00E029058623__INCLUDED_)
#define AFX_AUCTIONMASTER_H__D02F3B81_5542_11D1_BD7A_00E029058623__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "VDList.h"
#include "IntlText.h"
#include "AuctionMaster.h"
#include "ODBCMage.h"


typedef struct _sAuctionMasterSold
{
   DWORD   dwIndex;
   DWORD   OwnerID;
   CString VendeurName;
   CString ObjType;
   CString ObjName;
   DWORD   Qty;
   DWORD   EquipPos;
   CString MadeBy;
   DWORD   BuyItNow;
   DWORD   MinimumBid;
   DWORD   TimeEnter;
   DWORD   TimeMax;
   DWORD   CurrentBid;
   DWORD   BidOwnerID;
   long    lCharge;
   DWORD   BuyItNowNMS;
}sAuctionMasterSold;

typedef struct _sAuctionMasterGive
{
   DWORD   dwIndex;
   DWORD   AuctionStatus;
   DWORD   OwnerID;
   CString ObjType;
   CString ObjName;
   DWORD   Qty;
   CString MadeBy;
   DWORD   Gold;
   long    lCharge;
   DWORD   NMSEcu;
}sAuctionMasterGive;


#define AUCTION_STATUS_SOLD_NOW  0
#define AUCTION_STATUS_SOLD_BID  1
#define AUCTION_STATUS_CANCEL    2
#define AUCTION_STATUS_OVERBID   3
#define AUCTION_STATUS_EXPIRED   4
#define AUCTION_STATUS_BANK_GIVE 5



class AuctionMaster;

// The class
class __declspec(dllexport) AuctionMaster
{
public:
	
	static void Create();  // Creates all internal structures
   static void Destroy(); // Destroys all internal structures
	
   static int  SendAHList(Character *pUser,DWORD dwIsShowDialog);
   static int  AddSoldItem(Players *pPlayer, CString strObjType, CString strObjName, DWORD dwQty, DWORD dwEquipPos,
                               CString strMadeBy,DWORD dwBuyNow,DWORD dwBuyNowNMS, DWORD dwBid, DWORD dwTimeMax, long lCharge);
   static int  BuySoldItem(Character *pUser, DWORD dwIndex, DWORD dwBuy, DWORD dwPrix,DWORD dwPrixNMS, DWORD dwTS);
   static int  CancelSoldItem(Character *pUser, DWORD dwIndex, DWORD dwTS);
   static int  AddMoneyGiveList(Character *pUser, int iMontant);
   static int  ForceExpireAllItems(Character *pUser);
   static int  InfoSoldItem(Character *pUser, DWORD dwIndex);
   

   static int  ManageSoldList();
   static int  ManageGiveList();
   


   

   static sAuctionMasterSold   * GetAHSoldItem(int iIndex);

   static bool IsCanAddItems(Character *pUser);
   
	static void SaveAll(bool bForce);
   static bool IsAuctionChanged();
protected:

private:
	static sAuctionMasterSold* GetAuction(DWORD auction_id);
	static void _RemoveAuction(DWORD auction_id);
	static int _GetAuctionArrayPosition(DWORD auction_id);
	static void _DeleteAll(TemplateList< SQL_REQUEST > *lptlSQLRequests);
	static void _SaveAuction(sAuctionMasterSold* auction, TemplateList< SQL_REQUEST > *lptlSQLRequests);
	static void _SaveGiveList(sAuctionMasterGive* give_list, TemplateList< SQL_REQUEST > *lptlSQLRequests);
	static DWORD _HighestAuctionID();
	static DWORD _HighestGiveListID();
   static int  GetNbrSold();
   static int  GetNbrGive();
   static DWORD  GetValidSoldID();
   static DWORD  GetValidGiveID();


   
public:

protected:

private:
	static int iAuctionSold;
   static int iAuctionGive;
   static bool bAuctionChanged;
   
	static CPtrArray c_aAuctionSold;
   static CPtrArray c_aAuctionGive;


};

#endif // !defined(AFX_AUCTIONMASTER_H__D02F3B81_5542_11D1_BD7A_00E029058623__INCLUDED_)
