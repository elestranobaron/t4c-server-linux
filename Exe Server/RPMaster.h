// RPMaster.h: interface for the Professions class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RPMASTER_H__D02F3B81_5542_11D1_BD7A_00E029058623__INCLUDED_)
#define AFX_RPMASTER_H__D02F3B81_5542_11D1_BD7A_00E029058623__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

typedef struct _sRPPl
{
   Players *pPlayer;
}sRPPl;

typedef struct _sIntercationRP
{
   int     iSignaledNOTRP;
   int     iMessageID;
   Players *pPlayer;
   Players *pPlayerWaitingAnswer;
   CString strRPSujet;
   CPtrArray aPlayerList;
   
   DWORD   iWaitingCnt;
}sIntercationRP;


typedef struct _sInviteRP
{
   int     iCntTimeout;
   Players *pPlayerInvite;
   Players *pPlayerInvited;
}sInviteRP;



class RPMaster;

// The class
class __declspec(dllexport) RPMaster
{
public:
	
	static void Create();  // Creates all internal structures
   static void Destroy(); // Destroys all internal structures

   static int  CreateNewRP(Players *pPlayer,CString strMessage);
   static int  RPBroadcastInfo(Players *pPlayer);
   static int  RPInteractionTerminate(Players *pPlayer);
   static int  RPRejoindreRP(Players *pPlayer,int iRPID);
   static int  RPRejoindreRPResult(Players *pPlayer,int iRPAnswer);
   static int  RPInteractionExpluser(Players *pPlayer,int iPlID);
   static int  RPInviteRP(Players *pPlayer,int iInvitedID);
   static int  RPInviteRPResult(Players *pPlayer,int iRPAnswer);
   static int  RPSignalerRP(Players *pPlayer,int iRPID);
   static int  RPEchangerRPDirect(Players *pPlayer);
   static int  RPSendRPList(Players *pPlayer);
   static BOOL RPTerminateRP(int iMessageID);
   static BOOL RpExist(int iRPID);
   static BOOL RpExistAndCreateur(int iRPID, int iPlID);
   static int  RPInteractionTerminateLogOff(Players *pPlayer);
   static int  RPInteractionQuitterLogOff(Players *pPlayer);

   static int  ManageInteractionRP();
	
 
private:

   static int  RPGetIsOnRP(int iplID,char &chIsRP, int &iRPIndex);
   static BOOL IsInRPRange(int iP1X,int iP1Y,int iP2X,int iP2Y);
   static int  CalculatePlayerPoints(int n,Players *pPlayer,int &iRule1,int &iRule2,int &iRule3,int &iRule4);

   
private:
	static CPtrArray c_aInterRP;
   static CPtrArray c_aInviteRP;
   static int       c_InterRPIndex;
};

#endif // !defined(AFX_RPMASTER_H__D02F3B81_5542_11D1_BD7A_00E029058623__INCLUDED_)
