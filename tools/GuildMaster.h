// GuildMaster.h: interface for the Professions class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GUILDMASTER_H__D02F3B81_5542_11D1_BD7A_00E029058623__INCLUDED_)
#define AFX_GUILDMASTER_H__D02F3B81_5542_11D1_BD7A_00E029058623__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

typedef struct _sGuildMaster
{
   CString strGuildName;
   DWORD   dwOwnerID;
   CString strNote;
   ItemContainer *pGuildChest; // The chest :D
   BOOL  bGCChanged;
   BOOL  bGCLoaded;
   CLock LockGC;
}sGuildMaster;

typedef struct _sGuildUser
{
   
   CString strGuildName;
   DWORD   dwPJOwnerID;
   DWORD   dwTitre;
   DWORD   dwPermession;
   CString strLastPlayerName;
}sGuildUser;

typedef struct _sGuildPermission
{
   DWORD   Leader      :1;
   DWORD   CanInvite   :1;
   DWORD   CanKick     :1;
   DWORD   ViewChest   :1;
   DWORD   CanDeposit  :1;
   DWORD   CanTake     :1;
   DWORD   CanWriteNote:1;
   DWORD   CanChPerm   :1;
   DWORD   CanSeeLog   :1;
   DWORD   Reserved04:23;
} sGuildPermission;

typedef union _uGuildPermission
{
   DWORD  dwVal;
   sGuildPermission Permission;
}uGuildPermission;


#define GUILD_TITRE_FONDATEUR 1
#define GUILD_TITRE_CHEF      2
#define GUILD_TITRE_RECRUTEUR 3
#define GUILD_TITRE_MEMBRE    4
#define GUILD_TITRE_RECRU     5

#define GUILD_FONDATEUR_RIGHT  0x000001FF
#define GUILD_RECRU_RIGHT      0x00000000



class GuildMaster;

// The class
class __declspec(dllexport) GuildMaster
{
public:
	
	static void Create();  // Creates all internal structures
   static void Destroy(); // Destroys all internal structures
	
   static int  CreateNewGuild(CString strGuildName,DWORD dwUserID, Players *pPlayer);
   static int  ModifyGuild(CString strGuildName,DWORD dwUserID, Players *pPlayer);
   static int  DeleteGuild(CString strGuildName);
   static int  RenameGuild(CString strGuildName,CString strNewGuildName);
  

   static void AddUserGuild(Character *pUser, char *pStrGuildName);
   static int  RemoveUserGuild(Character *pUser,bool bKick,CString strKickName);
   static int  RemoveUserGuildOffline(DWORD dwID,CString strKickName);
   static int  ChangeUserSettings(Character *pUser,DWORD dwID,DWORD dwTitre, DWORD dwPermission,CString strName);
   static void ChangeNoteSettings(CString strGuildName,char*pstrNote,CString strName);
   static void SendLogsList(Character *pUser);
   static void RenameLastUserName(Character *pUser,CString strName);
   static void AddGuildLog(CString strGuildName,LPCTSTR pszFmt, ...);
   static int  SendGuildList(Character *pUser, char chShow);

   static bool GetUserGuildInfo(UINT uiID, char *pStrName, char *pStrLastName, DWORD &dwLevel, DWORD &dwPermission);
   static void GetGuildList(CStringArray &astrGuild);
   static bool IsExistGuildP(char *pStrGuildName);

   //static void ResetGuildChest(CString strGuildName, DWORD dwID);

   static void MoveObjectFromBackpackToChest(Character *pUser,DWORD dwObjectID, DWORD dwQty ,BOOL bLogLive);
   static void MoveObjectFromChestToBackpack(Character *pUser,DWORD dwObjectID, DWORD dwQty ,BOOL bLogLive);

   static int  GetChestPacket(BOOL bLock,Character *pUser, TFCPacket &packet);
   static BOOL GetChestUnitName(Character *pUser, DWORD dwID);

   static void SaveAllGuildChest();
   

   static sGuildMaster* GetGuildByName(CString sGuildName);
   static sGuildUser*   GetGuildOwner(CString sGuildName);

   static void SaveAllGuilds(bool bForce);
   
   static int  LoadGuildChest(CString strGuildName);

   static bool IsGuildChanged();
private:
	static void _DeleteAllGuilds(TemplateList< SQL_REQUEST > *lptlSQLRequests);
	static void _SaveGuild(sGuildMaster* guild, TemplateList< SQL_REQUEST > *lptlSQLRequests);
	static void _SaveGuildUser(sGuildUser* user, TemplateList< SQL_REQUEST > *lptlSQLRequests);

   
   static void SaveGuildChest(sGuildMaster *pGuildSelect);

   static int  GetNbrGuildPriv();
   static int  GetNbrGuildUser();
   static int  GetNbrGuildUserCount(CString strGuildName);
   static void GetNoteByGuildName(CString strGuildName,CString &strNoteGuild);
   static void GetNameGuild(DWORD dwID,CString &strNameGuild);
   static sGuildUser* GetGuildUserByIndex(int iIdx);
   static void GetNameOfflineUser(DWORD dwID,CString &strNameHL);

   static bool IsExistGuild(CString strGuildName);
   static void BroadcastToAllOnlineUser(CString strGuildName,BOOL bResetGuildInfo,BOOL bUpdateGuildList,BOOL bUpdateGuildChest,LPCTSTR pszFmt, ...);

   
private:
   static bool m_bGuildChnaged;
   
   //vector< sGuildMaster * > c_aGuildMaster;
   //vector< sGuildUser   * > c_aGuildMaster;
	static CPtrArray c_aGuildMaster;
   static CPtrArray c_aGuildUsers;

	/* Item Flags and Boosts are saved belonging to a baseOwner (the character)
	* For item's in the guild chests, the owner is the chest, for which we 
	* use zero as ID */
	static const int GUILD_CHEST_UID = 0;

};

#endif // !defined(AFX_GUILDMASTER_H__D02F3B81_5542_11D1_BD7A_00E029058623__INCLUDED_)
