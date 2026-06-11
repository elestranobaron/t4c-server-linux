/* Note to self: 
 *   This is possibly the reason for the Deadlocks:
 *     CPlayerManager::GetCharacterRessourceByID();
 */

#include "stdafx.h"
#include "TFC Server.h"
#include "TFC_MAIN.h"
#include "GuildMaster.h"
#include "IntlText.h"
#include "Unit.h"
#include "DynObjManager.h"
#include "PlayerManager.h"
#include "ODBCMage.h"

#include "T4CLog.h"
#include "DeadlockDetector.h"

extern CTFCServerApp theApp;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CPtrArray GuildMaster::c_aGuildMaster;
CPtrArray GuildMaster::c_aGuildUsers;

bool GuildMaster::m_bGuildChnaged;


static cODBCMage ODBCGuild;//OK Load Direct et save Threader avec MANUAL commit
static CLock     g_ALockGuild;

#define ADD_QUERY_GUILD	{ \
	LPSQL_REQUEST lpSql = new SQL_REQUEST;\
	lpSql->csQuery = strQuery;\
	lptlSQLRequests->AddToTail( lpSql );\
}



//////////////////////////////////////////////////////////////////////////////////////////
void GuildMaster::Create( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Creates the class
// 
//////////////////////////////////////////////////////////////////////////////////////////
{
	ODBCGuild.Connect( USERS_DSN, USERS_USER, USERS_PWD );
	ODBCGuild.ConnectOption( SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF );
	ODBCGuild.Lock();


	//load la liste de toutes les guild....
	CString csQuery;
	csQuery.Format( "SELECT GuildName, OwnerID, Notes FROM GuildMaster");
	ODBCGuild.SendRequest( (LPCTSTR)csQuery );

	int iCnt  = 0;
	while( ODBCGuild.Fetch())
	{
		sGuildMaster *pNewGuild = new sGuildMaster();

		char lpszGuildName [ 50 ];
		char lpszGuildNotes[ 255 ];

		ODBCGuild.GetString( 1, lpszGuildName, 50 );
		ODBCGuild.GetDWORD ( 2,	&pNewGuild->dwOwnerID);
		ODBCGuild.GetString( 3, lpszGuildNotes, 255 );


		pNewGuild->strGuildName .Format("%s",lpszGuildName);
		pNewGuild->strNote      .Format("%s",lpszGuildNotes);
		pNewGuild->pGuildChest  = NULL;
        pNewGuild->bGCChanged   = FALSE;
		pNewGuild->bGCLoaded    = FALSE;

		c_aGuildMaster.SetAtGrow(iCnt, pNewGuild );

		iCnt++;
	}

	ODBCGuild.Cancel();

	//load la liste de tous les membre dans une list...
	csQuery.Format( "SELECT GuildName, OwnerID, GuildTitle, GuildPermission, PlayerLastName FROM GuildUsers");
	ODBCGuild.SendRequest( (LPCTSTR)csQuery );

	iCnt  = 0;
	while( ODBCGuild.Fetch())
	{
		sGuildUser *pNewUser = new sGuildUser();

		char lpszGuildName      [ 50 ];
		char lpszlastPlayerName [ 50 ];
		memset(lpszGuildName,0x00,50);
		memset(lpszlastPlayerName,0x00,50);

		ODBCGuild.GetString( 1, lpszGuildName, 50 );
		ODBCGuild.GetDWORD ( 2,	&pNewUser->dwPJOwnerID);
		ODBCGuild.GetDWORD ( 3,	&pNewUser->dwTitre);
		ODBCGuild.GetDWORD ( 4,	&pNewUser->dwPermession);
		ODBCGuild.GetString( 5,	lpszlastPlayerName,50);

		pNewUser->strGuildName.Format("%s",lpszGuildName);

		if(lpszlastPlayerName[0] != NULL)
			pNewUser->strLastPlayerName.Format("%s",lpszlastPlayerName);
		else
			pNewUser->strLastPlayerName.Format("");

		c_aGuildUsers.SetAtGrow(iCnt, pNewUser );
		iCnt++;

		//printf( "\n  GuildUser(s)  GuildName == %s  lastname == %s",pNewUser->strGuildName,pNewUser->strLastPlayerName);
	}

	ODBCGuild.Cancel();
	ODBCGuild.Unlock();

   m_bGuildChnaged = false;


	printf( "\n  Found %d Guild(s)",c_aGuildMaster.GetSize() );
	printf( "\n  Found %d Guild User(s)",c_aGuildUsers.GetSize() );
}


//////////////////////////////////////////////////////////////////////////////////////////
void GuildMaster::Destroy( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Destroys skills
// 
//////////////////////////////////////////////////////////////////////////////////////////
{
	sGuildMaster *lpGuild;
	sGuildUser   *lpGuildUser;
	int i;

	for(i = 0; i < c_aGuildMaster.GetSize(); i++)
	{
		lpGuild = (sGuildMaster *)c_aGuildMaster.GetAt(i);
		if(lpGuild)
		{
			if(lpGuild->pGuildChest)
				delete lpGuild->pGuildChest;
			delete lpGuild;
			lpGuild = NULL;
		}
	}

	for(i = 0; i < c_aGuildUsers.GetSize(); i++)
	{
		lpGuildUser = (sGuildUser *)c_aGuildUsers.GetAt(i);
		if(lpGuildUser)
		{
			delete lpGuildUser;
			lpGuildUser = NULL;
		}
	}



	ODBCGuild.Disconnect( );
}


bool GuildMaster::IsGuildChanged()
{
   CAutoLock autoGuildLock( &g_ALockGuild );
   return m_bGuildChnaged;
}

/**
 * SaveAllGuilds()
 * Dumps in-memory Guild info to the Database
 */
void GuildMaster::SaveAllGuilds(bool bForce) {
	CAutoLock autoGuildLock( &g_ALockGuild );


   if(bForce || m_bGuildChnaged)
   {
	   /* Saving guilds to DB happens inside a transaction to make sure the DB is always
	    * in a valid state and never corrupted if the server crashes during save.
	    * As such, we create a buffer list of SQL Requests, call functions to populate it
	    * and when finished, we send it to be run asynchronously */
	   TemplateList< SQL_REQUEST > *lptlSQLRequests = new TemplateList< SQL_REQUEST >;

	   /* Clearing current data from DB */
	   _DeleteAllGuilds(lptlSQLRequests);

	   /* Collects data from each guild */
	   for(int i=0;i<c_aGuildMaster.GetSize();i++) {
		   sGuildMaster *guild = (sGuildMaster *)c_aGuildMaster.GetAt(i);
		   _SaveGuild(guild, lptlSQLRequests);
	   }

	   /* Collects guilds memberships info */
	   for(int i = 0; i < c_aGuildUsers.GetSize(); i++) {
		   sGuildUser *user = (sGuildUser *)c_aGuildUsers.GetAt(i);
		   _SaveGuildUser(user, lptlSQLRequests);
	   }

	   /* Finally, send all the collected SQL Requests to be run */
	   ODBCGuild.SendBatchRequest( lptlSQLRequests, NULL, NULL, "ODBCGuild" );
   
      m_bGuildChnaged = false;
   }
}

/**
 * _DeleteAllGuilds(...)
 * Adds into the TemplateList<> parameter SQL Queries to clear/delete guild info from DB
 */
void GuildMaster::_DeleteAllGuilds(TemplateList< SQL_REQUEST > *lptlSQLRequests) {
	CString strQuery;

	strQuery.Format( "DELETE FROM GuildMaster");
	ADD_QUERY_GUILD;
	strQuery.Format( "DELETE FROM GuildUsers");
	ADD_QUERY_GUILD;
}

/**
 * _SaveGuild(...)
 * Adds into the TemplateList<> parameter SQL Queries to save a particular guild
 */
void GuildMaster::_SaveGuild(sGuildMaster* guild, TemplateList< SQL_REQUEST > *lptlSQLRequests) {
	CString strQuery, guild_name(guild->strGuildName), guild_note(guild->strNote);
	guild_name.Replace("'","''");
	guild_note.Replace("'","''");

	strQuery.Format(
		"INSERT INTO GuildMaster(GuildName,OwnerID,Notes) VALUES ('%s',%d,'%s')",
		guild_name, guild->dwOwnerID, guild_note
	);
	ADD_QUERY_GUILD;
}

/**
 * _SaveGuildUser(...)
 * Adds into the TemplateList<> parameter SQL Queries to save a particular guild's list of members
 */
void GuildMaster::_SaveGuildUser(sGuildUser* user, TemplateList< SQL_REQUEST > *lptlSQLRequests) {
	CString strQuery, guild_name(user->strGuildName), last_playername(user->strLastPlayerName);
	guild_name.Replace("'","''");
	last_playername.Replace("'","''");
   if(last_playername == "")
      last_playername = " ";
	strQuery.Format(
		"INSERT INTO GuildUsers(GuildName,OwnerID,GuildTitle,GuildPermission, PlayerLastName) VALUES ('%s',%d,%d,%d,'%s')",
		guild_name,
		user->dwPJOwnerID,
		user->dwTitre,
		user->dwPermession,
		last_playername
	);
	ADD_QUERY_GUILD;
}

int GuildMaster::GetNbrGuildPriv()
{
	return c_aGuildMaster.GetSize();
}

int GuildMaster::GetNbrGuildUser()
{
	return c_aGuildUsers.GetSize();
}

int GuildMaster::CreateNewGuild(CString strGuildName,DWORD dwUserID, Players *pPlayer)
{
	CAutoLock autoGuildLock( &g_ALockGuild );

	//here we assume than the USER ID is OK and exist...
	//here we assume this user ID are nor already in a guild...
	//first look if a guild with this name already exist...

	if (IsExistGuild(strGuildName)) {
		return -1; //this guild name already exist...
	}

	//we can create guild...
	//we add it on list and we insert it on database...

	//Ajoute la guild
	sGuildMaster *pNewGuild = new sGuildMaster();
	pNewGuild->strGuildName.Format("%s",strGuildName);
	pNewGuild->strNote     .Format("");
	pNewGuild->dwOwnerID    = dwUserID;
	pNewGuild->pGuildChest  = new ItemContainer();
	pNewGuild->bGCChanged   = FALSE;
	pNewGuild->bGCLoaded    = TRUE;
	pNewGuild->pGuildChest->SetMaxWeight( theApp.dwGUILDChestEncumbrance);
	int iPos = c_aGuildMaster.GetSize();
	if(iPos <0)
		iPos = 0;
	c_aGuildMaster.SetAtGrow(iPos, pNewGuild );

	//Ajoute ce user a la liste des user 
	sGuildUser *pNewUser = new sGuildUser();
	pNewUser->strGuildName.Format("%s",strGuildName);
	pNewUser->dwPJOwnerID   = dwUserID;
	pNewUser->dwTitre       = GUILD_TITRE_FONDATEUR;
	pNewUser->dwPermession  = GUILD_FONDATEUR_RIGHT;
	pNewUser->strLastPlayerName.Format("%s",pPlayer->self->GetTrueName());
	iPos = c_aGuildUsers.GetSize();
	if(iPos <0)
		iPos = 0;
	c_aGuildUsers.SetAtGrow(iPos, pNewUser );

	//Setup this guild info to the user...
	pPlayer->self->SetGuildName(strGuildName.GetBuffer(0));
	pPlayer->self->SetGuildTitle(GUILD_TITRE_FONDATEUR);
	pPlayer->self->SetGuildPermission(GUILD_FONDATEUR_RIGHT);

	//Add cc to fondator
	ChatterChannels &cChatter = CPlayerManager::GetChatter();
	cChatter.AddCCPlayer( pPlayer, strGuildName.GetBuffer(0), "",false );
	cChatter.SendRegisteredChannelList(pPlayer);


	CString strMessage;
	strMessage.Format(_STR( 15071, pPlayer->self->GetLang() ),strGuildName);
	pPlayer->self->SendInfoMessage(strMessage,0x0080FF);

	AddGuildLog(strGuildName,_STR( 15054, pPlayer->self->GetLang() ));

   m_bGuildChnaged = true;//CreateNewGuild
	return 0;
}

sGuildUser* GuildMaster::GetGuildOwner(CString sGuildName) {
	CAutoLock autoGuildLock( &g_ALockGuild );
	sGuildUser* guMember;
	int i;
	for(i = 0; i < c_aGuildUsers.GetSize(); i++)
	{
		guMember = (sGuildUser *)c_aGuildUsers.GetAt(i);
		if(guMember->dwPermession == GUILD_FONDATEUR_RIGHT && guMember->strGuildName.CompareNoCase(sGuildName) == 0)
		{
			return guMember;
		}
	}
	return NULL;
}

/**
 * Modify Guild is actually guild->change_owner()
 */
int GuildMaster::ModifyGuild(CString strGuildName,DWORD dwUserID, Players *pPlayer)
{
	CAutoLock autoGuildLock( &g_ALockGuild );
	// Get Guild
	sGuildMaster* gmGuild = GetGuildByName(strGuildName);
	if(!gmGuild) return -1; //Guild not found

	// Get current owner
	sGuildUser *pUserOld = GetGuildOwner(strGuildName);
	if (pUserOld == NULL) return -1; // Guild Owner not found

	//here we assume than the USER ID is OK and exist...
	//here we assume this user ID are nor already in a guild...
	//first look if a guild with this name already exist...
	int i;
	bool    bNeedAddUser = false;

	if(pPlayer->self->GetGuildName().CompareNoCase(strGuildName) != 0) {
		bNeedAddUser = true;
	}

	// Handle in-memory structure

	//Change the guild founder
	gmGuild->dwOwnerID = dwUserID;
	//Demote the old founder
	pUserOld->dwTitre      = GUILD_TITRE_RECRU;
	pUserOld->dwPermession = GUILD_RECRU_RIGHT;

	//if need to add new fondator user...
	if(bNeedAddUser)
	{
		//Ajoute ce user a la liste des user 
		sGuildUser *pNewUser = new sGuildUser();
		pNewUser->strGuildName.Format("%s",strGuildName);
		pNewUser->dwPJOwnerID   = dwUserID;
		pNewUser->dwTitre       = GUILD_TITRE_FONDATEUR;
		pNewUser->dwPermession  = GUILD_FONDATEUR_RIGHT;
		pNewUser->strLastPlayerName.Format("%s",pPlayer->self->GetTrueName());
		int iPos = c_aGuildUsers.GetSize();
		if(iPos <0)
			iPos = 0;
		c_aGuildUsers.SetAtGrow(iPos, pNewUser );

		//new user we add the chatter chanel
		ChatterChannels &cChatter = CPlayerManager::GetChatter();
		cChatter.AddCCPlayer( pPlayer, strGuildName.GetBuffer(0), "",false );
		cChatter.SendRegisteredChannelList(pPlayer);
	}
	else
	{
		//need to change user to fondator Right...
		sGuildUser *pUser;
		for(i = 0; i < c_aGuildUsers.GetSize(); i++)
		{
			pUser = (sGuildUser *)c_aGuildUsers.GetAt(i);
			if(pUser->strGuildName.CompareNoCase(strGuildName) == 0 && pUser->dwPJOwnerID == dwUserID)
			{
				//here we have old fondator...
				pUser->dwTitre      = GUILD_TITRE_FONDATEUR;
				pUser->dwPermession = GUILD_FONDATEUR_RIGHT;
				i = c_aGuildUsers.GetSize(); //sort de la boucle...
			}
		}
	}

	//Setup this guild info to the user...
	pPlayer->self->SetGuildName(strGuildName.GetBuffer(0));
	pPlayer->self->SetGuildTitle(GUILD_TITRE_FONDATEUR);
	pPlayer->self->SetGuildPermission(GUILD_FONDATEUR_RIGHT);

	//try to found old fondator user if is online we change status...
	Players * pPlayerOld = CPlayerManager::GetCharacterRessourceByID(pUserOld->dwPJOwnerID);//PM
	if(pPlayerOld)
	{
		pPlayerOld->self->SetGuildPermission(GUILD_RECRU_RIGHT);
		pPlayerOld->self->SetGuildTitle(GUILD_TITRE_RECRU);

		CPlayerManager::FreePlayerResource(pPlayerOld);
	}


	//now delete all online user guild info... if user are on this guild...
	BroadcastToAllOnlineUser(strGuildName,FALSE,FALSE,FALSE,_STR( 15120, pPlayer->self->GetLang() ),pPlayer->self->GetTrueName());

	AddGuildLog(strGuildName,_STR( 15120, pPlayer->self->GetLang() ),pPlayer->self->GetTrueName());

   m_bGuildChnaged = true;//ModifyGuild
	return 0;
}


int GuildMaster::DeleteGuild(CString strGuildName)
{
	CAutoLock autoGuildLock( &g_ALockGuild );
	if(IsExistGuild(strGuildName))
	{
		//delete guild chest from DB ( TODO: refactor me, please! )
		CString strQuery;
		TemplateList< SQL_REQUEST > *lptlSQLRequests = new TemplateList< SQL_REQUEST >;

		strQuery.Format( "DELETE FROM ChestGuild Where GuildName='%s'",strGuildName);
		ADD_QUERY_GUILD;
		ODBCGuild.SendBatchRequest( lptlSQLRequests, NULL, NULL, "ODBCGuild" );

		//now on doit virer de la liste des guild onlide...
		sGuildMaster *lpGuild;
		int i;
		for(i = 0; i < c_aGuildMaster.GetSize(); i++)
		{
			lpGuild = (sGuildMaster *)c_aGuildMaster.GetAt(i);
			if(lpGuild->strGuildName.CompareNoCase(strGuildName) == 0)
			{
				if(lpGuild->pGuildChest)
					delete lpGuild->pGuildChest;

				delete lpGuild;
				lpGuild = NULL;
				c_aGuildMaster.RemoveAt(i);
				i = c_aGuildMaster.GetSize(); //sort de la boucle...
			}
		}

		//delete tous les user de la guilds...
		sGuildUser *pUser;
		for(i = 0; i < c_aGuildUsers.GetSize(); i++)
		{
			pUser = (sGuildUser *)c_aGuildUsers.GetAt(i);
			if(pUser->strGuildName.CompareNoCase(strGuildName) == 0)
			{
				delete pUser;
				pUser = NULL;
				c_aGuildUsers.RemoveAt(i);
				i--;
			} 
		}

		//now delete all online user guild info... if user are on this guild...
		BroadcastToAllOnlineUser(strGuildName,TRUE,FALSE,FALSE,_STR( 15070, IntlText::GetDefaultLng() ),strGuildName);

      m_bGuildChnaged = true;//DeleteGuild

		//AddGuildLog(strGuildName,_STR( 15055, pUser->GetLang() ));
	}
	else
	{
		return -1;
	}

	return 0;
}

int GuildMaster::RenameGuild(CString strGuildName,CString strNewGuildName)
{
	CAutoLock autoGuildLock( &g_ALockGuild );
	if(IsExistGuild(strNewGuildName))
	{
		return -2;
	}
	else if(IsExistGuild(strGuildName))
	{
		CString strQuery;
		TemplateList< SQL_REQUEST > *lptlSQLRequests = new TemplateList< SQL_REQUEST >;

		// TODO: Refactor me, please!
		//Rename the guild from guild chest
		strQuery.Format( "UPDATE ChestGuild SET GuildName='%s' WHERE GuildName='%s'",strNewGuildName.GetBuffer(0),strGuildName.GetBuffer(0));
		ADD_QUERY_GUILD;

		//Rename the guild from guild Logs
		strQuery.Format( "UPDATE GuildLogs SET GuildName='%s' WHERE GuildName='%s'",strNewGuildName.GetBuffer(0),strGuildName.GetBuffer(0));
		ADD_QUERY_GUILD;

		ODBCGuild.SendBatchRequest( lptlSQLRequests, NULL, NULL, "ODBCGuild" );


		//now on doit Updater la guild master en ligne...
		sGuildMaster *lpGuild;
		int i;
		for(i = 0; i < c_aGuildMaster.GetSize(); i++)
		{
			lpGuild = (sGuildMaster *)c_aGuildMaster.GetAt(i);
			if(lpGuild->strGuildName.CompareNoCase(strGuildName)==0)
			{
				lpGuild->strGuildName = strNewGuildName;
				i = c_aGuildMaster.GetSize(); //sort de la boucle...
			}
		}

		//Update de tous les user de la guilds...
		sGuildUser *pUser;
		for(i = 0; i < c_aGuildUsers.GetSize(); i++)
		{
			pUser = (sGuildUser *)c_aGuildUsers.GetAt(i);
			if(pUser->strGuildName.CompareNoCase(strGuildName) == 0)
			{
				pUser->strGuildName  = strNewGuildName;
			}
		} 


		CPlayerManager::SetPlayerGuildNameAndCC(strGuildName,strNewGuildName);


		//now delete all online user guild info... if user are on this guild...
		BroadcastToAllOnlineUser(strNewGuildName,FALSE,FALSE,FALSE,_STR( 15111, IntlText::GetDefaultLng() ),strGuildName,strNewGuildName);


		AddGuildLog(strNewGuildName,_STR( 15112, IntlText::GetDefaultLng() ),strGuildName,strNewGuildName);

      m_bGuildChnaged = true;//RenameGuild
	}
	else
	{
		return -1;
	}

	return 0;
}


bool GuildMaster::GetUserGuildInfo(UINT uiID, char *pStrName, char *pStrLastName, DWORD &dwLevel, DWORD &dwPermission)
{
	CAutoLock autoGuildLock( &g_ALockGuild );

	sGuildUser *lpGuildUser;
	for(int i = 0; i < c_aGuildUsers.GetSize(); i++)
	{
		lpGuildUser = (sGuildUser *)c_aGuildUsers.GetAt(i);
		if(lpGuildUser->dwPJOwnerID == uiID)
		{
			sprintf_s(pStrName    ,50,"%s",lpGuildUser->strGuildName);
			sprintf_s(pStrLastName,50,"%s",lpGuildUser->strLastPlayerName);

			dwLevel      = lpGuildUser->dwTitre;
			dwPermission = lpGuildUser->dwPermession;
		}
	}

	return IsExistGuildP(pStrName);
}

void GuildMaster::AddUserGuild(Character *pUser, char *pStrGuildName)
{
	CAutoLock autoGuildLock( &g_ALockGuild );

	//Setup this guild info to the user...
	pUser->SetGuildName(pStrGuildName);
	pUser->SetGuildNameInvited("",NULL);
	pUser->SetGuildTitle(GUILD_TITRE_RECRU);
	pUser->SetGuildPermission(GUILD_RECRU_RIGHT);

	//Add le CC au nouveau user
	Players *pl = pUser->GetPlayer();
	ChatterChannels &cChatter = CPlayerManager::GetChatter();
	cChatter.AddCCPlayer( pl, pStrGuildName, "",false );
	cChatter.SendRegisteredChannelList(pl);

	//Ajoute ce user a la liste des user 
	sGuildUser *pNewUser = new sGuildUser();
	pNewUser->strGuildName.Format("%s",pStrGuildName);
	pNewUser->dwPJOwnerID   = pUser->GetID();
	pNewUser->dwTitre       = GUILD_TITRE_RECRU;
	pNewUser->dwPermession  = GUILD_RECRU_RIGHT;
	pNewUser->strLastPlayerName.Format("%s",pUser->GetTrueName());
	int iPos = c_aGuildUsers.GetSize();
	if(iPos <0)
		iPos = 0;
	c_aGuildUsers.SetAtGrow(iPos, pNewUser );

	//now Send update and info he have a new user on guild...
	BroadcastToAllOnlineUser(pStrGuildName,FALSE,TRUE,FALSE,_STR( 15072 , pUser->GetLang() ),pStrGuildName,pUser->GetTrueName());

	AddGuildLog(pStrGuildName,_STR( 15056, pUser->GetLang() ),pUser->GetTrueName());

   m_bGuildChnaged = true;//AddUserGuild
}

int GuildMaster::RemoveUserGuild(Character *pUser,bool bKick,CString strKickName)
{
	CAutoLock autoGuildLock( &g_ALockGuild );

	//check if the ID are the fondateur
	//now on doit virer de la liste des guild onlide...
	sGuildUser *lpGuildUser;
	for(int i = 0; i < c_aGuildUsers.GetSize(); i++)
	{
		lpGuildUser = (sGuildUser *)c_aGuildUsers.GetAt(i);
		if(lpGuildUser->dwPJOwnerID == pUser->GetID())
		{
			if(lpGuildUser->dwPermession == GUILD_FONDATEUR_RIGHT)
			{
				return -1;
			}
		}
	}

	//now on doit virer de la liste des guild onlide...
	CString strGuildName = "";
	for(int i = 0; i < c_aGuildUsers.GetSize(); i++)
	{
		lpGuildUser = (sGuildUser *)c_aGuildUsers.GetAt(i);
		if(lpGuildUser->dwPJOwnerID == pUser->GetID())
		{
			strGuildName = lpGuildUser->strGuildName;
			delete lpGuildUser;
			lpGuildUser = NULL;
			c_aGuildUsers.RemoveAt(i);
			i = c_aGuildUsers.GetSize(); //sort de la boucle...
		}
	}

   if(bKick)
   {
      //now Send update user have been kicked out the guild...
      BroadcastToAllOnlineUser(strGuildName,FALSE,TRUE,FALSE,_STR( 15073 , pUser->GetLang() ),pUser->GetTrueName(),strKickName);
      AddGuildLog(pUser->GetGuildName(),_STR( 15073 , pUser->GetLang() ),pUser->GetTrueName(),strKickName);
   }
   else
   {
      BroadcastToAllOnlineUser(strGuildName,FALSE,TRUE,FALSE,_STR( 15074 , pUser->GetLang() ),pUser->GetTrueName());
      AddGuildLog(pUser->GetGuildName(),_STR( 15074 , pUser->GetLang() ),pUser->GetTrueName());
   }

	pUser->SetGuildName("");
	pUser->SetGuildTitle(0);
	pUser->SetGuildPermission(0);
	pUser->SetGuildNameInvited("",NULL);

	//on dois virer ce user du CC de guilde...
	Players *pl = pUser->GetPlayer();
	ChatterChannels &cChatter = CPlayerManager::GetChatter();
	cChatter.Remove(pl,strGuildName.GetBuffer(0));
	cChatter.SendRegisteredChannelList(pl);

   m_bGuildChnaged = true;//RemoveUserGuild

	return 0;
}

int GuildMaster::RemoveUserGuildOffline(DWORD dwID,CString strKickName)
{
	CAutoLock autoGuildLock( &g_ALockGuild );

	//first delete the guild info from guildmaster...


	//check if the ID are the fondateur
	//now on doit virer de la liste des guild onlide...
	sGuildUser *lpGuildUser;
	for(int i = 0; i < c_aGuildUsers.GetSize(); i++)
	{
		lpGuildUser = (sGuildUser *)c_aGuildUsers.GetAt(i);
		if(lpGuildUser->dwPJOwnerID == dwID)
		{
			if(lpGuildUser->dwPermession == GUILD_FONDATEUR_RIGHT)
			{
				return -1;
			}
			i = c_aGuildUsers.GetSize();
		}
	}


	//now on doit virer de la liste des guild onlide...
	CString strGuildName = "";
	for(int i = 0; i < c_aGuildUsers.GetSize(); i++)
	{
		lpGuildUser = (sGuildUser *)c_aGuildUsers.GetAt(i);
		if(lpGuildUser->dwPJOwnerID == dwID)
		{
			strGuildName = lpGuildUser->strGuildName;
			delete lpGuildUser;
			lpGuildUser = NULL;
			c_aGuildUsers.RemoveAt(i);
			i = c_aGuildUsers.GetSize(); //sort de la boucle...
		}
	}


	CString strNameOffline;
	GetNameOfflineUser(dwID,strNameOffline);

	//now Send update user have been kicked out the guild...
	BroadcastToAllOnlineUser(strGuildName,FALSE,TRUE,FALSE,_STR( 15073 , IntlText::GetDefaultLng() ),strNameOffline,strKickName);
	AddGuildLog(strGuildName,_STR( 15073, IntlText::GetDefaultLng() ),strNameOffline,strKickName);

   m_bGuildChnaged = true;//RemoveUserGuildOffline
	return 0;
}

int GuildMaster::ChangeUserSettings(Character *pUser,DWORD dwID,DWORD dwTitre, DWORD dwPermission,CString strName)
{
	CAutoLock autoGuildLock( &g_ALockGuild );

	DWORD dwOldTitre;
	DWORD dwOldPerm ;


	//check if the ID are the fondateur
	//now on doit virer de la liste des guild onlide...
	CString strGuildName= "";
	CString strPlayerName;
	sGuildUser *lpGuildUser;
	int i=0;
	for(i = 0; i < c_aGuildUsers.GetSize(); i++)
	{
		lpGuildUser = (sGuildUser *)c_aGuildUsers.GetAt(i);
		if(lpGuildUser->dwPJOwnerID == dwID)
		{
			if(lpGuildUser->dwPermession == GUILD_FONDATEUR_RIGHT)
			{
				return -1;
			} 

			dwOldTitre = lpGuildUser->dwTitre;
			dwOldPerm  = lpGuildUser->dwPermession;
			lpGuildUser->dwTitre = dwTitre;
			lpGuildUser->dwPermession = dwPermission;
			strGuildName.Format("%s",lpGuildUser->strGuildName);
			strPlayerName.Format("%s",lpGuildUser->strLastPlayerName);
			i = c_aGuildUsers.GetSize();
		}
	}

	//Si le joueur ets en ligne... on dois affecter son status live...

	Players * pPlayer = CPlayerManager::GetCharacterRessourceByID(dwID);//PM
	if(pPlayer)
	{
		pPlayer->self->SetGuildPermission(dwPermission);
		pPlayer->self->SetGuildTitle(dwTitre);
		strPlayerName = pPlayer->self->GetTrueName();

		CPlayerManager::FreePlayerResource(pPlayer);
	}


	//Log les changement de permissions...


	int iPermChange[9];
	for(int kk=0;kk<9;kk++)
		iPermChange[kk]=0;

	uGuildPermission uPO;
	uGuildPermission uPN;

	uPO.dwVal = dwOldPerm;
	uPN.dwVal = dwPermission;

	iPermChange[0] = uPN.Permission.Leader       - uPO.Permission.Leader;
	iPermChange[1] = uPN.Permission.CanInvite    - uPO.Permission.CanInvite;
	iPermChange[2] = uPN.Permission.CanKick      - uPO.Permission.CanKick;
	iPermChange[3] = uPN.Permission.ViewChest    - uPO.Permission.ViewChest;
	iPermChange[4] = uPN.Permission.CanDeposit   - uPO.Permission.CanDeposit;
	iPermChange[5] = uPN.Permission.CanTake      - uPO.Permission.CanTake;
	iPermChange[6] = uPN.Permission.CanWriteNote - uPO.Permission.CanWriteNote;
	iPermChange[7] = uPN.Permission.CanChPerm    - uPO.Permission.CanChPerm;
	iPermChange[8] = uPN.Permission.CanSeeLog    - uPO.Permission.CanSeeLog;


	CString strTmp;
	for(i=0;i<9;i++)
	{
		if(iPermChange[i] >0) //Ajout d'une permission
		{
			//AddGuildLog(strGuildName,_STR( 15089, pUser->GetLang() ),strName,_STR( 15080+i, pUser->GetLang() ),strPlayerName);

			strTmp.Format(_STR( 15089, pUser->GetLang() ),strName,_STR( 15080+i, pUser->GetLang() ),strPlayerName);
			//theApp.AddGuildRequest(pUser,NULL,NULL,GUILD_ADD_LOGS,0,0,0,0,strTmp,"");
			theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_ADD_LOGS,pUser->GetID(),0,0,0,strTmp,"");
		}
		else if(iPermChange[i] <0 ) //remove d'une permission
		{
			//AddGuildLog(strGuildName,_STR( 15090, pUser->GetLang() ),strName,_STR( 15080+i, pUser->GetLang() ),strPlayerName);

			strTmp.Format(_STR( 15090, pUser->GetLang() ),strName,_STR( 15080+i, pUser->GetLang() ),strPlayerName);
			//theApp.AddGuildRequest(pUser,NULL,NULL,GUILD_ADD_LOGS,0,0,0,0,strTmp,"");
			theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_ADD_LOGS,pUser->GetID(),0,0,0,strTmp,"");
		}
	}

	if(dwTitre != dwOldTitre)
	{
		//on dois aviser un changement de titre egalement
		//AddGuildLog(strGuildName,_STR( 15091, pUser->GetLang() ),strName,strPlayerName,_STR( 15058+dwTitre, pUser->GetLang() ));

		strTmp.Format(_STR( 15091, pUser->GetLang() ),strName,strPlayerName,_STR( 15058+dwTitre, pUser->GetLang() ));
		//theApp.AddGuildRequest(pUser,NULL,NULL,GUILD_ADD_LOGS,0,0,0,0,strTmp,"");
		theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_ADD_LOGS,pUser->GetID(),0,0,0,strTmp,"");
	}


	//now Send update user have been kicked out the guild...
	BroadcastToAllOnlineUser(strGuildName,FALSE,TRUE,FALSE,"");

   m_bGuildChnaged = true;//ChangeUserSettings
	return 0;
}

void GuildMaster::ChangeNoteSettings(CString strGuildName, char*pstrNote, CString strName)
{
	CAutoLock autoGuildLock( &g_ALockGuild );

	//Change la note de la liste des guild en memoire...
	sGuildMaster *lpGuild;
	int i;
	for(i = 0; i < c_aGuildMaster.GetSize(); i++)
	{
		lpGuild = (sGuildMaster *)c_aGuildMaster.GetAt(i);
		if(lpGuild->strGuildName.CompareNoCase(strGuildName) == 0)
		{
			lpGuild->strNote.Format("%s",pstrNote);
			i = c_aGuildMaster.GetSize(); //sort de la boucle...
		}
	}


	//now Send update user have been kicked out the guild...
	BroadcastToAllOnlineUser(strGuildName,FALSE,TRUE,FALSE,_STR( 15079  , IntlText::GetDefaultLng() ),strName);
	AddGuildLog(strGuildName,_STR( 15079  , IntlText::GetDefaultLng() ),strName);

   m_bGuildChnaged = true;//ChangeNoteSettings
}

void GuildMaster::SendLogsList(Character *pUser)
{
	// TODO: Refactor me, please!
	CAutoLock autoGuildLock( &g_ALockGuild );

	ODBCGuild.Lock();
	CString csQuery;

	csQuery.Format( "SELECT TOP 50 * FROM GuildLogs WHERE GuildName = '%s' ORDER BY LogID DESC",pUser->GetGuildName().GetBuffer(0));
	ODBCGuild.SendRequest( (LPCTSTR)csQuery );

	DWORD dwIDTmp;
	char lpszGuildName [ 50 ];
	char lpszGuildLT   [ 50 ];
	char lpszGuildLogs [ 1024 ];

	CString strGuildLT;
	CString strGuildLogs;


	BYTE chStatus = 0;
	TFCPacket sending;
	sending << (RQ_SIZE)RQ_NM_GuildGetLogs;
	sending << (char)chStatus; //dit de cleara la liste des logs...
	pUser->SendPlayerMessage(sending);
	sending.Destroy();

	chStatus = 1; //vas recevoir les log un a un...

	while( ODBCGuild.Fetch())
	{

		ODBCGuild.GetDWORD ( 1,	&dwIDTmp);
		ODBCGuild.GetString( 2, lpszGuildName, 50 );
		ODBCGuild.GetString( 3, lpszGuildLT  , 50 );
		ODBCGuild.GetString( 4, lpszGuildLogs, 1024 );

		strGuildLT  .Format("%s",lpszGuildLT);
		strGuildLogs.Format("%s",lpszGuildLogs);



		//Send to player...
		sending << (RQ_SIZE)RQ_NM_GuildGetLogs;
		sending << (char)chStatus; //dit de cleara la liste des logs...
		sending << strGuildLT;
		sending << strGuildLogs;
		pUser->SendPlayerMessage(sending);
		sending.Destroy();

		//pUser->SendInfoMessage(lpszGuildLogs,0x1B52C5);

	}

	ODBCGuild.Cancel();
	ODBCGuild.Unlock();  

	chStatus = 2; //fin on peu updater la page des log avec ce que lon a...
	sending << (RQ_SIZE)RQ_NM_GuildGetLogs;
	sending << (char)chStatus; //dit de cleara la liste des logs...
	pUser->SendPlayerMessage(sending);
	sending.Destroy();
}

void GuildMaster::RenameLastUserName(Character *pUser,CString strName)
{
	CAutoLock autoGuildLock( &g_ALockGuild );

	//update ce nom dans la liste des online....
	sGuildUser *lpGuildUser;
	for(int i = 0; i < c_aGuildUsers.GetSize(); i++)
	{
		lpGuildUser = (sGuildUser *)c_aGuildUsers.GetAt(i);
		if(lpGuildUser->dwPJOwnerID == pUser->GetID())
		{
			lpGuildUser->strLastPlayerName = strName;
			i =  c_aGuildUsers.GetSize();
		}
	}

   m_bGuildChnaged = true;//RenameLastUserName
}

void GuildMaster::GetGuildList(CStringArray &astrGuild)
{
	CAutoLock autoGuildLock( &g_ALockGuild );

	sGuildMaster *pGuild;
	for(int i=0;i<c_aGuildMaster.GetSize();i++)
	{
		pGuild = (sGuildMaster *)c_aGuildMaster.GetAt(i);
		astrGuild.Add(pGuild->strGuildName);
	}
}

sGuildMaster* GuildMaster::GetGuildByName(CString sGuildName) {
	sGuildMaster *pGuild;
	for(int i=0;i<GetNbrGuildPriv();i++)
	{
		pGuild = (sGuildMaster *)c_aGuildMaster.GetAt(i);
		if(pGuild->strGuildName.CompareNoCase(sGuildName) == 0)
		{
			return pGuild;
		}
	}
	return NULL;
}

void GuildMaster::GetNoteByGuildName(CString strGuildName,CString &strNoteGuild)
{
	sGuildMaster *pGuild = GetGuildByName(strGuildName);
	if (pGuild) {
		strNoteGuild = pGuild->strNote;
	}
}

int GuildMaster::GetNbrGuildUserCount(CString strGuildName)
{
	int iCnt = 0;

	sGuildUser *lpGuildUser;
	for(int i = 0; i < c_aGuildUsers.GetSize(); i++)
	{
		lpGuildUser = (sGuildUser *)c_aGuildUsers.GetAt(i);
		if(lpGuildUser->strGuildName.CompareNoCase(strGuildName) == 0)
		{
			iCnt++;
		}
	}

	return iCnt;
}

void GuildMaster::GetNameGuild(DWORD dwID,CString &strNameGuild)
{
	if(dwID <0 || dwID >= GetNbrGuildPriv())
	{
		return;
	}

	sGuildMaster *pGuild;
	pGuild = (sGuildMaster *)c_aGuildMaster.GetAt(dwID);
	strNameGuild = pGuild->strGuildName;
}


bool GuildMaster::IsExistGuild(CString strGuildName)
{
	return (GetGuildByName(strGuildName) != NULL);
}

bool GuildMaster::IsExistGuildP(char *pStrGuildName)
{
	bool bExist = false;

	CAutoLock autoGuildLock( &g_ALockGuild );

	CString strGName;
	strGName.Format("%s",pStrGuildName);
	bExist = IsExistGuild(strGName);

	return bExist;
}

//dois etre locker de l<exterieur...
sGuildUser* GuildMaster::GetGuildUserByIndex(int iIdx)
{
	if(iIdx <0 || iIdx >= c_aGuildUsers.GetSize())
		return NULL;

	sGuildUser *lpGuildUser = (sGuildUser *)c_aGuildUsers.GetAt(iIdx);
	return lpGuildUser;
}

void GuildMaster::GetNameOfflineUser(DWORD dwID,CString &strNameHL)
{

	sGuildUser *lpGuildUser;
	for(int i = 0; i < c_aGuildUsers.GetSize(); i++)
	{
		lpGuildUser = (sGuildUser *)c_aGuildUsers.GetAt(i);
		if(lpGuildUser->dwPJOwnerID == dwID)
		{
			if(lpGuildUser->strLastPlayerName != "")
				strNameHL = lpGuildUser->strLastPlayerName;
			else
				strNameHL = "Joueur hors ligne";
			return;
		}
	}
}

void GuildMaster::AddGuildLog(CString strGuildName,LPCTSTR pszFmt, ...)
{
	TCHAR pszBuffer[2048];

	va_list argptr;
	va_start( argptr, pszFmt );
	int cch = _vsntprintf_s( pszBuffer, 2048,
		sizeof( pszBuffer )/sizeof( TCHAR ),
		pszFmt, 
		argptr );
	va_end(argptr);




	if ( cch == -1 )
		return;


	CString csTimeStamp;
	CString csLogs;

	// Get the TimeStamp
	SYSTEMTIME sysTime; 
	GetLocalTime(&sysTime);
	csTimeStamp.Format("%04d-%02d-%02d %02dh%02d",sysTime.wYear, sysTime.wMonth,sysTime.wDay,sysTime.wHour, sysTime.wMinute);
	csLogs.Format("%s",pszBuffer);
	csLogs.Replace("'", "''");


	CString strQuery;
	TemplateList< SQL_REQUEST > *lptlSQLRequests = new TemplateList< SQL_REQUEST >;

	//add the guild user to GuildUSer
	strQuery.Format( "INSERT INTO GuildLogs(GuildName, TimeInfo, Logs) VALUES ('%s','%s','%s')",strGuildName.GetBuffer(0),csTimeStamp.GetBuffer(0),csLogs.GetBuffer(0));
	ADD_QUERY_GUILD;
	ODBCGuild.SendBatchRequest( lptlSQLRequests, NULL, NULL, "ODBCGuild" );

	_LOG_GUILDHIST
		LOG_MISC_1,
		"%s  %s  %s",strGuildName.GetBuffer(0),csTimeStamp.GetBuffer(0),csLogs.GetBuffer(0)
		LOG_
}


void GuildMaster::BroadcastToAllOnlineUser(CString strGuildName,BOOL bResetGuildInfo,BOOL bUpdateGuildList,BOOL bUpdateGuildChest,LPCTSTR pszFmt, ...)
{
   if(strGuildName == "")
      return ;

	TCHAR pszBuffer[2048];

	va_list argptr;
	va_start( argptr, pszFmt );
	int cch = _vsntprintf_s( pszBuffer, 2048,
		sizeof( pszBuffer )/sizeof( TCHAR ),
		pszFmt, 
		argptr );
	va_end(argptr);
	if ( cch == -1 )
		return;


	CPlayerManager::BroadcastGuildInfo(pszBuffer,strGuildName,bResetGuildInfo,bUpdateGuildList,bUpdateGuildChest);
}







void GuildMaster::SaveAllGuildChest() {
	CAutoLock autoGuildLock( &g_ALockGuild );

   for(int i=0;i<c_aGuildMaster.GetSize();i++) 
   {
      sGuildMaster *guild = (sGuildMaster *)c_aGuildMaster.GetAt(i);
      guild->LockGC.Lock();
      BOOL bChnaged = guild->bGCChanged;
      BOOL bLoaded  = guild->bGCLoaded;
      guild->bGCChanged = FALSE;
      guild->LockGC.Unlock();

      if(bChnaged && bLoaded)
      {
         _LOG_DEBUG
            LOG_DEBUG_HIGH,
            "Save Chest guild %s",guild->strGuildName
            LOG_

            SaveGuildChest(guild);
      }
   }
}



int GuildMaster::LoadGuildChest(CString strGuildName)
{
	CAutoLock autoGuildLock( &g_ALockGuild );
	sGuildMaster *pGuildSelect = GetGuildByName(strGuildName);

	// si pas trouver la guild ou que le coffre est deja loader...
	// on fou rien...
	if(pGuildSelect == NULL )
	{
		return -1;
	}
	if( pGuildSelect->pGuildChest != NULL || pGuildSelect->bGCLoaded)
	{
		return 0; // chest already loaded...
	}

	//Load le chest de la guild...

	pGuildSelect->pGuildChest = new ItemContainer();
	pGuildSelect->pGuildChest->SetMaxWeight( theApp.dwGUILDChestEncumbrance);

	DWORD dwTempID    =     0;
	const int DB_ObjID    =		1;				
	const int DB_ObjType  =		2;
	const int DB_Qty      =    3;
	const int DB_MadeBy   =    4;

	LOADED_ITEM *lpLoadedItem;

	// Temporary list which will contain the loaded items.				
	TemplateList < LOADED_ITEM > tlLoadedItems;

	_LOG_DEBUG
		LOG_DEBUG_HIGH,
		"Fetching chest objects"
		LOG_

	ODBCGuild.Lock();
	CString strQuery;
	strQuery.Format( "SELECT ObjID, ObjType, Qty, MadeBy FROM ChestGuild WHERE GuildName='%s'",strGuildName );

	if( ODBCGuild.SendRequest( (LPCTSTR)strQuery ) )
	{
		while( ODBCGuild.Fetch() )
		{	
			lpLoadedItem = new LOADED_ITEM;
			lpLoadedItem->bEquipPos = 0;
			lpLoadedItem->lpszObjType[0] = 0;

			ODBCGuild.GetDWORD ( DB_ObjID,    &lpLoadedItem->dwObjID );
			ODBCGuild.GetString( DB_ObjType,  (LPTSTR)lpLoadedItem->lpszObjType, 50 );
			ODBCGuild.GetDWORD ( DB_Qty,      &lpLoadedItem->dwQty );
			if( lpLoadedItem->dwQty == 0 )
			{
				lpLoadedItem->dwQty = 1;
			}
			lpLoadedItem->lpszMadeBy[0] = 0;
			ODBCGuild.GetString( DB_MadeBy,  (LPTSTR)lpLoadedItem->lpszMadeBy, 50 );

			// Add them to the loaded items list.
			tlLoadedItems.AddToTail( lpLoadedItem );
		}

		// Cancel the previous fetch operation
		ODBCGuild.Cancel();

		// Scroll through the loaded items
		tlLoadedItems.ToHead();
		while( tlLoadedItems.QueryNext() )
		{
			lpLoadedItem = tlLoadedItems.Object();

			Objects *lpuItem = new Objects();

			// If object could be created
			if( lpuItem->Create( U_OBJECT, Unit::GetIDFromName( lpLoadedItem->lpszObjType, U_OBJECT, TRUE ) ) )
			{
				// Temporarly replace the unit's ID to load its flags and effects
				dwTempID = lpuItem->GetID();
				lpuItem->SetID       ( lpLoadedItem->dwObjID );
				lpuItem->SetQty      ( lpLoadedItem->dwQty );
				if(lpLoadedItem->lpszMadeBy)
					lpuItem->SetCreatedBy( (char*)lpLoadedItem->lpszMadeBy);
				else
					lpuItem->SetCreatedBy("");

				_item *itemStructure = NULL;

				lpuItem->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &itemStructure );

				int chargeValue = 0;
				if( itemStructure != NULL && itemStructure->magic.charges < 0 )
				{
					chargeValue = -1;
				}

				// Flush the object's CHARGE flag created by ObjectStructure
				// If the object has flags, they will be loaded.
				lpuItem->SetFlag( __FLAG_CHARGES, chargeValue );

				// Load the object's flags 
				lpuItem->LoadFlags( ODBCGuild, GUILD_CHEST_UID );
				ODBCGuild.Cancel();

				// If the item is not unique, set its charge to 1 since
				// it wasn't saved to avoid cluttering the database.
				if( !lpuItem->IsUnique() )
				{
					lpuItem->SetFlag( __FLAG_CHARGES, 1 );
				}

				// Reset the item's ID to its newly assigned one.
				lpuItem->SetID( dwTempID );

				//Put item into chest.
				pGuildSelect->pGuildChest->Put( lpuItem, true );
			}
			else
			{
				lpuItem->DeleteUnit();
			}
			tlLoadedItems.DeleteAbsolute();
		}
	}
	ODBCGuild.Cancel();
	ODBCGuild.Unlock(); 

	pGuildSelect->bGCLoaded = TRUE;
	return 0;
}



void GuildMaster::SaveGuildChest(sGuildMaster *pGuildSelect)
{
	CAutoLock autoGuildLock( &g_ALockGuild );
	CString strGuildName = pGuildSelect->strGuildName;

	// si pas trouver la guild ou que le coffre est pas la...
	// on fou rien...
	if(pGuildSelect == NULL || pGuildSelect->pGuildChest == NULL)
	{
		return;
	}

	CString strQuery;
	TemplateList< SQL_REQUEST > *lptlSQLRequests = new TemplateList< SQL_REQUEST >;



	//delete the guild from guild users
	strQuery.Format( "DELETE FROM Flags WHERE BaseOwnerID=%d AND OwnerID IN (SELECT ObjID FROM ChestGuild WHERE GuildName='%s')", GUILD_CHEST_UID,strGuildName.GetBuffer(0));
	ADD_QUERY_GUILD;
	strQuery.Format( "DELETE FROM ChestGuild WHERE GuildName='%s'",strGuildName.GetBuffer(0));
	ADD_QUERY_GUILD;


	BYTE lpszName[ 256 ];
	// Begin by saving the chest
	TemplateList<Objects> *tlChestList = pGuildSelect->pGuildChest->LockAndGetList();

	tlChestList->ToHead();
	while( tlChestList->QueryNext())
	{
		Objects *obj = tlChestList->Object();

		// If the unit name ID could be found
		if( Unit::GetNameFromID( obj->GetStaticReference(), (LPTSTR)(LPCTSTR)lpszName, U_OBJECT ) )
		{
			// Insert new object into PlayerItems table
			strQuery.Format( 
				"INSERT INTO ChestGuild( GuildName, ObjID, ObjType, Qty, MadeBy ) VALUES ( "
				"'%s',"
				"%u,"
				"'%s',"
				"%u,"
				"'%s'"
				" )",
				strGuildName,
				obj->GetID(),
				lpszName,
				obj->GetQty(),
				obj->GetCreatedBy()
				);

			ADD_QUERY_GUILD

				// If the object is shared.
				if( !obj->IsUnique() )
				{
					// Remove the charges flag to avoid cluttering the database.
					obj->RemoveFlag( __FLAG_CHARGES );
				}


				// Then save the object's specific flags.
				if( obj->SaveFlags( lptlSQLRequests, GUILD_CHEST_UID ) ){
					_LOG_DEBUG    
						LOG_CRIT_ERRORS,
						"Failed saving flags for item (ID %s) on Guild Chest (GUILD %s)",
						lpszName,
						strGuildName
					LOG_
				}


				// If the object is shared.
				if( !obj->IsUnique() ){
					// Re-set the flags to continue playing.
					obj->SetFlag( __FLAG_CHARGES, 1 );
				}

		}
	}
	pGuildSelect->pGuildChest->UnlockAndReleaseList();


	ODBCGuild.SendBatchRequest( lptlSQLRequests, NULL, NULL, "ODBCGuild" );
}

void  GuildMaster::MoveObjectFromBackpackToChest(Character *pUser,DWORD dwObjectID, DWORD dwQty ,BOOL bLogLive)
{
	CAutoLock autoGuildLock( &g_ALockGuild );
	sGuildMaster *pGuildSelect = GetGuildByName(pUser->GetGuildName());

	if(pGuildSelect == NULL || pGuildSelect->pGuildChest == NULL && pGuildSelect->bGCLoaded)
	{
		//on fou rien soit ya pas de guild, soit pas de player soit pas de coffre valide,...
		return;
	}

   pGuildSelect->LockGC.Lock();
   pGuildSelect->bGCChanged = TRUE;
   pGuildSelect->LockGC.Unlock();


	Objects *droppedObj = NULL;
	pUser->GetBackpack()->Lock();
	pUser->GetBackpack()->ToHead();
	try
	{
		while( pUser->GetBackpack()->QueryNext() )
		{

			Objects *obj = static_cast<Objects*>(pUser->GetBackpack()->Object());

			if (obj && obj->GetID() == dwObjectID) 
			{

				_item *lpItem = NULL;
				obj->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItem );
				if( lpItem == NULL )
				{
					continue;
				} 
				else if( lpItem->dwDropFlags & CANNOT_DROP_ITEM )
				{
					// This object cannot be dropped.
					pUser->SendSystemMessage( _STR( 7694, pUser->GetLang() ) );
					break;
				}

				if ( dwQty > obj->GetQty() ) 
					dwQty = obj->GetQty();


				// Determine how many of this item can be carried by the user.



				DWORD maxQty;
				if ( obj->GetStaticReference() == (UINT)__OBJ_GOLD ) 
				{ // If the user is trying to chest gold..
					maxQty = pUser->GetGold(); // Do not allow it to chest more gold than it have
					obj->SetQty(maxQty); // and make sure the gold object he is using have the right amount as Qty
				} 
				else if( obj->GetWeight() == 0 )
				{
					maxQty = 0xFFFFFFFF;
				}
				else
				{
					maxQty = ( pGuildSelect->pGuildChest->GetFreeWeight() ) / obj->GetWeight();
				}

				//CString strTmp;
				//strTmp.Format("Chest Limit = %d   Chest Libre = %d  Item Poids = %d  maxQty = %d",pGuildSelect->pGuildChest->GetMaxWeight(),
				//                                                       pGuildSelect->pGuildChest->GetFreeWeight(),
				//                                                       obj->GetWeight(),maxQty)  ;
				//pUser->SendSystemMessage( strTmp );

				// Impossible
				if( maxQty == 0 )
				{
					// No space left on chest
					pUser->SendSystemMessage( _STR( 15093, pUser->GetLang() ) );
					break;
				}

				if (dwQty > maxQty) 
				{
					dwQty = maxQty;
				}

				bool forceQtyToOne = false;
				// If there are more than 1 item in the backpack
				if( obj->GetQty() > 1 )
				{
					// Create a copy of the backpack item
					droppedObj = new Objects();
					if( !droppedObj->Create( U_OBJECT, obj->GetStaticReference() ) )
					{
						droppedObj->DeleteUnit();
						droppedObj = NULL;
						continue;
					}
					// Set the quantity of items to the quantity of dropped items.
					droppedObj->SetQty( dwQty );
				}
				else
				{
					// Used the backpack item as the chest item.
					droppedObj = obj;
					// Add 1 qty count to avoid deleting the unit.
					forceQtyToOne = true;

					pUser->GetBackpack()->Remove();
				}

				// Remove the quantity of objects dropped from the backpack item.

				if( forceQtyToOne )
				{
					obj->SetQty( 1 );
				}
				else
				{
					obj->Remove( dwQty );
				}
				if( obj->GetQty() == 0 )
				{
					pUser->GetBackpack()->Remove();
					obj->DeleteUnit();
				}

				break;
			}

		}
	}
	catch (...)
	{
		droppedObj = NULL;
	}
	pUser->GetBackpack()->Unlock();


	if (droppedObj != NULL) 
	{
		//Save name and ID to use on LOG
		CString csItemName = droppedObj->GetName(_DEFAULT_LNG);
		char szItemID[256];
		Unit::GetNameFromID( droppedObj->GetStaticReference(), szItemID, U_OBJECT );

		if( droppedObj->GetStaticReference() == (UINT)__OBJ_GOLD )
		{
			pUser->SetGold( pUser->GetGold() - droppedObj->GetQty(), false );
		}

		if ( !pGuildSelect->pGuildChest->Put(droppedObj) ) 
		{
			if( droppedObj->GetStaticReference() == (UINT)__OBJ_GOLD )
			{
				pUser->SetGold( pUser->GetGold() + droppedObj->GetQty(), false );
			}
			pUser->AddToBackpack(droppedObj);
		} 
		else 
		{

			// Shoot a chest update!
			pUser->SendGuildChestPacket(FALSE);
			// Shoot a backpack update.
			TFCPacket sending;

			sending << (RQ_SIZE)RQ_ViewBackpack2;
			sending << (char)0;	// Do not show content of backpack, only update it.
			sending << (long)pUser->GetID();
			pUser->PacketBackpack(sending);

			pUser->SendPlayerMessage( sending );

			if(bLogLive)
			{
				AddGuildLog(pUser->GetGuildName(),_STR( 15094, pUser->GetLang() ),pUser->GetTrueName(),dwQty,csItemName);
			}
			else
			{
				CString strTmp;
				strTmp.Format(_STR( 15094, pUser->GetLang() ),pUser->GetTrueName(),dwQty,csItemName);
				//theApp.AddGuildRequest(pUser,NULL,NULL,GUILD_ADD_LOGS,0,0,0,0,strTmp,"");
				theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_ADD_LOGS,pUser->GetID(),0,0,0,strTmp,"");
			}

			BroadcastToAllOnlineUser(pUser->GetGuildName(),FALSE,FALSE,TRUE,_STR( 15095, pUser->GetLang() ),pUser->GetTrueName(),dwQty,csItemName);

			_LOG_GUILDCHEST
				LOG_MISC_1,
				"Player %s (%s) added %u item %s ID( %s ) to guild chest : %s",
				pUser->GetTrueName(),
				pUser->GetPlayer()->GetFullAccountName(),
				dwQty,
				csItemName,
				szItemID,
				pUser->GetGuildName()
				LOG_
		}
	}
}


void  GuildMaster::MoveObjectFromChestToBackpack(Character *pUser,DWORD dwObjectID, DWORD dwQty ,BOOL bLogLive)
{
	CAutoLock autoGuildLock( &g_ALockGuild );
	sGuildMaster *pGuildSelect = GetGuildByName(pUser->GetGuildName());

	if(pGuildSelect == NULL || pGuildSelect->pGuildChest == NULL && pGuildSelect->bGCLoaded)
	{
		//on fou rien soit ya pas de guild, soit pas de player soit pas de coffre valide,...
		return;
	}

   pGuildSelect->LockGC.Lock();
   pGuildSelect->bGCChanged = TRUE;
   pGuildSelect->LockGC.Unlock();


	int iObjWeight=0;
	if ( pGuildSelect->pGuildChest->GetItemWeight(dwObjectID, iObjWeight) ) 
	{
		// Determine how many of this item can be carried by the user.
		DWORD maxQty = 0;
		if( iObjWeight == 0 )
		{
			maxQty = 0x7FFFFFFF;
		}
		else if (pUser->GetWeight() >= pUser->GetMaxWeight()) 
		{
			maxQty = 0;
		} 
		else 
		{
			maxQty = ( pUser->GetMaxWeight() - pUser->GetWeight() ) / iObjWeight;
		}

		// Impossible
		if( maxQty == 0 )
		{
			pUser->SendSystemMessage( _STR( 17, pUser->GetLang() ) );
			return;
		}

		// If the item has more items than the user can carry.
		if( dwQty > maxQty )
		{
			dwQty = maxQty;
		}

		/*
		_LOG_ITEMS
		LOG_MISC_1,
		"Avant OBJ TAKE"
		LOG_
		*/

		Objects *obj = pGuildSelect->pGuildChest->Take(dwObjectID, dwQty);
		if (obj != NULL) 
		{
         if(obj->GetQty() != dwQty)
            dwQty = obj->GetQty();



			CString csItemName = obj->GetName(_DEFAULT_LNG);
			char szItemID[256];
			Unit::GetNameFromID( obj->GetStaticReference(), szItemID, U_OBJECT );


			if( obj->GetStaticReference() == (UINT)__OBJ_GOLD )
			{
				pUser->SetGold( pUser->GetGold() + obj->GetQty(), false );
			} 
			else 
			{ // Only call AddToBackpack if the object is not a gold pile
				pUser->AddToBackpack(obj);
			}



			// Shoot a chest update!
			pUser->SendGuildChestPacket(FALSE);

			// Shoot a backpack update.
			TFCPacket sending;
			sending << (RQ_SIZE)RQ_ViewBackpack2;
			sending << (char)0;	// Do not show content of backpack, only update it.
			sending << (long)pUser->GetID();
			pUser->PacketBackpack(sending);

			pUser->SendPlayerMessage( sending );




			if(bLogLive)
			{
				AddGuildLog(pUser->GetGuildName(),_STR( 15096, pUser->GetLang() ),pUser->GetTrueName(),dwQty,csItemName);
			}
			else
			{
				CString strTmp;
				strTmp.Format(_STR( 15096, pUser->GetLang() ),pUser->GetTrueName(),dwQty,csItemName);
				//theApp.AddGuildRequest(pUser,NULL,NULL,GUILD_ADD_LOGS,0,0,0,0,strTmp,"");
				theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_ADD_LOGS,pUser->GetID(),0,0,0,strTmp,"");
			}



			BroadcastToAllOnlineUser(pUser->GetGuildName(),FALSE,FALSE,TRUE,_STR( 15097, pUser->GetLang() ),pUser->GetTrueName(),dwQty,csItemName);

			_LOG_GUILDCHEST
				LOG_MISC_1,
				"Player %s (%s) removed %u item %s ID( %s ) from guild chest : %s",
				pUser->GetTrueName(),
				pUser->GetPlayer()->GetFullAccountName(),
				dwQty,
				csItemName,
				szItemID,
				pUser->GetGuildName()
				LOG_

		} 
		else 
		{
			pUser->SendSystemMessage( _STR( 12918,pUser->GetLang() ) );
		}
	}
}

int  GuildMaster::GetChestPacket(BOOL bLock,Character *pUser, TFCPacket &packet)
{
	if(bLock)
		CAutoLock autoGuildLock( &g_ALockGuild );
	sGuildMaster *pGuildSelect = GetGuildByName(pUser->GetGuildName());

	if(pGuildSelect == NULL || pGuildSelect->pGuildChest == NULL)
	{
		//on fou rien soit ya pas de guild, soit pas de player soit pas de coffre valide,...
		return -1;
	}

   if(pUser->ViewFlag(__FLAG_PLAYER_USE_NEW_CHEST) == 1 && theApp.dwChestListEnable == 1)
	   pGuildSelect->pGuildChest->GetPacketNew(packet,pUser->GetLang());
   else
      pGuildSelect->pGuildChest->GetPacket(packet);

	return 0;
}

BOOL GuildMaster::GetChestUnitName(Character *pUser, DWORD dwID)
{
	CAutoLock autoGuildLock( &g_ALockGuild );

	BOOL boFound = FALSE;

	CString sGuildName(pUser->GetGuildName());
	if (pUser->GetGameOpContext()) {
		sGuildName = pUser->GetGameOpContext()->GetGuildName();
	}
	sGuildMaster *pGuildSelect = GetGuildByName(sGuildName);

	if(pGuildSelect == NULL || pGuildSelect->pGuildChest == NULL)
	{
		//on fou rien soit ya pas de guild, soit pas de player soit pas de coffre valide,...
		return boFound;
	}


	BYTE chSearchWhat = 4; //   PL_SEARCHGUILDCHEST
	ItemContainer *lpicChest = pGuildSelect->pGuildChest;
	TemplateList<Objects> *tlChestList = lpicChest->LockAndGetList();
	tlChestList->ToHead();
	while( tlChestList->QueryNext() && !boFound )
	{
		Objects *obj = tlChestList->Object();

		if( obj->GetID() == dwID )
		{
			TFCPacket sending;
			sending << (RQ_SIZE)RQ_QueryItemName;
			sending << (char)chSearchWhat;
			sending << (long)obj->GetID();
			CString csName = obj->GetName( pUser->GetLang() );
			sending << (CString)( csName );
			pUser->SendPlayerMessage( sending );
			boFound = TRUE;                
		}
	}
	lpicChest->UnlockAndReleaseList();

	return boFound;
}

int  GuildMaster::SendGuildList(Character *pUser, char chShow)
{
	CAutoLock autoGuildLock( &g_ALockGuild );

	CString strNom;
	CString strNotes;
	DWORD   dwUserPermission;
	USHORT  ushQty = 0;

	strNom           = pUser->GetGuildName();
	dwUserPermission = pUser->GetGuildPermission();


	GetNoteByGuildName(strNom,strNotes);
	ushQty = GetNbrGuildUserCount(strNom);

	TFCPacket sending;
	sending << (RQ_SIZE)RQ_NM_GetGuildList;


	//envoie les 4 nom de rang autre que le fondateur...
	for(int i=0;i<5;i++)
	{
		CString strRangName;
		strRangName.Format("%s",_STR( 15059 +i, pUser->GetLang() ));
		sending << strRangName;
	}

	sending << (char)chShow;
	sending << strNom;
	sending << (long)dwUserPermission;
	sending << strNotes;
	sending << (short)ushQty;

	CString strPlayerName   = "Unknow";
	BYTE chOnline           = 0;

	int iSend = 0;

	for(int i=0;i<GetNbrGuildUser();i++)
	{

		strPlayerName   = "Unknow";
		chOnline        = 0;  

		sGuildUser* pGUser = GetGuildUserByIndex(i);
		if(pGUser )
		{

			if(pGUser->strGuildName.CompareNoCase(strNom) == 0 && iSend <ushQty)
			{
				//On a un user de la guild, on ajoute les info...
				Players * pPlayer = CPlayerManager::GetCharacterRessourceByID(pGUser->dwPJOwnerID);//PM
				if(pPlayer)
				{
					strPlayerName = pPlayer->self->GetTrueName();
					chOnline = 1;
					CPlayerManager::FreePlayerResource(pPlayer);
				}
				else
				{
					//strPlayerName = "Joueur hors ligne";
					GetNameOfflineUser(pGUser->dwPJOwnerID,strPlayerName); ///NMNMNM_HL
				}


				sending << strPlayerName;
				sending << (long)pGUser->dwTitre;
				sending << (long)pGUser->dwPJOwnerID;
				sending << (long)pGUser->dwPermession;
				sending << (char)chOnline;

				iSend++;
			}
		}
	}


	if(pUser)
		pUser->SendPlayerMessage(sending);

	return 0;
}

