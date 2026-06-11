/******************************************************************************
Modify for vs2008 (30/04/2009)
Add Guild CC reserved by Nightmare (27/06/2009)
Add CC listing sort   by Nightmare (27/06/2009)
Add NMS death system validation by Nightmare (27/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "ChatterChannels.h"
#include "TFCPacket.h"
#include "TFC_MAIN.h"
#include "PlayerManager.h"
#include "GuildMaster.h"
#include "StatModifierFlagsListing2.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

using namespace std;

/******************************************************************************/
ChatterChannels::ChatterChannels() : mainChannel( "Main" )
/******************************************************************************/
{
   Channel *lpChannel = &( mSystemChannels[ mainChannel ] );
   lpChannel->ciChannelID = mainChannel;

   publicChannelsForOperators = false;
}
/******************************************************************************/
ChatterChannels::~ChatterChannels()
/******************************************************************************/
{
}
/******************************************************************************/
//  Adds a system channel to the chatter channels.
void ChatterChannels::AddSystemChannel(std::string channelId)
/******************************************************************************/
{
   CAutoLock cAutoLock( this );

   Channel *lpChannel = &( mSystemChannels[ channelId ] );
   lpChannel->ciChannelID = channelId;
}
/******************************************************************************/
//  Clears all system channels except the main one.
void ChatterChannels::ClearSystemChannels( void )
/******************************************************************************/
{   
   CAutoLock cAutoLock( this );

   mSystemChannels.clear();

   Channel *lpChannel = &( mSystemChannels[ mainChannel ] );
   lpChannel->ciChannelID = mainChannel;
}
/******************************************************************************/
//  Returns the main channel's name
string ChatterChannels::GetMainChannel( void )
/******************************************************************************/
{
    return mainChannel;
}
/******************************************************************************/
// Add a player to a chatter channel, or create the chatter channel.
bool ChatterChannels::AddCCPlayer(
 Players *lpPlayer,     // UserID of the player to add.
 ChannelID ciChannelID, // ID of the channel to add player to.
 string csPassword,     // The password for the channel.
 bool   bValidGuildCC   // if need to valid the guild reserved CC
)
/******************************************************************************/
{
   CAutoLock cAutoLock( this );

   // Get the channel.
   ChannelMap::iterator i = mChannels.find( ciChannelID );    

   //lock if the chanal is a Guildname...
   //we reserve guildname chanal for the guild...

   if(bValidGuildCC)
   {
      if(GuildMaster::IsExistGuildP((char*)ciChannelID.c_str()))
      {
         //send error is a reserved guild canal...
         //if user is on this guild, we nor create it, because is created by system...
         //but not send message
         if(_strcmpi(lpPlayer->self->GetGuildName().GetBuffer(0),(char*)ciChannelID.c_str()) != 0)
            lpPlayer->self->SendSystemMessage( _STR( 15110, lpPlayer->self->GetLang() ) );

         return false;
      }
   }

   Channel *lpChannel = NULL;

   // If the channel wasn't in the map.
   if( i == mChannels.end() )
   {
      // Try to find it in the system channels.
      i = mSystemChannels.find( ciChannelID );

      // If it couldn't be found in the system channels.
      if( i == mSystemChannels.end() )
      {
         // Get the channel (this will create a channel slot).
         lpChannel = &( mChannels[ ciChannelID ] );

         // Set the password.
         lpChannel->csPassword  = csPassword;

         // Set the channelID.
         lpChannel->ciChannelID = ciChannelID;
      }
      else
      {
         // No need to authenticate for system channels.
         lpChannel = &( (*i).second );
      }
   }
   else
   {
      lpChannel = &( (*i).second );
      // If passwords do not correspond and player isn't master.
      if( lpChannel->GetPassword() != csPassword && !( lpPlayer->GetGodFlags() & GOD_CHAT_MASTER ) )
      {
         // Verify if any channels are now empty.
         VerifyEmptyChannels();

         // Send a message sending the password was wrong
         lpPlayer->self->SendSystemMessage( _STR( 7506, lpPlayer->self->GetLang() ) );

         // Refuse channel adding.
         return false;
      }
   }

   // Add the user to this channel.
   if( !lpChannel->vUsers.insert( Channel::ChannelUser( lpPlayer ) ).second )
   {
      TRACE( "\nInsertion failed. Users in channel:" );
   }

   return true;
}
/******************************************************************************/
// Remove a player from a chatter channel
void ChatterChannels::Remove(
                             Players *lpPlayer,     // ID of the user to remove.
                             ChannelID ciChannelID  // The ID of the channel to remove the user from.
                             )
                             /******************************************************************************/
{
   CAutoLock cLock( this );

   ChannelMap::iterator i;

   // Try to find the channel
   i = mChannels.find( ciChannelID );

   // If the channel was not found.
   if( i == mChannels.end() )
   {        
      // Try to find in system channels.
      i = mSystemChannels.find( ciChannelID );

      // If its not even in the system channels.
      if( i == mSystemChannels.end() )
      {
         return;
      }

      Channel &cChannel = (*i).second;

      // Try to find the player inside the system channel.
      Channel::UserCont::iterator k = cChannel.vUsers.find( lpPlayer );

      // If the player wasn't found (??)
      if( k == cChannel.vUsers.end() )
      {
         return;
      }

      // If we were previously listening.
      if( (*k).listening )
      {
         // Simply toggle listening to false.
         (*k).listening = false;
      }

      // We cannot remove a user from a system channel.
      return;
   }

   Channel &cChannel = (*i).second;

   // Find the corresponding user in the chatter channel.
   Channel::UserCont::iterator j = cChannel.vUsers.find( lpPlayer );

   // If the user was not found in the chatter channel.
   if( j == cChannel.vUsers.end() )
   {
      return;
   }

   // Remove it from the channel.
   cChannel.vUsers.erase( j );

   // Verify for empty channels.
   VerifyEmptyChannels();
}    
/******************************************************************************/
// Remove any reference to a player in the chatter channels.
void ChatterChannels::Remove( Players *lpPlayer )// The player to remove from all chatter channels.
/******************************************************************************/
{
   CAutoLock cLock( this );

   ChannelMap::iterator i;

   // For all channels available
   for( i = mChannels.begin(); i != mChannels.end(); i++ )
   {
      Channel &cChannel = (*i).second;
      cChannel.vUsers.erase( lpPlayer );
   }

   // Search the system channels too.
   for( i = mSystemChannels.begin(); i != mSystemChannels.end(); i++ )
   {
      Channel &cChannel = (*i).second;
      cChannel.vUsers.erase( lpPlayer );
   }

   // Verify for empty channels.
   VerifyEmptyChannels();
}
/******************************************************************************/
// Sends a message through a chatter channel.
void ChatterChannels::Talk(
                           Players *lpSender,     // ID of the sender.
                           ChannelID ciChannelID, // Channel to send message from.
 string csMessage      // Message.
)
/******************************************************************************/
{
   CAutoLock cLock( this );

   CString csTheMessage = csMessage.c_str();

   if(csTheMessage.GetLength() > 256)
   {
      _LOG_CHEAT
         LOG_MISC_1,
         "Player %s (IP:%s) TRY Crash RQ_SHOUT, message > 256",
         (LPCTSTR)lpSender->self->GetTrueName(),
         lpSender->GetIP()
         
         LOG_
         return;

   }

   // If user got squelched or has no more shouts.
   if( !lpSender->boCanShout || !lpSender->boCanPage )
   {
      lpSender->self->SendSystemMessage( _STR( 7171, lpSender->self->GetLang() ) );
      return;
   }

   if( lpSender->self->ViewFlag( __FLAG_NMS_PLAYER_DEATH ) != 0 )
   {
      //le PJ est mort, il ne peu pas bouger...
      return;
   }
   ChannelMap::iterator i;

   bool systemChannel = false;

   // Try to find the channel
   i = mChannels.find( ciChannelID );

   // If the channel was not found.
   if( i == mChannels.end() )
   {
      // Search the system channels.
      i = mSystemChannels.find( ciChannelID );
      if( i == mSystemChannels.end() ){
         return;
      }
      systemChannel = true;
   }

   Channel &cChannel = (*i).second;
   Channel::UserCont::iterator k = cChannel.vUsers.find( lpSender );

   // If the user cannot be found in this channel (access right authentication) or isn't master.
   if( k == cChannel.vUsers.end() && !( lpSender->GetGodFlags() & GOD_CHAT_MASTER ) )
   {
      return;
   }

   if( k != cChannel.vUsers.end() )
   {
      // If the user isn't listening on the channel.
      if( !(*k).listening ){
         // It does not have the right to talk.
         return;
      }
   }

   if( systemChannel )
   {
      // If the channel is locked by operators and the sender isn't a gameop
      if( publicChannelsForOperators && !( lpSender->GetGodFlags() & GOD_CHAT_MASTER ) )
      {
         lpSender->self->SendSystemMessage( _STR( 467, lpSender->self->GetLang() ) );
         return;
      }

      // If the player cannot talk in public channels right now.
      if( !lpSender->CanShout() )
      {
         lpSender->self->SendSystemMessage( _STR( 467, lpSender->self->GetLang() ) );
         return;
      }
      lpSender->ToggleShout();

      _LOG_SHOUTS
         LOG_MISC_1,
         "Player %s said in public channel %s: %s",
         (LPCTSTR)lpSender->self->GetTrueName(),
         ciChannelID.c_str(),
         csMessage.c_str()
         LOG_
   }
   else
   {
      _LOG_PAGE
         LOG_MISC_1,
         "Player %s said in chatter channel %s: %s",
         (LPCTSTR)lpSender->self->GetTrueName(),
         ciChannelID.c_str(),
         csMessage.c_str()
         LOG_
   }

   // Send the message to all players in the chatter channel.
   CString csSender     = lpSender->self->GetTrueName();
   CString csChatterID  = ciChannelID.c_str();

   // Scroll through the list of all player on this channel.
   Channel::UserCont::const_iterator j;
   for( j = cChannel.vUsers.begin(); j != cChannel.vUsers.end(); j++ )
   {

      // If the player is listening on the channel.
      Players *pNMPJ = (*j).player;
      if( (*j).listening && (*j).player->in_game && (*j).player->self) 
      {            
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_SendChatterMessage;
         sending << csChatterID;
         sending << csSender;
         sending << csTheMessage;

         TRACE( "\nSending message to %s.", (*j).player->self->GetTrueName() );

         // Send them the message.
         (*j).player->self->SendPlayerMessage( sending );
      }
   }
}

void ChatterChannels::TalkSystem(CString csSender ,ChannelID ciChannelID,string csMessage)
/******************************************************************************/
{
   CAutoLock cLock( this );
   CString csTheMessage = csMessage.c_str();
   // If user got squelched or has no more shouts.

   ChannelMap::iterator i;
   bool systemChannel = false;
   // Try to find the channel
   i = mChannels.find( ciChannelID );
   // If the channel was not found.
   if( i == mChannels.end() )
   {
      // Search the system channels.
      i = mSystemChannels.find( ciChannelID );
      if( i == mSystemChannels.end() ){
         return;
      }
      systemChannel = true;
   }
   Channel &cChannel = (*i).second;

   // Send the message to all players in the chatter channel.
   CString csChatterID  = ciChannelID.c_str();

   // Scroll through the list of all player on this channel.
   Channel::UserCont::const_iterator j;
   for( j = cChannel.vUsers.begin(); j != cChannel.vUsers.end(); j++ )
   {

      // If the player is listening on the channel.
      Players *pNMPJ = (*j).player;
      if( (*j).listening && (*j).player->in_game && (*j).player->self) 
      {            
         TFCPacket sending;
         sending << (RQ_SIZE)RQ_SendChatterMessage;
         sending << csChatterID;
         sending << csSender;
         sending << csTheMessage;
         // Send them the message.
         (*j).player->self->SendPlayerMessage( sending );
      }
   }
}

/******************************************************************************/
// Sets the new 'listening' state for a given channel.
void ChatterChannels::ToggleListening(
 Players *lpPlayer,     // The player requesting the state change.
 ChannelID ciChannelID, // The channel to change
 bool listenState       // The new listening state.
)
/******************************************************************************/
{
   CAutoLock cLock( this );

   ChannelMap::iterator i;

   // Try to find the channel
   i = mChannels.find( ciChannelID );

   // If the channel was not found.
   if( i == mChannels.end() )
   {

      // Search the system channels.
      i = mSystemChannels.find( ciChannelID );
      if( i == mSystemChannels.end() ){
         return;
      }
   }

   Channel &cChannel = (*i).second;

   Channel::UserCont::iterator k = cChannel.vUsers.find( lpPlayer );

   // If the user isn't found in the chatter channel.
   if( k == cChannel.vUsers.end() )
   {
      return;
   }

   // Update the listening state.
   (*k).listening = listenState;
}
/******************************************************************************/
// Retreives the total list of channels.
void ChatterChannels::GetTotalChannelList(vector< Channel > &vChannels) // A container vector to put list of channels in.
/******************************************************************************/
{
   CAutoLock cLock( this );

   // Copy the map of system channels.
   ChannelMap::iterator i;
   for( i = mSystemChannels.begin(); i != mSystemChannels.end(); i++ )
   {
      vChannels.push_back( (*i).second );
   }

   // Copy map of channels into the provided container.    
   for( i = mChannels.begin(); i != mChannels.end(); i++ )
   {
      vChannels.push_back( (*i).second );
   }
}
/******************************************************************************/
// Returns the users in a chatter channel.
bool ChatterChannels::GetChannelUsers(
                                      Players *lpPlayer,                 // The UserID of the player (for access right verification).
                                      ChannelID ciChannelID,             // The channel ID to get the list from.
                                      std::vector< Channel::ChannelUser > &vUsers   // The container into which we should copy the list of users.
                                      )
                                      /******************************************************************************/
{
   CAutoLock cLock( this );

   ChannelMap::iterator i;

   // Try to find the channel
   i = mChannels.find( ciChannelID );

   // If the channel was not found.
   if( i == mChannels.end() )
   {
      // Try to find it in the system channels.
      i = mSystemChannels.find( ciChannelID );

      if( i == mSystemChannels.end() )
      {
         return false;
      }
   }

   Channel &cChannel = (*i).second;
   // If the user is not god AND the channel has a password
   // AND the user isn't registered in this channel.
   if( !( lpPlayer->GetGodFlags() & GOD_CHAT_MASTER ) && !( cChannel.csPassword.empty() ) &&
      cChannel.vUsers.find( lpPlayer ) == cChannel.vUsers.end() )
   {
      // Refuse access to user listing.
      return false;
   }

   bool bSeeAll = false;
   if(lpPlayer->GetGodFlags() & GOD_SEE_ALL )
   {
      bSeeAll = true;
   }

   // For all players in this channel.
   Channel::UserCont::iterator j;
   for( j = cChannel.vUsers.begin(); j != cChannel.vUsers.end(); j++ )
   {
      // If the player is not invisible.
      if( (!(*j).player->boWhoInvisible || bSeeAll) && (*j).player->in_game && (*j).player->self)
      {
         // Add the player to the provided list of players.
         if((*j).player->self->ViewFlag(__FLAG_JUST_DO_IT) != 666) //oki // never send this user...
            vUsers.push_back( (*j) );
      }
   }

   return true;
}
/******************************************************************************/
// Retreives the list of channels a user is registered in.
void ChatterChannels::GetRegisteredChannelList(
   Players *lpPlayer,                 // The ID of the player.
   std::vector< Channel > &vChannels  // The container of channels to put the list in.
   )
   /******************************************************************************/
{
   CAutoLock cLock( this );

   ChannelMap::iterator i;

   // For all channels available
   for( i = mChannels.begin(); i != mChannels.end(); i++ )
   {
      Channel &cChannel = (*i).second;

      // If the user is found in this channel
      if( cChannel.vUsers.find( lpPlayer ) != cChannel.vUsers.end() )
      {
         // Add the channel to the list of registered channels for this user.
         vChannels.push_back( cChannel );            
      }
   }
}
/******************************************************************************/
// Retreive the list of channels a user is allowed to view.
void ChatterChannels::GetSystemChannelList(
   Players *lpPlayer,                 // The userID, for access-right verification.
   std::vector< Channel >&vChannels   // The container to put the channels in.
   )
   /******************************************************************************/
{
   CAutoLock cLock( this );

   ChannelMap::iterator i;

   // For all system channels.
   for( i = mSystemChannels.begin(); i != mSystemChannels.end(); i++ )
   {
      // Add them
      vChannels.push_back( (*i).second );
   }
}
/******************************************************************************/
// Helper function which determines any empty channels and kills them.
void ChatterChannels::VerifyEmptyChannels( void )
/******************************************************************************/
{
   CAutoLock cLock( this );

   ChannelMap::iterator i;

   // For all non-system channels available
   for( i = mChannels.begin(); i != mChannels.end(); )
   {
      Channel &cChannel = (*i).second;

      TRACE( "\nvUsers.empty()=%u Password.empty()=%u.", cChannel.vUsers.empty(), cChannel.GetPassword().empty() );
      // If channel has no users
      if( cChannel.vUsers.empty() )
      {
         // Get a temporary iterator.
         ChannelMap::iterator iDelete = i;
         // Scroll to the next element
         i++;
         // Destroy the temporary iterator.
         mChannels.erase( iDelete );
      }
      else
      {
         // Otherwise simply scroll to next element.
         i++;
      }
   }
}
/******************************************************************************/
//  Packets a channel list.
void ChatterChannels::PacketChannelList(
 Players *target,
 std::vector< Channel > &vChannels, // The channel list to packet.
 TFCPacket &sending                 // The packet.
)
/******************************************************************************/
{
   sending << (short)vChannels.size();
   // For all channels
   vector< Channel >::iterator i;
   for( i = vChannels.begin(); i != vChannels.end(); i++ )
   {
      CString csChannelID = (*i).GetID().c_str();
      // Send the channel ID
      sending << ( csChannelID );

      // If player cannot be found inside this channel
      Channel::UserCont::iterator k = (*i).vUsers.find( target );
      if( k == (*i).vUsers.end() )
      {
         // It does not listen on the channel.
         sending << (char)0;
      }
      else
      {
         // Otherwise send the listening byte.
         sending << (char)( (*k).listening ? 1 : 0 );            
      }
   }
}
/******************************************************************************/
// Sends the total list of channels to the lpPlayer.
void ChatterChannels::SendTotalChannelList(
 Players *lpPlayer // The player to send the channel list to.
)
/******************************************************************************/
{
    CAutoLock cLock( this );

    vector< Channel > vChannels;

    // Get the total list of channels
    GetTotalChannelList( vChannels );

    TFCPacket sending;
    sending << (RQ_SIZE)RQ_GetChatterChannelList;
    PacketChannelList( lpPlayer, vChannels, sending );
    
    lpPlayer->self->SendPlayerMessage( sending );
}


/******************************************************************************/
// Sends the channel users to the lpPlayer.
/******************************************************************************/
void ChatterChannels::SendChannelUsers2(Players *lpPlayer,ChannelID ciChannelID)
{
   CAutoLock cLock( this );

   vector< Channel::ChannelUser > vUsers;

   TFCPacket sending;
   sending << (RQ_SIZE)RQ_GetChatterUserList2;

   sending << (string &)ciChannelID;

   bool getAccountNames = false;
   if( lpPlayer->GetGodFlags() & GOD_CAN_SEE_ACCOUNTS )
   {
      getAccountNames = true;
   }

   // If the user could retreive the list of players in the chatter channel.
   if( GetChannelUsers( lpPlayer, ciChannelID, vUsers ) )
   {
      CStringArray ArrayTmp;
      CString      csName;
      CString      csNameT;
      int          iIndexOK;
      // Packet quantity of users.
      short shSize = (short)vUsers.size();
      shSize+=20000;
      sending << shSize;

      vector< Channel::ChannelUser >::iterator its;        
      for( its = vUsers.begin(); its != vUsers.end(); its++ )
      {
         csName.Format("%s",(*its).player->self->GetTrueName());
         iIndexOK = 0;
         for(int i=0;i<ArrayTmp.GetSize();i++)
         {
            if(ArrayTmp[i].CompareNoCase(csName) < 0)
               iIndexOK++;
         }
         ArrayTmp.InsertAt(iIndexOK,csName);
      }

      int iMax  = vUsers.size();
      int iSend = 0;

      do 
      {
         vector< Channel::ChannelUser >::iterator it;        
         for( it = vUsers.begin(); it != vUsers.end(); it++ )
         {


            // Get their names
            csNameT = (*it).player->self->GetTrueName();

            if (ArrayTmp[iSend] == csNameT)
            {
               bool bHidden = false;
               char chGodColor = GetPlayerGodTitleRight((*it).player->self,bHidden);
               csName = csNameT;
               
               // Packet their names
               sending << csName;
               sending << (*it).player->self->GetGuildName();
               sending << (*it).player->self->GetTitle( getAccountNames );
               sending << (char)( (*it).listening ? 1 : 0 );
               sending << (char)chGodColor;
			      sending << (char)bHidden;
               iSend++;
               break;
            }
         }
      } while (iSend < iMax);
   }
   else
   {
      sending << (short)0;
   }

   // Send packet.
   lpPlayer->self->SendPlayerMessage( sending );
}

char ChatterChannels::GetPlayerGodTitleRight(Character* pUser, bool &bHidden)
{
   bHidden = false;
   if(pUser->GetPlayer()->boWhoInvisible)
      bHidden = true;

   if(pUser->GetGodFlags() & GOD_CAN_LOCKOUT_USER)
      return 3;
   else if(pUser->GetGodFlags() & GOD_CAN_TELEPORT)
      return 2;
   else if(pUser->GetGodFlags() & GOD_CAN_SQUELCH)
      return 1;

   return 0;
}



/******************************************************************************/
// Sends the channel listing to the lpPlayer
void ChatterChannels::SendRegisteredChannelList(Players *lpPlayer) // The player to send the listing to.
/******************************************************************************/
{
    CAutoLock cLock( this );
    vector< Channel > vChannels;

    // Get the list of private and system channels.
    GetSystemChannelList( lpPlayer, vChannels );
    GetRegisteredChannelList( lpPlayer, vChannels );

    TFCPacket sending;
    sending << (RQ_SIZE)RQ_GetChatterChannelList;
    PacketChannelList( lpPlayer, vChannels, sending );
    
    lpPlayer->self->SendPlayerMessage( sending );
}    
/******************************************************************************/
// Adds a new player to all the system channels.
void ChatterChannels::AddToSystemChannels(Players *lpPlayer) // The player.
/******************************************************************************/
{    
   CAutoLock cLock( this );

   ChannelMap::iterator i;

   for( i = mSystemChannels.begin(); i != mSystemChannels.end(); i++ )
   {
      Channel *lpChannel = &(*i).second;

      // Add the user to this channel.
      if( !lpChannel->vUsers.insert( Channel::ChannelUser( lpPlayer ) ).second )
      {
         TRACE( "\nInsertion failed. Users in channel:" );
      }
   }
}
/******************************************************************************/
// Sets whether or not the public channels are now for operators only.
void ChatterChannels::SetOperatorPublicChannelControl( bool val)
/******************************************************************************/
{
    publicChannelsForOperators = val;
}