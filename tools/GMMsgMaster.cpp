#include "stdafx.h"
#include "TFC Server.h"
#include "TFC_MAIN.h"
#include "GMMsgMaster.h"
#include "IntlText.h"
#include "Unit.h"
#include "PlayerManager.h"
#include "ODBCMage.h"

#include "T4CLog.h"
#include "DeadlockDetector.h"

extern CTFCServerApp theApp;

CPtrArray GMMsgMaster::c_aGMMsgMaster;
BOOL      GMMsgMaster::g_GMMsgChanged;

static cODBCMage ODBCGMMsg;//OK  Load Direct et save Threader avec MANUAL commit
static CLock     g_ALockGMMsg;

#define ADD_QUERY_GMMSG	{ \
	LPSQL_REQUEST lpSql = new SQL_REQUEST;\
	lpSql->csQuery = strQuery;\
	lptlSQLRequests->AddToTail( lpSql );\
}
//                           nbr sec/day * 60 days
const int iFlushTimeSec = (60*60*24)*60; //== 5 184 000


//////////////////////////////////////////////////////////////////////////////////////////
// Creates the class
void GMMsgMaster::Create( void )
{
	ODBCGMMsg.Connect( USERS_DSN, USERS_USER, USERS_PWD );
	ODBCGMMsg.ConnectOption( SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF );
	ODBCGMMsg.Lock();

	//load la liste de toutes les GMMsg....
	CString csQuery;
	csQuery.Format( "SELECT ID, Status, CreateTime, CreateTimeT, OwnerID, PlayerName, Message FROM GMMsg");
	ODBCGMMsg.SendRequest( (LPCTSTR)csQuery );

   time_t TimeMsNow =  time(NULL);
	int iCntT  = 0;
   int iCntO  = 0;
   int iCntR  = 0;
	while( ODBCGMMsg.Fetch())
	{
		sGMMsg *pNewMsg = new sGMMsg();

		char lpszTimeStamp  [ 50 ];
      char lpszPlayerName [ 50 ];
		char lpszMessage    [ 255 ];

      ODBCGMMsg.GetDWORD ( 1,	&pNewMsg->dwIndexID);
      ODBCGMMsg.GetDWORD ( 2,	&pNewMsg->bStatus);
      ODBCGMMsg.GetDWORD ( 3,	&pNewMsg->dwCreateTime);
		ODBCGMMsg.GetString( 4, lpszTimeStamp, 50 );
		ODBCGMMsg.GetDWORD ( 5,	&pNewMsg->dwOwnerID);
      ODBCGMMsg.GetString( 6, lpszPlayerName, 50 );
		ODBCGMMsg.GetString( 7, lpszMessage, 255 );


		pNewMsg->strTimeStamp .Format("%s",lpszTimeStamp);
		pNewMsg->strPlayer    .Format("%s",lpszPlayerName);
      pNewMsg->strMessage   .Format("%s",lpszMessage);
		
      pNewMsg->strMessage.Replace("''","'");

      //les message plus vieux de 60 jours FERMER on les deletes...
      //donne rien de garder cette liste vu que c<est du ponctuel non un historique...
      if(!pNewMsg->bStatus && pNewMsg->dwCreateTime+iFlushTimeSec < TimeMsNow)
      {
         iCntR++;
      }
      else
      {
		   c_aGMMsgMaster.SetAtGrow(iCntT, pNewMsg );
		   iCntT++;
         if(pNewMsg->bStatus)
            iCntO++;
      }

	}

	ODBCGMMsg.Cancel();
	ODBCGMMsg.Unlock();

	printf( "\n  Found %d GMMsg(s)",c_aGMMsgMaster.GetSize() );
	printf( "\n  Found %d GMMsg(s) opened",iCntO );
   printf( "\n  Found %d GMMsg(s) closed older than 60 day",iCntR );

   g_GMMsgChanged = FALSE;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Destroys skills
void GMMsgMaster::Destroy( void )
{
	sGMMsg *lpMsg;
	int i;

	for(i = 0; i < c_aGMMsgMaster.GetSize(); i++)
	{
		lpMsg = (sGMMsg *)c_aGMMsgMaster.GetAt(i);
		if(lpMsg)
		{
			delete lpMsg;
			lpMsg = NULL;
		}
	}
	ODBCGMMsg.Disconnect( );
}

//////////////////////////////////////////////////////////////////////////////////////////
// Destroys skills
void GMMsgMaster::SaveAllGMMsg() 
{
	CAutoLock autoMsgLock( &g_ALockGMMsg );

   if(!g_GMMsgChanged) //if no chnage we not save...
      return;

	/* Saving GMMsgs to DB happens inside a transaction to make sure the DB is always
	 * in a valid state and never corrupted if the server crashes during save.
	 * As such, we create a buffer list of SQL Requests, call functions to populate it
	 * and when finished, we send it to be run asynchronously */
	TemplateList< SQL_REQUEST > *lptlSQLRequests = new TemplateList< SQL_REQUEST >;

   CString strQuery;

   /* Clearing current data from DB */
   strQuery.Format( "DELETE FROM GMMsg");
   ADD_QUERY_GMMSG;

	/* Collects data from each Gmmsg */
	for(int i=0;i<c_aGMMsgMaster.GetSize();i++) 
   {
		sGMMsg *pMsg = (sGMMsg *)c_aGMMsgMaster.GetAt(i);
      
      CString strQuery, strMessage;
      strMessage = pMsg->strMessage;
      strMessage.Replace("'","''");

      strQuery.Format("INSERT INTO GMMsg(Status,CreateTime,CreateTimeT,OwnerID,PlayerName,Message) VALUES (%d,%d,'%s',%d,'%s','%s')",
                       pMsg->bStatus,pMsg->dwCreateTime,pMsg->strTimeStamp,pMsg->dwOwnerID,pMsg->strPlayer,strMessage);
      ADD_QUERY_GMMSG;
	}

	/* Finally, send all the collected SQL Requests to be run */
	ODBCGMMsg.SendBatchRequest( lptlSQLRequests, NULL, NULL, "ODBCGMMsg" );
   g_GMMsgChanged = FALSE;
}


int GMMsgMaster::GetGMMessageNbr()
{
	return c_aGMMsgMaster.GetSize();
}

int GMMsgMaster::GetGMMessageStatus(UINT uiMessageID)
{
   for(int i=0;i<c_aGMMsgMaster.GetSize();i++) 
   {
      sGMMsg *pMsg = (sGMMsg *)c_aGMMsgMaster.GetAt(i);
      if(pMsg->dwIndexID == uiMessageID)
         return pMsg->bStatus;
   }

   return FALSE;
}

int GMMsgMaster::GetGMMessageUnique(DWORD dwOwnerID)
{
   for(int i=0;i<c_aGMMsgMaster.GetSize();i++) 
   {
      sGMMsg *pMsg = (sGMMsg *)c_aGMMsgMaster.GetAt(i);
      if(pMsg->bStatus && pMsg->dwOwnerID == dwOwnerID)
         return FALSE;
   }

   return TRUE;
}

sGMMsg* GMMsgMaster::GetGMMessage(UINT uiMessageID)
{
   for(int i=0;i<c_aGMMsgMaster.GetSize();i++) 
   {
      sGMMsg *pMsg = (sGMMsg *)c_aGMMsgMaster.GetAt(i);
      if(pMsg->dwIndexID == uiMessageID)
         return pMsg;
   }

   return NULL;
}





int GMMsgMaster::PostGMMessage(Players *pPlayer, CString strMessage)
{
	CAutoLock autoMsgLock( &g_ALockGMMsg );

	//here we assume than the USER ID is OK and exist...
	//here we assume this user ID have not an active message
	//first look if a guild with this name already exist...

   if(strMessage.GetLength()>255 || strMessage.GetLength() < 10)
      return -1;

   if(!GetGMMessageUnique(pPlayer->self->GetID()))
      return -2;

	//we can create GmMsg...
   //New ID
   DWORD dwNewID = 0;
   if(c_aGMMsgMaster.GetSize() > 0)
   {
      sGMMsg *pMsg = (sGMMsg *)c_aGMMsgMaster.GetAt(c_aGMMsgMaster.GetSize()-1);
      dwNewID = pMsg->dwIndexID+1;
   }
   //TimeStamp
   CString csTimeStamp;
   SYSTEMTIME sysTime; 
   GetLocalTime(&sysTime);
   csTimeStamp.Format("%04d-%02d-%02d %02dh%02d.%02d",sysTime.wYear, sysTime.wMonth,sysTime.wDay,sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
   time_t TimeMsTmp =  time(NULL);

	//Ajoute la guild
	sGMMsg *pNewMsg = new sGMMsg();
   pNewMsg->dwIndexID    = dwNewID;
   pNewMsg->bStatus      = 1; 
   pNewMsg->dwCreateTime = TimeMsTmp;
   pNewMsg->strTimeStamp = csTimeStamp;
   pNewMsg->dwOwnerID    = pPlayer->self->GetID();
   pNewMsg->strPlayer    = pPlayer->self->GetTrueName();
   pNewMsg->strMessage   = strMessage;

	int iPos = c_aGMMsgMaster.GetSize();
	if(iPos <0)
		iPos = 0;
	c_aGMMsgMaster.SetAtGrow(iPos, pNewMsg );

   g_GMMsgChanged = TRUE;

	return 0;
}

int GMMsgMaster::PostGMSystemMessage(CString strSystem, CString strMessage)
{
	CAutoLock autoMsgLock( &g_ALockGMMsg );

	//here we assume than the USER ID is OK and exist...
	//here we assume this user ID have not an active message
	//first look if a guild with this name already exist...

	if(strMessage.GetLength()>255)
		return -1;

	//we can create GmMsg...
	//New ID
	DWORD dwNewID = 0;
	if(c_aGMMsgMaster.GetSize() > 0)
	{
		sGMMsg *pMsg = (sGMMsg *)c_aGMMsgMaster.GetAt(c_aGMMsgMaster.GetSize()-1);
		dwNewID = pMsg->dwIndexID+1;
	}
	//TimeStamp
	CString csTimeStamp;
	SYSTEMTIME sysTime; 
	GetLocalTime(&sysTime);
	csTimeStamp.Format("%04d-%02d-%02d %02dh%02d.%02d",sysTime.wYear, sysTime.wMonth,sysTime.wDay,sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
	time_t TimeMsTmp =  time(NULL);

	//Ajoute la guild
	sGMMsg *pNewMsg = new sGMMsg();
	pNewMsg->dwIndexID    = dwNewID;
	pNewMsg->bStatus      = 1; 
	pNewMsg->dwCreateTime = TimeMsTmp;
	pNewMsg->strTimeStamp = csTimeStamp;
	pNewMsg->dwOwnerID    = 0;
	pNewMsg->strPlayer    = strSystem;
	pNewMsg->strMessage   = strMessage;

	int iPos = c_aGMMsgMaster.GetSize();
	if(iPos <0)
		iPos = 0;
	c_aGMMsgMaster.SetAtGrow(iPos, pNewMsg );

	g_GMMsgChanged = TRUE;

	return 0;
}

int GMMsgMaster::CloseGMMessage(UINT uiMessageID)
{
   CAutoLock autoMsgLock( &g_ALockGMMsg );

   for(int i=0;i<c_aGMMsgMaster.GetSize();i++) 
   {
      sGMMsg *pMsg = (sGMMsg *)c_aGMMsgMaster.GetAt(i);
      if(pMsg->bStatus && pMsg->dwIndexID == uiMessageID)
      {
         pMsg->bStatus = 0; //close this request
         g_GMMsgChanged = TRUE;
         return 0; //
      }
   }
   return -1;
}

void GMMsgMaster::SendAllOpenedGMMessage(Players *pPlayer)
{
   CAutoLock autoMsgLock( &g_ALockGMMsg );

   long lNbrOpenned = 0;
   //step 1 count nbr opened message
   for(int i=0;i<c_aGMMsgMaster.GetSize();i++) 
   {
      sGMMsg *pMsg = (sGMMsg *)c_aGMMsgMaster.GetAt(i);
      if(pMsg->bStatus)
         lNbrOpenned++;
   }

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_GMMSG_Get;
   sending << (long)lNbrOpenned;

   for(int i=0;i<c_aGMMsgMaster.GetSize();i++) 
   {
      sGMMsg *pMsg = (sGMMsg *)c_aGMMsgMaster.GetAt(i);
      if(pMsg->bStatus)
      {
         sending << (long)pMsg->dwIndexID;
         sending << (CString)( pMsg->strTimeStamp );
         sending << (CString)( pMsg->strPlayer );
         sending << (CString)( pMsg->strMessage );
      }
   }
   pPlayer->self->SendPlayerMessage( sending );
}
