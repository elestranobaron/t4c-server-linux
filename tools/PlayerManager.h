/******************************************************************************
Modify for vs2008 (06/05/2009)
Add Many use tools fct for AH, Guild, Firewall, etc etc by NightMare (29/06/2009)
/******************************************************************************/
#pragma once

#include "ChatterChannels.h"
#include "MultiReaderSingleWriter.h"

using namespace std;

class Players;
class ChatterChannels;
typedef pair< string, string > UserInfo;

typedef struct _sSockBooth
{
   sockaddr_in skI;
   sockaddr_in skO;
}sSockBooth;

/******************************************************************************/
class __declspec(dllexport) CPlayerManager  
   /******************************************************************************/
{
public:

   static void Create( void );
   static void Destroy( void );

   // Player resource checking.
   static Players *GetPlayerResourceFct( sockaddr_in sockAddr ); //oki protected...
   static void     FreePlayerResourceFct( Players *lpPlayer );
   static BOOL     IsPlayerResourceExist( sockaddr_in sockAddr );// The address of the player to fetch.

   static Players *CreatePlayer( sockaddr_in sockAddrO,sockaddr_in sockAddrI, CString csAccountName ); //oki protected...
   static void     DeletePlayer( Players *lpPlayer, BOOL boIdle = FALSE );

   // Fetches the IPs of all players targetted by a broadcasted message.
   static void GetGlobalBroadcastAddress( vector< sSockBooth > &vAddresses, SendPacketVisitor *packetVisitor, bool inGame = true );
   static void GetGlobalBroadcastAddressLevelRange(vector< sSockBooth > &vAddresses,SendPacketVisitor *packetVisitor,bool inGame, int iLevelMin,int iLevelMax);
   static void GetLocalBroadcastAddress ( vector< sSockBooth > &vAddresses, WorldPos wlPos, int nRange, SendPacketVisitor *packetVisitor );

   // Returns user names or adresses.
   //static BOOL PageUser( Players *sender, CString &csName, CString csSender, CString csMessage, bool boCanPageInvisible );
   //static void PacketUserList( Players *lpUser );
   static void PacketUserPos( Players *lpUser );


   static int  GetUserCount( void ){ return nUserCount; };

   static BOOL VerifyPlayerUnique( Players *lpPlayer );


   static Players *GetCharacterOld( CString csName );          //oki Utiliser uniquement en commande GM donc a faire plus tard...
   static Players *GetCharacterOldByID( DWORD dwID );          //oki Utiliser uniquement en commande GM donc a faire plus tard...

   static Players *GetCharacterRessource( CString csName);  //oki protected...
   static Players *GetCharacterRessourceByID(DWORD dwID);   //oki protected...
   static void     FreePlayerResource( Players *lpPlayer );


   static void SendNeerUnitMessage(int X, int Y, int W, CString strMsg);
   static void KillUserInJaill();
   //static int  SendHLLListTo(Players *pPlayer);
   static void SendMessagetoAllGOD(CString strMsg);
   static void SetPlayerGuildNameAndCC(CString strGN, CString strNGN);
   static void BroadcastGuildInfo(CString StrInfo, CString strGuildName,BOOL bResetGuildInfo,BOOL bUpdateGuildList,BOOL bUpdateGuildChest);
   static int  GetNbrPlayerInGameOnThisGuild(CString strGuildName, int &iPlayerID);
   static void ForceSaveGuildMember(CString strGuildName,Character *pUser);


   static void SaveAll( void );
   static void RemoveTargetReferences( Unit *targetUnit , bool bResetContext);
   static ChatterChannels &GetChatter( void );
   static void RefreshSystemChannels();

protected:
   static void GrowBufferSpace( void );
   static UINT CALLBACK PlayerMaintenance( LPVOID lpData );
   static UINT CALLBACK AsyncDeletePlayer( LPVOID lpData );
   static UINT CALLBACK SystemProcess( LPVOID lpData );
   static UINT CALLBACK AutoCastProcess( LPVOID lpData );

   static HANDLE hDeletionIo;

   static MultiReaderSingleWriter *pMultiRSingleW;
   static Players **lpRegisteredUsers;
   static int nUserCount;
   static int nBufferSize;

   static CLock cMaintenanceLock;

   static HANDLE hMaintenanceThread;
   static HANDLE hSystemProcessThread;
   static HANDLE hAutoCastProcessThread;

   static ChatterChannels *lpChatter;
};
