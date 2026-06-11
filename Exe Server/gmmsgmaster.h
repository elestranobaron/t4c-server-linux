// GMMsgMaster.h: interface for the Professions class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
//GMMsg
typedef struct _sGMMsg
{
   DWORD   dwIndexID;         //ID
   DWORD   bStatus;           //Status    
   DWORD   dwCreateTime;      //CreateTime
   CString strTimeStamp;      //TimeStamp 
   DWORD   dwOwnerID;         //OwnerID
   CString strPlayer;         //PlayerName
   CString strMessage;        //Message
}sGMMsg;

class GMMsgMaster;

// The class
class __declspec(dllexport) GMMsgMaster
{
public:
	
	static void Create();  // Creates all internal structures
   static void Destroy(); // Destroys all internal structures
	static void SaveAllGMMsg();

   

   static int     PostGMSystemMessage(CString strSystem, CString strMessage);
   static int     PostGMMessage(Players *pPlayer, CString strMessage);
   static int     CloseGMMessage(UINT uiMessageID);
   static void    SendAllOpenedGMMessage(Players *pPlayer);
   
   
  
   

private:
   static int     GetGMMessageNbr();
   static BOOL    GetGMMessageStatus(UINT uiMessageID);
   static BOOL    GetGMMessageUnique(DWORD dwOwnerID);
   static sGMMsg* GetGMMessage(UINT uiMessageID);

   
private:
	static BOOL      g_GMMsgChanged;
	static CPtrArray c_aGMMsgMaster;
};


