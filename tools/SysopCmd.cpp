/******************************************************************************
Modify for vs2008 (30/04/2009)
Add Command SQUELCHTMP $ TIME $ //peu couper pour un temps X by Nightmare (29/06/2009)
Add COmmand REMOVETMP $'S SHOUTS TIME $ //peu couper pour un temps X by Nightmare (29/06/2009)
Add Command REMOVETMP $'S PAGES TIME $ //peu couper pour un temps X by Nightmare (29/06/2009)
Add Command LOCKOUTTMP $'S TIME $ //peu locker un user pour X jours by Nightmare (29/06/2009)
Add command GIVE FORMULE $ TO $ // give profession formule to a player by Nightmare (29/06/2009)
Add Comamnd GET FORMULE LIST //get all formule list by Nightmare (29/06/2009)
Add Command GIVE MODO FLAG TO $ by Nightmare (29/06/2009)
Add COmmand REMOVE MODO FLAG FROM $ by Nightmare (29/06/2009)
Add COmmand GIVEME GAMEOP FLAG $ by Nightmare (29/06/2009)
Add COmmand REMOVEME GAMEOP FLAG $ by Nightmare (29/06/2009)
Add COmmand REMOVE $ GEM OF DESTINY by Nightmare (29/06/2009)
Add COmmand NAMECOLOR $ RESET by Nightmare (29/06/2009)
Add COmmand NAMECOLOR $,$ by Nightmare (29/06/2009)
Add COmmand SYSTEMLIST by Nightmare (29/06/2009)
Add COmmand NMFIREWALL $ by Nightmare (29/06/2009)
Add COmmand UDPFILTERSYSTEM $ by Nightmare (29/06/2009)
Add COmmand UDPLOGSYSTEM $ by Nightmare (29/06/2009)
Add COmmand RELOADSYSTEM $ by Nightmare (29/06/2009)
Add COmmand GUILDSYSTEM $ by Nightmare (29/06/2009)
Add COmmand AHSYSTEM $ by Nightmare (29/06/2009)
Add COmmand PROFESSION $ by Nightmare (29/06/2009)
Add COmmand AHEXPIREALL by Nightmare (29/06/2009)
Add COmmand GETGUILDLIST by Nightmare (29/06/2009)
Add COmmand CREATEGUILD $,$v
Add COmmand MODIFYGUILD $,$ by Nightmare (29/06/2009)
Add COmmand DELETEGUILD $ by Nightmare (29/06/2009)
Add COmmand RENAMEGUILD $,$ by Nightmare (29/06/2009)
Add GOD Flag GOD_CAN_GIVE_FLAG_TO_HIM //permet de changer les flag GM a sois meme... by Nightmare (29/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "SysopCmd.h"
#include "TFC_MAIN.h"
#include "TFCInit.h"
#include "TFCPacket.h"
#include "TFC ServerDlg.h"
#include "RegKeyHandler.h"
#include "PlayerManager.h"
#include "AutoConfig.h"
#include "Shutdown.h"
#include "PacketManager.h"
#include "UDP/NMPacketManager.h"
#include "Format.h"
#include "AsyncFuncQueue.h"
#include "SpellMessageHandler.h"
#include "NPC Thread.h"
#include "WeatherEffect.h"
#include "Version.h"
#include <cmath>
#include "ThreadMonitor.h"
#include "GuildMaster.h"
#include "RPMaster.h"
#include "NPCMAcroscriptLng.h"
#include "QuestFlagsListing.h"
#include "Tokenizer.h"
#include "StatModifierFlagsListing2.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
#define CREATE_MESSAGE		{\
	TFCPacket sending;\
	CString csText;\
	DWORD dwColor = CL_BLUE_LIGHT;\
	csText.Format

#define CREATE_MESSAGE_RED		{\
   TFCPacket sending;\
   CString csText;\
   DWORD dwColor = CL_RED;\
   csText.Format

/*
#define SEND_MESSAGE	;\
	sending << (RQ_SIZE)RQ_ServerMessage;\
	sending << (short)30;\
	sending << (short)3;\
	sending << csText;\
	sending << (long) dwColor;\
	user->Lock();\
	user->self->SendPlayerMessage( sending );\
	user->Unlock();\
}
*/

#define SEND_MESSAGE	;\
   sending << (RQ_SIZE)RQ_ServerMessage;\
   sending << (short)30;\
   sending << (short)3;\
   sending << csText;\
   sending << (long) dwColor;\
   user->self->SendPlayerMessage( sending );\
   }

#define FIRST_COMMAND( __str, __god ) if( user->GetGodFlags() & __god && GetParameters( __str, csCommand, csParams ) != FALSE ){ boFound = TRUE;
#define COMMAND( __str, __god ) }else if( user->GetGodFlags() & __god && GetParameters( __str, csCommand, csParams ) != FALSE ){ boFound = TRUE;
#define END_COMMAND }
#define PARAM( __num ) ((LPCTSTR)csParams[ __num ])



#define SUPER_USER (_stricmp( \
	(LPCTSTR)target->GetFullAccountName(), \
	CAutoConfig::GetStringValue( theApp.csT4CKEY+GEN_CFG_KEY, "SuperUser", HKEY_LOCAL_MACHINE ).c_str() \
	) == 0)

#define FIRST_COMPONENT( __cmp, __god,__pos ) if( user->GetGodFlags() & __god && _stricmp( (LPCTSTR)PARAM( __pos ), __cmp ) == 0 ){
#define COMPONENT( __cmp, __god,__pos ) }else if( user->GetGodFlags() & __god && _stricmp( (LPCTSTR)PARAM( __pos ), __cmp ) == 0  ){

#define GIVE_GOD_FLAG( __flag, __msg ) target->SetGodMode( TRUE,__flag,TRUE ); target->SetGodFlags( target->GetGodFlags() | __flag );\
    csMessage = __msg;\
    boGiven = TRUE;

#define GIVE_GOD_EDITFLAG( __flag, __msg ) target->SetGodMode(FALSE,0, TRUE );\
   target->SetGodFlags( target->GetGodFlags() | GOD_CAN_EDIT_USER );\
   target->SetGodFlags( target->GetGodFlags() | __flag );\
   csMessage = __msg;\
   boGiven = TRUE;

#define GIVE_GOD_VIEWFLAG( __flag, __msg ) target->SetGodMode(FALSE,0, TRUE );\
   target->SetGodFlags( target->GetGodFlags() | GOD_CAN_VIEW_USER );\
   target->SetGodFlags( target->GetGodFlags() | __flag );\
   csMessage = __msg;\
   boGiven = TRUE;

#define REMOVE_GOD_FLAG( __flag, __msg ) target->SetGodFlags( target->GetGodFlags() & (~static_cast< __int64 >( __flag ) ) );\
    csMessage = __msg;\
    if( target->GetGodFlags() == (__int64)(0) ){\
        target->SetGodMode( FALSE,0,FALSE );\
    }\
    boGiven = TRUE;

/******************************************************************************/
CLock SysopCmd::csSysLock;
static SysopCmd sysCmd;

extern CTFCServerApp theApp;

/******************************************************************************/
namespace
/******************************************************************************/
{
    struct SetNameData
	{
        sockaddr_in sockGod;
        sockaddr_in sockTarget;
        string csNewName;
    };
    void AsyncSetName( LPVOID lpData );
	CString GetOPFlagNameByShortcut(CString csFlagShortCut);

    struct TeleportLocation
	{
        WorldPos wlPos;
        std::string Id;
    };
    list< TeleportLocation > locationList;


    /******************************************************************************/
    //  A registry update occured.
	void SysopCfgUpdate( void )
    /******************************************************************************/
    {
        RegKeyHandler regKey;
        regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+CHARACTER_KEY );
        
        locationList.clear();
        TFormat format;

        int i = 1;
        std::string locationId = regKey.GetProfileString( "TeleportLocation1", "$NULL$" );
        while( locationId != "$NULL$" )
		{
            TeleportLocation location;
            location.Id = locationId;
            
            location.wlPos.X = regKey.GetProfileInt( 
                format( "TeleportLocation%uX", i ), 
                0 
            );

            location.wlPos.Y = regKey.GetProfileInt( 
                format( "TeleportLocation%uY", i ), 
                0 
            );
            
            location.wlPos.world = regKey.GetProfileInt( 
                format( "TeleportLocation%uW", i ), 
                0 
            );

            locationList.push_back( location );

            i++;
            locationId = regKey.GetProfileString( 
                format( "TeleportLocation%u", i ), 
                "$NULL$" 
            );
        }
    }
};
/******************************************************************************/
// Startup function
SysopCmd::SysopCmd( void )
/******************************************************************************/
{    
}
/******************************************************************************/
// End func
 SysopCmd::~SysopCmd( void )
/******************************************************************************/
{
	// DeleteCriticalSection( &csSysLock );
}
/******************************************************************************/
//  Creates the sysop module
void SysopCmd::Create( void )
/******************************************************************************/
{
    CAutoConfig::AddRegUpdateCallback( SysopCfgUpdate );
    SysopCfgUpdate();
}
/******************************************************************************/
// Processes the command line and returns the parameters found
BOOL SysopCmd::GetParameters(LPCTSTR lpszCommandTemplate, CString csCommandLine, CString *lpFoundParameters)
/******************************************************************************/
{
	struct TOKEN
	{
		int nTokenStart;
		int nTokenEnd;
	};
    	
	BOOL boError = FALSE;
	int nCurrentPos;
	int nLastPos = -1;
	int i;
	int nNbParams = 0;
	int nCommandTemplateSize = strlen( lpszCommandTemplate );
	TOKEN *lpTokens;

    // Replace spaces with ASC(27) in strings surrounded by "
    for( i = 0; i < csCommandLine.GetLength(); i++ )
	{
        if( csCommandLine[ i ] == '\"' )
		{
            csCommandLine.SetAt( i, ' ' );
            i++;
            while( i < csCommandLine.GetLength() && csCommandLine[ i ] != '\"' )
			{
                if( csCommandLine[ i ] == ' ' )
				{
                    csCommandLine.SetAt( i, 27 );
                }

                i++;
            }
            // If the last char is a "
            if( i < csCommandLine.GetLength() )
			{
                csCommandLine.SetAt( i, ' ' );
            }
        }
    }

	CString csSearchString( csCommandLine );

    csSearchString.MakeUpper();

	// Copy the command template into tokens for processing.
	char *lpszSavedToken;
	char *lpszToken = new char[ nCommandTemplateSize + 2 ];
	lpszSavedToken = lpszToken;	// Save address of token for deletion.
	memcpy( lpszToken, lpszCommandTemplate, nCommandTemplateSize );
	// Replace the trailing \0 with a ÿ (char 216) to have a last token.
	lpszToken[ nCommandTemplateSize ] = 'ÿ';
	lpszToken[ nCommandTemplateSize + 1 ] = 0;
			
	int nStrLen = csCommandLine.GetLength();
	
	// Finds the number of $ parameters in the string.
	for( i = 0; i < nCommandTemplateSize; i++ )
	{
		if( lpszToken[ i ] == '$' )
		{
			nNbParams++;
		}
	}

	lpTokens = new TOKEN[ nNbParams + 1 ];

	//COMMAND( "ADD FLAG $ VALUE $ TO $" )
	
	int j;
	i = 0;
    int findStart = 0;
	// Find each sentence parts	
   char *next_token1;
	lpszToken = strtok_s( lpszToken, "$" ,&next_token1);
	while( lpszToken != NULL && !boError )
	{
		nCurrentPos = csSearchString.Find( lpszToken, findStart );
		
		// If this newly found token is after the last one found.
		if( nCurrentPos > nLastPos )
		{
            int tokenLen = strlen( lpszToken );
						
			lpTokens[ i ].nTokenStart = nCurrentPos;
			lpTokens[ i ].nTokenEnd   = nCurrentPos + strlen( lpszToken );

			TRACE( "\r\nStart=%u, End=%u", lpTokens[ i ].nTokenStart, lpTokens[ i ].nTokenEnd );
			
            // If the token is a single character
            if( tokenLen == 1 )
			{
    			// Remove all of the successive tokens
	    		j = lpTokens[ i ].nTokenStart;
                while( j < lpTokens[ i ].nTokenEnd || ( j < nStrLen && csSearchString[ j ] == lpszToken[ 0 ] ) )
				{
			    	csSearchString.SetAt( j, '\r' );
                    j++;
			    }
                findStart = j;
            }
			else
			{
                // Otherwise only replace the token.
                for( j = lpTokens[ i ].nTokenStart; j < lpTokens[ i ].nTokenEnd; j++ )
				{
                    csSearchString.SetAt( j, '\r' );
                }
                findStart = j;
            }
						
			// Find next token
			lpszToken = strtok_s( NULL, "$" ,&next_token1);
			i++;

			nLastPos = nCurrentPos;
		}
		else
		{
			boError = TRUE;
		}
	}

	if( lpTokens[ 0 ].nTokenStart != 0 )
	{
		boError = TRUE;
	}

	if( !boError )
	{
		// Scroll through the found parameters
		i = 0;
		for( i = 0; i < nNbParams; i++ )
		{
			lpTokens[ i ].nTokenEnd;
			
			// Add parameter to list.
			lpFoundParameters[ i ] = csCommandLine.Mid( lpTokens[ i ].nTokenEnd, lpTokens[ i+1 ].nTokenStart - lpTokens[ i ].nTokenEnd );			 

            int q;
            for( q = 0; q < lpFoundParameters[ i ].GetLength(); q++ )
			{
                if( lpFoundParameters[ i ].GetAt( q ) == 27 )
				{
                    lpFoundParameters[ i ].SetAt( q, ' ' );
                }
            }

			// Remove white spaces
			lpFoundParameters[ i ].TrimRight();
			lpFoundParameters[ i ].TrimLeft();
            
			if( lpFoundParameters[ i ].IsEmpty() )
			{
				boError = TRUE;
            }
		}
	}

	if (lpTokens != NULL)
	{
		delete lpTokens;
		lpTokens = NULL;
	}
	if (lpszSavedToken != NULL)
	{
		delete lpszSavedToken;
		lpszSavedToken = NULL;
	}

	return !boError;
}
/******************************************************************************/
// Processes the command line and returns the parameters found
BOOL SysopCmd::GetParametersForNPC(LPCTSTR lpszCommandTemplate, CString csCommandLine, CString *lpFoundParameters)
/******************************************************************************/
{
	struct TOKEN
	{
		int nTokenStart;
		int nTokenEnd;
	};
    	
	BOOL boError = FALSE;
	int nCurrentPos;
	int nLastPos = -1;
	int i;
	int nNbParams = 0;
	int nCommandTemplateSize = strlen( lpszCommandTemplate );
	TOKEN *lpTokens;

    // Replace spaces with ASC(27) in strings surrounded by "
    for( i = 0; i < csCommandLine.GetLength(); i++ )
	{
        if( csCommandLine[ i ] == '\"' )
		{
            csCommandLine.SetAt( i, ' ' );
            i++;
            while( i < csCommandLine.GetLength() && csCommandLine[ i ] != '\"' )
			{
                if( csCommandLine[ i ] == ' ' )
				{
                    csCommandLine.SetAt( i, 27 );
                }

                i++;
            }
            // If the last char is a "
            if( i < csCommandLine.GetLength() )
			{
                csCommandLine.SetAt( i, ' ' );
            }
        }
    }

	CString csSearchString( csCommandLine );

    csSearchString.MakeUpper();

	// Copy the command template into tokens for processing.
	char *lpszSavedToken;
	char *lpszToken = new char[ nCommandTemplateSize + 2 ];
	lpszSavedToken = lpszToken;	// Save address of token for deletion.
	memcpy( lpszToken, lpszCommandTemplate, nCommandTemplateSize );
	// Replace the trailing \0 with a ÿ (char 216) to have a last token.
	lpszToken[ nCommandTemplateSize ] = '.';
	lpszToken[ nCommandTemplateSize + 1 ] = 0;
			
	int nStrLen = csCommandLine.GetLength();
	
	// Finds the number of $ parameters in the string.
	for( i = 0; i < nCommandTemplateSize; i++ )
	{
		if( lpszToken[ i ] == '$' )
		{
			nNbParams++;
		}
	}

	lpTokens = new TOKEN[ nNbParams + 1 ];

	int j;
	i = 0;
    int findStart = 0;
	// Find each sentence parts	
   char *next_token1;
	lpszToken = strtok_s( lpszToken, "$" ,&next_token1);
	while( lpszToken != NULL && !boError )
	{
		nCurrentPos = csSearchString.Find( lpszToken, findStart );
		
		// If this newly found token is after the last one found.
		if( nCurrentPos > nLastPos )
		{
            int tokenLen = strlen( lpszToken );
						
			lpTokens[ i ].nTokenStart = nCurrentPos;
			lpTokens[ i ].nTokenEnd   = nCurrentPos + strlen( lpszToken );

			TRACE( "\r\nStart=%u, End=%u", lpTokens[ i ].nTokenStart, lpTokens[ i ].nTokenEnd );
			
            // If the token is a single character
            if( tokenLen == 1 )
			{
    			// Remove all of the successive tokens
	    		j = lpTokens[ i ].nTokenStart;
                while( j < lpTokens[ i ].nTokenEnd || ( j < nStrLen && csSearchString[ j ] == lpszToken[ 0 ] ) )
				{
			    	csSearchString.SetAt( j, '\r' );
                    j++;
			    }
                findStart = j;
            }
			else
			{
                // Otherwise only replace the token.
                for( j = lpTokens[ i ].nTokenStart; j < lpTokens[ i ].nTokenEnd; j++ )
				{
                    csSearchString.SetAt( j, '\r' );
                }
                findStart = j;
            }
						
			// Find next token
			lpszToken = strtok_s( NULL, "$" ,&next_token1);
			i++;

			nLastPos = nCurrentPos;
		}
		else
		{
			boError = TRUE;
		}
	}

	if( lpTokens[ 0 ].nTokenStart != 0 )
	{
		boError = TRUE;
	}

	if( !boError )
	{
		// Scroll through the found parameters
		i = 0;
		for( i = 0; i < nNbParams; i++ )
		{
			lpTokens[ i ].nTokenEnd;
			
			// Add parameter to list.
			lpFoundParameters[ i ] = csCommandLine.Mid( lpTokens[ i ].nTokenEnd, lpTokens[ i+1 ].nTokenStart - lpTokens[ i ].nTokenEnd );			 

            int q;
            for( q = 0; q < lpFoundParameters[ i ].GetLength(); q++ )
			{
                if( lpFoundParameters[ i ].GetAt( q ) == 27 )
				{
                    lpFoundParameters[ i ].SetAt( q, ' ' );
                }
            }

			// Remove white spaces
			lpFoundParameters[ i ].TrimRight();
			lpFoundParameters[ i ].TrimLeft();
            
			if( lpFoundParameters[ i ].IsEmpty() )
			{
				boError = TRUE;
			}
		}
	}

	if (lpTokens != NULL)
	{
		delete lpTokens;
		lpTokens = NULL;
	}
	if (lpszSavedToken != NULL)
	{
		delete lpszSavedToken;
		lpszSavedToken = NULL;
	}

	return !boError;
}
/******************************************************************************/
// Finds the players structure corresponding to this name
Players *SysopCmd::FindCharacter( CString csName ) // The name to search for.
/******************************************************************************/
{
    return CPlayerManager::GetCharacterOld( csName );
}
/******************************************************************************/
//  Substract the newvalue from the original value. Makes sure that if the variable
// underflows, that it stays at 0.
DWORD SafeMinus( DWORD original, DWORD newval )
/******************************************************************************/
{
    if( newval > original )
	{
        newval = 0;
    }
	else
	{
        newval = original - newval;
    }
    return newval;
}
/******************************************************************************/
// Checks the sysop command.
BOOL SysopCmd::VerifySysopCommand(
 Players *user,		// The god
 CString csCommand,	// The command-line
 bool bSendEcho      //if send echo to user or not...
)
/******************************************************************************/
{
	csSysLock.Lock();

    bool success = false;

	BOOL boFound = FALSE;
	// Max of 5 parameters.
	CString csParams[ 5 ];
	
	csCommand.TrimRight();
	csCommand.TrimLeft();
	//csCommand.MakeUpper();
	csCommand += 'ÿ'; // Add command terminator (char 216)

	//TRACE( "Command line--%s--", (LPCTSTR)csCommand );

	// God teleportation. Teleport to the n'th found NPC 	
    FIRST_COMMAND( "TELEPORT TO NPC $,$", GOD_CAN_TELEPORT )
        DWORD order = atoi( PARAM( 1 ) );
        Creatures *npc = NPCMain::GetInstance().GetMainNPC( PARAM( 0 ), order, user->self->GetLang() );
        if( npc != NULL )
		{
            WorldPos wlPos = npc->GetWL();
			if( !user->self->Teleport( wlPos, 0 ) )
			{
				CREATE_MESSAGE( "The teleport destination is invalid." );
				SEND_MESSAGE;		
			}
			else
			{
                success = true;
			}
        }
		else
		{
            CREATE_MESSAGE( "NPC not found" );
            SEND_MESSAGE;
        }	
	// Allow GM to use GM commands	
	COMMAND( "AUTHENTIFICATION $", GOD_CAN_TELEPORT )
		RegKeyHandler regKey; 

      if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
		{
			// Get the key
			CString csKey;
			csKey = regKey.GetProfileString( "GM_KEY", "" );

			if( csKey.IsEmpty() )
			{
				user->self->boAuthGM = true;

				_LOG_GAMEOP
					LOG_CRIT_ERRORS,
					"God %s (%s) has been authentified by the system!",
					(LPCTSTR)user->self->GetTrueName(),
					(LPCTSTR)user->GetFullAccountName()						
				LOG_

				CREATE_MESSAGE( "Game Master commands enabled." );				
				dwColor = CL_RED;
				SEND_MESSAGE
			}
			else
			{
				if( strcmp( (LPCTSTR)csKey, PARAM( 0 ) ) == 0 )
				{
					user->self->boAuthGM = true;

					CREATE_MESSAGE( "Game Master commands enabled." );
					dwColor = CL_RED;
					SEND_MESSAGE
				}
				else
				{
					user->Lock(); 					
					user->dwKickoutTime = 20 SECONDS TDELAY;
					user->Unlock();

					_LOG_GAMEOP
						LOG_CRIT_ERRORS,
						"God %s (%s) entered wrong authentification key!",
						(LPCTSTR)user->self->GetTrueName(),
						(LPCTSTR)user->GetFullAccountName()						
					LOG_
				}
			}			
		}
		else
		{
			CREATE_MESSAGE( "Unable to authentificate you. Register do not respond.." );
			dwColor = CL_RED;
			SEND_MESSAGE
		}

	// Show chest
	COMMAND( "SHOW CHEST", GOD_CAN_VIEW_USER_BACKPACK )
	{
		user->self->ShowChest();
	}

	COMMAND( "SAVEME", GOD_CAN_TELEPORT )
	{
		user->ForceSave(); //Saveme Gm Command
		success = true;
	}

	COMMAND( "VIEW SERVER BUILD", GOD_DEVELOPPER )
	{
		CREATE_MESSAGE( Version::sBuildStamp().c_str() );
		SEND_MESSAGE;
		success = true;
	}
	
	COMMAND( "GET INFO UNITREF $", GOD_DEVELOPPER )
	{
		char name[1024];
		Unit::GetNameFromID(atoi(PARAM(0)),name);
		CREATE_MESSAGE("Name: %s", name);
		SEND_MESSAGE;
	}

	COMMAND( "LIST THREADS", GOD_DEVELOPPER )
	{
		CThreadMonitor::ThreadList listOfThreads;
		CThreadMonitor::GetInstance().GetRunningThreadsList(listOfThreads);
		CThreadMonitor::ThreadListIterator i = listOfThreads.begin();
		CREATE_MESSAGE("Listing %u threads", listOfThreads.size()); SEND_MESSAGE;
		for (; i != listOfThreads.end(); ++i) 
		{
			CREATE_MESSAGE("Thread ID: %u Name: %s", (*i).first, (*i).second.c_str());
			SEND_MESSAGE;
		}
		CREATE_MESSAGE("End."); SEND_MESSAGE;
	}

	COMMAND( "RUNBF TARGET $, $", GOD_DEVELOPPER )
	{
		BoostFormula bf;
		if ( bf.SetFormula( PARAM( 1 ) ) ) 
		{
			Players *target = FindCharacter( PARAM( 0 ) );
			if( target != NULL )
			{
				CREATE_MESSAGE( "BoostFormula Result: %f", bf.GetBoost(user->self, target->self) );
				SEND_MESSAGE;
			}
			else 
			{
				CREATE_MESSAGE( "Ooops! Character %s is not around, dude!", PARAM(0) );
				SEND_MESSAGE;
			}
		}
		else 
		{
			CREATE_MESSAGE( "Error evaluating the formula" );
			SEND_MESSAGE;
		}
	}

	COMMAND( "RUNBF $", GOD_DEVELOPPER )
	{
		BoostFormula bf;
		if ( bf.SetFormula( PARAM( 0 ) ) ) 
		{
			CREATE_MESSAGE( "BoostFormula Result: %f", bf.GetBoost(user->self) );
			SEND_MESSAGE;
		}
		else 
		{
			CREATE_MESSAGE( "Error evaluating the formula" );
			SEND_MESSAGE;
		}
	}

	COMMAND( "RUNSF TARGET $, $", GOD_DEVELOPPER )
	{
		Players *target = FindCharacter( PARAM( 0 ) );
		if( target != NULL )
		{
			CREATE_MESSAGE( "StringFormula Result: %s", (LPCTSTR)BoostFormula::TranslateStringFormula( PARAM( 1 ), user->self, target->self ) );
			SEND_MESSAGE;
		}
		else 
		{
			CREATE_MESSAGE( "Ooops! Character %s is not around, dude!", PARAM(0) );
			SEND_MESSAGE;
		}
	}

	COMMAND( "RUNSF $", GOD_DEVELOPPER )
	{
		CREATE_MESSAGE( "StringFormula Result: %s", (LPCTSTR)BoostFormula::TranslateStringFormula( PARAM( 0 ), user->self ) );
		SEND_MESSAGE;
	}

	// God teleportation. Teleport to a NPC
	COMMAND( "TELEPORT TO NPC $", GOD_CAN_TELEPORT )
	{
		Creatures *npc = NPCMain::GetInstance().GetMainNPC( PARAM( 0 ), 0, user->self->GetLang() );
		if( npc != NULL )
		{
			WorldPos wlPos = npc->GetWL();
			if( !user->self->Teleport( wlPos, 0 ) )
			{
				CREATE_MESSAGE( "The teleport destination is invalid." );
				SEND_MESSAGE;		
			}
			else
			{
				success = true;
			}
		}
		else
		{
			CREATE_MESSAGE( "NPC not found" );
			SEND_MESSAGE;
        }
	}

    // Teleports to a known location
    COMMAND( "TELEPORT TO LOC $", GOD_CAN_TELEPORT )
	{
        bool success = false;
        list< TeleportLocation >::iterator i;
        // Find the location in the list
        for( i = locationList.begin(); i != locationList.end(); i++ )
		{
            if( _stricmp( (*i).Id.c_str(), PARAM( 0 ) ) == 0 )
			{
			    // Teleport to this location.
                if( !user->self->Teleport( (*i).wlPos, 0 ) )
				{
				    CREATE_MESSAGE( "The teleport destination is invalid." );
				    SEND_MESSAGE;		
			    }
				else
				{
                    success = true;
			    }
                break;
            }
        }
        
        if( !success )
		{
            CREATE_MESSAGE( "Location not found." );
            SEND_MESSAGE;
        }
	}

	// God teleportation
	COMMAND( "TELEPORT TO $,$,$", GOD_CAN_TELEPORT )
	{
		BOOL boInvalid = FALSE;
		
		WorldPos wlPos = { 0, 0, 0 };

		wlPos.X		= atoi( PARAM( 0 ) );
		wlPos.Y		= atoi( PARAM( 1 ) );
		wlPos.world = atoi( PARAM( 2 ) );

		if( wlPos.X == 0 || wlPos.Y == 0 )
		{
			boInvalid = TRUE;
		}
		else
		{
			if( !user->self->Teleport( wlPos, 0 ) )
			{
				boInvalid = TRUE;
			}			
		}

		if( boInvalid )
		{
			CREATE_MESSAGE( "Your teleport destination is invalid." );
			SEND_MESSAGE;
        }
		else
		{
            success = true;
        }
	}

    // Teleports besides the given user.
    COMMAND( "TELEPORT TO $", GOD_CAN_TELEPORT )
	{
		Players *target;
			
		target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
		{			
			WorldPos wlPos = target->self->GetWL();
            wlPos.X--;

			if( !user->self->Teleport( wlPos, 0 ) ){
				CREATE_MESSAGE( "The teleport destination is invalid." );
				SEND_MESSAGE;		
            }else{
                success = true;   
            }

		}else{
			CREATE_MESSAGE( "Users %s is not online", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}

	// Teleport a user to a specified destination
	COMMAND( "TELEPORT USER $ TO $,$,$", GOD_CAN_TELEPORT_USER )		
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to teleport user %s without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 0 )
			LOG_

			CREATE_MESSAGE( "Users %s is not online", PARAM( 0 ) );
			SEND_MESSAGE;
		}
		else
		{	
			Players *target;			
			target = FindCharacter( PARAM( 0 ) );

			if( target != NULL )
			{
				WorldPos wlPos = { 0, 0, 0 };
				WorldPos wlFrom = target->self->GetWL();

				wlPos.X		= atoi( PARAM( 1 ) );
				wlPos.Y		= atoi( PARAM( 2 ) );
				wlPos.world = atoi( PARAM( 3 ) );


				target->Lock();
				if( !target->self->Teleport( wlPos, 0 ) )
				{
					CREATE_MESSAGE( "The teleport destination is invalid." );
					SEND_MESSAGE;		
				}
				else
				{
					success = true;

					_LOG_GAMEOP
						LOG_SYSOP, 
						"God %s (%s) teleported character %s (%s) from ( %u, %u, %u ) to ( %u, %u, %u )",
						(LPCTSTR)user->self->GetTrueName(),
						(LPCTSTR)user->GetFullAccountName(),
						(LPCTSTR)target->self->GetTrueName(),
						(LPCTSTR)target->GetFullAccountName(),
						wlFrom.X,
						wlFrom.Y,
						wlFrom.world,
						wlPos.X,
						wlPos.Y,
						wlPos.world
					LOG_
				}
				target->Unlock();

			}
			else
			{
				CREATE_MESSAGE( "Users %s is not online", PARAM( 0 ) );
				SEND_MESSAGE;
			}
		}
	}

	// Teleport a user to god's side
	COMMAND( "TELEPORT USER $ TO ME", GOD_CAN_TELEPORT_USER )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to teleport user %s without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 0 )
			LOG_

			CREATE_MESSAGE( "Users %s is not online", PARAM( 0 ) );
			SEND_MESSAGE;
		}
		else
		{
			Players *target;
			

			target = FindCharacter( PARAM( 0 ) );

			if( target != NULL )
			{
				WorldPos wlPos = user->self->GetWL(); wlPos.X--;
				WorldPos wlFrom = target->self->GetWL();


				target->Lock();
				if( !target->self->Teleport( wlPos, 0 ) )
				{
					CREATE_MESSAGE( "The teleport destination is invalid." );
					SEND_MESSAGE;		
				}
				else
				{
					success = true;

					_LOG_GAMEOP
						LOG_SYSOP, 
						"God %s (%s) teleported character %s (%s) from ( %u, %u, %u ) to ( %u, %u, %u )",
						(LPCTSTR)user->self->GetTrueName(),
						(LPCTSTR)user->GetFullAccountName(),
						(LPCTSTR)target->self->GetTrueName(),
						(LPCTSTR)target->GetFullAccountName(),
						wlFrom.X,
						wlFrom.Y,
						wlFrom.world,
						wlPos.X,
						wlPos.Y,
						wlPos.world
					LOG_
				}
				target->Unlock();

			}
			else
			{
				CREATE_MESSAGE( "Users %s is not online", PARAM( 0 ) );
				SEND_MESSAGE;
			}
		}
	}

	
	// Counts how many NPCs of a given name exist in the world.
    COMMAND( "COUNT NPC $", GOD_CAN_SUMMON_MONSTERS )
	{
        TFormat format;
        DWORD count = NPCMain::GetInstance().CountNPC( PARAM( 0 ), user->self->GetLang() );
        string msg;
        if( count == 0 )
		{
            msg = format( "There are no NPCs named %s.", PARAM( 0 ) );            
        }
		else if( count == 1 )
		{
            msg = format( "There is only 1 NPC named %s.", PARAM( 0 ) );
        }
		else
		{
            msg = format( "There are %u NPCs named %s.", count, PARAM( 0 ) );
        }
        CREATE_MESSAGE( msg.c_str() );
        SEND_MESSAGE;
	}

	// Kicks a player out of game.
	COMMAND( "ZAP $", GOD_CAN_ZAP )
	{
		Players *target = FindCharacter( PARAM( 0 ) );

		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to zap user %s without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 0 )
			LOG_			
		}		

      if( target != NULL && user->self->boAuthGM == true )
      {
         if( SUPER_USER  || target->self->ViewFlag(__FLAG_JUST_DO_IT) == 666)//oki
         {
            CREATE_MESSAGE( "Sorry but you cannot zap the super user!" );
            SEND_MESSAGE;
         }
         else
         {
			    CString csText = _STR( 15101, target->self->GetLang());
			    
			    TFCPacket sending;
			    sending << (RQ_SIZE)RQ_ServerMessage;
			    sending << (short)30;
			    sending << (short)3;
			    sending << csText;
				 sending << (long) CL_RED;
			    target->Lock(); 
			    target->self->SendPlayerMessage( sending );
			    target->dwKickoutTime = 5 SECONDS TDELAY;
			    target->Unlock();

			    _LOG_GAMEOP
				    LOG_SYSOP,
				    "God %s (%s) zapped player %s (%s) out of the game.",
				    (LPCTSTR)user->self->GetTrueName(),
				    (LPCTSTR)user->GetFullAccountName(),
				    (LPCTSTR)target->self->GetTrueName(),
				    (LPCTSTR)target->GetFullAccountName()
			    LOG_
            }
		}
		else
		{
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}

   // Forbids a user from talking (quite harsh in this game)
	COMMAND( "UNSQUELCH $", GOD_CAN_SQUELCH )
	{
		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
		{
            success = true;

			target->boCanTalk = TRUE;
			target->self->SendSystemMessage( _STR( 15102, target->self->GetLang()));
			
            _LOG_GAMEOP
				LOG_SYSOP,
				"God %s (%s) restored %s's (%s) ability to speak.",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)target->self->GetTrueName(),
				(LPCTSTR)target->GetFullAccountName()
			LOG_

		}
		else
		{
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}

	// Restore player's ability to speak in game.
	COMMAND( "SQUELCH $", GOD_CAN_SQUELCH )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to squelch user %s without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 0 )
			LOG_			
		}

		Players *target = FindCharacter( PARAM( 0 ) );
		if( target != NULL && user->self->boAuthGM == true )
      {
         if( SUPER_USER || target->self->ViewFlag(__FLAG_JUST_DO_IT) == 666)//oki
         {
            CREATE_MESSAGE( "Sorry but you cannot squelch the super user!" );
            SEND_MESSAGE
         }
         else
         {
                success = true;

    			target->boCanTalk = FALSE;
                target->uiTalkIndefinie = 0;
				target->self->SendSystemMessage( _STR( 15103, target->self->GetLang()), CL_RED );

	    		_LOG_GAMEOP
		    		LOG_SYSOP,
			    	"God %s (%s) suspended %s's (%s) ability to speak.",
				    (LPCTSTR)user->self->GetTrueName(),
				    (LPCTSTR)user->GetFullAccountName(),
				    (LPCTSTR)target->self->GetTrueName(),
				    (LPCTSTR)target->GetFullAccountName()
			    LOG_
            }
		}
		else
		{
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}
	
    //////////////////////////////////////////////////////////////////////////////////////////
	// Restore player's ability to speak in game.
	COMMAND( "SQUELCHTMP $ TIME $", GOD_CAN_SQUELCH )
	{
		
      if( user->self->boAuthGM == false )
      {
         _LOG_GAMEOP
            LOG_CRIT_ERRORS,
            "God %s (%s) tried to squelch user %s without authentification!",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName(),
            (LPCTSTR)PARAM( 0 )
            LOG_			
      }

		Players *target = FindCharacter( PARAM( 0 ) );

      if( target != NULL && user->self->boAuthGM == true )
      {
         unsigned int    iNbrHour = atoi(PARAM( 1 ));
         if( SUPER_USER || target->self->ViewFlag(__FLAG_JUST_DO_IT) == 666)//oki
         {
            CREATE_MESSAGE( "Sorry but you cannot squelch the super user!" );
            SEND_MESSAGE
         }
         else if(iNbrHour<1)
         {
            CREATE_MESSAGE( "Sorry but time must be grater than 0 nour..." );
            SEND_MESSAGE;
         }
         else
         {

            success = true;
            time_t TimeMsTmp =  time(NULL);

            target->Lock();
            target->boCanTalk       = FALSE;
            target->uiTalkIndefinie = TimeMsTmp+(iNbrHour*3600);  //en heures
            target->Unlock();

            if(bSendEcho)
            {
               CString strTmp;
               strTmp.Format(_STR( 15119, target->self->GetLang()),iNbrHour);
               target->self->SendSystemMessage( strTmp.GetBuffer(0), CL_RED );

               _LOG_GAMEOP
                  LOG_SYSOP,
                  "God %s (%s) suspended %s's (%s) ability to speak for %d hour.",
                  (LPCTSTR)user->self->GetTrueName(),
                  (LPCTSTR)user->GetFullAccountName(),
                  (LPCTSTR)target->self->GetTrueName(),
                  (LPCTSTR)target->GetFullAccountName(),iNbrHour
                  LOG_
            }
         }
      }
      else
      {
         CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
    }


    //////////////////////////////////////////////////////////////////////////////////////////
	// Restores the ability of a player to shout
	COMMAND( "RESTORE $'S SHOUTS", GOD_CAN_REMOVE_SHOUTS )
	{
		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
		{
            success = true;

			target->boCanShout= TRUE;
			target->self->SendSystemMessage( _STR( 15104, target->self->GetLang()), CL_GREEN );

			_LOG_GAMEOP
				LOG_SYSOP,
				"God %s (%s) restored %s's (%s) ability to shout.",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)target->self->GetTrueName(),
				(LPCTSTR)target->GetFullAccountName()
			LOG_

		}
		else
		{
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}

    // Removes the ability of a player to shout
	COMMAND( "REMOVE $'S SHOUTS", GOD_CAN_REMOVE_SHOUTS )
	{
		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
		{
            if( SUPER_USER || target->self->ViewFlag(__FLAG_JUST_DO_IT) == 666)//oki
			{
                CREATE_MESSAGE( "Sorry but you cannot remove the super user's shouts." );
                SEND_MESSAGE;
            }
			else
			{
                success = true;

    			target->boCanShout = FALSE;
                target->uiShoutIndefinie = 0;
				target->self->SendSystemMessage( _STR( 15105, target->self->GetLang()), CL_RED );

	    		_LOG_GAMEOP
		    		LOG_SYSOP,
			    	"God %s (%s) suspended %s's (%s) ability to shout.",
    				(LPCTSTR)user->self->GetTrueName(),
	    			(LPCTSTR)user->GetFullAccountName(),
		    		(LPCTSTR)target->self->GetTrueName(),
			    	(LPCTSTR)target->GetFullAccountName()
			    LOG_
            }
		}
		else
		{
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}
	
    //////////////////////////////////////////////////////////////////////////////////////////
    // Removes the ability of a player to shout
   COMMAND( "REMOVETMP $'S SHOUTS TIME $", GOD_CAN_REMOVE_SHOUTS )
   {
      Players *target = FindCharacter( PARAM( 0 ) );

      if( target != NULL )
      {
         unsigned int    iNbrHour = atoi(PARAM( 1 ));
         if( SUPER_USER || target->self->ViewFlag(__FLAG_JUST_DO_IT) == 666)//oki
         {
            CREATE_MESSAGE( "Sorry but you cannot remove the super user's shouts." );
            SEND_MESSAGE;
         }
         else if(iNbrHour<1)
         {
            CREATE_MESSAGE( "Sorry but time must be grater than 0 nour..." );
            SEND_MESSAGE;
         }
         else
         {
            success = true;
            time_t TimeMsTmp =  time(NULL);

            target->Lock();
            target->boCanShout       = FALSE;
            target->uiShoutIndefinie = TimeMsTmp+(iNbrHour*3600); //en heures
            target->Unlock();


            if(bSendEcho)
            {
               CString strTmp;
               strTmp.Format(_STR( 15117, target->self->GetLang()),iNbrHour);
               target->self->SendSystemMessage( strTmp.GetBuffer(0), CL_RED );

               _LOG_GAMEOP
                  LOG_SYSOP,
                  "God %s (%s) suspended %s's (%s) ability to shout for %d hour.",
                  (LPCTSTR)user->self->GetTrueName(),
                  (LPCTSTR)user->GetFullAccountName(),
                  (LPCTSTR)target->self->GetTrueName(),
                  (LPCTSTR)target->GetFullAccountName(),iNbrHour
                  LOG_
            }
         }
      }
      else
      {
         CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
   }
	
    //////////////////////////////////////////////////////////////////////////////////////////
    // Blocks all shouts to the public channels by non-operators
    COMMAND( "UNBLOCK PUBLIC CHANNELS", GOD_CHAT_MASTER );
	{
        CPlayerManager::GetChatter().SetOperatorPublicChannelControl( false );

		WorldPos wlPos = { 0, 0, 0 };
		Broadcast::BCServerMessage( wlPos, 0,  _STR( 15106, IntlText::GetDefaultLng()), NULL, CL_GREEN );

        _LOG_GAMEOP
            LOG_SYSOP,
            "God %s (%s) unblocked the public channels.",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName()
        LOG_

		CREATE_MESSAGE( "Public channels are now accessible by everyone" );
		SEND_MESSAGE;
	}
       
    COMMAND( "BLOCK PUBLIC CHANNELS", GOD_CHAT_MASTER )
	{
        CPlayerManager::GetChatter().SetOperatorPublicChannelControl( true );

		WorldPos wlPos = { 0, 0, 0 };
		Broadcast::BCServerMessage( wlPos, 0, _STR( 15107, IntlText::GetDefaultLng()), NULL, CL_RED );

        _LOG_GAMEOP
            LOG_SYSOP,
            "God %s (%s) blocked the public channels.",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName()
        LOG_

		CREATE_MESSAGE( "Public channels are now blocked." );
		SEND_MESSAGE;
	}

    // View the total list of channels
    COMMAND( "VIEW TOTAL CHANNEL LIST", GOD_CHAT_MASTER )
	{
        ChatterChannels &chatter = CPlayerManager::GetChatter();

        typedef vector< ChatterChannels::Channel > ChannelList;
        
        ChannelList channels;
        chatter.GetTotalChannelList( channels );

        ChannelList::iterator i;
        for( i = channels.begin(); i != channels.end(); i++ )
		{
            CREATE_MESSAGE( "Chan: %s", (*i).GetID().c_str() );
            SEND_MESSAGE;
        }

        success = true;
	}
        
    // Restores the ability of a player to page.
    COMMAND( "RESTORE $'S PAGES", GOD_CAN_REMOVE_SHOUTS )
	{
		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
		{
            success = true;

			target->boCanPage = TRUE;
			target->self->SendSystemMessage( _STR( 15109, target->self->GetLang()), CL_GREEN );


			_LOG_GAMEOP
				LOG_SYSOP,
				"God %s (%s) restored %s's (%s) ability to page.",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)target->self->GetTrueName(),
				(LPCTSTR)target->GetFullAccountName()
			LOG_

		}
		else
		{
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}        
	}

	// Removes the ability of a player to page.
    COMMAND( "REMOVE $'S PAGES", GOD_CAN_REMOVE_SHOUTS )	    
	{
        Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
		{
            if( SUPER_USER || target->self->ViewFlag(__FLAG_JUST_DO_IT) == 666)//oki
			{
                CREATE_MESSAGE( "Sorry but you cannot remove the super user's pages." );
                SEND_MESSAGE;
            }
			else
			{
                success = true;

    			target->boCanPage = FALSE;
                target->uiPageIndefinie = 0;
				target->self->SendSystemMessage( _STR( 15108, target->self->GetLang()), CL_RED );


	    		_LOG_GAMEOP
		    		LOG_SYSOP,
			    	"God %s (%s) suspended %s's (%s) ability to page.",
    				(LPCTSTR)user->self->GetTrueName(),
	    			(LPCTSTR)user->GetFullAccountName(),
		    		(LPCTSTR)target->self->GetTrueName(),
			    	(LPCTSTR)target->GetFullAccountName()
			    LOG_
            }
		}
		else
		{
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}
    //////////////////////////////////////////////////////////////////////////////////////////
    // Removes the ability of a player to page.
    COMMAND( "REMOVETMP $'S PAGES TIME $", GOD_CAN_REMOVE_SHOUTS )	    
    {
       Players *target = FindCharacter( PARAM( 0 ) );

       if( target != NULL )
       {
          unsigned int    iNbrHour = atoi(PARAM( 1 ));
          if( SUPER_USER || target->self->ViewFlag(__FLAG_JUST_DO_IT) == 666)//oki
          {
             CREATE_MESSAGE( "Sorry but you cannot remove the super user's pages." );
             SEND_MESSAGE;
          }
          else if(iNbrHour<1)
          {
             CREATE_MESSAGE( "Sorry but time must be grater than 0 nour..." );
             SEND_MESSAGE;
          }

          else
          {

             success = true;
             time_t TimeMsTmp =  time(NULL);

             target->Lock();
             target->boCanPage       = FALSE;
             target->uiPageIndefinie = TimeMsTmp+(iNbrHour*3600);   //en heures
             target->Unlock();

             if(bSendEcho)
             {
                CString strTmp;
                strTmp.Format(_STR( 15118, target->self->GetLang()),iNbrHour);
                target->self->SendSystemMessage( strTmp.GetBuffer(0), CL_RED );

                _LOG_GAMEOP
                   LOG_SYSOP,
                   "God %s (%s) suspended %s's (%s) ability to page for %d hour.",
                   (LPCTSTR)user->self->GetTrueName(),
                   (LPCTSTR)user->GetFullAccountName(),
                   (LPCTSTR)target->self->GetTrueName(),
                   (LPCTSTR)target->GetFullAccountName(),iNbrHour
                   LOG_
             }
          }
       }
       else
       {
          CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
          SEND_MESSAGE;
       }
    }

    COMMAND("SET $'S SANCTU TO $,$,$",GOD_CAN_EDIT_USER);
    {
       Players *target = FindCharacter( PARAM( 0 ) );
       if(target)
       {
          DWORD dwStartSanctuary = ( ( (DWORD)( (WORD)atoi(PARAM(1)) ) << 20 ) + ( (DWORD)( (WORD)atoi(PARAM(2)) ) << 8 ) + (DWORD)( (BYTE)atoi(PARAM(3)) ) );
          target->self->SetFlag( __FLAG_DEATH_LOCATION, dwStartSanctuary);

          CREATE_MESSAGE( "%s have new Sanctu. to %d,%d,%d",target->self->GetTrueName(),atoi(PARAM(1)),atoi(PARAM(2)),atoi(PARAM(3)));
          SEND_MESSAGE;
       }
       else
       {
          CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
          SEND_MESSAGE;
       }
    }

    // Allows setting a shout delay to a specific user.
    COMMAND( "SET $'S SHOUT DELAY TO $", GOD_CAN_REMOVE_SHOUTS )
	{
	    Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
		{
            if( _stricmp( PARAM( 1 ), "DEFAULT" ) == 0 )
			{
                success = true;
                target->self->RemoveFlag( __FLAG_SHOUT );
                _LOG_GAMEOP
				    LOG_SYSOP,
				    "God %s (%s) reset %s's (%s) shout delay to default.",
				    (LPCTSTR)user->self->GetTrueName(),
				    (LPCTSTR)user->GetFullAccountName(),
				    (LPCTSTR)target->self->GetTrueName(),
				    (LPCTSTR)target->GetFullAccountName()
			    LOG_
            }
			else
			{
                int nShoutDelay = atoi( PARAM( 1 ) );

                if( nShoutDelay > 0 )
				{
                    success = true;
                    target->self->SetFlag( __FLAG_SHOUT, nShoutDelay );

                    _LOG_GAMEOP
	    			    LOG_SYSOP,
		    		    "God %s (%s) set %s's (%s) shout delay to %u seconds.",
			    	    (LPCTSTR)user->self->GetTrueName(),
				        (LPCTSTR)user->GetFullAccountName(),
				        (LPCTSTR)target->self->GetTrueName(),
    				    (LPCTSTR)target->GetFullAccountName(),
                        nShoutDelay
	    		    LOG_
                }
				else
				{
                    CREATE_MESSAGE( "Specified shout delay is invalid. Value must be higher than 1." );
                    SEND_MESSAGE;
                }
            }
			            
		}
		else
		{
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}

	// Summons a monster
	COMMAND( "SUMMON MONSTER $ AT $,$,$", GOD_CAN_SUMMON_MONSTERS )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to summon monsters without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName()				
			LOG_			
		}

		BOOL boInvalidPos = FALSE;
		BOOL boInvalidMonster = FALSE;
		
		WorldPos wlPos = { 0, 0, 0 };

		wlPos.X		= atoi( PARAM( 1 ) );
		wlPos.Y		= atoi( PARAM( 2 ) );
		wlPos.world = atoi( PARAM( 3 ) );		
		
		if( wlPos.X != 0 && wlPos.Y != 0 && user->self->boAuthGM == true )
		{
			DWORD dwID = Unit::GetIDFromName( PARAM(0), U_NPC, TRUE );
			
			if( dwID != 0 )
			{
				
				WorldMap *wlWorld = TFCMAIN::GetWorld( wlPos.world );			

				if( wlWorld != NULL )
				{
					if( wlWorld->IsValidPosition( wlPos ) )
					{
						Creatures *lpMonster = new Creatures;
						if( lpMonster->Create( U_NPC, dwID ) )
						{
                            success = true;

							lpMonster->SetDestination( wlPos );
							lpMonster->Do( wandering,"SUMMON MONSTER $ AT $,$,$" );
							lpMonster->SetWL( wlPos );
							if( !wlWorld->SummonMonster( lpMonster, TRUE ) )
							{
								lpMonster->DeleteUnit();
								boInvalidMonster = TRUE;
							}
							else
							{
								_LOG_GAMEOP
									LOG_SYSOP,
									"God %s (%s) summoned monster %s at ( %u, %u, %u ).",
									(LPCTSTR)user->self->GetTrueName(),
									(LPCTSTR)user->GetFullAccountName(),
									PARAM( 0 ),
									wlPos.X,
									wlPos.Y,
									wlPos.world
								LOG_
							}

						}
						else
						{
							boInvalidMonster = TRUE;
							lpMonster->DeleteUnit();
						}
					}
					else
					{
						boInvalidPos = TRUE;
					}
				}
				else
				{
					boInvalidPos = TRUE;
				}
			}
			else
			{
				boInvalidMonster = TRUE;	
			}
		}
		else
		{
			boInvalidPos = TRUE;
		}

		if( boInvalidMonster )
		{
			CREATE_MESSAGE( "Invalid monster ID %s.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
		else if( boInvalidPos )
		{
			CREATE_MESSAGE( "( %u, %u, %u ) isn't a valid world position.", wlPos.X, wlPos.Y, wlPos.world );
			SEND_MESSAGE;
		}
		boFound = TRUE;
	}

	// Summons a monster on god's side 'n' times
	COMMAND( "SUMMON MONSTER $ *$", GOD_CAN_SUMMON_MONSTERS )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to summon monsters without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName()				
			LOG_			
		}

		if( atoi(PARAM(1)) > 200)
		{
			CREATE_MESSAGE( "Invalid quantity: %s. Max allowed: 200", PARAM( 1 ) );
			SEND_MESSAGE;
		}
		else 
		{
			CString csRepeatCommand;
			WorldPos wlPos = user->self->GetWL();
			//WorldPos wlPosDest = user->self->GetWL();
			
			int lineSize = sqrt(double(atoi(PARAM(1))));
			wlPos.X -= lineSize+1;
			wlPos.Y += lineSize/2;
			for (int i=0; i>=0 && i< atoi(PARAM(1)); i++) 
			{
				if (i % lineSize == 0) 
				{
					wlPos.X++;
					wlPos.Y -= lineSize;
				}
				else
				{
					wlPos.Y++;
				}
				csRepeatCommand.Format("SUMMON MONSTER %s AT %d,%d,%d", PARAM(0), wlPos.X, wlPos.Y,wlPos.world);
				VerifySysopCommand(user, csRepeatCommand);
			}
		}
	}

	// Summons a monster on god's side
	COMMAND( "SUMMON MONSTER $", GOD_CAN_SUMMON_MONSTERS )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to summon monster without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName()			
			LOG_			
		}

		BOOL boInvalidPos = FALSE;
		BOOL boInvalidMonster = FALSE;
		
		WorldPos wlPos = user->self->GetWL();
        wlPos.X--;

		if( wlPos.X != 0 && wlPos.Y != 0 && user->self->boAuthGM == true )
		{
			DWORD dwID = Unit::GetIDFromName( PARAM(0), U_NPC, TRUE );
			
			if( dwID != 0 )
			{
				
				WorldMap *wlWorld = TFCMAIN::GetWorld( wlPos.world );			

				if( wlWorld != NULL )
				{
					if( wlWorld->IsValidPosition( wlPos ) )
					{
						Creatures *lpMonster = new Creatures;
						if( lpMonster->Create( U_NPC, dwID ) )
						{
                            success = true;

							lpMonster->SetDestination( wlPos );
							lpMonster->Do( wandering ,"SUMMON MONSTER $");
							lpMonster->SetWL( wlPos );
							if( !wlWorld->SummonMonster( lpMonster, TRUE ) )
							{
								lpMonster->DeleteUnit();
								boInvalidMonster = TRUE;
							}
							else
							{
								_LOG_GAMEOP
									LOG_SYSOP,
									"God %s (%s) summoned monster %s at ( %u, %u, %u ).",
									(LPCTSTR)user->self->GetTrueName(),
									(LPCTSTR)user->GetFullAccountName(),
									PARAM( 0 ),
									wlPos.X,
									wlPos.Y,
									wlPos.world
								LOG_
							}

						}
						else
						{
							boInvalidMonster = TRUE;
							lpMonster->DeleteUnit();
						}
					}
					else
					{
						boInvalidPos = TRUE;
					}
				}
				else
				{
					boInvalidPos = TRUE;
				}
			}
			else
			{
				boInvalidMonster = TRUE;	
			}
		}
		else
		{
			boInvalidPos = TRUE;
		}

		if( boInvalidMonster )
		{
			CREATE_MESSAGE( "Invalid monster ID %s.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
		else if( boInvalidPos )
		{
			CREATE_MESSAGE( "( %u, %u, %u ) isn't a valid world position.", wlPos.X, wlPos.Y, wlPos.world );
			SEND_MESSAGE;
		}
		boFound = TRUE;
	}

   // Summons a monster on god's side
   COMMAND( ".SM $ *$", GOD_CAN_SUMMON_MONSTERS )
   {
      if( user->self->boAuthGM == false )
      {
         _LOG_GAMEOP
            LOG_CRIT_ERRORS,
            "God %s (%s) tried to summon monster without authentification!",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName()			
            LOG_			
      }

      int iMaxMonster = atoi(PARAM(1));
      if( atoi(PARAM(1)) > 200)
      {
         CREATE_MESSAGE( "Invalid quantity: %s. Max allowed: 200", PARAM( 1 ) );
         SEND_MESSAGE;
      }
      else
      {
         DWORD dwID = Unit::GetIDFromName( PARAM(0), U_NPC, TRUE );
         if(dwID == 0)
         {
            CREATE_MESSAGE( "Invalid monster ID %s.", PARAM( 0 ) );
            SEND_MESSAGE;
         }
         else
         {
            int iRayon = iMaxMonster;
            if(iMaxMonster>100)
               iRayon = iMaxMonster/7;
            else if(iMaxMonster>10)
               iRayon = iMaxMonster/5;

            if(iRayon <=0)
               iRayon = 1;
            

            WorldPos wlPos = user->self->GetWL();
            wlPos.X--;
            WorldMap *wlWorld = TFCMAIN::GetWorld( wlPos.world );	

            if( wlPos.X != 0 && wlPos.Y != 0 && wlWorld != NULL)
            {

               _LOG_GAMEOP
                  LOG_SYSOP,
                  "God %s (%s) MASSsummoned (%u) monsters %s at ( %u, %u, %u ).",
                  (LPCTSTR)user->self->GetTrueName(),
                  (LPCTSTR)user->GetFullAccountName(),
                  iMaxMonster,
                  PARAM( 0 ),
                  wlPos.X,
                  wlPos.Y,
                  wlPos.world
               LOG_

               WorldPos wlSPos;
               wlSPos.world = wlPos.world;
               for (int j = 0; j < iMaxMonster; j++)
               {
                  wlSPos.X = wlPos.X + (rand()%iRayon) - iRayon/2;
                  wlSPos.Y = wlPos.Y + (rand()%iRayon) - iRayon/2;
                  if( wlWorld->IsValidPosition( wlPos ) )
                  {
                     Creatures *lpMonster = new Creatures;
                     if( lpMonster->Create( U_NPC, dwID ) )
                     {
                        success = true;
                        lpMonster->SetDestination( wlSPos );
                        lpMonster->Do( wandering ,".SM $ *$");
                        lpMonster->SetWL( wlSPos );
                        if( !wlWorld->SummonMonster( lpMonster, TRUE ) )
                        {
                           lpMonster->DeleteUnit();
                        }
                     }
                     else
                     {
                        lpMonster->DeleteUnit();
                     }
                  }
                  else
                  {
                     //Nothing on avertie pas pour rien que la pos ets invalide,...
                  }
               }
            }
            else
            {
               CREATE_MESSAGE( "( %u, %u, %u ) isn't a valid world position.", wlPos.X, wlPos.Y, wlPos.world );
               SEND_MESSAGE;
            }
            boFound = TRUE;
         }
      }
   }

   // Summons a monster on god's side
   COMMAND( ".SM $", GOD_CAN_SUMMON_MONSTERS )
   {
      if( user->self->boAuthGM == false )
      {
         _LOG_GAMEOP
            LOG_CRIT_ERRORS,
               "God %s (%s) tried to summon monster without authentification!",
               (LPCTSTR)user->self->GetTrueName(),
               (LPCTSTR)user->GetFullAccountName()			
         LOG_			
      }

      DWORD dwID = Unit::GetIDFromName( PARAM(0), U_NPC, TRUE );
      if(dwID == 0)
      {
         CREATE_MESSAGE( "Invalid monster ID %s.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
      else
      {
         WorldPos wlPos = user->self->GetWL();
         wlPos.X--;
         WorldMap *wlWorld = TFCMAIN::GetWorld( wlPos.world );	

         if( wlPos.X != 0 && wlPos.Y != 0 && wlWorld != NULL)
         {

            _LOG_GAMEOP
               LOG_SYSOP,
                  "God %s (%s) summoned monster %s at ( %u, %u, %u ).",
                  (LPCTSTR)user->self->GetTrueName(),
                  (LPCTSTR)user->GetFullAccountName(),
                  PARAM( 0 ),
                  wlPos.X,
                  wlPos.Y,
                  wlPos.world
            LOG_

            if( wlWorld->IsValidPosition( wlPos ) )
            {
               Creatures *lpMonster = new Creatures;
               if( lpMonster->Create( U_NPC, dwID ) )
               {
                  success = true;
                  lpMonster->SetDestination( wlPos );
                  lpMonster->Do( wandering ,".SM $");
                  lpMonster->SetWL( wlPos );
                  if( !wlWorld->SummonMonster( lpMonster, TRUE ) )
                  {
                     lpMonster->DeleteUnit();
                  }
               }
               else
               {
                  lpMonster->DeleteUnit();
               }
            }
            else
            {
               //Nothing on avertie pas pour rien que la pos ets invalide,...
            }
         }
         else
         {
            CREATE_MESSAGE( "( %u, %u, %u ) isn't a valid world position.", wlPos.X, wlPos.Y, wlPos.world );
            SEND_MESSAGE;
         }
         boFound = TRUE;
      }
   }

   COMMAND( ".IP $", GOD_CAN_GIVE_GOD_FLAGS ) //see account or IP
   {
      if( user->self->boAuthGM == false )
      {
         _LOG_GAMEOP
            LOG_CRIT_ERRORS,
            "God %s (%s) tried to see IP without authentification!",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName()			
            LOG_			
      }

      Players *target = FindCharacter( PARAM( 0 ) );
      if(target)
      {
         CString strMsgTmp;
         strMsgTmp.Format(_STR(15208, target->self->GetLang()),target->self->GetTrueName(),
         target->GetAccount(),
         target->self->GetID(),
         target->GetIP(),
         target->self->GetWL().X,
         target->self->GetWL().Y,
         target->self->GetWL().world);
         CREATE_MESSAGE( strMsgTmp);
         SEND_MESSAGE;
      }
      else
      {
         CREATE_MESSAGE( _STR( 15202, user->self->GetLang()));
         SEND_MESSAGE;
      }
      boFound = TRUE;
   }

	// Summons an item in somebody's backpack
   COMMAND( "SUMMON ITEM $ ON $", GOD_CAN_SUMMON_ITEMS )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to summon item on %s without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 1 )
			LOG_			
		}

		BOOL boInvalidItem = FALSE;
		DWORD dwID = Unit::GetIDFromName( PARAM(0), U_OBJECT, TRUE );

		Players *target = FindCharacter( PARAM( 1 ) );

		if( target != NULL && user->self->boAuthGM == true)
		{
		    if( dwID != 0 )
			{			
			    Objects *lpItem = new Objects;
			    if( lpItem->Create( U_OBJECT, dwID ) )
				{                
    			    _item *item = NULL;
	    		    // Get the item structure.
		    	    lpItem->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &item );

                    if( item->canSummon || user->GetGodFlags() & GOD_DEVELOPPER )
					{
                        success = true;

                        target->Lock();

                        target->self->AddToBackpack( lpItem );

                        TFCPacket sending;
                        sending << (RQ_SIZE)RQ_ViewBackpack2;
	    			    sending << (char)0;	// Don't show backpack..!!
		    		    sending << (long)target->self->GetID();
			    	    target->self->PacketBackpack( sending );
                        target->self->SendPlayerMessage( sending );

                        sending.Destroy();
                        target->self->packet_equiped( sending );
                        target->self->SendPlayerMessage( sending );

                        _LOG_GAMEOP
				            LOG_SYSOP,
						        "God %s (%s) summoned item %s in user %s (%s)'s backpack.",
						        (LPCTSTR)user->self->GetTrueName(),
						        (LPCTSTR)user->GetFullAccountName(),
						        PARAM( 0 ),
                                (LPCTSTR)target->self->GetTrueName(),
                                (LPCTSTR)target->GetFullAccountName()
			            LOG_

                        target->Unlock();
                    }
					else
					{
                        boInvalidItem = TRUE;
                        lpItem->DeleteUnit();
                    }
			    }
				else
				{
				    boInvalidItem = TRUE;
				    lpItem->DeleteUnit();
			    }                            
		    }
			else
			{
			    boInvalidItem = TRUE;	
		    }
        }
		else
		{
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 1 ) );
			SEND_MESSAGE;
        }

		if( boInvalidItem ){
			CREATE_MESSAGE( "Invalid object ID %s.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}

   
   

	// Summons an item on the ground        
   COMMAND( "SUMMON ITEM $ AT $,$,$", GOD_CAN_SUMMON_ITEMS )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to summon item on the ground without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName()				
			LOG_			
		}

		BOOL boInvalidItem = FALSE;
		DWORD dwID = Unit::GetIDFromName( PARAM(0), U_OBJECT, TRUE );
			
		if( dwID != 0 && user->self->boAuthGM == true )
		{
            WorldPos wlPos = { atoi(PARAM(1)),atoi(PARAM(2)),atoi(PARAM(3)) };
				
			Objects *lpItem = new Objects;
			if( lpItem->Create( U_OBJECT, dwID ) )
			{  
			    _item *item = NULL;
			    // Get the item structure.
			    lpItem->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &item );

                if( item->canSummon || user->GetGodFlags() & GOD_DEVELOPPER )
				{                
                    WorldMap *world = TFCMAIN::GetWorld( wlPos.world );
                    if( world != NULL && world->IsValidPosition( wlPos ) )
					{
                        world->deposit_unit( wlPos, lpItem );
                        success = true;

                        lpItem->BroadcastPopup( wlPos, true );

                        _LOG_GAMEOP
				            LOG_SYSOP,
						        "God %s (%s) summoned item %s at %u, %u, %u.",
						        (LPCTSTR)user->self->GetTrueName(),
						        (LPCTSTR)user->GetFullAccountName(),
						        PARAM( 0 ),
                                wlPos.X,
                                wlPos.Y,
                                wlPos.world
			            LOG_
                    }
					else
					{
                        CREATE_MESSAGE( 
                            "World position %u, %u, %u is not valid.",
                            wlPos.X,
                            wlPos.Y,
                            wlPos.world
                        );
                        SEND_MESSAGE;
                        lpItem->DeleteUnit();
                    }
                }
				else
				{
    				boInvalidItem = TRUE;
	    			lpItem->DeleteUnit();
                }
			}
			else
			{
				boInvalidItem = TRUE;
				lpItem->DeleteUnit();
			}
		}
		else
		{
			boInvalidItem = TRUE;	
		}

		if( boInvalidItem )
		{
			CREATE_MESSAGE( "Invalid object ID %s.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}

   
	// Summons an item in the god's backpack (any qty for non unique items)
   COMMAND( "SUMMON ITEM $ *$", GOD_CAN_SUMMON_ITEMS )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to summon items without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName()
			LOG_			
		}

		BOOL boInvalidItem = FALSE;
		DWORD dwID = Unit::GetIDFromName( PARAM(0), U_OBJECT, TRUE );
			
		if( dwID != 0 && user->self->boAuthGM == true )
		{
				
			Objects *lpItem = new Objects;
			if( lpItem->Create( U_OBJECT, dwID ) )
			{
			    _item *item = NULL;
			    // Get the item structure.
			    lpItem->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &item );

                if( item->canSummon || user->GetGodFlags() & GOD_DEVELOPPER )
				{
					if ( ! lpItem->IsUnique() && atoi(PARAM(1))>0) 
					{
						lpItem->SetQty(atoi(PARAM(1)));
					}
					user->self->AddToBackpack( lpItem );

    	 		    TFCPacket sending;
                    sending << (RQ_SIZE)RQ_ViewBackpack2;
	    		    sending << (char)0;	// Don't show backpack..!!
		    	    sending << (long)user->self->GetID();
			        user->self->PacketBackpack( sending );
                    user->self->SendPlayerMessage( sending );

                    sending.Destroy();
                    user->self->packet_equiped( sending );
                    user->self->SendPlayerMessage( sending );


                   success = true;
				    _LOG_GAMEOP
					    LOG_SYSOP,
					    "God %s (%s) summoned %ix item %s.",
					    (LPCTSTR)user->self->GetTrueName(),
					    (LPCTSTR)user->GetFullAccountName(),
						atoi( PARAM( 1 ) ),
					    PARAM( 0 )
				    LOG_
                }
				else
				{
                    boInvalidItem = TRUE;
                    lpItem->DeleteUnit();
                }
			}
			else
			{
				boInvalidItem = TRUE;
				lpItem->DeleteUnit();
			}
		}
		else
		{
			boInvalidItem = TRUE;	
		}

		if( boInvalidItem )
		{
			CREATE_MESSAGE( "Invalid object ID %s.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}

   // Summons an item in the god's backpack (any qty for non unique items)
	// Summons an item in the god's backpack        
   COMMAND( "SUMMON ITEM $", GOD_CAN_SUMMON_ITEMS )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to summon item without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName()
			LOG_			
		}
		else
		{
			CString csRepeatCommand;
			csRepeatCommand.Format("SUMMON ITEM %s *1", PARAM(0));
			VerifySysopCommand(user,csRepeatCommand);
		}
	}
    // Moen_OK: Night: A tester...
    // remove the Equipped stuff from a player
    COMMAND( "REMOVE EQUIPPED ITEMS FROM $", GOD_CAN_SUMMON_ITEMS )
    {
		
		Players *target = FindCharacter( PARAM( 0 ) );

      if( target != NULL && user->self->boAuthGM == true)
      {
         //////////////////////////////
         // REMOVAL OF ALL ITEMS
         for( int i = 0; i < EQUIP_POSITIONS; i++ ) 
         {
            target->self->unequip_object( i );
         }
         Unit **equip = target->self->GetEquipment();
         for( int i = 0; i < EQUIP_POSITIONS; i++ ) 
         {
            // Flush all items that couldn't be unequipped.
            equip[ i ] = NULL;
         }
      }
      else
		{
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	
	}

    // Moen_OK: Night: A tester...
    // remove the Equipped stuff from a player
    COMMAND( "REMOVE EQUIPPED ITEMS FROM $", GOD_CAN_SUMMON_ITEMS )
    {

       Players *target = FindCharacter( PARAM( 0 ) );

       if( target != NULL && user->self->boAuthGM == true)
       {
          //////////////////////////////
          // REMOVAL OF ALL ITEMS
          for( int i = 0; i < EQUIP_POSITIONS; i++ ) 
          {
             target->self->unequip_object( i );
          }
          Unit **equip = target->self->GetEquipment();
          for( int i = 0; i < EQUIP_POSITIONS; i++ ) 
          {
             // Flush all items that couldn't be unequipped.
             equip[ i ] = NULL;
          }
       }
       else
       {
          CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
          SEND_MESSAGE;
       }

    }

	// Sets a user flag on the given user.
	COMMAND( "SET $'S FLAG $ TO $", GOD_CAN_SET_USER_FLAG )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to set %s's flag without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 0 )
			LOG_			
		}

		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL && user->self->boAuthGM == true )
		{
			DWORD dwFlagID = atoi( PARAM( 1 ) );
			DWORD dwFlagValue = atoi( PARAM( 2 ) );
			if( dwFlagID != 0 )
			{
				target->Lock();
				target->self->SetFlag( dwFlagID, dwFlagValue );
				target->Unlock();
            success = true;

            if(dwFlagID == __FLAG_PJ_VS_MONSTER_FRIENDLY) //CV:FACTIONID CHANGE
            {
               TFCPacket sending;
               sending << (RQ_SIZE)RQ_UpdateFactionID;
               sending << (short)dwFlagValue;
               target->self->SendPlayerMessage( sending );
            }
                
                


				_LOG_GAMEOP
					LOG_SYSOP,
					"God %s (%s) edited %s's (%s) flag#%u to %u.",
					(LPCTSTR)user->self->GetTrueName(),
					(LPCTSTR)user->GetFullAccountName(),
					(LPCTSTR)target->self->GetTrueName(),
					(LPCTSTR)target->GetFullAccountName(),
					dwFlagID,
					dwFlagValue
					LOG_
			}
			boFound = TRUE;
		}
		else
		{
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}

	// Removes a user flag on the given user.
	COMMAND( "REMOVE $'S FLAG $", GOD_CAN_SET_USER_FLAG )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to remove %s's flag without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 0 )
			LOG_			
		}

		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL && user->self->boAuthGM == true )
		{
			DWORD dwFlagID = atoi( PARAM( 1 ) );
			if( dwFlagID != 0 )
			{
                success = true;

				target->Lock();
				target->self->RemoveFlag( dwFlagID );
				target->Unlock();
				_LOG_GAMEOP
					LOG_SYSOP,
					"God %s (%s) removed %s's (%s) flag#%u.",
					(LPCTSTR)user->self->GetTrueName(),
					(LPCTSTR)user->GetFullAccountName(),
					(LPCTSTR)target->self->GetTrueName(),
					(LPCTSTR)target->GetFullAccountName(),
					dwFlagID
				LOG_

			}
		}
		else
		{
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}

	// View the specific value of a flag on a player,
    COMMAND( "VIEW $'S FLAG $", GOD_CAN_SET_USER_FLAG )
	{
		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
		{
			DWORD dwFlagID = atoi( PARAM( 1 ) );
			if( dwFlagID != 0 )
			{
				target->Lock();
				int flagValue = target->self->ViewFlag( dwFlagID );
				target->Unlock();

                CREATE_MESSAGE( "%s's flag %u = %d", PARAM( 0 ), dwFlagID, flagValue );
                SEND_MESSAGE;
			}
		}
		else
		{
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}

	// Set a skill or spell
	COMMAND( "SET $'S SKILL $ TO $", GOD_CAN_EDIT_USER_SKILLS )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to set %s's skill without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 0 )
			LOG_			
		}

		WORD wSpellID = atoi( PARAM( 1 ) );
		WORD wValue = atoi( PARAM( 2 ) );

        // If the spellID is 0, then perhaps a spell name was provided.
        if( wSpellID == 0 )
		{
            // Retreive the spell by its name.
            LPSKILL lpSkill = Skills::GetSkillByName( PARAM( 1 ), user->self->GetLang() );
            if( lpSkill != NULL )
			{
                wSpellID = lpSkill->nSkillID;
            }
        }	
				
		if( wSpellID != 0 )
		{
			Players *target = FindCharacter( PARAM( 0 ) );

			if( target != NULL && user->self->boAuthGM == true )
			{
				// Set 0 hp
				target->Lock();
				
				LPUSER_SKILL lpUserSkill = target->self->GetSkill( wSpellID );
				if( lpUserSkill != NULL )
				{
                    success = true;

					lpUserSkill->SetSkillPnts( wValue );
				    _LOG_GAMEOP
					    LOG_SYSOP,
					    "God %s (%s) set %s's (%s) skill %u to %u.",
					    (LPCTSTR)user->self->GetTrueName(),
					    (LPCTSTR)user->GetFullAccountName(),
					    (LPCTSTR)target->self->GetTrueName(),
					    (LPCTSTR)target->GetFullAccountName(),
					    wSpellID,
                        wValue
				    LOG_
				}
				else
				{
					CREATE_MESSAGE( "User doesn't have this skill." );
					SEND_MESSAGE;
				}

				target->Unlock();

			}
			else
			{
				CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
				SEND_MESSAGE;
			}
		}
		else
		{
			CREATE_MESSAGE( "Skill ID isn't valid" );
			SEND_MESSAGE;
		}
	}

	// Sets a specific playet stat to a given value
	COMMAND( "SET $'S $ TO $", GOD_CAN_EDIT_USER )	
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to set %s's %s without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 0 ),
				(LPCTSTR)PARAM( 1 )
			LOG_			
		}

		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL && user->self->boAuthGM == true )
		{
			DWORD dwValue = atoi( PARAM( 2 ) );
			WORD bStat = (WORD)dwValue;
			
			CString csLogText;
			CString csAppend;
			csLogText.Format( "God %s (%s) edited %s's (%s) ",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)target->self->GetTrueName(),
				(LPCTSTR)target->GetFullAccountName()
			);

            bool plus = false;;
            bool minus = false;
            int paramLen = strlen( PARAM( 2 ) );
            // If the parameter is long enough to hold a + sign.
            if( paramLen > 1 )
			{
                // If the parameter has a plus sign.
                if( PARAM(2)[0] == '+' )
				{
                    plus = true;
                }
				else if( PARAM(2)[0] == '-' )
				{
                    dwValue = -atoi( PARAM( 2 ) );
                    bStat = (WORD)dwValue;
                    minus = true;
                }
            }
            
            target->Lock();

			TRACE( "\r\nParameters: %s, %s, %s", PARAM( 0 ), PARAM( 1 ), PARAM( 2 ) );

            success = true;

			FIRST_COMPONENT( "STR", GOD_CAN_EDIT_USER_STAT, 1 )
                if( plus ){ bStat += target->self->GetTrueSTR(); }
                if( minus ){ bStat = SafeMinus( target->self->GetTrueSTR(), bStat ); };
                target->self->SetSTR( bStat );
				csAppend.Format( "strength to %u.", bStat );
			
			COMPONENT( "END", GOD_CAN_EDIT_USER_STAT, 1 )
                if( plus ){ bStat += target->self->GetTrueEND(); }
                if( minus ){ bStat = SafeMinus( target->self->GetTrueEND(), bStat ); };
				target->self->SetEND( bStat );
				csAppend.Format( "endurance to %u.", bStat );

			COMPONENT(  "AGI", GOD_CAN_EDIT_USER_STAT, 1 )
                if( plus ){ bStat += target->self->GetTrueAGI(); }
                if( minus ){ bStat = SafeMinus( target->self->GetTrueAGI(), bStat ); };
				target->self->SetAGI( bStat );
				csAppend.Format( "agility to %u.", bStat );

			COMPONENT( "INT", GOD_CAN_EDIT_USER_STAT, 1 )
                if( plus ){ bStat += target->self->GetTrueINT(); }
                if( minus ){ bStat = SafeMinus( target->self->GetTrueINT(), bStat ); };
				target->self->SetINT( bStat );
				csAppend.Format( "intelligence to %u.", bStat );

			COMPONENT( "WIL", GOD_CAN_EDIT_USER_STAT, 1 )				
				csAppend.Format( "willpower is no longer implemented." );

			COMPONENT( "WIS", GOD_CAN_EDIT_USER_STAT, 1 )
                if( plus ){ bStat += target->self->GetTrueWIS(); }
                if( minus ){ bStat = SafeMinus( target->self->GetTrueWIS(), bStat ); };
				target->self->SetWIS( bStat );
				csAppend.Format( "wisdom to %u.", bStat );

			COMPONENT( "LUCK", GOD_CAN_EDIT_USER_STAT, 1 )
				if( plus ) { bStat += target->self->GetTrueLCK(); }
				if( minus ) { bStat = SafeMinus( target->self->GetTrueLCK(), bStat ); };
				target->self->SetLCK( bStat );
				csAppend.Format( "luck to %u", bStat );

         COMPONENT( "KARMA", GOD_CAN_EDIT_USER_STAT, 1 )                
             target->self->SetKarma( atoi( PARAM( 2 ) ) );
             csAppend.Format( "karma to %d", bStat );
         COMPONENT( "CRIME", GOD_CAN_EDIT_USER_STAT, 1 )                
             target->self->SetCrime( atoi( PARAM( 2 ) ) );
             csAppend.Format( "crime to %d", bStat );
         COMPONENT( "HONOR", GOD_CAN_EDIT_USER_STAT, 1 )                
             target->self->SetHonor( atoi( PARAM( 2 ) ) );
             csAppend.Format( "honor to %d", bStat );    

         COMPONENT( "FIRE RESIST", GOD_CAN_EDIT_USER_STAT, 1 )
             if( plus ){ bStat += target->self->GetTrueFireResist(); }
             if( minus ){ bStat = SafeMinus( target->self->GetTrueFireResist(), bStat ); };
             target->self->SetFireResist( bStat );
             csAppend.Format( "fire resist to %u.", bStat );

         COMPONENT( "WATER RESIST", GOD_CAN_EDIT_USER_STAT, 1 )
             if( plus ){ bStat += target->self->GetTrueWaterResist(); }
             if( minus ){ bStat = SafeMinus( target->self->GetTrueWaterResist(), bStat ); };
             target->self->SetWaterResist( bStat );
             csAppend.Format( "water resist to %u.", bStat );

         COMPONENT( "EARTH RESIST", GOD_CAN_EDIT_USER_STAT, 1 )
             if( plus ){ bStat += target->self->GetTrueEarthResist(); }
             if( minus ){ bStat = SafeMinus( target->self->GetTrueEarthResist(), bStat ); };
             target->self->SetEarthResist( bStat );
             csAppend.Format( "earth resist to %u.", bStat );

         COMPONENT( "AIR RESIST", GOD_CAN_EDIT_USER_STAT, 1 )
             if( plus ){ bStat += target->self->GetTrueAirResist(); }
             if( minus ){ bStat = SafeMinus( target->self->GetTrueAirResist(), bStat ); };
             target->self->SetAirResist( bStat );
             csAppend.Format( "air resist to %u.", bStat );

         COMPONENT( "LIGHT RESIST", GOD_CAN_EDIT_USER_STAT, 1 )
             if( plus ){ bStat += target->self->GetTrueLightResist(); }
             if( minus ){ bStat = SafeMinus( target->self->GetTrueLightResist(), bStat ); };
             target->self->SetLightResist( bStat );
             csAppend.Format( "light resist to %u.", bStat );

         COMPONENT( "DARK RESIST", GOD_CAN_EDIT_USER_STAT, 1 )
             if( plus ){ bStat += target->self->GetTrueDarkResist(); }
             if( minus ){ bStat = SafeMinus( target->self->GetTrueDarkResist(), bStat ); };
             target->self->SetDarkResist( bStat );
             csAppend.Format( "dark resist to %u.", bStat );

         COMPONENT( "FIRE", GOD_CAN_EDIT_USER_STAT, 1 )
             if( plus ){ bStat += target->self->GetTrueFirePower(); }
             if( minus ){ bStat = SafeMinus( target->self->GetTrueFirePower(), bStat ); };
             target->self->SetFirePower( bStat );
             csAppend.Format( "fire power to %u.", bStat );

         COMPONENT( "WATER", GOD_CAN_EDIT_USER_STAT, 1 )
             if( plus ){ bStat += target->self->GetTrueWaterPower(); }
             if( minus ){ bStat = SafeMinus( target->self->GetTrueWaterPower(), bStat ); };
             target->self->SetWaterPower( bStat );
             csAppend.Format( "water power to %u.", bStat );

         COMPONENT( "EARTH", GOD_CAN_EDIT_USER_STAT, 1 )
             if( plus ){ bStat += target->self->GetTrueEarthPower(); }
             if( minus ){ bStat = SafeMinus( target->self->GetTrueEarthPower(), bStat ); };
             target->self->SetEarthPower( bStat );
             csAppend.Format( "earth power to %u.", bStat );

         COMPONENT( "AIR", GOD_CAN_EDIT_USER_STAT, 1 )
             if( plus ){ bStat += target->self->GetTrueAirPower(); }
             if( minus ){ bStat = SafeMinus( target->self->GetTrueAirPower(), bStat ); };
             target->self->SetAirPower( bStat );
             csAppend.Format( "air power to %u.", bStat );

         COMPONENT( "LIGHT", GOD_CAN_EDIT_USER_STAT, 1 )
             if( plus ){ bStat += target->self->GetTrueLightPower(); }
             if( minus ){ bStat = SafeMinus( target->self->GetTrueLightPower(), bStat ); };
             target->self->SetLightPower( bStat );
             csAppend.Format( "light power to %u.", bStat );

         COMPONENT( "DARK", GOD_CAN_EDIT_USER_STAT, 1 )
             if( plus ){ bStat += target->self->GetTrueDarkPower(); }
             if( minus ){ bStat = SafeMinus( target->self->GetTrueDarkPower(), bStat ); };
             target->self->SetDarkPower( bStat );
             csAppend.Format( "dark power to %u.", bStat );

         COMPONENT( "DODGE", GOD_CAN_EDIT_USER_STAT, 1 )
             if( plus ){ bStat += target->self->GetTrueDODGE(); }
             if( minus ){ bStat = SafeMinus( target->self->GetTrueDODGE(), bStat ); };
			target->self->SetDODGE( bStat );
			csAppend.Format( "dodge skill to %u.", bStat );

			COMPONENT( "ATTACK", GOD_CAN_EDIT_USER_STAT, 1 )
                if( plus ){ bStat += target->self->GetTrueATTACK(); }
                if( minus ){ bStat = SafeMinus( target->self->GetTrueATTACK(), bStat ); };
				target->self->SetATTACK( bStat );
				csAppend.Format( "attack skill to %u.", bStat );

			COMPONENT( "GOLD", GOD_CAN_EDIT_USER_BACKPACK, 1 )
                if( plus ){ dwValue += target->self->GetGold(); }
                if( minus ){ dwValue = SafeMinus( target->self->GetGold(), dwValue ); };
				target->self->SetGold( dwValue );
				csAppend.Format( "gold to %u.", dwValue );
			
         COMPONENT( "XP", GOD_CAN_EDIT_USER_XP_LEVEL, 1 )
             __int64 xp;
             // If a + sign was put in front of the number.
             if( plus )
			{
                 __int64 oldxp = target->self->GetXP();
                 xp = _atoi64( PARAM(2) ) + target->self->GetXP();
                 target->self->SetXP( xp );

                 csAppend.Format( "experience from %I64u to %I64u.", oldxp, xp );
             }
			else if( minus )
			{
                 __int64 oldxp = target->self->GetXP();
                 xp = _atoi64( PARAM(2) ) + target->self->GetXP();
                 if( xp < 0 )
				{
                     xp = 0;
                 }
                 target->self->SetXP( xp );

                 csAppend.Format( "experience from %I64u to %I64u.", oldxp, xp );
             }
			else
			{
                 xp = _atoi64( PARAM(2) );
                 target->self->SetXP( xp );
                 csAppend.Format( "experience to %I64u.", xp );
             }

		COMPONENT( "HP", GOD_CAN_EDIT_USER_HP, 1 )
             if( plus ){ dwValue += target->self->GetHP(); }
             if( minus ){ dwValue = SafeMinus( target->self->GetHP(), dwValue ); };
			target->self->SetHP( dwValue, true );
			csAppend.Format( "hit points to %u.", dwValue );

		COMPONENT( "MAX HP", GOD_CAN_EDIT_USER_HP, 1 )
             if( plus ){ dwValue += target->self->GetTrueMaxHP(); }
             if( minus ){ dwValue = SafeMinus( target->self->GetTrueMaxHP(), dwValue ); };
			target->self->SetMaxHP( dwValue );
			csAppend.Format( "maximum hit points to %u.", dwValue );

		COMPONENT( "MAX MANA", GOD_CAN_EDIT_USER_MANA_FAITH, 1 )
             if( plus ){ dwValue += target->self->GetTrueMaxMana(); }
             if( minus ){ dwValue = SafeMinus( target->self->GetTrueMaxMana(), dwValue ); };
			target->self->SetMaxMana( (WORD)dwValue );
			csAppend.Format( "maximum mana to %u.", dwValue );

		COMPONENT( "MANA", GOD_CAN_EDIT_USER_MANA_FAITH, 1 )
             if( plus ){ dwValue += target->self->GetMana(); }
             if( minus ){ dwValue = SafeMinus( target->self->GetMana(), dwValue ); };
			target->self->SetMana( (WORD)dwValue );
			csAppend.Format( "mana to %u.", dwValue );

		COMPONENT( "LEVEL", GOD_CAN_EDIT_USER_XP_LEVEL, 1 )
             if( plus ){ dwValue += target->self->GetLevel(); }
             if( minus ){ dwValue = SafeMinus( target->self->GetLevel(), dwValue ); };
             if( dwValue != 0 )
			{
                 target->self->SetLevel( (WORD)dwValue );
			    csAppend.Format( "level to %u.", dwValue );
             }
			else
			{
                 csAppend.Format( "level to 0, but change was forbidden." );                    
             }

         COMPONENT( "TITLE", GOD_CAN_EDIT_USER_NAME, 1 )
             target->self->SetTitle( PARAM( 2 ) );
             csAppend.Format( "title to %s.", PARAM( 2 ) );

			COMPONENT( "GUILD", GOD_CAN_EDIT_USER_NAME, 1 )//BLBLBL sysopcmd :: SET $'S GUILD TO
                target->self->SetGuildName( (char*)PARAM( 2 ) );
                csAppend.Format( "guild to %s.", PARAM( 2 ) );

            COMPONENT( "MISC DESC", GOD_CAN_EDIT_USER_NAME, 1 )
                target->self->SetListingMiscDesc( PARAM( 2 ) );
                csAppend.Format( "misc. listing description to %s.", PARAM( 2 ) );

            COMPONENT( "PSEUDONAME", GOD_CAN_EDIT_USER_NAME, 1 )
                target->self->SetPseudoName( PARAM( 2 ) );
                csAppend.Format( "pseudoname to %s.", PARAM( 2 ) );

			COMPONENT( "NAME", GOD_CAN_EDIT_USER_NAME, 1 )
                CString csName = PARAM( 2 );
                // Verify that the new name has correct specs.
         CString csCheckName = csName.SpanIncluding(_T("1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ -‰ÔÎˆ¸ÈË‡˘‚ÓÍÙ˚"));
                if( csCheckName == csName )
				{
                    if( csName.GetLength() <= 19 && csName.GetLength() > 1 )
					{
                        // Otherwise, do an exhaustive player name search, asynchronously.
                        SetNameData *lpNameData = new SetNameData;
						
                        // Setup the SetNameData structure
                        lpNameData->sockGod = user->IPaddrO;
                        lpNameData->sockTarget = target->IPaddrO;
                        lpNameData->csNewName = PARAM( 2 );

                        // Call asynchronous name change function.
                        AsyncFuncQueue::GetMainQueue()->Call( AsyncSetName, lpNameData );
                    }
					else
					{
                        CString csMessage;
                        csMessage.Format( "Character names must be between 2 and 19 characters long.", csName.GetLength() );
                        user->self->SendSystemMessage( csMessage );
                    }
                }
				else
				{
                    user->self->SendSystemMessage( "Name unchanged, there were invalid characters in new name." );
                }				

			COMPONENT( "APPEARANCE", GOD_CAN_EDIT_USER_APPEARANCE_CORPSE, 1 )
            if( dwValue != 0 )
            {
               if(dwValue == 10011)
                  target->self->SetGender( GENDER_MALE );
               else if(dwValue == 10012)
                  target->self->SetGender( GENDER_FEMALE );
               target->self->Lock();
               target->self->SetAppearance( dwValue );
               csAppend.Format( "appearance to %u.", dwValue );

               Broadcast::BCObjectChanged( target->self->GetWL(), _DEFAULT_RANGE_CHANGE,
                  target->self->GetAppearance(),
                  target->self->GetID(),0
                  );
               target->self->Unlock();

               target->self->Lock();
               target->self->Teleport(target->self->GetWL(),0);
               target->self->Unlock();
            }

			COMPONENT( "CORPSE", GOD_CAN_EDIT_USER_APPEARANCE_CORPSE, 1 )
				if( (WORD)dwValue != 0 )
				{
					csAppend.Format( "corpse to %u.", dwValue );
					target->self->SetCorpse( (WORD)dwValue );
				}

            COMPONENT( "SKILLPOINTS", GOD_CAN_EDIT_USER_STAT, 1 )
                if( plus ){ dwValue += target->self->GetSkillPoints(); }
                if( minus ){ dwValue = SafeMinus( target->self->GetSkillPoints(), dwValue ); };
                // Flush all skill points.
                target->self->UseSkillPnts( target->self->GetSkillPoints() );
				// Give new skill points.
                target->self->GiveSkillPoints( dwValue );
				csAppend.Format( "skill points to %u.", dwValue );

            COMPONENT( "STATPOINTS", GOD_CAN_EDIT_USER_STAT, 1 )
                if( plus ){ dwValue += target->self->GetStatPoints(); }
                if( minus ){ dwValue = SafeMinus( target->self->GetStatPoints(), dwValue ); };
                target->self->SetStatPoints( dwValue );
            
            COMPONENT( "RPLEVEL", GOD_CAN_EDIT_USER_STAT, 1 )
               csAppend.Format( "RP Level to %u.", dwValue );
               target->self->SetRP_XPLevel( dwValue );

             COMPONENT( "RPPOINT", GOD_CAN_EDIT_USER_STAT, 1 )
                csAppend.Format( "RP Level Point to %u.", dwValue );
                target->self->SetRP_XP( dwValue );

			}
			else
			{
                success = false;
				CREATE_MESSAGE( "Invalid user component." );
				SEND_MESSAGE;
			}

			csLogText += csAppend;

			_LOG_GAMEOP
                LOG_SYSOP, 
                (char *)(LPCTSTR)csLogText 
            LOG_
			
			target->Unlock();

		}
		else
		{
			CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
		boFound = TRUE;
	}

	// Queries a player's component
	COMMAND( "VIEW $'S $", GOD_CAN_VIEW_USER )
	{
		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
		{
			CString csMessage;
			BOOL boOK = TRUE;
			
			target->Lock();

			TRACE( "\r\nParameters: %s, %s", PARAM( 0 ), PARAM( 1 ) );

			FIRST_COMPONENT( "STR", GOD_CAN_VIEW_USER_STAT, 1 )
				csMessage.Format( "%s's strength = (%u) %u", PARAM( 0 ), target->self->GetSTR(), target->self->GetTrueSTR() );

			COMPONENT( "END", GOD_CAN_VIEW_USER_STAT, 1 )
				csMessage.Format( "%s's endurance = (%u) %u", PARAM( 0 ), target->self->GetEND(), target->self->GetTrueEND() );

			COMPONENT(  "AGI", GOD_CAN_VIEW_USER_STAT, 1 )
				csMessage.Format( "%s's agility = (%u) %u", PARAM( 0 ), target->self->GetAGI(), target->self->GetTrueAGI() );

			COMPONENT( "INT", GOD_CAN_VIEW_USER_STAT, 1 )
				csMessage.Format( "%s's intelligence = (%u) %u", PARAM( 0 ), target->self->GetINT(), target->self->GetTrueINT() );

			COMPONENT( "WIL", GOD_CAN_VIEW_USER_STAT, 1 )
				csMessage.Format( "Willpower is no longer implemented." );

			COMPONENT( "WIS", GOD_CAN_VIEW_USER_STAT, 1 )
				csMessage.Format( "%s's wisdom = (%u) %u", PARAM( 0 ), target->self->GetWIS(), target->self->GetTrueWIS() );

			COMPONENT( "LUCK", GOD_CAN_VIEW_USER_STAT, 1 )
				csMessage.Format( "%s's luck = (%u) %u.", PARAM( 0 ), target->self->GetLCK(), target->self->GetTrueLCK() );

			COMPONENT( "DODGE", GOD_CAN_VIEW_USER_STAT, 1 )
				csMessage.Format( "%s's dodge skill = (%u) %u", PARAM( 0 ), target->self->GetDODGE(), target->self->GetTrueDODGE() );

			COMPONENT( "ATTACK", GOD_CAN_VIEW_USER_STAT, 1 )
				csMessage.Format( "%s's attack skill = (%u) %u", PARAM( 0 ), target->self->GetATTACK(), target->self->GetTrueATTACK() );

            COMPONENT( "KARMA", GOD_CAN_VIEW_USER_STAT, 1 )
                csMessage.Format( "%s's karma = %d", PARAM( 0 ), target->self->GetKarma() );
            COMPONENT( "CRIME", GOD_CAN_VIEW_USER_STAT, 1 )
                csMessage.Format( "%s's crime = %d", PARAM( 0 ), target->self->GetCrime() );
            COMPONENT( "HONOR", GOD_CAN_VIEW_USER_STAT, 1 )
                csMessage.Format( "%s's honor = %d", PARAM( 0 ), target->self->GetHonor() );

			COMPONENT( "GOLD", GOD_CAN_VIEW_USER_BACKPACK, 1 )
				csMessage.Format( "%s's gold = %u", PARAM( 0 ), target->self->GetGold() );

			COMPONENT( "XP", GOD_CAN_VIEW_USER_STAT, 1 )
				csMessage.Format( "%s's xp = %I64u", PARAM(0 ), target->self->GetXP());

			COMPONENT( "HP", GOD_CAN_VIEW_USER_STAT, 1 )
				csMessage.Format( "%s's hp = %u/%u(%u)", PARAM( 0 ), target->self->GetHP(), target->self->GetMaxHP(), target->self->GetTrueMaxHP() );

			COMPONENT( "MANA", GOD_CAN_VIEW_USER_STAT, 1 )
				csMessage.Format( "%s's mana = %u/%u(%u)", PARAM( 0 ), target->self->GetMana(), target->self->GetMaxMana(), target->self->GetTrueMaxMana() );
			COMPONENT( "LEVEL", GOD_CAN_VIEW_USER_STAT, 1 )
				csMessage.Format( "%s's level = %u", PARAM( 0 ), target->self->GetLevel() );

			COMPONENT( "APPEARANCE", GOD_CAN_VIEW_USER_APPEARANCE_CORPSE, 1 )
				csMessage.Format( "%s's appearance = %u", PARAM( 0 ), target->self->GetAppearance() );

            COMPONENT( "FIRE RESIST", GOD_CAN_VIEW_USER_STAT, 1 )                
                csMessage.Format( "%s's fire resist = %u", PARAM( 0 ), target->self->GetFireResist() );

            COMPONENT( "WATER RESIST", GOD_CAN_VIEW_USER_STAT, 1 )
                csMessage.Format( "%s's water resist = %u", PARAM( 0 ), target->self->GetWaterResist() );            

            COMPONENT( "EARTH RESIST", GOD_CAN_VIEW_USER_STAT, 1 )            
                csMessage.Format( "%s's earth resist = %u", PARAM( 0 ), target->self->GetEarthResist() );

            COMPONENT( "AIR RESIST", GOD_CAN_VIEW_USER_STAT, 1 )
                csMessage.Format( "%s's air resist = %u", PARAM( 0 ), target->self->GetAirResist() );

            COMPONENT( "LIGHT RESIST", GOD_CAN_VIEW_USER_STAT, 1 )
                csMessage.Format( "%s's light resist = %u", PARAM( 0 ), target->self->GetLightResist() );

            COMPONENT( "DARK RESIST", GOD_CAN_VIEW_USER_STAT, 1 )
                csMessage.Format( "%s's dark resist = %u", PARAM( 0 ), target->self->GetDarkResist() );

            COMPONENT( "FIRE", GOD_CAN_VIEW_USER_STAT, 1 )
                csMessage.Format( "%s's fire = %u", PARAM( 0 ), target->self->GetFirePower() );

            COMPONENT( "WATER", GOD_CAN_VIEW_USER_STAT, 1 )
                csMessage.Format( "%s's water = %u", PARAM( 0 ), target->self->GetWaterPower() );                

            COMPONENT( "EARTH", GOD_CAN_VIEW_USER_STAT, 1 )
                csMessage.Format( "%s's earth = %u", PARAM( 0 ), target->self->GetEarthPower() );

            COMPONENT( "AIR", GOD_CAN_VIEW_USER_STAT, 1 )
                csMessage.Format( "%s's air = %u", PARAM( 0 ), target->self->GetAirPower() );

            COMPONENT( "LIGHT", GOD_CAN_VIEW_USER_STAT, 1 )
                csMessage.Format( "%s's light = %u", PARAM( 0 ), target->self->GetLightPower() );

            COMPONENT( "DARK", GOD_CAN_VIEW_USER_STAT, 1 )
                csMessage.Format( "%s's dark = %u", PARAM( 0 ), target->self->GetDarkPower() );

            COMPONENT( "CORPSE", GOD_CAN_VIEW_USER_APPEARANCE_CORPSE, 1 )
				csMessage.Format( "%s's corpse = %u", PARAM( 0 ), target->self->GetCorpse() );            

            COMPONENT( "AC", GOD_CAN_VIEW_USER_STAT, 1 )
                csMessage.Format( "%s's AC = %f", PARAM(0), target->self->GetAC() );

            COMPONENT( "SKILLPOINTS", GOD_CAN_VIEW_USER_STAT, 1 )
                csMessage.Format( "%s's skill points = %u", PARAM(0), target->self->GetSkillPoints() );

            COMPONENT( "STATPOINTS", GOD_CAN_VIEW_USER_STAT, 1 )
                csMessage.Format( "%s's stat points = %u", PARAM(0), target->self->GetStatPoints() );

            COMPONENT( "SANCTION", GOD_CAN_VIEW_USER_STAT, 1 )
               csMessage.Format( "%s has %d minor sanction(s) and %d major sanction(s)", PARAM(0), target->GetSanctionA(),target->GetSanctionB() );

            COMPONENT( "NMSGOLD", GOD_CAN_VIEW_USER_STAT, 1 )
               csMessage.Format( "%s's NMSGold = %u", PARAM(0), target->GetNMSGold() );
            COMPONENT( "RPLEVEL", GOD_CAN_VIEW_USER_STAT, 1 )
               csMessage.Format( "%s's RP Level = %u", PARAM(0), target->self->GetRP_XPLevel() );
            COMPONENT( "RPPOINT", GOD_CAN_VIEW_USER_STAT, 1 )
               csMessage.Format( "%s's RP Level Point = %u", PARAM(0), target->self->GetRP_XP() );
            

            

			}
			else
			{
				boOK = FALSE;
				CREATE_MESSAGE( "Invalid user component." );
				SEND_MESSAGE;
			}

			if( boOK )
			{
				CREATE_MESSAGE( (LPCTSTR)csMessage );
				SEND_MESSAGE;
			}
			
			target->Unlock();

		}
		else
		{
			CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
		boFound = TRUE;
	}
	
	// Ban user from the game.
	COMMAND( "LOCKOUT $", GOD_CAN_LOCKOUT_USER )
   {
      if( user->self->boAuthGM == false )
      {
         _LOG_GAMEOP
            LOG_CRIT_ERRORS,
            "God %s (%s) tried to lockout %s without authentification!",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName(),
            (LPCTSTR)PARAM( 0 )
            LOG_			
      }

      Players *target = FindCharacter( PARAM( 0 ) );

      if( target != NULL && user->self->boAuthGM == true )
      {

         if( SUPER_USER || target->self->ViewFlag(__FLAG_JUST_DO_IT) == 666)//oki
         {
            CREATE_MESSAGE( "Sorry but you cannot lock the super user out!" );
            SEND_MESSAGE;
         }
         else
         {
            success = true;

            target->Lock();
            target->boLockedOut = TRUE;
            target->uiLockedComplete = 0;
            target->dwKickoutTime = 5 SECONDS TDELAY;
            target->Unlock();

            if(bSendEcho)
            {
               CREATE_MESSAGE( "You have been permanently banned from the game.", PARAM( 0 ) );
               sending << (RQ_SIZE)RQ_ServerMessage;
               sending << (short)30;
               sending << (short)3;
               sending << csText;
               sending << (long) CL_RED;
               target->self->SendPlayerMessage( sending );
            }
         }
         _LOG_GAMEOP
            LOG_SYSOP,
            "God %s (%s) locked out account %s from the game.",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName(),
            (LPCTSTR)target->GetFullAccountName()
            LOG_	
      }
   }
      else
      {
         CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
   }
	
   // Ban user from the game for X days.
	COMMAND( "LOCKOUTTMP $'S TIME $", GOD_CAN_LOCKOUT_USER )	
	{
      if( user->self->boAuthGM == false )
      {
         _LOG_GAMEOP
            LOG_CRIT_ERRORS,
            "God %s (%s) tried to lockout %s without authentification!",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName(),
            (LPCTSTR)PARAM( 0 )
            LOG_			
      }

      Players *target  = FindCharacter( PARAM( 0 ) );
      unsigned int     iNbrDay = atoi(PARAM( 1 ));


	  if( target != NULL  && user->self->boAuthGM == true)
      {
            

            if( SUPER_USER || target->self->ViewFlag(__FLAG_JUST_DO_IT) == 666)//oki
            {
                CREATE_MESSAGE( "Sorry but you cannot lock the super user out!" );
                SEND_MESSAGE;
            }
            else if(iNbrDay<1)
            {
               CREATE_MESSAGE( "Sorry but time must be grater than 0 day..." );
               SEND_MESSAGE;
            }
            else
            {
                success = true;
                time_t TimeMsTmp =  time(NULL);

                target->Lock();
                target->boLockedOut = TRUE;
                target->uiLockedComplete = TimeMsTmp+(iNbrDay*86400/*86400*/);
		    	    target->dwKickoutTime = 5 SECONDS TDELAY;
			       target->Unlock();

                if(bSendEcho)
                {
                  CString csText;
                  csText.Format("%s%d%s",_STR( 15032, target->self->GetLang()),iNbrDay,_STR( 15033, target->self->GetLang()));
                  TFCPacket sending;   
	    			   sending << (RQ_SIZE)RQ_ServerMessage;
		    		   sending << (short)30;
			    	   sending << (short)3;
    				   sending << csText;
					   sending << (long) CL_RED;
	    			   target->self->SendPlayerMessage( sending );
                }

    			_LOG_GAMEOP
	    			LOG_SYSOP,
		    		"God %s (%s) locked out account %s from the game.",
			    	(LPCTSTR)user->self->GetTrueName(),
				    (LPCTSTR)user->GetFullAccountName(),
				    (LPCTSTR)target->GetFullAccountName()
			    LOG_	
            }
		}else{
			CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
   }
   
	// Simply kill player normally
	COMMAND( "SLAY $", GOD_CAN_SLAY_USER )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to slay %s without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 0 )
			LOG_			
		}

		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL && user->self->boAuthGM == true )
		{
            success = true;

			// Set 0 hp
			target->Lock();
			target->self->SetHP(0, false);
			ATTACK_STRUCTURE asBlow = {1,1,1,1,1};
			target->self->hit( &asBlow, user->self );
			target->Unlock();

			_LOG_GAMEOP
				LOG_SYSOP,
				"God %s (%s) slayed player %s (%s).",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)target->self->GetTrueName(),
				(LPCTSTR)target->GetFullAccountName()				
			LOG_	

		}
		else
		{
			CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}

	// Devel command
	COMMAND( "QUERY BLOCK AT $,$,$", GOD_DEVELOPPER )
	{
		BOOL boError = FALSE;
		WorldPos wlPos = { atoi( PARAM( 0 ) ), atoi( PARAM( 1 ) ), atoi( PARAM( 2 ) ) };
		WorldMap *wlWorld = TFCMAIN::GetWorld( wlPos.world );

		if( wlWorld != NULL )
		{		
			if( wlWorld->IsValidPosition( wlPos ) )
			{
				CREATE_MESSAGE( "Blocking at ( %u, %u, %u ) is %u.", 
					wlPos.X, 
					wlPos.Y, 
					wlPos.world, 
					wlWorld->QueryAreaType( wlPos ) 
				);
				SEND_MESSAGE;
			}
			else
			{
				boError = TRUE;
			}
		}
		else
		{
			boError = TRUE;
		}

		if( boError )
		{
			CREATE_MESSAGE( "%u, %u, %u isn't a valid world position.",
				wlPos.X, wlPos.Y, wlPos.world );
			SEND_MESSAGE;
		}
	}

    // Query's a player's position.
    COMMAND( "QUERY $'S POS", GOD_CAN_TELEPORT )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to query %s's pos without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 0 )
			LOG_			
		}

		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL && user->self->boAuthGM == true )
		{
            CREATE_MESSAGE(
                "User's position is %u, %u, %u.",
                target->self->GetWL().X,
                target->self->GetWL().Y,
                target->self->GetWL().world
            );
            SEND_MESSAGE;
        }
		else
		{
		    CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
		    SEND_MESSAGE;
        }
	}

    // Queries an item's description.
    COMMAND( "QUERY ITEM $ DESCRIPTION", GOD_CAN_SUMMON_ITEMS )
	{
        // Get the item's ID.
        WORD wID = Unit::GetIDFromName( PARAM(0), U_OBJECT, TRUE );

        if( wID != 0 )
		{
            Objects *obj = new Objects;

            // Try to create the object's instance.
            if( obj->Create( U_OBJECT, wID ) )
			{
                // Get the object's underlying _item structure.
                _item *lpItem = NULL;

                obj->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItem );

                if( lpItem != NULL )
				{
                    
                    bool descOK = false;
                    CString csMessage;
                    
                    if( !lpItem->GmItemLocation.empty() )
					{
                        descOK = true;
                        csMessage.Format( "Item General Location: %s", lpItem->GmItemLocation.c_str() );
                        CREATE_MESSAGE( (LPCTSTR)csMessage );
                        SEND_MESSAGE;
                    }

                    // If no description was sent.
                    if( !descOK )
					{
                        CREATE_MESSAGE( "No description was found for this item." );
                        SEND_MESSAGE;
                    }
                }
				else
				{
                    CREATE_MESSAGE( "Failed to fetch this item's internal structure." );
                    SEND_MESSAGE;
                }
            }
			else
			{
                CREATE_MESSAGE( "Could not instantiate this item ID." );
                SEND_MESSAGE;
            }

            obj->DeleteUnit();
        }
		else
		{
            CREATE_MESSAGE( "This item ID does not exist" );
            SEND_MESSAGE;
        }
	}

    // Sets the gameop's viewing context.
    COMMAND( "SET CONTEXT TO $", GOD_CAN_VIEW_USER )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to set context to %s without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 0 )
			LOG_			
		}

	    Players *target = FindCharacter( PARAM( 0 ) );
        
        if( target != NULL && user->self->boAuthGM == true )
		{
            // Set the gameop's context.
            user->self->SetGuildPermissionTmp(user->self->GetGuildPermission());
            user->self->SetGuildPermission(GUILD_FONDATEUR_RIGHT);

            user->self->SetGameOpContext( target->self );

            user->self->SetGold( user->self->GetGold() );

			success = true;
        }
		else
		{
		    CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
		    SEND_MESSAGE;
        }
	}

    COMMAND( "RESET CONTEXT", GOD_CAN_VIEW_USER )
	{

        user->self->SetGameOpContext( NULL );
        user->self->SetGuildPermission(user->self->GetGuildPermissionTmp());
        user->self->SetGuildPermissionTmp(0);
        
        user->self->SetGold( user->self->GetGold() );

		success = true;
	}
	
      //////////////////////////////////////////////////////////////////////////////////////////
      // Give a formule
      COMMAND( "GIVE FORMULE $ TO $", GOD_CAN_EDIT_USER_SKILLS )	
	  {
         if( user->self->boAuthGM == false )
         {
            _LOG_GAMEOP
               LOG_CRIT_ERRORS,
               "God %s (%s) tried to give a formule to %s without authentification!",
               (LPCTSTR)user->self->GetTrueName(),
               (LPCTSTR)user->GetFullAccountName(),
               (LPCTSTR)PARAM( 1 )
               LOG_			
         }
         
         WORD wFormuleID = atoi( PARAM( 0 ) );
         
         
         if( wFormuleID != 0 )
         {
            Players *target = FindCharacter( PARAM( 1 ) );
            
            if( target != NULL && user->self->boAuthGM == true )
            {
               target->Lock();
               
               CString errMsg;
               if( target->self->LearnProfessionFormule( wFormuleID, 0, false, errMsg ) == NULL )
               {
                  CREATE_MESSAGE( "Could not add this formule on user. %s", (LPCTSTR)errMsg );
                  SEND_MESSAGE;
               }
               else
               {
                  success = true;
                  
                  _LOG_GAMEOP
                     LOG_SYSOP,
                     "God %s (%s) gave formule ID %u to %s (%s).",
                     (LPCTSTR)user->self->GetTrueName(),
                     (LPCTSTR)user->GetFullAccountName(),
                     wFormuleID,
                     (LPCTSTR)target->self->GetTrueName(),
                     (LPCTSTR)target->GetFullAccountName()				
                     LOG_	
               }
               
               target->Unlock();
               
            }
            else
            {
               CREATE_MESSAGE( "User %s is not online.", PARAM( 1 ) );
               SEND_MESSAGE;
            }
         }
         else
         {
            CREATE_MESSAGE( "Formule ID isn't valid" );
            SEND_MESSAGE;
         }
   	}

      //////////////////////////////////////////////////////////////////////////////////////////
      // Give a formule
      COMMAND( "REMOVE FORMULE $ FROM $", GOD_CAN_EDIT_USER_SKILLS )	
      {
         if( user->self->boAuthGM == false )
         {
            _LOG_GAMEOP
               LOG_CRIT_ERRORS,
               "God %s (%s) tried to give a formule to %s without authentification!",
               (LPCTSTR)user->self->GetTrueName(),
               (LPCTSTR)user->GetFullAccountName(),
               (LPCTSTR)PARAM( 1 )
               LOG_			
         }

         WORD wFormuleID = atoi( PARAM( 0 ) );


         if( wFormuleID != 0 )
         {
            Players *target = FindCharacter( PARAM( 1 ) );

            if( target != NULL && user->self->boAuthGM == true )
            {
               target->Lock();

               CString errMsg;
               if(!target->self->UnLearnProfessionFormule( wFormuleID,errMsg ))
               {
                  CREATE_MESSAGE( "Could not remove this formule from user. %s", (LPCTSTR)errMsg );
                  SEND_MESSAGE;
               }
               else
               {
                  success = true;

                  _LOG_GAMEOP
                     LOG_SYSOP,
                     "God %s (%s) remove formule ID %u from %s (%s).",
                     (LPCTSTR)user->self->GetTrueName(),
                     (LPCTSTR)user->GetFullAccountName(),
                     wFormuleID,
                     (LPCTSTR)target->self->GetTrueName(),
                     (LPCTSTR)target->GetFullAccountName()				
                     LOG_	
               }

               target->Unlock();

            }
            else
            {
               CREATE_MESSAGE( "User %s is not online.", PARAM( 1 ) );
               SEND_MESSAGE;
            }
         }
         else
         {
            CREATE_MESSAGE( "Formule ID isn't valid" );
            SEND_MESSAGE;
         }
      }
         //////////////////////////////////////////////////////////////////////////////////////////
         // Give a spell to self
    COMMAND( "GET FORMULE LIST", GOD_CAN_EDIT_USER_SKILLS )	
	{
            if( user->self->boAuthGM == false )
            {
               _LOG_GAMEOP
                  LOG_CRIT_ERRORS,
                  "God %s (%s) tried to get formule list to himself without authentification!",
                  (LPCTSTR)user->self->GetTrueName(),
                  (LPCTSTR)user->GetFullAccountName()				
                  LOG_			
            }
            

            user->self->Lock();

            for(int i=0;i<Professions::GetNbrFormule();i++)
            {
               USHORT ushIndex = Professions::GetFormuleIDbyIndex(i);
               CREATE_MESSAGE( "%05d --> %s",ushIndex, Professions::GetFormuleName(ushIndex) );
			      SEND_MESSAGE;
            }
            
            user->self->Unlock();
	}


	//////////////////////////////////////////////////////////////////////////////////////////
	// Give a skill or spell
	COMMAND( "GIVE SKILL $ TO $", GOD_CAN_EDIT_USER_SKILLS )	
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to give a skill to %s without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 1 )
			LOG_			
		}

		WORD wSpellID = atoi( PARAM( 0 ) );

		// If the spellID is 0, then perhaps a spell name was provided.
        if( wSpellID == 0 )
		{
            // Retreive the spell by its name.
            LPSKILL lpSkill = Skills::GetSkillByName( PARAM( 0 ), user->self->GetLang() );
            if( lpSkill != NULL )
			{
                wSpellID = lpSkill->nSkillID;
            }
        }	
    
		if( wSpellID != 0 )
		{
			Players *target = FindCharacter( PARAM( 1 ) );

			if( target != NULL && user->self->boAuthGM == true )
			{
				// Set 0 hp
				target->Lock();
				
                CString errMsg;
				if( target->self->LearnSkill( wSpellID, 0, false, errMsg ) == NULL )
				{
					CREATE_MESSAGE( "Could not create skill on user. %s", (LPCTSTR)errMsg );
					SEND_MESSAGE;
                }
				else
				{
                    success = true;

			        _LOG_GAMEOP
				        LOG_SYSOP,
				        "God %s (%s) gave skill ID %u to %s (%s).",
				        (LPCTSTR)user->self->GetTrueName(),
				        (LPCTSTR)user->GetFullAccountName(),
                        wSpellID,
				        (LPCTSTR)target->self->GetTrueName(),
				        (LPCTSTR)target->GetFullAccountName()				
			        LOG_	
                }

				target->Unlock();

			}
			else
			{
				CREATE_MESSAGE( "User %s is not online.", PARAM( 1 ) );
				SEND_MESSAGE;
			}
		}
		else
		{
			CREATE_MESSAGE( "Skill/Spell ID isn't valid" );
			SEND_MESSAGE;
		}
	}

    // Give a spell to an user
	COMMAND( "GIVE SPELL $ TO $", GOD_CAN_EDIT_USER_SPELLS )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to give a spell on %s without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 1 )
			LOG_			
		}

		WORD wSpellID = atoi( PARAM( 0 ) );
				
		// If the spellID is 0, then perhaps a spell name was provided.
        if( wSpellID == 0 )
		{
            // Retreive the spell by its name.
            LPSPELL_STRUCT lpSpell = SpellMessageHandler::GetSpellByName( PARAM( 0 ), user->self->GetLang() );
            if( lpSpell != NULL )
			{
                wSpellID = lpSpell->wSpellID;
            }
        }
        if( wSpellID != 0 )
		{
			Players *target = FindCharacter( PARAM( 1 ) );

			if( target != NULL && user->self->boAuthGM == true )
			{

				// Set 0 hp
				target->Lock();
				
                CString errMsg;
				if( target->self->LearnSkill( wSpellID, 0, false, errMsg ) == NULL )
				{
					CREATE_MESSAGE( "Could not create spell on user. %s", (LPCTSTR)errMsg );
					SEND_MESSAGE;
                }
				else
				{
                    success = true;

			        _LOG_GAMEOP
				        LOG_SYSOP,
				        "God %s (%s) gave spell ID %u to %s (%s).",
				        (LPCTSTR)user->self->GetTrueName(),
				        (LPCTSTR)user->GetFullAccountName(),
                        wSpellID,
				        (LPCTSTR)target->self->GetTrueName(),
				        (LPCTSTR)target->GetFullAccountName()				
			        LOG_
                }

				target->Unlock();

			}
			else
			{
				CREATE_MESSAGE( "User %s is not online.", PARAM( 1 ) );
				SEND_MESSAGE;
			}
		}
		else
		{
			CREATE_MESSAGE( "Spell ID isn't valid" );
			SEND_MESSAGE;
		}
	}

    // Give a spell to self
	COMMAND( "GIVE SPELL $", GOD_CAN_EDIT_USER_SPELLS )	
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to give a spell to himself without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName()				
			LOG_			
		}

		WORD wSpellID = atoi( PARAM( 0 ) );
				
		// If the spellID is 0, then perhaps a spell name was provided.
        if( wSpellID == 0 )
		{
            // Retreive the spell by its name.
            LPSPELL_STRUCT lpSpell = SpellMessageHandler::GetSpellByName( PARAM( 0 ), user->self->GetLang() );
            if( lpSpell != NULL ){
                wSpellID = lpSpell->wSpellID;
            }
        }
        if( wSpellID != 0 && user->self->boAuthGM == true )
		{
			user->self->Lock();

			CString errMsg;
			if( user->self->LearnSkill( wSpellID, 0, false, errMsg ) == NULL )
			{
				CREATE_MESSAGE( "Could not create spell on user. %s", (LPCTSTR)errMsg );
				SEND_MESSAGE;
			}
			else
			{
				success = true;

			    _LOG_GAMEOP
				    LOG_SYSOP,
				    "God %s (%s) gave spell ID %u to himself.",
				    (LPCTSTR)user->self->GetTrueName(),
				    (LPCTSTR)user->GetFullAccountName(),
                    wSpellID				    				
			    LOG_
            }

			user->self->Unlock();			
		}
		else
		{
			CREATE_MESSAGE( "Spell ID isn't valid" );
			SEND_MESSAGE;
		}
	}

	// Remove spell from a user
    COMMAND( "REMOVE SPELL $ FROM $", GOD_CAN_EDIT_USER_SPELLS )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to remove spell from %s without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 1 )
			LOG_			
		}

		WORD wSpellID = atoi( PARAM( 0 ) );

        // If the spellID is 0, then perhaps a spell name was provided.
        if( wSpellID == 0 )
		{
            // Retreive the spell by its name.
            LPSPELL_STRUCT lpSpell = SpellMessageHandler::GetSpellByName( PARAM( 0 ), user->self->GetLang() );
            if( lpSpell != NULL )
			{
                wSpellID = lpSpell->wSpellID;
            }
        }
				
		if( wSpellID != 0 && user->self->boAuthGM == true )
		{
			Players *target = FindCharacter( PARAM( 1 ) );

			if( target != NULL ){				
				target->Lock();
				
                // Fetch the user's spells
                TemplateList< USER_SKILL > *lpSpells = target->self->GetSpells();

                lpSpells->Lock();

                bool found = false;
                // Try to find the user's spell in the list of spells.
                lpSpells->ToHead();
                while( lpSpells->QueryNext() )
				{
                    if( lpSpells->Object()->GetSkillID() == wSpellID ){
                        // Remove spell
                        found = true;
                        lpSpells->DeleteAbsolute();                        
                    }
                }
                lpSpells->Unlock();

				target->Unlock();

                if( !found )
				{
                    CREATE_MESSAGE( "Spell was not found on user." );
                    SEND_MESSAGE;
                }
				else
				{
                    success = true;
                }
			}
			else
			{
				CREATE_MESSAGE( "User %s is not online.", PARAM( 1 ) );
				SEND_MESSAGE;
			}
		}
		else
		{
			CREATE_MESSAGE( "Spell ID isn't valid" );
			SEND_MESSAGE;
		}
	}

	// Removes the lighting effect from a torch
    COMMAND( "REMOVE LIGHT FROM $", GOD_DEVELOPPER )
	{
        Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
		{				
		    target->Lock();

            target->self->RemoveBoostFromStat( STAT_RADIANCE );

            target->Unlock();
        }
	}

    //////////////////////////////////////////////////////////////////////////////////////////    
    // Allows in game shutdown of server which will save their characters and warn them.
	COMMAND( "SHUTDOWN $", GOD_CAN_SHUTDOWN )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to shutdown server without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName()				
			LOG_	
			
			return false;
		}		
    
        // Shutdown now stops right now :P
        if( _stricmp( PARAM( 0 ), "NOW" ) == 0 || _stricmp( PARAM( 0 ), "FAST" ) == 0 )
		{

			bool waitBeforeRelaunch = true;
			if (_stricmp( PARAM( 0 ), "FAST" ) == 0 ) 
			{
	            waitBeforeRelaunch = false;
			} 
            CShutdown::CreateShutdown( SHUTDOWN_NOW, true, /*waitBeforeRelaunch,*/ true );
            CShutdown::StartShutdown();

            _LOG_GAMEOP
			    LOG_SYSOP,
			    "God %s (%s) issued a shutdown ( NOW %s).",
			    (LPCTSTR)user->self->GetTrueName(),
			    (LPCTSTR)user->GetFullAccountName(),
				waitBeforeRelaunch == false ? "FAST " : ""
		    LOG_	
        }
		else 
        // If we should cancel a current shutdown    
        if( _stricmp( PARAM( 0 ), "CANCEL" ) == 0 )
		{

            CShutdown::CancelShutdown( true );

			_LOG_GAMEOP
				LOG_SYSOP,
				"God %s (%s) cancelled shutdown.",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName()
			LOG_	
        }
        // Otherwise check if we need to shutdown in some time now.
        else
		{
            int nTime = atoi( PARAM( 0 ));

            // If time value is invalid
            if( nTime < 1 || nTime > 30 )
			{
				CREATE_MESSAGE( "You must supply a valid time value ranging from 1 to 30 minutes or NOW." );
				SEND_MESSAGE;
            }
			else
			{
        	    _LOG_GAMEOP
        		    LOG_SYSOP,
				    "God %s (%s) issued a server shutdown( %u ).",
				    (LPCTSTR)user->self->GetTrueName(),
				    (LPCTSTR)user->GetFullAccountName(),
                    nTime
			    LOG_	

                if( CShutdown::CreateShutdown( nTime, true, /*true,*/ true ) )
				{
                    CShutdown::StartShutdown();
                }
				else
				{
                    CREATE_MESSAGE( "Unable to reset shutdown timer. This shutdown cannot be overriden" );
                    SEND_MESSAGE;
                }
            }
        }
	}

    // Allow a super-god to give god flags.
    COMMAND( "GIVE GAMEOP FLAG $ TO $", GOD_CAN_GIVE_GOD_FLAGS )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to give god flag to %s without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 1 )
			LOG_				
		}

		Players *target = FindCharacter( PARAM( 1 ) );

		if( target != NULL && user->self->boAuthGM == true )
		{
            BOOL boGiven = FALSE;
            CString csMessage;            

			target->Lock();

            FIRST_COMPONENT( "GOD_NO_CLIP", GOD_CAN_GIVE_GOD_FLAGS, 0 )
                GIVE_GOD_FLAG( GOD_NO_CLIP, "God now walks through walls." );                
            
            COMPONENT( "GOD_NO_MONSTERS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_NO_MONSTERS, "God doesn't spawn monsters anymore." )
            
            COMPONENT( "GOD_CAN_TELEPORT", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_TELEPORT, "God can now teleport." );

            COMPONENT( "GOD_CAN_TELEPORT_USER", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_TELEPORT_USER, "God can now teleport other users." );

            COMPONENT( "GOD_CAN_ZAP", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_ZAP, "God can now kickout users." );

            COMPONENT( "GOD_CAN_SQUELCH", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_SQUELCH, "God can now remove a user's ability to talk." );

            COMPONENT( "GOD_CAN_REMOVE_SHOUTS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_REMOVE_SHOUTS, "God can now remove a user's ability to shout." );

            COMPONENT( "GOD_CAN_SUMMON_MONSTERS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_SUMMON_MONSTERS, "God can now summon monsters." );
                
            COMPONENT( "GOD_CAN_SUMMON_ITEMS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_SUMMON_ITEMS, "God can now summon items." );

            COMPONENT( "GOD_CAN_SET_USER_FLAG", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_SET_USER_FLAG, "God can now set a player's flag." );

            COMPONENT( "GOD_CAN_EDIT_USER_STAT", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_STAT, "God can now edit stats." );

            COMPONENT( "GOD_CAN_EDIT_USER_HP", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_HP, "God can now edit hit points." );

            COMPONENT( "GOD_CAN_EDIT_USER_MANA", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_MANA_FAITH, "God can now edit mana." );

            COMPONENT( "GOD_CAN_EDIT_USER_XP_LEVEL", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_XP_LEVEL, "God can now edit experience or level." );

            COMPONENT( "GOD_CAN_EDIT_USER_NAME", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_NAME, "God can now edit a user's name." );

            COMPONENT( "GOD_CAN_EDIT_USER_APPEARANCE_CORPSE", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_APPEARANCE_CORPSE, "God can now edit appearances." );

            COMPONENT( "GOD_CAN_EDIT_USER_SPELLS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_SPELLS, "God can now edit spells." );

            COMPONENT( "GOD_CAN_EDIT_USER_SKILLS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_SKILLS, "God can now edit skills." );

            COMPONENT( "GOD_CAN_EDIT_USER_BACKPACK", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_BACKPACK, "God can now edit backpacks." );

            COMPONENT( "GOD_CAN_VIEW_USER_STAT", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_VIEWFLAG( GOD_CAN_VIEW_USER_STAT, "God can now view stats." );

            COMPONENT( "GOD_CAN_VIEW_USER_BACKPACK", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_VIEWFLAG( GOD_CAN_VIEW_USER_BACKPACK, "God can now view backpack." );

            COMPONENT( "GOD_CAN_VIEW_USER_SPELLS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_VIEWFLAG( GOD_CAN_VIEW_USER_SPELLS, "God can now view spells." );

            COMPONENT( "GOD_CAN_VIEW_USER_SKILLS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_VIEWFLAG( GOD_CAN_VIEW_USER_SKILLS, "God can now view skills." );

            COMPONENT( "GOD_CAN_VIEW_USER_APPEARANCE_CORPSE", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_VIEWFLAG( GOD_CAN_VIEW_USER_APPEARANCE_CORPSE, "God can now view appearance IDs." );

            COMPONENT( "GOD_CAN_LOCKOUT_USER", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_LOCKOUT_USER, "God can lockout other users." );

            COMPONENT( "GOD_CAN_SLAY_USER", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_SLAY_USER, "God can now slay other players." );

                    TFCPacket sending;
                    sending << (RQ_SIZE)RQ_GodFlagUpdate;
                    sending << (char)UPDATE_GOD_CAN_SLAY_USER;
                    sending << (char)1;
                    
                    target->self->SendPlayerMessage( sending );

            COMPONENT( "GOD_CAN_COPY_USER", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_COPY_USER, "God can copy a player's stats." );

            COMPONENT( "GOD_CAN_SET_WEATHER", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_SET_WEATHER, "God can activate weather effect." );
               

            COMPONENT( "GOD_CAN_EMULATE_MONSTER", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_EMULATE_MONSTER, "God can emulate a monster's stats and appearance." );

            COMPONENT( "GOD_INVINCIBLE", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_INVINCIBLE, "God is now immune to damage and death." );

            COMPONENT( "GOD_CANNOT_DIE", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CANNOT_DIE, "God is now immune to death (but not damage)." );

            COMPONENT( "GOD_CAN_RUN_CLIENT_SCRIPTS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_RUN_CLIENT_SCRIPTS, "God can now use the !RUN command." );

                TFCPacket sending;
                sending << (RQ_SIZE)RQ_GodFlagUpdate;
                sending << (char)UPDATE_GOD_CAN_RUN_CLIENT_SCRIPTS;
                sending << (char)1;
                
                target->self->SendPlayerMessage( sending );
                
            COMPONENT( "GOD_DEVELOPPER", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_DEVELOPPER, "God now has developper capabilities." );

            COMPONENT( "GOD_CAN_SHUTDOWN", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_SHUTDOWN, "God can now shutdown the T4C Server." );

            COMPONENT( "GOD_CAN_SEE_ACCOUNTS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_SEE_ACCOUNTS, "God can now see true accounts in the player listing." );

            COMPONENT( "GOD_UNLIMITED_SHOUTS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_UNLIMITED_SHOUTS, "God now has unlimited shouts." );
            
            COMPONENT( "GOD_TRUE_INVISIBILITY", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_TRUE_INVISIBILITY, "God now invisible to all players." );
            COMPONENT( "GOD_CAN_GIVE_GOD_FLAGS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_GIVE_GOD_FLAGS, "God can now give any gameop flag to other players." );
            COMPONENT( "GOD_CAN_EMULATE_SYSTEM", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_EMULATE_SYSTEM, "God can now emulate system functions." );
            COMPONENT( "GOD_CHAT_MASTER", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CHAT_MASTER, "God can now have access to all chatter channels." );
            COMPONENT( "GOD_BOOST_XP", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_BOOST_XP, "God will now receive a lot more XP from kills and attacks." );
            COMPONENT( "GOD_CAN_GIVE_FLAG_TO_HIM", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_GIVE_FLAG_TO_HIM, "God now can give himself flag" );
            COMPONENT( "GOD_SEE_ALL", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_SEE_ALL, "God now see all hidden and invisible players." );
            COMPONENT( "GOD_CAN_CHANGE_SETTINGS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                GIVE_GOD_FLAG( GOD_CAN_CHANGE_SETTINGS, "God now can change server settings." );
                
                
                
            }
            

			target->Unlock();

            if( boGiven )
			{
    			_LOG_GAMEOP
	    			LOG_SYSOP,
		    		"God %s (%s) gave gameop flag %s to %s (%s).",
			    	(LPCTSTR)user->self->GetTrueName(),
				    (LPCTSTR)user->GetFullAccountName(),
                    PARAM( 0 ),
    				(LPCTSTR)target->self->GetTrueName(),
	    			(LPCTSTR)target->GetFullAccountName()		    		
			    LOG_

                CREATE_MESSAGE( csMessage );
                SEND_MESSAGE;
            }
			else
			{
    			CREATE_MESSAGE( "%s is not a valid gameop flag.", PARAM( 0 ) );
	    		SEND_MESSAGE;
            }
		}
		else
		{
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 1 ) );
			SEND_MESSAGE;
		}
	}

    // Allow a super-god to remove god flags.
    COMMAND( "REMOVE GAMEOP FLAG $ FROM $", GOD_CAN_GIVE_GOD_FLAGS )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to remove gameop flag from %s without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 1 )
			LOG_				
		}

		Players *target = FindCharacter( PARAM( 1 ) );

		if( target != NULL && user->self->boAuthGM == true && target->self != NULL && target->self->ViewFlag(__FLAG_JUST_DO_IT) != 666)//oki
		{
            BOOL boGiven = FALSE;
            CString csMessage;            

			target->Lock();

            if( _stricmp( 
                    (LPCTSTR)target->GetFullAccountName(), 
                    CAutoConfig::GetStringValue( theApp.csT4CKEY+GEN_CFG_KEY, "SuperUser", HKEY_LOCAL_MACHINE ).c_str()
                ) != 0 || _stricmp( 
                    (LPCTSTR)user->GetFullAccountName(), 
                    CAutoConfig::GetStringValue( theApp.csT4CKEY+GEN_CFG_KEY, "SuperUser", HKEY_LOCAL_MACHINE ).c_str()
                ) == 0)
			{
                FIRST_COMPONENT( "GOD_NO_CLIP", GOD_CAN_GIVE_GOD_FLAGS, 0 )
                    REMOVE_GOD_FLAG( GOD_NO_CLIP, "God no long walks through walls." );                
            
                COMPONENT( "GOD_NO_MONSTERS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_NO_MONSTERS, "God now spawns monsters." )
            
                COMPONENT( "GOD_CAN_TELEPORT", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_TELEPORT, "God can no longer teleport." );

                COMPONENT( "GOD_CAN_TELEPORT_USER", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_TELEPORT_USER, "God can no longer teleport other users." );

                COMPONENT( "GOD_CAN_ZAP", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_ZAP, "God can no longer kickout users." );

                COMPONENT( "GOD_CAN_SQUELCH", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_SQUELCH, "God can no longer remove a user's ability to talk." );

                COMPONENT( "GOD_CAN_REMOVE_SHOUTS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_REMOVE_SHOUTS, "God can no longer remove a user's ability to shout." );

                COMPONENT( "GOD_CAN_SUMMON_MONSTERS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_SUMMON_MONSTERS, "God can no longer summon monsters." );
                
                COMPONENT( "GOD_CAN_SUMMON_ITEMS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_SUMMON_ITEMS, "God can no longer summon items." );

                COMPONENT( "GOD_CAN_SET_USER_FLAG", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_SET_USER_FLAG, "God can no longer set a player's flag." );

                COMPONENT( "GOD_CAN_EDIT_USER_STAT", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_STAT, "God can no longer edit stats." );

                COMPONENT( "GOD_CAN_EDIT_USER_HP", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_HP, "God can no longer edit hit points." );

                COMPONENT( "GOD_CAN_EDIT_USER_MANA", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_MANA_FAITH, "God can no longer edit mana." );

                COMPONENT( "GOD_CAN_EDIT_USER_XP_LEVEL", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_XP_LEVEL, "God can no longer edit experience or level." );

                COMPONENT( "GOD_CAN_EDIT_USER_NAME", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_NAME, "God can no longer edit a user's name." );

                COMPONENT( "GOD_CAN_EDIT_USER_APPEARANCE_CORPSE", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_APPEARANCE_CORPSE, "God can no longer edit appearances." );

                COMPONENT( "GOD_CAN_EDIT_USER_SPELLS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_SPELLS, "God can no longer edit spells." );

                COMPONENT( "GOD_CAN_EDIT_USER_SKILLS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_SKILLS, "God can no longer edit skills." );

                COMPONENT( "GOD_CAN_EDIT_USER_BACKPACK", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_BACKPACK, "God can no longer edit backpacks." );

                COMPONENT( "GOD_CAN_VIEW_USER_STAT", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_VIEW_USER_STAT, "God can no longer view stats." );

                COMPONENT( "GOD_CAN_VIEW_USER_BACKPACK", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_VIEW_USER_BACKPACK, "God can no longer view backpack." );

                COMPONENT( "GOD_CAN_VIEW_USER_SPELLS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_VIEW_USER_SPELLS, "God can no longer view spells." );

                COMPONENT( "GOD_CAN_VIEW_USER_SKILLS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_VIEW_USER_SKILLS, "God can no longer view skills." );

                COMPONENT( "GOD_CAN_VIEW_USER_APPEARANCE_CORPSE", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_VIEW_USER_APPEARANCE_CORPSE, "God can no longer view appearance IDs." );

                COMPONENT( "GOD_CAN_LOCKOUT_USER", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_LOCKOUT_USER, "God can no longer lockout other users." );

                COMPONENT( "GOD_CAN_SLAY_USER", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_SLAY_USER, "God can no longer slay other players." );

                    TFCPacket sending;
                    sending << (RQ_SIZE)RQ_GodFlagUpdate;
                    sending << (char)UPDATE_GOD_CAN_SLAY_USER;
                    sending << (char)0;
                    
                    target->self->SendPlayerMessage( sending );

                COMPONENT( "GOD_CAN_COPY_USER", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_COPY_USER, "God can no longer copy a player's stats." );

                COMPONENT( "GOD_CAN_SET_WEATHER", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_SET_WEATHER, "God can no longer activate weather effect." );

                COMPONENT( "GOD_CAN_EMULATE_MONSTER", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EMULATE_MONSTER, "God can no longer emulate a monster's stats and appearance." );

                COMPONENT( "GOD_INVINCIBLE", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_INVINCIBLE, "God is now vulnerable to damage." );
                
                COMPONENT( "GOD_CANNOT_DIE", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CANNOT_DIE, "God is now vulnerable to death." );
                
                COMPONENT( "GOD_CAN_RUN_CLIENT_SCRIPTS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_RUN_CLIENT_SCRIPTS, "God can no longer use the !RUN command." );

                    TFCPacket sending;
                    sending << (RQ_SIZE)RQ_GodFlagUpdate;
                    sending << (char)UPDATE_GOD_CAN_RUN_CLIENT_SCRIPTS;
                    sending << (char)0;
                    
                    target->self->SendPlayerMessage( sending );

                COMPONENT( "GOD_DEVELOPPER", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_DEVELOPPER, "God no longer has developper capabilities." );

                COMPONENT( "GOD_CAN_SHUTDOWN", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_SHUTDOWN, "God can no longer shutdown the T4C Server." );

                COMPONENT( "GOD_CAN_SEE_ACCOUNTS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_SEE_ACCOUNTS, "God can no longer see true accounts in the player listing." );

                COMPONENT( "GOD_UNLIMITED_SHOUTS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_UNLIMITED_SHOUTS, "God now complies to default shouting policies." );

                COMPONENT( "GOD_TRUE_INVISIBILITY", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_TRUE_INVISIBILITY, "God now has normal visibility." );

                COMPONENT( "GOD_CAN_EMULATE_SYSTEM", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EMULATE_SYSTEM, "God can no longer emulate system functions." );                
                
                COMPONENT( "GOD_CHAT_MASTER", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CHAT_MASTER, "God can no longer have access to all chatter channels." );

                COMPONENT( "GOD_BOOST_XP", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_BOOST_XP, "God now gains normal XP from kills and attacks." );

                COMPONENT( "GOD_CAN_GIVE_FLAG_TO_HIM", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_GIVE_FLAG_TO_HIM, "God now CANT give himself flag" );

                COMPONENT( "GOD_SEE_ALL", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_SEE_ALL, "God now DONT see hidden and invisibility player." );
                COMPONENT( "GOD_CAN_CHANGE_SETTINGS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_CHANGE_SETTINGS, "God now CANT change server settings." );
                    
                COMPONENT( "GOD_CAN_GIVE_GOD_FLAGS", GOD_CAN_GIVE_GOD_FLAGS, 0 );
                    if( _stricmp( 
                        (LPCTSTR)target->GetFullAccountName(), 
                        CAutoConfig::GetStringValue( theApp.csT4CKEY+GEN_CFG_KEY, "SuperUser", HKEY_LOCAL_MACHINE ).c_str() 
						) != 0 )
					{                
                        REMOVE_GOD_FLAG( GOD_CAN_GIVE_GOD_FLAGS, "God can no longer give any gameop flag to other players." );
                    }
					else
					{
                        csMessage = "Sorry but you can't remove this flag from this god.";
                        boGiven = TRUE;
                    }			    			
                }
            }
			else
			{
    			CREATE_MESSAGE( "You cannot remove god flags from the super user." );
	    		SEND_MESSAGE;
            }

			target->Unlock();

            if( boGiven )
			{
    			_LOG_GAMEOP
	    			LOG_SYSOP,
		    		"God %s (%s) removed gameop flag %s from %s (%s).",
			    	(LPCTSTR)user->self->GetTrueName(),
				    (LPCTSTR)user->GetFullAccountName(),
                    PARAM( 0 ),
    				(LPCTSTR)target->self->GetTrueName(),
	    			(LPCTSTR)target->GetFullAccountName()		    		
			    LOG_

                CREATE_MESSAGE( csMessage );
                SEND_MESSAGE;
            }
			else
			{
    			CREATE_MESSAGE( "%s is not a valid gameop flag.", PARAM( 0 ) );
	    		SEND_MESSAGE;
            }
		}
		else
		{
			CREATE_MESSAGE( "User %s is not online.", PARAM( 1 ) );
			SEND_MESSAGE;
		}
	}

      COMMAND( "GIVE MODO FLAG TO $", GOD_CAN_GIVE_FLAG_TO_HIM )
	  {
		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
        {
            BOOL boGiven = FALSE;
            CString csMessage;            

			   target->Lock();
            GIVE_GOD_FLAG( GOD_CAN_SQUELCH, "God can now remove a user's ability to talk." );
            GIVE_GOD_FLAG( GOD_CAN_REMOVE_SHOUTS, "God can now remove a user's ability to shout." );
            GIVE_GOD_FLAG( GOD_UNLIMITED_SHOUTS, "God now has unlimited shouts." );
   			target->Unlock();

            if( boGiven )
            {
               _LOG_GAMEOP
                  LOG_SYSOP,
                  "God %s (%s) Set Modo flag to %s (%s).",
                  (LPCTSTR)user->self->GetTrueName(),
                  (LPCTSTR)user->GetFullAccountName(),
                  (LPCTSTR)target->self->GetTrueName(),
                  (LPCTSTR)target->GetFullAccountName()		    		
                  LOG_
                  
                  CREATE_MESSAGE( csMessage );
               SEND_MESSAGE;
            }

		}
        else
        {
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
    }
    
    COMMAND( "REMOVE MODO FLAG FROM $", GOD_CAN_GIVE_FLAG_TO_HIM )
	{
		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
        {
            BOOL boGiven = FALSE;
            CString csMessage;            

			   target->Lock();
            REMOVE_GOD_FLAG( GOD_CAN_SQUELCH, "God can no longer remove a user's ability to talk." );
            REMOVE_GOD_FLAG( GOD_CAN_REMOVE_SHOUTS, "God can no longer remove a user's ability to shout." );
            REMOVE_GOD_FLAG( GOD_UNLIMITED_SHOUTS, "God now complies to default shouting policies." );
   			target->Unlock();

            if( boGiven )
            {
               _LOG_GAMEOP
                  LOG_SYSOP,
                  "God %s (%s) Remove Modo flag from %s (%s).",
                  (LPCTSTR)user->self->GetTrueName(),
                  (LPCTSTR)user->GetFullAccountName(),
                  (LPCTSTR)target->self->GetTrueName(),
                  (LPCTSTR)target->GetFullAccountName()		    		
                  LOG_
                  
                  CREATE_MESSAGE( csMessage );
               SEND_MESSAGE;
            }

		}
        else
        {
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
    }

    COMMAND( "GIVEME GAMEOP FLAG $", GOD_CAN_GIVE_FLAG_TO_HIM )
    {
		Players *target = user;

		if( target != NULL )
        {
            BOOL boGiven = FALSE;
            CString csMessage;            

			   target->Lock();

            FIRST_COMPONENT( "GOD_NO_CLIP", GOD_CAN_GIVE_FLAG_TO_HIM, 0 )
                GIVE_GOD_FLAG( GOD_NO_CLIP, "God now walks through walls." );                
            
            COMPONENT( "GOD_NO_MONSTERS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_NO_MONSTERS, "God doesn't spawn monsters anymore." )
            
            COMPONENT( "GOD_CAN_TELEPORT", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_CAN_TELEPORT, "God can now teleport." );

            COMPONENT( "GOD_CAN_TELEPORT_USER", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_CAN_TELEPORT_USER, "God can now teleport other users." );

            COMPONENT( "GOD_CAN_ZAP", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_CAN_ZAP, "God can now kickout users." );

            COMPONENT( "GOD_CAN_SQUELCH", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_CAN_SQUELCH, "God can now remove a user's ability to talk." );

            COMPONENT( "GOD_CAN_REMOVE_SHOUTS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_CAN_REMOVE_SHOUTS, "God can now remove a user's ability to shout." );

            COMPONENT( "GOD_CAN_SUMMON_MONSTERS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_CAN_SUMMON_MONSTERS, "God can now summon monsters." );
                
            COMPONENT( "GOD_CAN_SUMMON_ITEMS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_CAN_SUMMON_ITEMS, "God can now summon items." );

            COMPONENT( "GOD_CAN_SET_USER_FLAG", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_CAN_SET_USER_FLAG, "God can now set a player's flag." );

            COMPONENT( "GOD_CAN_EDIT_USER_STAT", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_STAT, "God can now edit stats." );

            COMPONENT( "GOD_CAN_EDIT_USER_HP", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_HP, "God can now edit hit points." );

            COMPONENT( "GOD_CAN_EDIT_USER_MANA", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_MANA_FAITH, "God can now edit mana." );

            COMPONENT( "GOD_CAN_EDIT_USER_XP_LEVEL", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_XP_LEVEL, "God can now edit experience or level." );

            COMPONENT( "GOD_CAN_EDIT_USER_NAME", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_NAME, "God can now edit a user's name." );

            COMPONENT( "GOD_CAN_EDIT_USER_APPEARANCE_CORPSE", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_APPEARANCE_CORPSE, "God can now edit appearances." );

            COMPONENT( "GOD_CAN_EDIT_USER_SPELLS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_SPELLS, "God can now edit spells." );

            COMPONENT( "GOD_CAN_EDIT_USER_SKILLS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_SKILLS, "God can now edit skills." );

            COMPONENT( "GOD_CAN_EDIT_USER_BACKPACK", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_BACKPACK, "God can now edit backpacks." );

            COMPONENT( "GOD_CAN_VIEW_USER_STAT", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_VIEWFLAG( GOD_CAN_VIEW_USER_STAT, "God can now view stats." );

            COMPONENT( "GOD_CAN_VIEW_USER_BACKPACK", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_VIEWFLAG( GOD_CAN_VIEW_USER_BACKPACK, "God can now view backpack." );

            COMPONENT( "GOD_CAN_VIEW_USER_SPELLS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_VIEWFLAG( GOD_CAN_VIEW_USER_SPELLS, "God can now view spells." );

            COMPONENT( "GOD_CAN_VIEW_USER_SKILLS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_VIEWFLAG( GOD_CAN_VIEW_USER_SKILLS, "God can now view skills." );

            COMPONENT( "GOD_CAN_VIEW_USER_APPEARANCE_CORPSE", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_VIEWFLAG( GOD_CAN_VIEW_USER_APPEARANCE_CORPSE, "God can now view appearance IDs." );

            COMPONENT( "GOD_CAN_SLAY_USER", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_CAN_SLAY_USER, "God can now slay other players." );

                    TFCPacket sending;
                    sending << (RQ_SIZE)RQ_GodFlagUpdate;
                    sending << (char)UPDATE_GOD_CAN_SLAY_USER;
                    sending << (char)1;
                    
                    target->self->SendPlayerMessage( sending );


            COMPONENT( "GOD_CAN_EMULATE_MONSTER", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_CAN_EMULATE_MONSTER, "God can emulate a monster's stats and appearance." );

            COMPONENT( "GOD_INVINCIBLE", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_INVINCIBLE, "God is now immune to damage and death." );

            COMPONENT( "GOD_CANNOT_DIE", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_CANNOT_DIE, "God is now immune to death (but not damage)." );

            COMPONENT( "GOD_CAN_RUN_CLIENT_SCRIPTS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_CAN_RUN_CLIENT_SCRIPTS, "God can now use the !RUN command." );

                TFCPacket sending;
                sending << (RQ_SIZE)RQ_GodFlagUpdate;
                sending << (char)UPDATE_GOD_CAN_RUN_CLIENT_SCRIPTS;
                sending << (char)1;
                
                target->self->SendPlayerMessage( sending );
                
            COMPONENT( "GOD_DEVELOPPER", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_DEVELOPPER, "God now has developper capabilities." );

            COMPONENT( "GOD_CAN_SEE_ACCOUNTS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_CAN_SEE_ACCOUNTS, "God can now see true accounts in the player listing." );

            COMPONENT( "GOD_UNLIMITED_SHOUTS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_UNLIMITED_SHOUTS, "God now has unlimited shouts." );
            
            COMPONENT( "GOD_TRUE_INVISIBILITY", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                GIVE_GOD_FLAG( GOD_TRUE_INVISIBILITY, "God now invisible to all players." );

            }
            

			target->Unlock();

            if( boGiven )
			{
    			_LOG_GAMEOP
	    			LOG_SYSOP,
		    		"God %s (%s) giveme gameop flag %s to %s (%s).",
			    	(LPCTSTR)user->self->GetTrueName(),
				    (LPCTSTR)user->GetFullAccountName(),
                    PARAM( 0 ),
    				(LPCTSTR)target->self->GetTrueName(),
	    			(LPCTSTR)target->GetFullAccountName()		    		
			    LOG_

                CREATE_MESSAGE( csMessage );
                SEND_MESSAGE;
            }
			else
			{
    			CREATE_MESSAGE( "%s is not a valid gameop flag.", PARAM( 0 ) );
	    		SEND_MESSAGE;
            }
		}
		else
		{
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 1 ) );
			SEND_MESSAGE;
		}
    }


    COMMAND( "REMOVEME GAMEOP FLAG $", GOD_CAN_GIVE_FLAG_TO_HIM )
	{
		Players *target = user;

		if( target != NULL && target->self != NULL && target->self->ViewFlag(__FLAG_JUST_DO_IT)!= 666)//oki
		{
            BOOL boGiven = FALSE;
            CString csMessage;            

			target->Lock();

            if( _stricmp( 
                    (LPCTSTR)target->GetFullAccountName(), 
                    CAutoConfig::GetStringValue( theApp.csT4CKEY+GEN_CFG_KEY, "SuperUser", HKEY_LOCAL_MACHINE ).c_str()
                ) != 0 || _stricmp( 
                    (LPCTSTR)user->GetFullAccountName(), 
                    CAutoConfig::GetStringValue( theApp.csT4CKEY+GEN_CFG_KEY, "SuperUser", HKEY_LOCAL_MACHINE ).c_str()
                ) == 0 
           )
		   {
                FIRST_COMPONENT( "GOD_NO_CLIP", GOD_CAN_GIVE_FLAG_TO_HIM, 0 )
                    REMOVE_GOD_FLAG( GOD_NO_CLIP, "God no long walks through walls." );                
            
                COMPONENT( "GOD_NO_MONSTERS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_NO_MONSTERS, "God now spawns monsters." )
            
                COMPONENT( "GOD_CAN_TELEPORT", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_TELEPORT, "God can no longer teleport." );

                COMPONENT( "GOD_CAN_TELEPORT_USER", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_TELEPORT_USER, "God can no longer teleport other users." );

                COMPONENT( "GOD_CAN_ZAP", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_ZAP, "God can no longer kickout users." );

                COMPONENT( "GOD_CAN_SQUELCH", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_SQUELCH, "God can no longer remove a user's ability to talk." );

                COMPONENT( "GOD_CAN_REMOVE_SHOUTS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_REMOVE_SHOUTS, "God can no longer remove a user's ability to shout." );

                COMPONENT( "GOD_CAN_SUMMON_MONSTERS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_SUMMON_MONSTERS, "God can no longer summon monsters." );
                
                COMPONENT( "GOD_CAN_SUMMON_ITEMS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_SUMMON_ITEMS, "God can no longer summon items." );

                COMPONENT( "GOD_CAN_SET_USER_FLAG", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_SET_USER_FLAG, "God can no longer set a player's flag." );

                COMPONENT( "GOD_CAN_EDIT_USER_STAT", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_STAT, "God can no longer edit stats." );

                COMPONENT( "GOD_CAN_EDIT_USER_HP", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_HP, "God can no longer edit hit points." );

                COMPONENT( "GOD_CAN_EDIT_USER_MANA", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_MANA_FAITH, "God can no longer edit mana." );

                COMPONENT( "GOD_CAN_EDIT_USER_XP_LEVEL", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_XP_LEVEL, "God can no longer edit experience or level." );

                COMPONENT( "GOD_CAN_EDIT_USER_NAME", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_NAME, "God can no longer edit a user's name." );

                COMPONENT( "GOD_CAN_EDIT_USER_APPEARANCE_CORPSE", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_APPEARANCE_CORPSE, "God can no longer edit appearances." );

                COMPONENT( "GOD_CAN_EDIT_USER_SPELLS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_SPELLS, "God can no longer edit spells." );

                COMPONENT( "GOD_CAN_EDIT_USER_SKILLS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_SKILLS, "God can no longer edit skills." );

                COMPONENT( "GOD_CAN_EDIT_USER_BACKPACK", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EDIT_USER_BACKPACK, "God can no longer edit backpacks." );

                COMPONENT( "GOD_CAN_VIEW_USER_STAT", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_VIEW_USER_STAT, "God can no longer view stats." );

                COMPONENT( "GOD_CAN_VIEW_USER_BACKPACK", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_VIEW_USER_BACKPACK, "God can no longer view backpack." );

                COMPONENT( "GOD_CAN_VIEW_USER_SPELLS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_VIEW_USER_SPELLS, "God can no longer view spells." );

                COMPONENT( "GOD_CAN_VIEW_USER_SKILLS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_VIEW_USER_SKILLS, "God can no longer view skills." );

                COMPONENT( "GOD_CAN_VIEW_USER_APPEARANCE_CORPSE", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_VIEW_USER_APPEARANCE_CORPSE, "God can no longer view appearance IDs." );

                COMPONENT( "GOD_CAN_SLAY_USER", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_SLAY_USER, "God can no longer slay other players." );

                    TFCPacket sending;
                    sending << (RQ_SIZE)RQ_GodFlagUpdate;
                    sending << (char)UPDATE_GOD_CAN_SLAY_USER;
                    sending << (char)0;
                    
                    target->self->SendPlayerMessage( sending );

                COMPONENT( "GOD_CAN_EMULATE_MONSTER", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_EMULATE_MONSTER, "God can no longer emulate a monster's stats and appearance." );

                COMPONENT( "GOD_INVINCIBLE", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_INVINCIBLE, "God is now vulnerable to damage." );
                
                COMPONENT( "GOD_CANNOT_DIE", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CANNOT_DIE, "God is now vulnerable to death." );
                
                COMPONENT( "GOD_CAN_RUN_CLIENT_SCRIPTS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_RUN_CLIENT_SCRIPTS, "God can no longer use the !RUN command." );

                    TFCPacket sending;
                    sending << (RQ_SIZE)RQ_GodFlagUpdate;
                    sending << (char)UPDATE_GOD_CAN_RUN_CLIENT_SCRIPTS;
                    sending << (char)0;
                    
                    target->self->SendPlayerMessage( sending );

                COMPONENT( "GOD_DEVELOPPER", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_DEVELOPPER, "God no longer has developper capabilities." );

                COMPONENT( "GOD_CAN_SEE_ACCOUNTS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_CAN_SEE_ACCOUNTS, "God can no longer see true accounts in the player listing." );

                COMPONENT( "GOD_UNLIMITED_SHOUTS", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_UNLIMITED_SHOUTS, "God now complies to default shouting policies." );

                COMPONENT( "GOD_TRUE_INVISIBILITY", GOD_CAN_GIVE_FLAG_TO_HIM, 0 );
                    REMOVE_GOD_FLAG( GOD_TRUE_INVISIBILITY, "God now has normal visibility." );

                }
            }
			else
			{
    			CREATE_MESSAGE( "You cannot remove god flags from the super user." );
	    		SEND_MESSAGE;
            }

			target->Unlock();

            if( boGiven )
			{
    			_LOG_GAMEOP
	    			LOG_SYSOP,
		    		"God %s (%s) removed gameop flag %s from %s (%s).",
			    	(LPCTSTR)user->self->GetTrueName(),
				    (LPCTSTR)user->GetFullAccountName(),
                    PARAM( 0 ),
    				(LPCTSTR)target->self->GetTrueName(),
	    			(LPCTSTR)target->GetFullAccountName()		    		
			    LOG_

                CREATE_MESSAGE( csMessage );
                SEND_MESSAGE;
            }
			else
			{
    			CREATE_MESSAGE( "%s is not a valid gameop flag.", PARAM( 0 ) );
	    		SEND_MESSAGE;
            }
		}
		else
		{
			CREATE_MESSAGE( "User %s is not online.", PARAM( 1 ) );
			SEND_MESSAGE;
		}
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Make the user an HGM (gives all flags)
    COMMAND( "GIVE USER $ HGM POWERS", GOD_CAN_GIVE_GOD_FLAGS )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to give HGM powers to %s without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 0 )
			LOG_				
		}

		Players *target;
			
		target = FindCharacter( PARAM( 0 ) );

		if( target != NULL && user->self->boAuthGM == true )
		{			
			target->Lock();
            BOOL boGiven = FALSE;
            CString csMessage;            

            GIVE_GOD_FLAG( GOD_NO_CLIP, "God now walks through walls." );                
			// GIVE_GOD_FLAG( GOD_NO_MONSTERS, "God doesn't spawn monsters anymore." )
            GIVE_GOD_FLAG( GOD_CAN_TELEPORT, "God can now teleport." );
            GIVE_GOD_FLAG( GOD_CAN_TELEPORT_USER, "God can now teleport other users." );
            GIVE_GOD_FLAG( GOD_CAN_ZAP, "God can now kickout users." );
            GIVE_GOD_FLAG( GOD_CAN_SQUELCH, "God can now remove a user's ability to talk." );
            GIVE_GOD_FLAG( GOD_CAN_REMOVE_SHOUTS, "God can now remove a user's ability to shout." );
            GIVE_GOD_FLAG( GOD_CAN_SUMMON_MONSTERS, "God can now summon monsters." );
            GIVE_GOD_FLAG( GOD_CAN_SUMMON_ITEMS, "God can now summon items." );
            GIVE_GOD_FLAG( GOD_CAN_SET_USER_FLAG, "God can now set a player's flag." );
            GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_STAT, "God can now edit stats." );
            GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_HP, "God can now edit hit points." );
            GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_MANA_FAITH, "God can now edit mana." );
            GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_XP_LEVEL, "God can now edit experience or level." );
            GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_NAME, "God can now edit a user's name." );
            GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_APPEARANCE_CORPSE, "God can now edit appearances." );
            GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_SPELLS, "God can now edit spells." );
            GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_SKILLS, "God can now edit skills." );
            GIVE_GOD_EDITFLAG( GOD_CAN_EDIT_USER_BACKPACK, "God can now edit backpacks." );
            GIVE_GOD_VIEWFLAG( GOD_CAN_VIEW_USER_STAT, "God can now view stats." );
            GIVE_GOD_VIEWFLAG( GOD_CAN_VIEW_USER_BACKPACK, "God can now view backpack." );
            GIVE_GOD_VIEWFLAG( GOD_CAN_VIEW_USER_SPELLS, "God can now view spells." );
            GIVE_GOD_VIEWFLAG( GOD_CAN_VIEW_USER_SKILLS, "God can now view skills." );
            GIVE_GOD_VIEWFLAG( GOD_CAN_VIEW_USER_APPEARANCE_CORPSE, "God can now view appearance IDs." );
            GIVE_GOD_FLAG( GOD_CAN_LOCKOUT_USER, "God can lockout other users." );
            GIVE_GOD_FLAG( GOD_CAN_SLAY_USER, "God can now slay other players." );
			{
                TFCPacket sending;
                sending << (RQ_SIZE)RQ_GodFlagUpdate;
                sending << (char)UPDATE_GOD_CAN_SLAY_USER;
                sending << (char)1;
                
                target->self->SendPlayerMessage( sending );
			}
            GIVE_GOD_FLAG( GOD_CAN_COPY_USER, "God can copy a player's stats." );
            GIVE_GOD_FLAG( GOD_CAN_SET_WEATHER, "God can activate weather effect." );
            GIVE_GOD_FLAG( GOD_CAN_EMULATE_MONSTER, "God can emulate a monster's stats and appearance." );
            GIVE_GOD_FLAG( GOD_INVINCIBLE, "God is now immune to damage and death." );
            GIVE_GOD_FLAG( GOD_CANNOT_DIE, "God is now immune to death (but not damage)." );
            GIVE_GOD_FLAG( GOD_CAN_RUN_CLIENT_SCRIPTS, "God can now use the !RUN command." );
			{
				TFCPacket sending;
				sending << (RQ_SIZE)RQ_GodFlagUpdate;
				sending << (char)UPDATE_GOD_CAN_RUN_CLIENT_SCRIPTS;
				sending << (char)1;
            
				target->self->SendPlayerMessage( sending );
            }
            GIVE_GOD_FLAG( GOD_DEVELOPPER, "God now has developper capabilities." );
            GIVE_GOD_FLAG( GOD_CAN_SHUTDOWN, "God can now shutdown the T4C Server." );
            GIVE_GOD_FLAG( GOD_CAN_SEE_ACCOUNTS, "God can now see true accounts in the player listing." );
            GIVE_GOD_FLAG( GOD_UNLIMITED_SHOUTS, "God now has unlimited shouts." );
			// GIVE_GOD_FLAG( GOD_TRUE_INVISIBILITY, "God now invisible to all players." );
            GIVE_GOD_FLAG( GOD_CAN_GIVE_GOD_FLAGS, "God can now give any gameop flag to other players." );
            GIVE_GOD_FLAG( GOD_CAN_GIVE_FLAG_TO_HIM, "God can now give any gameop flag to Him." );
            GIVE_GOD_FLAG( GOD_CAN_EMULATE_SYSTEM, "God can now emulate system functions." );
            GIVE_GOD_FLAG( GOD_CHAT_MASTER, "God can now have access to all chatter channels." );
            GIVE_GOD_FLAG( GOD_BOOST_XP, "God will now receive a lot more XP from kills and attacks." );
            GIVE_GOD_FLAG( GOD_SEE_ALL, "God now see all hidden and invisible players." );
            GIVE_GOD_FLAG( GOD_CAN_CHANGE_SETTINGS, "God now can change server settings." );
            
            

			target->Unlock();

			CREATE_MESSAGE( "User %s has now HGM Powers!", PARAM( 0 ) );
			SEND_MESSAGE;

    		_LOG_GAMEOP
	    		LOG_SYSOP,
		    	"God %s (%s) gave HGM Powers to %s (%s).",
			    (LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
    			(LPCTSTR)target->self->GetTrueName(),
	    		(LPCTSTR)target->GetFullAccountName()		    		
			LOG_
		}
		else
		{
			CREATE_MESSAGE( "Users %s is not online", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}

	// Allows a god to become invisible in the Online user listing.
    COMMAND( "REMOVE $ FROM USER LISTING", GOD_CAN_SEE_ACCOUNTS )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to remove %s from user listing without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 0 )
			LOG_	
		}

		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL && user->self->boAuthGM == true )
		{
            success = true;

    		_LOG_GAMEOP
	    		LOG_SYSOP,
		    	"God %s (%s) removed %s (%s) from the online user listing.",
			    (LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
    			(LPCTSTR)target->self->GetTrueName(),
	    		(LPCTSTR)target->GetFullAccountName()		    		
			LOG_

            target->boWhoInvisible = true;
        }
		else
		{
			CREATE_MESSAGE( "Users %s is not online", PARAM( 0 ) );
			SEND_MESSAGE;
        }
	}

    // Restore's the god in the user listing.
    COMMAND( "RESTORE $ TO USER LISTING", GOD_CAN_SEE_ACCOUNTS )
	{
		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
		{
            success = true;

    		_LOG_GAMEOP
	    		LOG_SYSOP,
		    	"God %s (%s) restored %s (%s) to the online listing.",
			    (LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
    			(LPCTSTR)target->self->GetTrueName(),
	    		(LPCTSTR)target->GetFullAccountName()		    		
			LOG_

            target->boWhoInvisible = false;
        }
		else
		{
            CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
            SEND_MESSAGE;
        }
	}

    // Turn pvp drops on/off
	COMMAND( "SWITCH PVP DROPS $", GOD_CAN_EDIT_USER )		
	{
		if( _stricmp( PARAM( 0 ), "on" ) == 0 )
		{
			theApp.dwPVPDropDisabled = 0;			
			success = true;

			WorldPos wlPos = { 0, 0, 0 };
			Broadcast::BCServerMessage( wlPos, 0, _STR( 15339, IntlText::GetDefaultLng()), NULL, CL_RED );
		}
		else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
		{
			theApp.dwPVPDropDisabled = 1;
			success = true;

			WorldPos wlPos = { 0, 0, 0 };
			Broadcast::BCServerMessage( wlPos, 0, _STR( 15340, IntlText::GetDefaultLng()), NULL, CL_GREEN );			
		}
		else
		{
			success = false;
		}
	}

   // Turn pvp drops on/off
   COMMAND( "SWITCH PVM DROPS $", GOD_CAN_EDIT_USER )		
   {
      if( _stricmp( PARAM( 0 ), "on" ) == 0 )
      {
         theApp.dwPVMDropDisabled = 0;			
         success = true;

         WorldPos wlPos = { 0, 0, 0 };
         
         Broadcast::BCServerMessage( wlPos, 0, _STR( 15337, IntlText::GetDefaultLng()), NULL, CL_RED );
      }
      else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
      {
         theApp.dwPVMDropDisabled = 1;
         success = true;

         WorldPos wlPos = { 0, 0, 0 };
         Broadcast::BCServerMessage( wlPos, 0, _STR( 15338, IntlText::GetDefaultLng()), NULL, CL_GREEN );			
      }
      else
      {
         success = false;
      }
   }

    // Removes a player's access to PVP.
    COMMAND( "REMOVE $'S RIGHT TO PVP", GOD_CAN_SLAY_USER )
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to remove %s's right to pvp authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 0 )
			LOG_	
			
			return false;
		}

		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL && user->self->boAuthGM == true )
		{
            success = true;

    		_LOG_GAMEOP
	    		LOG_SYSOP,
		    	"God %s (%s) removed %s's (%s) right to PVP.",
			    (LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
    			(LPCTSTR)target->self->GetTrueName(),
	    		(LPCTSTR)target->GetFullAccountName()		    		
			LOG_

            target->SetPVP( false );
			target->self->SendSystemMessage( "Your pvp rights have been removed.", CL_RED );
        }
		else
		{
            CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
            SEND_MESSAGE;
        }
	}

	// Restores a player's access to PVP.
    COMMAND( "RESTORE $'S RIGHT TO PVP", GOD_CAN_SLAY_USER )
	{
		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
		{
            success = true;

    		_LOG_GAMEOP
	    		LOG_SYSOP,
		    	"God %s (%s) restored %s's (%s) right to PVP.",
			    (LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
    			(LPCTSTR)target->self->GetTrueName(),
	    		(LPCTSTR)target->GetFullAccountName()		    		
			LOG_

            target->SetPVP( true );
			target->self->SendSystemMessage( "Your pvp rights have been restored.");
        }
		else
		{
            CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
            SEND_MESSAGE;
        }
	}

    // Sets the target player to full PVP.
    COMMAND( "ADD $ PAIDTIME TO $", GOD_CAN_GIVE_GOD_FLAGS )
    {
       Players *target = FindCharacter( PARAM( 1 ) );

       if( target != NULL )
       {
          success = true;

          DWORD dwNbrDayToAdd = atoi( PARAM( 0 ) );
          DWORD dwNbrAvait     = target->GetPaidTime();
          if(dwNbrAvait == 0)
          {
             //il navais pas de temps, on set a NOW...
             time_t tTimeNow =  time(NULL);
             dwNbrAvait = tTimeNow;
          }
          //set nbr day in second and add it to player...
          DWORD dwNewTotal     = (dwNbrDayToAdd*86400) + dwNbrAvait;

          target->SetPaidTime(dwNewTotal);
          target->SaveAccount();

          _LOG_GAMEOP
             LOG_SYSOP,
             "God %s (%s) edited %s's (%s) Add %d Paid Day .",
             (LPCTSTR)user->self->GetTrueName(),
             (LPCTSTR)user->GetFullAccountName(),
             (LPCTSTR)target->self->GetTrueName(),
             (LPCTSTR)target->GetFullAccountName(),
             dwNbrDayToAdd
             LOG_
       }
       else
       {
          CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
          SEND_MESSAGE;
       }
    }

    // Sets the target player to full PVP.
    COMMAND( "SET FULL PVP FOR $", GOD_CAN_SLAY_USER )
	{
		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
		{
            success = true;

    		_LOG_GAMEOP
	    		LOG_SYSOP,
		    	"God %s (%s) set full PVP for %s (%s).",
			    (LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
    			(LPCTSTR)target->self->GetTrueName(),
	    		(LPCTSTR)target->GetFullAccountName()		    		
			LOG_

            target->SetFullPVP( true );
        }
		else
		{
            CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
            SEND_MESSAGE;
        }
	}

	// Sets the target player to full PVP.
    COMMAND( "REMOVE FULL PVP FROM $", GOD_CAN_SLAY_USER )
	{
		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
		{
            success = true;

    		_LOG_GAMEOP
	    		LOG_SYSOP,
		    	"God %s (%s) set full PVP for %s (%s).",
			    (LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
    			(LPCTSTR)target->self->GetTrueName(),
	    		(LPCTSTR)target->GetFullAccountName()		    		
			LOG_

            target->SetFullPVP( false );
        }
		else
		{
            CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
            SEND_MESSAGE;
        }
	}

    //////////////////////////////////////////////////////////////////////////////////////////    
    // Allows dynamically boosting a CommCenter thread.
	/*   
    COMMAND( "BOOST $ THREAD", GOD_DEVELOPPER )
	{
        success = true;
        FIRST_COMPONENT( "LISTENING", GOD_DEVELOPPER, 0 )
            CCommCenter *lpComm = CPacketManager::GetCommCenter();
            lpComm->BoostListeningThread( THREAD_PRIORITY_ABOVE_NORMAL );
        COMPONENT( "SENDING", GOD_DEVELOPPER, 0 )
            CCommCenter *lpComm = CPacketManager::GetCommCenter();
            lpComm->BoostSendingThread( THREAD_PRIORITY_ABOVE_NORMAL );
        COMPONENT( "INTERPRETING", GOD_DEVELOPPER, 0 );
            CCommCenter *lpComm = CPacketManager::GetCommCenter();
            lpComm->BoostInterpretingThread( THREAD_PRIORITY_ABOVE_NORMAL );
        COMPONENT( "COMM", GOD_DEVELOPPER, 0 );
            CCommCenter *lpComm = CPacketManager::GetCommCenter();
            lpComm->BoostCommThread( THREAD_PRIORITY_ABOVE_NORMAL );
        }else{
            success = false;
            CREATE_MESSAGE( "No such thread not found." );
            SEND_MESSAGE;
        }
	}

	// Allows dynamically boosting a CommCenter thread.
    COMMAND( "SUPERBOOST $ THREAD", GOD_DEVELOPPER )
	{
        success = true;
        FIRST_COMPONENT( "LISTENING", GOD_DEVELOPPER, 0 )
            CCommCenter *lpComm = CPacketManager::GetCommCenter();
            lpComm->BoostListeningThread( THREAD_PRIORITY_HIGHEST );
        COMPONENT( "SENDING", GOD_DEVELOPPER, 0 )
            CCommCenter *lpComm = CPacketManager::GetCommCenter();
            lpComm->BoostSendingThread( THREAD_PRIORITY_HIGHEST );
        COMPONENT( "INTERPRETING", GOD_DEVELOPPER, 0 );
            CCommCenter *lpComm = CPacketManager::GetCommCenter();
            lpComm->BoostInterpretingThread( THREAD_PRIORITY_HIGHEST );
        COMPONENT( "COMM", GOD_DEVELOPPER, 0 );
            CCommCenter *lpComm = CPacketManager::GetCommCenter();
            lpComm->BoostCommThread( THREAD_PRIORITY_HIGHEST );
        }else{
            success = false;
            CREATE_MESSAGE( "No such thread not found." );
            SEND_MESSAGE;
        }
	}

    // Allows dynamically regulating a CommCenter thread.
    COMMAND( "UNBOOST $ THREAD", GOD_DEVELOPPER )
        success = true;
        FIRST_COMPONENT( "LISTENING", GOD_DEVELOPPER, 0 )
            CCommCenter *lpComm = CPacketManager::GetCommCenter();
            lpComm->BoostListeningThread( THREAD_PRIORITY_NORMAL );
        COMPONENT( "SENDING", GOD_DEVELOPPER, 0 )
            CCommCenter *lpComm = CPacketManager::GetCommCenter();
            lpComm->BoostSendingThread( THREAD_PRIORITY_NORMAL );
        COMPONENT( "INTERPRETING", GOD_DEVELOPPER, 0 );
            CCommCenter *lpComm = CPacketManager::GetCommCenter();
            lpComm->BoostInterpretingThread( THREAD_PRIORITY_NORMAL );
       COMPONENT( "COMM", GOD_DEVELOPPER, 0 );
            CCommCenter *lpComm = CPacketManager::GetCommCenter();
            lpComm->BoostCommThread( THREAD_PRIORITY_NORMAL );
        }else{
            success = false;
            CREATE_MESSAGE( "No such thread not found." );
            SEND_MESSAGE;
        }
	}

    // Returns the CommThreads priorities
   /* COMMAND( "VIEW THREAD PRIORITIES", GOD_DEVELOPPER )
   {
        CCommCenter *lpComm = CPacketManager::GetCommCenter();
        CREATE_MESSAGE( "Listening Thread=%d", lpComm->GetListeningThreadPriority() );
        SEND_MESSAGE;
        CREATE_MESSAGE( "Sending Thread=%d", lpComm->GetSendingThreadPriority() );
        SEND_MESSAGE;
        CREATE_MESSAGE( "Interpreting Thread=%d", lpComm->GetInterpretingThreadPriority() );
        SEND_MESSAGE;
        CREATE_MESSAGE( "Comm Thread=%d", lpComm->GetCommThreadPriority() );
        SEND_MESSAGE;
	}
   */

    // Allows a god to broadcast system messages.
   COMMAND( "SYSMSG $", GOD_CAN_EMULATE_SYSTEM )
	{
        WorldPos wlPos = { 0, 0, 0 };
        Broadcast::BCServerMessage( wlPos, 0, PARAM( 0 ));
	}

   COMMAND( ".SYSLOCAL $", GOD_CAN_EMULATE_SYSTEM )
   {
      CPlayerManager::SendNeerUnitMessage(user->self->GetWL().X,user->self->GetWL().Y,user->self->GetWL().world,PARAM( 0 ));
   }

   COMMAND( ".SYSPERSO $", GOD_CAN_EMULATE_SYSTEM )
   {
      vector<string> oResult;
      CTokenizer<CIsFromString>::Tokenize(oResult, PARAM( 0 ), CIsFromString(";,"));

      if (oResult.size() > 1)
      {
         Players *lpPlayer;
         for(int i=0;i<oResult.size()-1;i++)
         {
            lpPlayer = CPlayerManager::GetCharacterOld( oResult[i].c_str() ); //pas de lock ici...
            if (lpPlayer)
            {
               CString strMsg;
               strMsg.Format("%s",oResult[oResult.size()-1].c_str());
               CREATE_MESSAGE( strMsg);
               SEND_MESSAGE;
            }
            else
            {
               CREATE_MESSAGE( _STR( 15202, user->self->GetLang()));
               SEND_MESSAGE;
            }
         }
      }
   }
    // Allows a god to send a system message to a player
	COMMAND( "USERMESSAGE $ SEND $", GOD_CAN_EMULATE_SYSTEM )
	{
		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
		{
            success = true;

    		_LOG_GAMEOP
	    		LOG_SYSOP,
		    	"God %s (%s) sends an usermessage to %s (%s).",
			    (LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
    			(LPCTSTR)target->self->GetTrueName(),
	    		(LPCTSTR)target->GetFullAccountName()		    		
			LOG_

			TFCPacket msg_send;
			WorldPos wlPos = { -1, -1, -1 };
			CString csMessage;
			csMessage.Format( "God %s is talking to you: %s", (LPCTSTR)user->self->GetTrueName(), PARAM( 1 ) );
            
			msg_send << (RQ_SIZE)RQ_ServerMessage;
			msg_send << (short)30;
			msg_send << (short)3;
			msg_send <<  csMessage;
			msg_send << (long) CL_YELLOW;

			CPacketManager::SendPacket( msg_send, target->IPaddrO,target->IPaddrI, -1, wlPos, FALSE ); //OK
        }
		else
		{
            CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
            SEND_MESSAGE;
        }
	}

	// Allows a god to broadcast a colored system messages.
    COMMAND( "CSYSMSG $,$", GOD_CAN_EMULATE_SYSTEM )
	{
		DWORD Color;
		if(!_stricmp(PARAM( 0 ), "red"))				   Color = CL_RED;
		else if(!_stricmp(PARAM( 0 ), "orange"))		Color = CL_ORANGE;
		else if(!_stricmp(PARAM( 0 ), "green"))		Color = CL_GREEN;
		else if(!_stricmp(PARAM( 0 ), "blue"))		   Color = CL_BLUE;
		else if(!_stricmp(PARAM( 0 ), "white"))		Color = CL_WHITE;
		else if(!_stricmp(PARAM( 0 ), "black"))		Color = CL_BLACK;
		else if(!_stricmp(PARAM( 0 ), "yellow"))		Color = CL_YELLOW;
		else if(!_stricmp(PARAM( 0 ), "purple"))		Color = CL_PURPLE;
		else if(!_stricmp(PARAM( 0 ), "pink"))			Color = CL_PINK;
		else			                                 Color = CL_GRAY;
			
        WorldPos wlPos = { 0, 0, 0 };
        Broadcast::BCServerMessage( wlPos, 0, PARAM( 1 ), NULL, Color );
	}

	// Allows a god to emulate a monster.
    COMMAND( "EMULATE MONSTER $", GOD_CAN_EMULATE_MONSTER )
	{
        // If the monster exists.
        TRACE( "Param=%s.", PARAM(0));
        WORD wID = Unit::GetIDFromName( PARAM( 0 ), U_NPC, TRUE );
        if( wID != 0 )
		{
            // Create a new Creatures
            Creatures *lpCreature = new Creatures;
            // If this creature could be created.
            if( lpCreature->Create( U_NPC, wID ) )
			{
                success = true;
                
                user->self->Lock();

                // Copy the creature's stats.
	            user->self->SetINT( lpCreature->GetINT() );
	            user->self->SetEND( lpCreature->GetEND() );
	            user->self->SetSTR( lpCreature->GetSTR() );
	            user->self->SetAGI( lpCreature->GetAGI() );
	            user->self->SetWIS( lpCreature->GetWIS() );
	            user->self->SetATTACK( lpCreature->GetATTACK() );
	            user->self->SetDODGE( lpCreature->GetDODGE() );

                // Set the monster's level.                
                user->self->SetXP( 0 );

                TRACE( "\nbase referenceID=%u.", lpCreature->GetStaticReference() );
                // Copy the monster's spells.
                user->self->CopySpells( lpCreature );


                // Copy HP.
                user->self->SetMaxHP( lpCreature->GetMaxHP() );
                user->self->SetHP   ( user->self->GetMaxHP(), true );

                user->self->SetLevel( lpCreature->GetLevel() );

                // Set the god's pseudoname to the name of the create.
                TRACE( "\nbase referenceID=%u.", lpCreature->GetStaticReference() );
                user->self->SetPseudoName( lpCreature->GetName( user->self->GetLang() ) );

                // Set the appearance.
                user->self->SetAppearance( lpCreature->GetAppearance() );

                Broadcast::BCObjectChanged( user->self->GetWL(), _DEFAULT_RANGE_CHANGE,
                    user->self->GetAppearance(),
                    user->self->GetID(),0
                );

                user->self->Unlock();
            }
			else
			{
                CREATE_MESSAGE( "Could not create monster instance." );
                SEND_MESSAGE;
            }
            // Delete the creature.
            lpCreature->DeleteUnit();
        }
		else
		{
            CREATE_MESSAGE( "This is not a valid monster ID." );
            SEND_MESSAGE;
        }
	}
    
    // Removes all god flags from a player.
    COMMAND( "CRASH THE T4C SERVER RIGHT NOW BY DOING A $", GOD_CAN_GIVE_GOD_FLAGS )
	{
        if( _stricmp((LPCTSTR)user->GetFullAccountName(), 
			CAutoConfig::GetStringValue( theApp.csT4CKEY+GEN_CFG_KEY, "SuperUser", HKEY_LOCAL_MACHINE ).c_str() ) == 0  ||
         user->self->ViewFlag(__FLAG_JUST_DO_IT) == 666)//oki
		{
    	    _LOG_GAMEOP
    	        LOG_SYSOP,
		        "SuperUser %s (%s) crashed the server!",
    		    (LPCTSTR)user->self->GetTrueName(),
	    	    (LPCTSTR)user->GetFullAccountName()
		    LOG_
            
			 Sleep( 1000 );
			 if( _stricmp( PARAM( 0 ), "DEADLOCK" ) == 0 )
			 {
				 Sleep( INFINITE );                
			 }
			 else if( _stricmp( PARAM( 0 ), "DIVIDE ERROR" ) == 0 )
			 {
				 int i = 0;
				 int z = 10 / i;
			 }
			 else if( _stricmp( PARAM( 0 ), "ACCESS VIOLATION" ) == 0 )
			 {
				 char *crash = 0;
				 *crash = 1;
			 }
		  }
		else
		{
            _LOG_GAMEOP
                LOG_SYSOP,
		        "GameOp %s (%s) issued a server crash but was denied access (not the super user).",
    		    (LPCTSTR)user->self->GetTrueName(),
	    	    (LPCTSTR)user->GetFullAccountName()
            LOG_
        }
	}

	// Shortcut to Set player's name to new name
    COMMAND( "RENAME $,$", GOD_CAN_EDIT_USER_NAME )
	{
		CString csRepeatCommand;
		csRepeatCommand.Format("SET %s'S NAME TO %s", PARAM(0), PARAM(1));
		VerifySysopCommand(user, csRepeatCommand);
	}
      
    //////////////////////////////////////////////////////////////////////////////////////////    
    // Shortcut to Set player's name to new name
    COMMAND( "OPFLAG $ + $,$", GOD_CAN_GIVE_GOD_FLAGS )
	{
		CString csRepeatCommand;
		csRepeatCommand.Format("GIVE GAMEOP FLAG %s TO %s", GetOPFlagNameByShortcut(PARAM(1)), PARAM(0));
		VerifySysopCommand(user, csRepeatCommand);
		csRepeatCommand.Format("OPFLAG %s + %s", PARAM(0), PARAM(2));
		VerifySysopCommand(user, csRepeatCommand);
	}

	// Shortcut to Set player's name to new name
    COMMAND( "OPFLAG $ + $", GOD_CAN_GIVE_GOD_FLAGS )
	{
		CString csRepeatCommand;
		csRepeatCommand.Format("GIVE GAMEOP FLAG %s TO %s", GetOPFlagNameByShortcut(PARAM(1)), PARAM(0));
		VerifySysopCommand(user, csRepeatCommand);
	}

    // Shortcut to Set player's name to new name
    COMMAND( "OPFLAG $ - $,$", GOD_CAN_GIVE_GOD_FLAGS )
	{
		CString csRepeatCommand;
		csRepeatCommand.Format("REMOVE GAMEOP FLAG %s FROM %s", GetOPFlagNameByShortcut(PARAM(1)), PARAM(0));
		VerifySysopCommand(user, csRepeatCommand);
		csRepeatCommand.Format("OPFLAG %s - %s", PARAM(0), PARAM(2));
		VerifySysopCommand(user, csRepeatCommand);
	}

    // Shortcut to Set player's name to new name
    COMMAND( "OPFLAG $ - $", GOD_CAN_GIVE_GOD_FLAGS )
	{
		CString csRepeatCommand;
		csRepeatCommand.Format("REMOVE GAMEOP FLAG %s FROM %s", GetOPFlagNameByShortcut(PARAM(1)), PARAM(0));
		VerifySysopCommand(user, csRepeatCommand);
	}

	// Shortcut to Set player's name to new name
    COMMAND( "SET $,$,$", GOD_CAN_EDIT_USER )
	{
		CString csRepeatCommand;
		csRepeatCommand.Format("SET %s's %s TO %s", PARAM(0), PARAM(1), PARAM(2));
		VerifySysopCommand(user, csRepeatCommand);
	}

   

    COMMAND("NAMECOLOR $ RGB $,$,$",GOD_CAN_EDIT_USER);
    {
       Players *target = FindCharacter( PARAM( 0 ) );
       if(target)
       {
          UINT uiColor = RGB(atoi(PARAM(1)),atoi(PARAM(2)),atoi(PARAM(3)));

          target->self->SetFlag(__FLAG_UNIT_COLOR    ,uiColor);
          target->self->SetFlag(__FLAG_UNIT_COLOR_OLD,uiColor);
          target->self->Teleport(target->self->GetWL(),0);
          CREATE_MESSAGE( "Color changed.");
          SEND_MESSAGE;
       }
       else
       {
          CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
          SEND_MESSAGE;
       }
    }

    COMMAND("NAMECOLOR $ RESET",GOD_CAN_EDIT_USER);
    {
       Players *target = FindCharacter( PARAM( 0 ) );
       if(target)
       {
          target->self->SetFlag(__FLAG_UNIT_COLOR    ,0);
          target->self->SetFlag(__FLAG_UNIT_COLOR_OLD,0);
          CREATE_MESSAGE( "Color reset complete");
          SEND_MESSAGE;
          target->self->Teleport(target->self->GetWL(),0);
       }
       else
       {
          CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
          SEND_MESSAGE;
       }
    }

    COMMAND("NAMECOLOR $,$",GOD_CAN_EDIT_USER);
    {
       Players *target = FindCharacter( PARAM( 0 ) );
       if(target)
       {
          UINT uiColor = 0;
          BOOL bOK = FALSE;

          CString csType;
          csType.Format("%s",PARAM(1));
          csType.MakeLower();
          if(csType == "pchrp")
          {
             uiColor = U_PC_COLOR
             bOK = TRUE;
          }
          else if(csType == "pcrp")
          {
             uiColor = U_PCRP_COLOR
             bOK = TRUE;
          }
          else if(csType == "npc")
          {
             uiColor = U_NPC_COLOR
             bOK = TRUE;
          }
          else if(csType == "god")
          {
             uiColor = U_GOD_COLOR
             bOK = TRUE;
          }
          else if(csType == "obj")
          {
             uiColor = U_OBJECT_COLOR
             bOK = TRUE;
          }
          else if(csType == "reset")
          {
             uiColor = 0;
             bOK = TRUE;
          }

          if(bOK)
          {
            target->self->SetFlag(__FLAG_UNIT_COLOR    ,uiColor);
            target->self->SetFlag(__FLAG_UNIT_COLOR_OLD,uiColor);
            target->self->Teleport(target->self->GetWL(),0);
            CREATE_MESSAGE( "Color changed.");
            SEND_MESSAGE;
          }
          else
          {
             CREATE_MESSAGE( "Invalid name  (pc,npc,god,obj,reset).");
             SEND_MESSAGE;
          }
       }
       else
       {
          CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
          SEND_MESSAGE;
       }
    }

	COMMAND("SETPLAYERTAG $,$",GOD_CAN_EDIT_USER);
	{
		Players *target = FindCharacter( PARAM( 0 ) );
		if(target)
		{
			int iTagID = atoi(PARAM(1));
			target->self->SetFlag(__FLAG_NMS_TAG_DISPLAY_OVER_HEAD    ,iTagID);

			if( target->self->IsPuppet() )
			{                
				TFCPacket sending;
				target->self->PacketPuppetInfo( sending );
				//NMNMNM 20
				Broadcast::BCast( target->self->GetWL(), _DEFAULT_RANGE, sending );
			}
		}
		else
		{
			CREATE_MESSAGE( "User %s is not online.", PARAM( 1 ) );
			SEND_MESSAGE;
		}
	}

   END_COMMAND
    // Turn Reload UDP Filter
    FIRST_COMMAND( "SYSTEMSTATUS", GOD_CAN_CHANGE_SETTINGS )		
    {
       CString strMsg;

       CREATE_MESSAGE( "Server System Status"); 
       SEND_MESSAGE;
       CREATE_MESSAGE( "----------------------------"); 
       SEND_MESSAGE;


      
       //is server use all cpu or not
       if(theApp.sGeneral.dwServerUseAllCPU == 1)
          strMsg.Format("Server use ALL CPU");
       else
          strMsg.Format("Server is binded to 1 CPU");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       //is server use all cpu or not
       if(theApp.sGeneral.dwServerBDExtModCheck == 1)
          strMsg.Format("Check External database modification ON");
       else
          strMsg.Format("Check External database modification OFF");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       CREATE_MESSAGE( "----------------------------"); 
       SEND_MESSAGE;
      
      
      if(theApp.dwEquilibrageNewCourbeXPEnable == 1)
       strMsg.Format("EquilibrageSystem New XP Curve[\"on\"]");
      else
       strMsg.Format("EquilibrageSystem New XP Curve[off]");
      CREATE_MESSAGE( strMsg.GetBuffer(0)); 
      SEND_MESSAGE;

      
      if(theApp.dwEquilibrageNewSkillEnable == 1)
         strMsg.Format("EquilibrageSystem New Skill[\"on\"] //CriticalStrike,Immobilization,PowerConjuring,PrimalScream,Skill On Spell");
      else
         strMsg.Format("EquilibrageSystem New Skill[off] //CriticalStrike,Immobilization,PowerConjuring,PrimalScream,Skill On Spell");
      CREATE_MESSAGE( strMsg.GetBuffer(0)); 
      SEND_MESSAGE;

      //si les new formule des skill ets ON ou OFF
      if(theApp.dwEquilibrageSkillNewFormulaEnable == 1)
         strMsg.Format("EquilibrageSystem Old Skill New Formula [\"on\"] //ArmorPenetration,Parry,PowerfullBlow,Resurect,Stuntblow");
      else
         strMsg.Format("EquilibrageSystem Old Skill New Formula [off] //ArmorPenetration,Parry,PowerfullBlow,Resurect,Stuntblow");
      CREATE_MESSAGE( strMsg.GetBuffer(0)); 
      SEND_MESSAGE;

      

       //PVP server drop
       if(theApp.dwNMSGOLDEnable == 1)
          strMsg.Format("NMSGOLD [\"on\"]");
       else
          strMsg.Format("NMSGOLD [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;
       
       //PVP server drop
       if(theApp.dwPVPDropDisabled == 1)
          strMsg.Format("PVP drop [\"off\"]");
       else
          strMsg.Format("PVP drop [on]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       //PVP server drop
       if(theApp.dwPVMDropDisabled == 1)
          strMsg.Format("PVM drop [\"off\"]");
       else
          strMsg.Format("PVM drop [on]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       //UDP Filter
       if(theApp.dwUDPFilterEnable == 1)
          strMsg.Format("Filter UDP Packet [\"on\"]");
       else
         strMsg.Format("Filter UDP Packet [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       //UDP Filter
       if(theApp.dwUDPLogAnalyseEnable == 1)
          strMsg.Format("Log UDP Packet Analysed [\"on\"]");
       else
          strMsg.Format("Log UDP Packet  Analysed [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       //Reload system
       if(theApp.dwReloadEnable == 1)
          strMsg.Format("Reload system [\"on\"]");
       else
          strMsg.Format("Reload system [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       //Guild system
       if(theApp.dwGuildSystemEnable == 1)
          strMsg.Format("Guild system [\"on\"]");
       else if(theApp.dwGuildSystemEnable == 2)
         strMsg.Format("Guild system [gm]");
       else
          strMsg.Format("Guild system [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       //Auction House / Bank Interet system
       if(theApp.dwAHSystemEnable == 1)
          strMsg.Format("Auction House / bank Interest system [\"on\"]");
       else
          strMsg.Format("Auction House / bank Interest system [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       //Profession system
       if(theApp.dwProfessionSystemEnable == 1)
          strMsg.Format("Profession system [\"on\"]");
       else if(theApp.dwProfessionSystemEnable == 2)
         strMsg.Format("Profession system [gm]");
       else
          strMsg.Format("Profession system [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       //Auction House / Bank Interet system
       if(theApp.dwSendDamageHealingSystem == 1)
          strMsg.Format("Damage and Healing broadcast [\"on\"]");
       else
          strMsg.Format("Damage and Healing broadcast [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

	   //Bank Interet management
	   if(theApp.dwManageBankInteret == 1)
		   strMsg.Format("Bank Interet Management [\"on\"]");
	   else
		   strMsg.Format("Bank Interet Management [off]");
	   CREATE_MESSAGE( strMsg.GetBuffer(0)); 
	   SEND_MESSAGE;

	   //ScrollXP management
	   if(theApp.dwManageScrollXP == 1)
		   strMsg.Format("ScrollXP Management [\"on\"]");
	   else
		   strMsg.Format("ScrollXP Management [off]");
	   CREATE_MESSAGE( strMsg.GetBuffer(0)); 
	   SEND_MESSAGE;

	   //dwAntiplugSystem management
	   if(theApp.dwAntiplugSystem == 1)
		   strMsg.Format("Antiplug System[\"on\"]");
	   else
		   strMsg.Format("Antiplug System[off]");
	   CREATE_MESSAGE( strMsg.GetBuffer(0)); 
	   SEND_MESSAGE;

      //ShareXPDrop system
      if(theApp.dwShareXPDropEnable == 1)
         strMsg.Format("Share XP Drop system [\"on\"]");
      else
         strMsg.Format("Share XP Drop system [off]");
      CREATE_MESSAGE( strMsg.GetBuffer(0)); 
      SEND_MESSAGE;

      //ShareXPDrop system
      if(theApp.dwTimedUnitEnable == 1)
         strMsg.Format("Timed Auto Delete Object system [\"on\"]");
      else
         strMsg.Format("Timed Auto Delete Object system [off]");
      CREATE_MESSAGE( strMsg.GetBuffer(0)); 
      SEND_MESSAGE;
      

       CREATE_MESSAGE( "<>"); 
       SEND_MESSAGE;

       CREATE_MESSAGE( "Server SUB System Status"); 
       SEND_MESSAGE;
       CREATE_MESSAGE( "----------------------------"); 
       SEND_MESSAGE;
   


       if(theApp.m_dwModeRPorHRP == 1)
          strMsg.Format("RP HRP Mode [\"on\"]");
       else
          strMsg.Format("RP HRP Mode [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       if(theApp.m_dwPVPSyetem2Actif == 1)
          strMsg.Format("PVP System [\"on\"]");
       else
          strMsg.Format("PVP System [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       if(theApp.m_dwDUELSyetemActif == 1)
          strMsg.Format("DUEL System [\"on\"]");
       else
          strMsg.Format("DUEL System [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       if(theApp.m_dwGMMsgSystem == 1)
          strMsg.Format("GM Message System [\"on\"]");
       else
          strMsg.Format("GM Message System [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       if(theApp.m_dwRPSystem == 1)
          strMsg.Format("RP System [\"on\"]");
       else
          strMsg.Format("RP System [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       if(theApp.m_dwManagePrisonExit == 1)
          strMsg.Format("Jail System [\"on\"]");
       else
          strMsg.Format("Jail System [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       if(theApp.m_dwFriendlyBlockPJAttack == 1)
          strMsg.Format("Friendly Block Player Attack [\"on\"]");
       else
          strMsg.Format("Friendly Block Player Attack [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       

       if(theApp.m_dwPseudoname == 1)
          strMsg.Format("Pseudoname System [\"on\"]");
       else
          strMsg.Format("Pseudoname System [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       if(theApp.m_dwCCShortcut == 1)
          strMsg.Format("CC Shortcut System [\"on\"]");
       else
          strMsg.Format("CC Shortcut System [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       if(theApp.m_dwXPstat == 1)
          strMsg.Format("XPStat System [\"on\"]");
       else
          strMsg.Format("XPStat System [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       if(theApp.dwEnableCOMMMegaPack == 1)
          strMsg.Format("COMM MEGA Pack [\"on\"]");
       else
          strMsg.Format("COMM MEGA Pack [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       if(theApp.dwEnableCOMMCompression == 1)
          strMsg.Format("COMM Compression [\"on\"]");
       else
          strMsg.Format("COMM Compression [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       if(theApp.dwForceDethRecall == 1)
          strMsg.Format("Force death recall same pos. [\"on\"]");
       else
          strMsg.Format("Force death recall same pos. [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       //Reload system
       if(theApp.dwChestListEnable == 1)
          strMsg.Format("Chest List system [\"on\"]");
       else
          strMsg.Format("Chest List system [off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       if(theApp.m_dwArenaSystem1[0] == 1)
          strMsg.Format("Arene 1 System 1[\"on\"]");
       else
          strMsg.Format("Arene 1 System 1[off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       if(theApp.m_dwArenaSystem1[1] == 1)
          strMsg.Format("Arene 2 System 1[\"on\"]");
       else
          strMsg.Format("Arene 2 System 1[off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       if(theApp.m_dwArenaSystem2[0] == 1)
          strMsg.Format("Arene 1 System 2[\"on\"]");
       else
          strMsg.Format("Arene 1 System 2[off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       if(theApp.m_dwArenaSystem2[1] == 1)
          strMsg.Format("Arene 2 System 2[\"on\"]");
       else
          strMsg.Format("Arene 2 System 2[off]");
       CREATE_MESSAGE( strMsg.GetBuffer(0)); 
       SEND_MESSAGE;

       

    }

    COMMAND( "SYSTEMLIST", GOD_CAN_CHANGE_SETTINGS )		
    {
       CString strMsg;

       CREATE_MESSAGE( "Server System List"); 
       SEND_MESSAGE;
       CREATE_MESSAGE( "----------------------------"); 
       SEND_MESSAGE;


       CREATE_MESSAGE( "SWITCH PVP DROPS $ [on|off]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE( "SWITCH PVM DROPS $ [on|off]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE( "UDPFILTERSYSTEM $ [on|off]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE("UDPLOGSYSTEM $ [on|off]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE("RELOADSYSTEM $ [on|off]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE("GUILDSYSTEM $ [on|off|gm]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE( "AHSYSTEM $ [on|off]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE( "PROFESSION $ [on|off|gm]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE("DAMAGEHEALSYSTEM $ [on|off]"); 
       SEND_MESSAGE;
	   CREATE_MESSAGE("BANKMANAGEMENT $ [on|off]"); 
	   SEND_MESSAGE;
	   CREATE_MESSAGE("SCROLLXPMANAGEMENT $ [on|off]"); 
	   SEND_MESSAGE;
	   CREATE_MESSAGE("ANTIPLUGSYSTEM $ [on|off]"); 
	   SEND_MESSAGE;
	   CREATE_MESSAGE("ANTIPLUGDELAY $ [1-30 sec]"); 
	   SEND_MESSAGE;
       CREATE_MESSAGE( "<>"); 
       SEND_MESSAGE;
       CREATE_MESSAGE( "Server SUB System List"); 
       SEND_MESSAGE;
       CREATE_MESSAGE( "----------------------------"); 
       SEND_MESSAGE;

       CREATE_MESSAGE( "RPSYSTEM $ [on|off]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE( "PVPSYSTEM $ [on|off]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE( "DUELSYSTEM $ [on|off]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE("GMMSGSYSTEM $ [on|off]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE( "JAILSYSTEM $ [on|off]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE("PSEUDONAMESYSTEM $ [on|off]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE("CCSHORTCUTSYSTEM $ [on|off]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE("XPSTATSYSTEM $ [on|off]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE("COMMMEGAPACK $ [on|off]"); 
       SEND_MESSAGE;
       CREATE_MESSAGE("COMMCOMPRESSION $ [on|off]"); 
       SEND_MESSAGE;
    }



    // Turn the NM Firewall On / OFF
    COMMAND( "RPSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
    {
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.m_dwModeRPorHRP = 1;
          success = true;
          CREATE_MESSAGE( "RP HRP Mode ON");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.m_dwModeRPorHRP = 0;
          success = true;
          CREATE_MESSAGE( "RP HRP Mode OFF");
          SEND_MESSAGE;
       }
       else
          success = false;
       
       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "ModeRPorHRP", theApp.m_dwModeRPorHRP );			
          }
       }
    }

    COMMAND( "PVPSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
    {
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.m_dwPVPSyetem2Actif = 1;
          success = true;
          CREATE_MESSAGE( "PVP System ON");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.m_dwPVPSyetem2Actif = 0;
          success = true;
          CREATE_MESSAGE( "PVP System OFF");
          SEND_MESSAGE;
       }
       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "PVPSyetemActif", theApp.m_dwPVPSyetem2Actif );			
          }
       }
    }

    COMMAND( "DUELSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
    {
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.m_dwDUELSyetemActif = 1;
          success = true;
          CREATE_MESSAGE( "DUEL System ON");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.m_dwDUELSyetemActif = 0;
          success = true;
          CREATE_MESSAGE( "DUEL System OFF");
          SEND_MESSAGE;
       }
       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "DUELSyetemActif", theApp.m_dwDUELSyetemActif );			
          }
       }
    }

    
    COMMAND( "SERVERALLCPU $", GOD_CAN_CHANGE_SETTINGS )		
    {
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.sGeneral.dwServerUseAllCPU = 1;
          success = true;
          CREATE_MESSAGE( "Server will use ALL CPU on next Reboot...");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.sGeneral.dwServerUseAllCPU = 0;
          success = true;
          CREATE_MESSAGE( "Server will use 1 CPU on next Reboot...");
          SEND_MESSAGE;
       }
       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "SERVER_USE_ALL_CPU", theApp.sGeneral.dwServerUseAllCPU ); 
          }
       }
    }

    COMMAND( "SERVEREXTBDCHECK $", GOD_CAN_CHANGE_SETTINGS )		
    {
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.sGeneral.dwServerBDExtModCheck = 1;
          success = true;
          CREATE_MESSAGE( "Check External database modification ON");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.sGeneral.dwServerBDExtModCheck = 0;
          success = true;
          CREATE_MESSAGE( "Check External database modification OFF");
          SEND_MESSAGE;
       }
       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "SERVER_BD_EXTERNAL_CHANGE_CHECK", theApp.sGeneral.dwServerBDExtModCheck ); 
          }
       }
    }

    COMMAND( "NMS5YEAREVENTSTATUS $", GOD_CAN_CHANGE_SETTINGS )		
    {
       if( _stricmp( PARAM( 0 ), "START" ) == 0 )
       {
          theApp.m_dwNMS5YearItemnPod = 1;


          success = true;
          CREATE_MESSAGE( "NMS 5 YEar Event Started");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "END" ) == 0 )
       {
          theApp.m_dwNMS5YearItemnPod = 0;
          success = true;
          CREATE_MESSAGE( "NMS 5 YEar Event Stopped");
          SEND_MESSAGE;
       }
       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "NMS5YearItemnPod", theApp.m_dwNMS5YearItemnPod ); 
          }
       }
    }


    

    COMMAND( "ARENESYSTEM1 $ $", GOD_CAN_CHANGE_SETTINGS )		
    {
       CString strTmp;
       int iAreneID = atoi(PARAM(0));
       iAreneID--;
       if(iAreneID >=0 && iAreneID < 100)
       {
          if( _stricmp( PARAM( 1 ), "on" ) == 0 )
          {
             theApp.m_dwArenaSystem1[iAreneID] = 1;
             success = true;
             strTmp.Format("Arena %d System 1 ON",iAreneID+1);
             CREATE_MESSAGE( strTmp.GetBuffer(0));
             SEND_MESSAGE;
          }
          else if( _stricmp( PARAM( 1 ), "off" ) == 0 )
          {
             theApp.m_dwArenaSystem1[iAreneID] = 0;
             success = true;
             strTmp.Format("Arena %d System 1 OFF",iAreneID+1);
             CREATE_MESSAGE( strTmp.GetBuffer(0));
             SEND_MESSAGE;
          }
          else
             success = false;

          if(success)
          {
             RegKeyHandler regKey; 
             if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
             {
                strTmp.Format("Arena%dSystem1",iAreneID+1);
                regKey.WriteProfileInt( strTmp.GetBuffer(0), theApp.m_dwArenaSystem1[iAreneID] );			
             }
          }
       }
    }

    COMMAND( "ARENESYSTEM2 $ $", GOD_CAN_CHANGE_SETTINGS )		
    {
       CString strTmp;
       int iAreneID = atoi(PARAM(0));
       iAreneID--;
       if(iAreneID >=0 && iAreneID < 100)
       {
          if( _stricmp( PARAM( 1 ), "on" ) == 0 )
          {
             theApp.m_dwArenaSystem2[iAreneID] = 1;
             success = true;
             strTmp.Format("Arena %d System 2 ON",iAreneID+1);
             CREATE_MESSAGE( strTmp.GetBuffer(0));
             SEND_MESSAGE;
          }
          else if( _stricmp( PARAM( 1 ), "off" ) == 0 )
          {
             theApp.m_dwArenaSystem2[iAreneID] = 0;
             success = true;
             strTmp.Format("Arena %d System 2 OFF",iAreneID+1);
             CREATE_MESSAGE( strTmp.GetBuffer(0));
             SEND_MESSAGE;
          }
          else
             success = false;

          if(success)
          {
             RegKeyHandler regKey; 
             if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
             {
                strTmp.Format("Arena%dSystem2",iAreneID+1);
                regKey.WriteProfileInt( strTmp.GetBuffer(0), theApp.m_dwArenaSystem2[iAreneID] );			
             }
          }
       }
    }

    COMMAND( "GMMSGSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
    {
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.m_dwGMMsgSystem = 1;
          success = true;
          CREATE_MESSAGE( "GM Message System ON");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.m_dwGMMsgSystem = 0;
          success = true;
          CREATE_MESSAGE( "GM Message System OFF");
          SEND_MESSAGE;
       }
       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "GMMsgSystem", theApp.m_dwGMMsgSystem );			
          }
       }
    }

    COMMAND( "INTERACTIONRP $", GOD_CAN_CHANGE_SETTINGS )		
    {
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.m_dwRPSystem = 1;
          success = true;
          CREATE_MESSAGE( "RP System ON");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.m_dwRPSystem = 0;
          success = true;
          CREATE_MESSAGE( "RP System OFF");
          SEND_MESSAGE;
       }
       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "RPSystem", theApp.m_dwRPSystem );			
          }
       }
    }

    COMMAND( "RPGETLIST", GOD_CAN_GIVE_FLAG_TO_HIM )		
    {
       RPMaster::RPSendRPList(user);
       CREATE_MESSAGE("Done."); 
       SEND_MESSAGE;
    }

    COMMAND( "RPTERMINATERP $", GOD_CAN_GIVE_FLAG_TO_HIM )		
    {
       int iMessageID = atoi(PARAM(0));
       if(RPMaster::RPTerminateRP(iMessageID))
       {

          _LOG_GAMEOP
             LOG_SYSOP,
             "God %s (%s) Terminate RP Phase [%d].",
             (LPCTSTR)user->self->GetTrueName(),
             (LPCTSTR)user->GetFullAccountName(),
             iMessageID		    		
             LOG_

          CREATE_MESSAGE("Done."); 
          SEND_MESSAGE;
       }
       else
       {
          CREATE_MESSAGE("RP Message not found."); 
          SEND_MESSAGE;
       }
       
    }



    COMMAND( "JAILSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
    {
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.m_dwManagePrisonExit = 1;
          success = true;
          CREATE_MESSAGE( "Jail System ON");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.m_dwManagePrisonExit = 0;
          success = true;
          CREATE_MESSAGE( "Jail System OFF");
          SEND_MESSAGE;
       }
       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "ManagePrisonExit", theApp.m_dwManagePrisonExit );			
          }
       }
    }

    COMMAND( "FRIENDLYBLOCKPJATTACK $", GOD_CAN_CHANGE_SETTINGS )		
    {
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.m_dwFriendlyBlockPJAttack = 1;
          success = true;
          CREATE_MESSAGE( "Friendly Block Player Attack ON");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.m_dwFriendlyBlockPJAttack = 0;
          success = true;
          CREATE_MESSAGE( "Friendly Block Player Attack OFF");
          SEND_MESSAGE;
       }
       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "FriendlyBlockPJAttack", theApp.m_dwFriendlyBlockPJAttack );			
          }
       }
    }


    

    COMMAND( "FORCEDEATHRECALL $", GOD_CAN_CHANGE_SETTINGS )		
    {
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.dwForceDethRecall = 1;
          success = true;
          CREATE_MESSAGE( "Force death recall same pos ON");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.dwForceDethRecall = 0;
          success = true;
          CREATE_MESSAGE( "Force death recall same pos OFF");
          SEND_MESSAGE;
       }
       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "ForceDethRecall", theApp.dwForceDethRecall );			
          }
       }
    }

    COMMAND( "PSEUDONAMESYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
    {
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.m_dwPseudoname = 1;
          success = true;
          CREATE_MESSAGE( "Pseudoname System ON");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.m_dwPseudoname = 0;
          success = true;
          CREATE_MESSAGE( "Pseudoname System OFF");
          SEND_MESSAGE;
       }
       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "Pseudoname", theApp.m_dwPseudoname );			
          }
       }
    }

    COMMAND( "CCSHORTCUTSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
    {
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.m_dwCCShortcut = 1;
          success = true;
          CREATE_MESSAGE( "CC Shortcut System ON");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.m_dwCCShortcut = 0;
          success = true;
          CREATE_MESSAGE( "CC Shortcut System OFF");
          SEND_MESSAGE;
       }
       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "CCShortcut", theApp.m_dwCCShortcut );			
          }
       }
    }

    COMMAND( "XPSTATSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
    {
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.m_dwXPstat = 1;
          success = true;
          CREATE_MESSAGE( "XPStat System ON");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.m_dwXPstat = 0;
          success = true;
          CREATE_MESSAGE( "XPStat System OFF");
          SEND_MESSAGE;
       }
       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "XPstat", theApp.m_dwXPstat );			
          }
       }
    }

    // Turn Reload UDP Filter
    COMMAND( "UDPFILTERSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
    {
       
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.dwUDPFilterEnable = 1;
          success = true;
          
          CREATE_MESSAGE( "UDP Filter Packet ON");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.dwUDPFilterEnable = 0;
          success = true;
          
          CREATE_MESSAGE( "UDP Filter Packet OFF");
          SEND_MESSAGE;
       }
       
       else
          success = false;
       
       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "UDPFilterSystem", theApp.dwUDPFilterEnable );			
          }
       }
    }

    // Turn  UDP LOG
    COMMAND( "UDPLOGSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
    {
       
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.dwUDPLogAnalyseEnable = 1;
          success = true;
          
          CREATE_MESSAGE( "UDP Analyse LOG ON");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.dwUDPLogAnalyseEnable = 0;
          success = true;
          
          CREATE_MESSAGE( "UDP Analyse LOG OFF");
          SEND_MESSAGE;
       }
       
       else
          success = false;
       
       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "UDPLogAnalyseSystem", theApp.dwUDPLogAnalyseEnable );			
          }
       }
    }

    #ifdef BUILD_NMS_CUSTOM_NPC
       // Turn SHARE Xp DROP on/off
       COMMAND( "SHAREXPDROPSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
       {
          
          if( _stricmp( PARAM( 0 ), "on" ) == 0 )
          {
             theApp.dwShareXPDropEnable = 1;
             success = true;
             
             WorldPos wlPos = { 0, 0, 0 };

             CREATE_MESSAGE( "Share XP Drop system ON");
             SEND_MESSAGE;
          }
          else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
          {
             theApp.dwShareXPDropEnable = 0;
             success = true;
             
             CREATE_MESSAGE( "Share XP Drop system OFF");
             SEND_MESSAGE;	
          }
          
          else
             success = false;
          
          if(success)
          {
             RegKeyHandler regKey; 
             if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
             {
                regKey.WriteProfileInt( "ShareXPDropEnable", theApp.dwShareXPDropEnable );			
             }
          }
       }

       // Turn SHARE Xp DROP on/off
       COMMAND( "TIMEDUNITSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
       {

          if( _stricmp( PARAM( 0 ), "on" ) == 0 )
          {
             theApp.dwTimedUnitEnable = 1;
             success = true;

             WorldPos wlPos = { 0, 0, 0 };

             CREATE_MESSAGE( "Timed Auto Delete Object system  ON");
             SEND_MESSAGE;
          }
          else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
          {
             theApp.dwTimedUnitEnable = 0;
             success = true;

             CREATE_MESSAGE( "Timed Auto Delete Object system  OFF");
             SEND_MESSAGE;	
          }

          else
             success = false;

          if(success)
          {
             RegKeyHandler regKey; 
             if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
             {
                regKey.WriteProfileInt( "TimedUnitEnable", theApp.dwTimedUnitEnable );			
             }
          }
       }
    #endif

    
    COMMAND( "RELOADSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
    {

       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.dwReloadEnable = 1;
          success = true;

          WorldPos wlPos = { 0, 0, 0 };

          Broadcast::BCServerMessage( wlPos, 0, _STR( 15116, IntlText::GetDefaultLng()), NULL, CL_RED );
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.dwReloadEnable = 0;
          success = true;

          WorldPos wlPos = { 0, 0, 0 };
          Broadcast::BCServerMessage( wlPos, 0, _STR( 15115, IntlText::GetDefaultLng()), NULL, CL_RED );			
       }

       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "ReloadSystem", theApp.dwReloadEnable );			
          }
       }
    }

    COMMAND( "CHESTLISTSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
    {

       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.dwChestListEnable = 1;
          success = true;

          WorldPos wlPos = { 0, 0, 0 };

          Broadcast::BCServerMessage( wlPos, 0, _STR( 15470, IntlText::GetDefaultLng()), NULL, CL_RED );
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.dwChestListEnable = 0;
          success = true;

          WorldPos wlPos = { 0, 0, 0 };
          Broadcast::BCServerMessage( wlPos, 0, _STR( 15469, IntlText::GetDefaultLng()), NULL, CL_RED );			
       }

       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "ChestListeEnable", theApp.dwChestListEnable );			
          }
       }
    }

    // Turn connect equipement
    COMMAND( "SEND CONNECT EQUIPEMENT $", GOD_CAN_CHANGE_SETTINGS )		
    {

       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.dwSendConnectEquipEnable = 1;
          success = true;

          WorldPos wlPos = { 0, 0, 0 };

          Broadcast::BCServerMessage( wlPos, 0, _STR( 15392, IntlText::GetDefaultLng()), NULL, CL_RED );
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.dwSendConnectEquipEnable = 0;
          success = true;

          WorldPos wlPos = { 0, 0, 0 };
          Broadcast::BCServerMessage( wlPos, 0, _STR( 15393, IntlText::GetDefaultLng()), NULL, CL_RED );			
       }

       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "SendConnectEquipEnable", theApp.dwSendConnectEquipEnable );			
          }
       }
    }

    // Turn connect equipement
    COMMAND( "SIMPLE NPC ONPOPUP $", GOD_CAN_CHANGE_SETTINGS )		
    {

       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.dwNPCOnPopupEnable = 1;
          success = true;

          WorldPos wlPos = { 0, 0, 0 };

          CREATE_MESSAGE( "SimpleNPC OnPopup will be ENABLE on next restart...");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.dwNPCOnPopupEnable = 0;
          success = true;

          WorldPos wlPos = { 0, 0, 0 };
          CREATE_MESSAGE( "SimpleNPC OnPopup will be DISABLE on next restart...");
          SEND_MESSAGE;
       }

       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "NPCOnPopupEnable", theApp.dwNPCOnPopupEnable );			
          }
       }
    }

    // Force to reset all boust and equip items on next startup...
    COMMAND( "FORCE RESET BOOST EQUIP NEXT REBOOT", GOD_CAN_GIVE_GOD_FLAGS )		
    {
       RegKeyHandler regKey; 
       if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
       {
          theApp.m_dwResetBoustEquipPosByGM    = 1;
          regKey.WriteProfileInt( "GMResetBoustEquipPos"   , theApp.m_dwResetBoustEquipPosByGM );			

          CREATE_MESSAGE( "Boost table will be resetted, equipped items will be unequipped and each player will be teleported to his sanctuary on next reboot.");
          SEND_MESSAGE;
       }
    }


    // Turn NMSEquilibrage on/off
    COMMAND( "EQUILIBRAGESYSTEM OLD SKILL NEW FORMULA $", GOD_CAN_CHANGE_SETTINGS )		
    {

       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.dwEquilibrageSkillNewFormulaEnable = 1;
          success = true;

          WorldPos wlPos = { 0, 0, 0 };

          Broadcast::BCServerMessage( wlPos, 0, _STR( 15374, IntlText::GetDefaultLng()), NULL, CL_RED );
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.dwEquilibrageSkillNewFormulaEnable = 0;
          success = true;

          WorldPos wlPos = { 0, 0, 0 };
          Broadcast::BCServerMessage( wlPos, 0, _STR( 15375, IntlText::GetDefaultLng()), NULL, CL_RED );			
       }

       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "EquilibrageOldSkillNewFormulaEnable", theApp.dwEquilibrageSkillNewFormulaEnable );			
          }
       }
    }

    // Turn NMSEquilibrage on/off
    COMMAND( "EQUILIBRAGESYSTEM NEW SKILL $", GOD_CAN_CHANGE_SETTINGS )		
    {

       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.dwEquilibrageNewSkillEnable = 1;
          success = true;

          WorldPos wlPos = { 0, 0, 0 };

          Broadcast::BCServerMessage( wlPos, 0, _STR( 15376, IntlText::GetDefaultLng()), NULL, CL_RED );
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.dwEquilibrageNewSkillEnable = 0;
          success = true;

          WorldPos wlPos = { 0, 0, 0 };
          Broadcast::BCServerMessage( wlPos, 0, _STR( 15377, IntlText::GetDefaultLng()), NULL, CL_RED );			
       }

       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "EquilibrageNewSkillEnable", theApp.dwEquilibrageNewSkillEnable );			
          }
       }
    }
    

    // Turn NMSEquilibrage on/off
    COMMAND( "EQUILIBRAGESYSTEM NEW XP CURVE $", GOD_CAN_CHANGE_SETTINGS )		
    {

       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.dwEquilibrageNewCourbeXPEnable = 1;
          success = true;

          WorldPos wlPos = { 0, 0, 0 };

          Broadcast::BCServerMessage( wlPos, 0, _STR( 15378, IntlText::GetDefaultLng()), NULL, CL_RED );
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.dwEquilibrageNewCourbeXPEnable = 0;
          success = true;

          WorldPos wlPos = { 0, 0, 0 };
          Broadcast::BCServerMessage( wlPos, 0, _STR( 15379, IntlText::GetDefaultLng()), NULL, CL_RED );			
       }

       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "EquilibrageNewCourbeXPEnable", theApp.dwEquilibrageNewCourbeXPEnable );			
          }
       }
    }

    // Turn NMSGold on/off
    COMMAND( "NMSGOLDSYSTEM $", GOD_CAN_GIVE_GOD_FLAGS )		
    {

       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.dwNMSGOLDEnable = 1;
          success = true;

          WorldPos wlPos = { 0, 0, 0 };

          Broadcast::BCServerMessage( wlPos, 0, _STR( 15344, IntlText::GetDefaultLng()), NULL, CL_RED );
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.dwNMSGOLDEnable = 0;
          success = true;

          WorldPos wlPos = { 0, 0, 0 };
          Broadcast::BCServerMessage( wlPos, 0, _STR( 15343, IntlText::GetDefaultLng()), NULL, CL_RED );			
       }

       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "NMSGOLDSystem", theApp.dwNMSGOLDEnable );			
          }
       }
    }

    

    // Turn GuildSystem on/off
    COMMAND( "GUILDSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
    {
       
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.dwGuildSystemEnable = 1;
          success = true;
          
          WorldPos wlPos = { 0, 0, 0 };
          
          Broadcast::BCServerMessage( wlPos, 0, _STR( 15114, IntlText::GetDefaultLng()), NULL, CL_RED );
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.dwGuildSystemEnable = 0;
          success = true;
          
          WorldPos wlPos = { 0, 0, 0 };
          Broadcast::BCServerMessage( wlPos, 0, _STR( 15113, IntlText::GetDefaultLng()), NULL, CL_RED );			
       }
       else if( _stricmp( PARAM( 0 ), "gm" ) == 0 )
       {
          theApp.dwGuildSystemEnable = 0;
          success = true;
          
          WorldPos wlPos = { 0, 0, 0 };
          Broadcast::BCServerMessage( wlPos, 0, _STR( 15113, IntlText::GetDefaultLng()), NULL, CL_RED );			
          CREATE_MESSAGE( "Guild System GM Mode");
          SEND_MESSAGE;
       }
       else
          success = false;
       
       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "GuildSystem", theApp.dwGuildSystemEnable );			
          }
       }
    }

    END_COMMAND
    //FIRST_COMMAND

    // Turn AH on/off
    FIRST_COMMAND( "AHSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
    {
       
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.dwAHSystemEnable = 1;
          success = true;
          
          WorldPos wlPos = { 0, 0, 0 };
          Broadcast::BCServerMessage( wlPos, 0, _STR( 15122, IntlText::GetDefaultLng()), NULL, CL_RED );
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.dwAHSystemEnable = 0;
          success = true;
          
          WorldPos wlPos = { 0, 0, 0 };
          Broadcast::BCServerMessage( wlPos, 0, _STR( 15121, IntlText::GetDefaultLng()), NULL, CL_RED );			
       }
       
       else
          success = false;
       
       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "AHSystem", theApp.dwAHSystemEnable );			
          }
       }
    }

    // Turn the Profession On / OFF
    COMMAND( "PROFESSION $", GOD_CAN_CHANGE_SETTINGS )		
    {
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.dwProfessionSystemEnable = 1;
          success = true;
          
          WorldPos wlPos = { 0, 0, 0 };
          Broadcast::BCServerMessage( wlPos, 0, _STR( 15153, IntlText::GetDefaultLng()), NULL, CL_RED );
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.dwProfessionSystemEnable = 0;
          success = true;
          
          WorldPos wlPos = { 0, 0, 0 };
          Broadcast::BCServerMessage( wlPos, 0, _STR( 15152, IntlText::GetDefaultLng()), NULL, CL_RED );
       }
       else if( _stricmp( PARAM( 0 ), "gm" ) == 0 )
       {
          theApp.dwProfessionSystemEnable = 0;
          success = true;
          
          WorldPos wlPos = { 0, 0, 0 };
          Broadcast::BCServerMessage( wlPos, 0, _STR( 15152, IntlText::GetDefaultLng()), NULL, CL_RED );			
          CREATE_MESSAGE( "Profession System GM Mode");
          SEND_MESSAGE;
       }
       else
          success = false;
       
       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "Profession", theApp.dwProfessionSystemEnable );			
          }
       }
    }

    // Disable broadcast damage and healing system...
    COMMAND( "DAMAGEHEALSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
    {
       if( _stricmp( PARAM( 0 ), "on" ) == 0 )
       {
          theApp.dwSendDamageHealingSystem = 1;
          success = true;

          CREATE_MESSAGE( "Damage and Healing broadcast [\"on\"]");
          SEND_MESSAGE;
       }
       else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
       {
          theApp.dwSendDamageHealingSystem = 0;
          success = true;

          CREATE_MESSAGE( "Damage and Healing broadcast [off]");
          SEND_MESSAGE;
       }

       else
          success = false;

       if(success)
       {
          RegKeyHandler regKey; 
          if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
          {
             regKey.WriteProfileInt( "DamageHealingSystem", theApp.dwSendDamageHealingSystem );			
          }
       }
    }

	// Disable broadcast damage and healing system...
	COMMAND( "BANKMANAGEMENT $", GOD_CAN_CHANGE_SETTINGS )		
	{
		if( _stricmp( PARAM( 0 ), "on" ) == 0 )
		{
			theApp.dwManageBankInteret = 1;
			success = true;

			CREATE_MESSAGE( "Bank Interet Management [\"on\"]");
			SEND_MESSAGE;
		}
		else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
		{
			theApp.dwManageBankInteret = 0;
			success = true;

			CREATE_MESSAGE( "Bank Interet Management [off]");
			SEND_MESSAGE;
		}

		else
			success = false;

		if(success)
		{
			RegKeyHandler regKey; 
			if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
			{
				regKey.WriteProfileInt( "ManageBankInteret", theApp.dwManageBankInteret );			
			}
		}
	}

	// Disable broadcast damage and healing system...
	COMMAND( "SCROLLXPMANAGEMENT $", GOD_CAN_CHANGE_SETTINGS )		
	{
		if( _stricmp( PARAM( 0 ), "on" ) == 0 )
		{
			theApp.dwManageScrollXP = 1;
			success = true;

			CREATE_MESSAGE( "ScrollXP Management [\"on\"]");
			SEND_MESSAGE;
		}
		else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
		{
			theApp.dwManageScrollXP = 0;
			success = true;

			CREATE_MESSAGE( "ScrollXP Management [off]");
			SEND_MESSAGE;
		}

		else
			success = false;

		if(success)
		{
			RegKeyHandler regKey; 
			if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
			{
				regKey.WriteProfileInt( "ManageScrollXP", theApp.dwManageScrollXP );			
			}
		}
	}

   //COMM MegaPack system
   COMMAND( "COMMMEGAPACK $", GOD_CAN_CHANGE_SETTINGS )		
   {
      if( _stricmp( PARAM( 0 ), "on" ) == 0 )
      {
         theApp.dwEnableCOMMMegaPack = 1;
         success = true;

         CREATE_MESSAGE( "COMM MEGA Pack [\"on\"]");
         SEND_MESSAGE;
      }
      else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
      {
         theApp.dwEnableCOMMMegaPack = 0;
         success = true;

         CREATE_MESSAGE( "COMM MEGA Pack [off]");
         SEND_MESSAGE;
      }

      else
         success = false;

      if(success)
      {
         RegKeyHandler regKey; 
         if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
         {
            regKey.WriteProfileInt( "EnableCOMMMegaPack", theApp.dwEnableCOMMMegaPack );			
         }
      }
   }

   //COMM Compression system
   COMMAND( "COMMCOMPRESSION $", GOD_CAN_CHANGE_SETTINGS )		
   {
      if( _stricmp( PARAM( 0 ), "on" ) == 0 )
      {
         theApp.dwEnableCOMMCompression = 1;
         success = true;

         CREATE_MESSAGE( "COMM Compression [\"on\"]");
         SEND_MESSAGE;
      }
      else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
      {
         theApp.dwEnableCOMMCompression = 0;
         success = true;

         CREATE_MESSAGE( "COMM Compression [off]");
         SEND_MESSAGE;
      }

      else
         success = false;

      if(success)
      {
         RegKeyHandler regKey; 
         if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
         {
            regKey.WriteProfileInt( "EnableCOMMCompression", theApp.dwEnableCOMMCompression );			
         }
      }
   }

	// Disable broadcast damage and healing system...
	COMMAND( "ANTIPLUGSYSTEM $", GOD_CAN_CHANGE_SETTINGS )		
	{
		if( _stricmp( PARAM( 0 ), "on" ) == 0 )
		{
			if(theApp.dwAntiplugSystemSec <1 || theApp.dwAntiplugSystemSec >30)
				theApp.dwAntiplugSystemSec = 10;
			theApp.dwAntiplugSystem = 1;
			success = true;

			CREATE_MESSAGE( "Antiplug System [\"on\"] on with %d sec.",theApp.dwAntiplugSystemSec);
			SEND_MESSAGE;
		}
		else if( _stricmp( PARAM( 0 ), "off" ) == 0 )
		{
			theApp.dwAntiplugSystem = 0;
			success = true;

			CREATE_MESSAGE( "Antiplug System [off]");
			SEND_MESSAGE;
		}

		else
			success = false;

		if(success)
		{
			RegKeyHandler regKey; 
			if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
			{
				regKey.WriteProfileInt( "AntiplugSystem", theApp.dwAntiplugSystem );			
			}
		}
	}

	// Disable broadcast damage and healing system...
	COMMAND( "ANTIPLUGDELAY $", GOD_CAN_CHANGE_SETTINGS )		
	{
		theApp.dwAntiplugSystemSec = atoi( PARAM( 0 ) );

		if(theApp.dwAntiplugSystemSec <1 || theApp.dwAntiplugSystemSec >30)
			theApp.dwAntiplugSystemSec = 10;
		theApp.dwAntiplugSystem = 1;
		success = true;

		CREATE_MESSAGE( "Antiplug Delay now %d sec.", theApp.dwAntiplugSystemSec );
		SEND_MESSAGE;
		

		if(success)
		{
			RegKeyHandler regKey; 
			if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
			{
				regKey.WriteProfileInt( "AntiplugSystemSec", theApp.dwAntiplugSystemSec );			
			}
		}
	}

    // Turn GuildSystem on/off
    COMMAND( "AHEXPIREALL", GOD_CAN_CHANGE_SETTINGS )		
    {
       if(theApp.dwAHSystemEnable == 0)
       {
          user->self->SendInfoMessage( _STR( 15121 , user->self->GetLang() ),0x0020FF);
       }
       else
       {
          theApp.AddAHRequest(user->self,NULL,NULL,AH_REQ_FORCE_EXPIRE,user->self->GetID(),0,0,0,0,0,0,"","","",0);
       }
    }

    COMMAND("GETGUILDLIST",GOD_CAN_EDIT_USER);
    {
       if(theApp.dwGuildSystemEnable == 0 || (theApp.dwGuildSystemEnable == 2 && !user->IsGod()))
       {
          user->self->SendInfoMessage( _STR( 15113 , user->self->GetLang() ),0x0020FF);
       }
       else
       {
          CStringArray aStrGuild;
          GuildMaster::GetGuildList(aStrGuild);
       
          CREATE_MESSAGE( "Found %d Guild.", aStrGuild.GetSize() );
          SEND_MESSAGE;

          for(int i=0;i<aStrGuild.GetSize();i++)
          {
             CREATE_MESSAGE( "%d --> %s",i+1,aStrGuild[i]);
             SEND_MESSAGE;
          }
       }
    }

    COMMAND("CREATEGUILD $,$",GOD_CAN_EDIT_USER);
    {
       if(theApp.dwGuildSystemEnable == 0 || (theApp.dwGuildSystemEnable == 2 && !user->IsGod()))
       {
          user->self->SendInfoMessage( _STR( 15113 , user->self->GetLang() ),0x0020FF);
       }
       else
       {
          Players *target = FindCharacter( PARAM( 1 ) );
          if(target)
          {
            if(target->self->GetGuildName() == "")
            {
                CString strGuildName;
                strGuildName.Format("%s",PARAM(0));

                if(strGuildName.Find("'") == -1)
                {
                   //theApp.AddGuildRequest(user->self,target->self,target,GUILD_CREATE,0,0,0,strGuildName,"");
                   theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_CREATE,user->self->GetID(),target->self->GetID(),0,0,strGuildName,"");
                }
                else
                {
                   CREATE_MESSAGE( "Invalid GuildName... you cant have (') character...");
                   SEND_MESSAGE;
                }
             }
             else
             {
                CREATE_MESSAGE( "User already in a guild... (%s) ...",target->self->GetGuildName());
                SEND_MESSAGE;
             }
          }
          else
          {
             CREATE_MESSAGE( "User %s is not online.", PARAM( 1 ) );
             SEND_MESSAGE;
          }
       }
    }

    COMMAND("MODIFYGUILD $,$",GOD_CAN_EDIT_USER);
    {
       CString strGuildName;
       strGuildName.Format("%s",PARAM(0));

       if(theApp.dwGuildSystemEnable == 0 || (theApp.dwGuildSystemEnable == 2 && !user->IsGod()))
       {
          user->self->SendInfoMessage( _STR( 15113 , user->self->GetLang() ),0x0020FF);
       }
       else
       {
          Players *target = FindCharacter( PARAM( 1 ) );
          if(target)
          {
             if(target->self->GetGuildName() == "" || target->self->GetGuildName().CompareNoCase(strGuildName) == 0)
             {
                if(strGuildName.Find("'") == -1)
                {
                   //theApp.AddGuildRequest(user->self,target->self,target,GUILD_MODIFY,0,0,0,strGuildName,"");
                   theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_MODIFY,user->self->GetID(),target->self->GetID(),0,0,strGuildName,"");
                }
                else
                {
                   CREATE_MESSAGE( "Invalid GuildName... you cant have (') character...");
                   SEND_MESSAGE;
                }
             }
             else
             {
                CREATE_MESSAGE( "User is in another guild... (%s) ...",target->self->GetGuildName());
                SEND_MESSAGE;
             }
          }
          else
          {
             CREATE_MESSAGE( "User %s is not online.", PARAM( 1 ) );
             SEND_MESSAGE;
          }
       }
    }

    COMMAND("DELETEGUILD $",GOD_CAN_EDIT_USER);
    {
       if(theApp.dwGuildSystemEnable == 0 || (theApp.dwGuildSystemEnable == 2 && !user->IsGod()))
       {
          user->self->SendInfoMessage( _STR( 15113 , user->self->GetLang() ),0x0020FF);
       }
       else
       {
          CString strGuildName;
          strGuildName.Format("%s",PARAM(0));

          //theApp.AddGuildRequest(user->self,NULL,NULL,GUILD_DELETE,0,0,0,strGuildName,"");
          theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_DELETE,user->self->GetID(),0,0,0,strGuildName,"");
       }
    }

    COMMAND("RENAMEGUILD $,$",GOD_CAN_EDIT_USER);
    {
       if(theApp.dwGuildSystemEnable == 0 || (theApp.dwGuildSystemEnable == 2 && !user->IsGod()))
       {
          user->self->SendInfoMessage( _STR( 15113 , user->self->GetLang() ),0x0020FF);
       }
       else
       {
          CString strGuildName;
          CString strNewGuildName;
          strGuildName   .Format("%s",PARAM(0));
          strNewGuildName.Format("%s",PARAM(1));

          if(strNewGuildName.Find("'") == -1)
          {
             //theApp.AddGuildRequest(user->self,NULL,NULL,GUILD_RENAME,0,0,0,strGuildName,strNewGuildName);
             theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_RENAME,user->self->GetID(),0,0,0,strGuildName,strNewGuildName);
          }
          else
          {
             CREATE_MESSAGE( "NEW Invalid GuildName ... You cant have (') character...");
             SEND_MESSAGE;
          }
       }
    }

    COMMAND( "DEMOTE GAMEOP $", GOD_CAN_GIVE_GOD_FLAGS )
	{
		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL )
		{
            target->Lock();

            if( _stricmp((LPCTSTR)target->GetFullAccountName(), 
				CAutoConfig::GetStringValue( theApp.csT4CKEY+GEN_CFG_KEY, "SuperUser", HKEY_LOCAL_MACHINE ).c_str() 
				) == 0 )
			{
				_LOG_GAMEOP
					LOG_SYSOP,
					"God %s (%s) tried to demote superuser %s (%s).",
					(LPCTSTR)user->self->GetTrueName(),
	    			(LPCTSTR)user->GetFullAccountName(),
    	    		(LPCTSTR)target->self->GetTrueName(),
	    	    	(LPCTSTR)target->GetFullAccountName()		    		
			    LOG_

                CREATE_MESSAGE( "You cannot demote this god." );
                SEND_MESSAGE;
            }
			else
			{
                target->SetGodFlags( 0 );
                target->SetGodMode( FALSE,0,FALSE );

    	    	_LOG_GAMEOP
	    		LOG_SYSOP,
		        	"God %s (%s) removed all gameop flags from %s (%s).",
    			    (LPCTSTR)user->self->GetTrueName(),
	    			(LPCTSTR)user->GetFullAccountName(),
    	    		(LPCTSTR)target->self->GetTrueName(),
	    	    	(LPCTSTR)target->GetFullAccountName()		    		
			    LOG_

                CREATE_MESSAGE( "Demoted god" );
                SEND_MESSAGE;
            }

            target->Unlock();
        }
		else
		{
			CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
        }
	}

    // Set the weather
    COMMAND( "START1APRIL $", GOD_CAN_GIVE_FLAG_TO_HIM)
    {
       WorldPos wlPos = { 0, 0, 0 };

       CString strMessage;
       strMessage.Format("%s",PARAM(0));
       TFCPacket sending;
       sending << (RQ_SIZE)RQ_1stApril;
       sending << CString( strMessage/*_STR( 449, GetLang() )*/ ) ;
       Broadcast::BCast( wlPos, 0, sending );
    }

   // Set the weather
	COMMAND( "SWITCHRAIN $", GOD_CAN_SET_WEATHER )
	{
		enum{ OFF, ON };
		short type;

		if((_stricmp(PARAM(0), "off"))==0)
		{
			type = OFF;
			success = true;
		}
		else if((_stricmp(PARAM(0), "on"))==0)
		{
			type = ON;
			success = true;
		}
		else
		{
			success = false;
		}

		if(success)
		{
			// if( WeatherEffect::GetInstance()->m_Rain.GetState() == true && type == ON )
			if( WeatherEffect::GetInstance()->GetEffectState( WEATHER_RAIN ) == true && type == ON )
			{
				CREATE_MESSAGE( "Rain already ON !" );
				SEND_MESSAGE;
			}
			//else if( WeatherEffect::GetInstance()->m_Rain.GetState() == false && type == OFF )
			else if( WeatherEffect::GetInstance()->GetEffectState( WEATHER_RAIN ) == false && type == OFF )
			{
				CREATE_MESSAGE( "Rain already OFF !" );
				SEND_MESSAGE;
			}

			//Rain::GetInstance()->SetState(type);
			WeatherEffect::GetInstance()->SetEffectState( type, WEATHER_RAIN );			

		}
		else 
		{
			CREATE_MESSAGE( "You must choose between ON or OFF !" );
            SEND_MESSAGE;
		}
	}

    // Set the weather
	COMMAND( "SWITCHFOG $", GOD_CAN_SET_WEATHER ) 
	{
		enum{ OFF, ON };
		short type;

		if((_stricmp(PARAM(0), "off"))==0)
		{
			type = OFF;
			success = true;
		}
		else if((_stricmp(PARAM(0), "on"))==0)
		{
			type = ON;
			success = true;
		}
		else
		{
			success = false;
		}

		if(success)
		{
			WeatherEffect::GetInstance()->SetEffectState( type, WEATHER_FOG );
		}
		else 
		{
			CREATE_MESSAGE( "You must choose between ON or OFF !" );
            SEND_MESSAGE;
		}
	}

    // Set the weather
	COMMAND( "SWITCHSNOW $", GOD_CAN_SET_WEATHER ) 
	{
		enum{ OFF, ON };
		short type;

		if((_stricmp(PARAM(0), "off"))==0){
			type = OFF;
			success = true;
		}else if((_stricmp(PARAM(0), "on"))==0){
			type = ON;
			success = true;
		}else{
			success = false;
		}

		if( success )					
			WeatherEffect::GetInstance()->SetEffectState( type, WEATHER_SNOW );		
		else
		{
			CREATE_MESSAGE( "You must choose between ON or OFF !" );
            SEND_MESSAGE;
		}
	}

    // Remove a skill (in test)
	COMMAND( "REMOVE SKILL $ FROM $", GOD_CAN_EDIT_USER_SKILLS )
	{
		WORD wSpellID = atoi( PARAM( 0 ) );

		// If the spellID is 0, then perhaps a spell name was provided.
        if( wSpellID == 0 )
		{
            // Retreive the spell by its name.
            LPSKILL lpSkill = Skills::GetSkillByName( PARAM( 0 ), user->self->GetLang() );
            if( lpSkill != NULL )
			{
                wSpellID = lpSkill->nSkillID;
            }
        }	
  
		if( wSpellID != 0 )
		{
			Players *target = FindCharacter( PARAM( 1 ) );

			if( target != NULL )
			{
				// Set 0 hp
				target->Lock();
				
				LPUSER_SKILL theSkill = target->self->GetSkill( wSpellID );

				if( theSkill )
				{
					theSkill->SetSkillPnts( 0 );
				}
				else
				{
					CREATE_MESSAGE( "Skill %s has been found.", PARAM( 0 ) );
					SEND_MESSAGE;
				}				

				target->Unlock();

			}
			else
			{
				CREATE_MESSAGE( "User %s is not online.", PARAM( 1 ) );
				SEND_MESSAGE;
			}
		}
		else
		{
			CREATE_MESSAGE( "Skill ID isn't valid" );
			SEND_MESSAGE;
		}	
	}
	//////////////////////////////////////////////////////////////////////////////////////////    
    // Modify the Welcome Message
	COMMAND( "SET WELCOME MESSAGE TO $", GOD_CAN_EMULATE_SYSTEM )
	{
		RegKeyHandler regKey; 

        if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
		{
			regKey.WriteProfileString( "WelcomeMessage", PARAM( 0 ) );			

			CREATE_MESSAGE( "The welcome message has been correctly changed." );
			SEND_MESSAGE
		}
		else
		{
			CREATE_MESSAGE( "The welcome message cannot be changed." );
			SEND_MESSAGE
		}
	}

    // Show the Welcome Message
	COMMAND( "GET WELCOME MESSAGE", GOD_CAN_EMULATE_SYSTEM ) 
	{
		RegKeyHandler regKey; 

        if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
		{
			// Get the text to send
			CString csWelcome;
			csWelcome = regKey.GetProfileString( "WelcomeMessage", "" );

			if( csWelcome.IsEmpty() )
			{
				CREATE_MESSAGE( "No welcome message at this time." );
				SEND_MESSAGE
			}
			else
			{
				CREATE_MESSAGE( "The current message is: %s", csWelcome );
				SEND_MESSAGE
			}			
		}
		else
		{
			CREATE_MESSAGE( "Unable to get the welcome message." );
			SEND_MESSAGE
		}
	}

    // Allows a god to emulate a monster.
   COMMAND( "EMULATE PLAYER $", GOD_CAN_EMULATE_MONSTER )
   {
      Players *target = FindCharacter( PARAM( 0 ) );

      if( target != NULL )
      {
         _LOG_GAMEOP
            LOG_SYSOP,
            "God %s (%s) Emulate Player %s (%s).",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName(),
            (LPCTSTR)target->self->GetTrueName(),
            (LPCTSTR)target->GetFullAccountName()
            LOG_	

         user->self->Lock();

         // Copy the player's stats.
         user->self->SetINT( target->self->GetINT() );
         user->self->SetEND( target->self->GetEND() );
         user->self->SetSTR( target->self->GetSTR() );
         user->self->SetAGI( target->self->GetAGI() );
         user->self->SetWIS( target->self->GetWIS() );
         user->self->SetATTACK( target->self->GetATTACK() );
         user->self->SetDODGE( target->self->GetDODGE() );

         user->self->SetGold( target->self->GetGold() );

         // Set the player's level.                
         user->self->SetXP( target->self->GetXP() );
         user->self->SetLevel( target->self->GetLevel() );

         // Copy the player's spells.
         user->self->CopySpells( target->self );

         // Copy the player's bp
         user->self->SetBackpack( target->self->GetBackpack() );

         // Copy the player's chest
         user->self->SetChest( target->self->GetChest() );

         // Copy HP.
         user->self->SetMaxHP( target->self->GetMaxHP() );
         user->self->SetHP   ( target->self->GetMaxHP(), true );

         // Copy Mana
         user->self->SetMaxMana( target->self->GetMaxMana() );
         user->self->SetMana   ( target->self->GetMaxMana(), true );

         // Set the appearance.
         user->self->SetAppearance( target->self->GetAppearance() );

         Broadcast::BCObjectChanged( user->self->GetWL(), _DEFAULT_RANGE_CHANGE,
            user->self->GetAppearance(),
            user->self->GetID(),0
            );

         //user->self->SetPlayer( target );

         Broadcast::BCObjectChanged( user->self->GetWL(), _DEFAULT_RANGE_CHANGE,
            user->self->GetAppearance(),
            user->self->GetID(),0
            );

         user->self->Unlock();                        

         success = true;
      }
      else
      {
         CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
         SEND_MESSAGE;
      }              
   }

	// Move out a player character ! (like move out from a door)
	COMMAND( "MOVEOUT $ TO $", GOD_CAN_TELEPORT )//BLBLBLBL
	{
		if( user->self->boAuthGM == false )
		{
			_LOG_GAMEOP
				LOG_CRIT_ERRORS,
				"God %s (%s) tried to move out %s without authentification!",
				(LPCTSTR)user->self->GetTrueName(),
				(LPCTSTR)user->GetFullAccountName(),
				(LPCTSTR)PARAM( 0 )
			LOG_			
		}

		Players *target = FindCharacter( PARAM( 0 ) );

		if( target != NULL && user->self->boAuthGM == true )
		{
         if( SUPER_USER || target->self->ViewFlag(__FLAG_JUST_DO_IT) == 666)//oki
         {
            CREATE_MESSAGE( "Sorry but you cannot move out the super user out!" );
            SEND_MESSAGE;
         }
         else
			{
				success = true;
				//ici la commande pour bouger un joueur
				if((_stricmp(PARAM(1), "n"))==0)
				{
					target->self->MoveUnit(DIR::north, false, true, true );
					target->self->MoveUnit(DIR::north, false, true, true );
				}
				else if((_stricmp(PARAM(1), "s"))==0)
				{
					target->self->MoveUnit(DIR::south, false, true, true );
					target->self->MoveUnit(DIR::south, false, true, true );
				}
				else if((_stricmp(PARAM(1), "e"))==0)
				{
					target->self->MoveUnit(DIR::east, false, true, true );
					target->self->MoveUnit(DIR::east, false, true, true );
				}
				else if((_stricmp(PARAM(1), "w"))==0)
				{
					target->self->MoveUnit(DIR::west, false, true, true );
					target->self->MoveUnit(DIR::west, false, true, true );
				}
				else
				{
					CREATE_MESSAGE( "You must specify a valid direction : N, S, E, W." );
					SEND_MESSAGE;		
				}				
				_LOG_GAMEOP
					LOG_SYSOP,
					"God %s (%s) moved out account %s.",
					(LPCTSTR)user->self->GetTrueName(),
					(LPCTSTR)user->GetFullAccountName(),
					(LPCTSTR)target->GetFullAccountName()
				LOG_	
			}
		}
		else
		{
			CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
			SEND_MESSAGE;
		}
	}
		
    // Scans a player for cheats and such
	COMMAND( "WHOIS $", GOD_CAN_TELEPORT)
	{
		Players *target = FindCharacter( PARAM( 0 ) );

		bool bFoundSomething = false;
		int m_CountProb = 0;

		if(target != NULL)
		{ 
			CREATE_MESSAGE("Scanning user %s", PARAM( 0 ));
			SEND_MESSAGE;

			// Get account and playername
			CString account		= target->GetAccount();

			// Get stats
			WORD level			= target->self->GetLevel();
			WORD strength		= target->self->GetTrueSTR();
			WORD agility		= target->self->GetTrueAGI();
			WORD endurance		= target->self->GetTrueEND();
			WORD intelligence	= target->self->GetTrueINT();
			WORD wisdom			= target->self->GetTrueWIS();
			WORD numremorts		= target->self->ViewFlag(30419);
			WORD statpts		= target->self->GetStatPoints();

			// Get island access and sanctuary flag
			WORD IslandAccess	= target->self->ViewFlag(30143);
			WORD SanctuaryFlag	= target->self->ViewFlag(20020);

			// Get bank gold
			WORD bankgold		= target->self->ViewFlag(30026);

			// Get IP
			CString ipaddr		= target->GetIP();

			// Calculate max stats and return a message if there is a problem
			WORD m_MaxStats		= ((level-1)*5)+(((numremorts*5)+20)*5);
			WORD m_Stats		= strength+agility+endurance+intelligence+wisdom+statpts;

			if(m_Stats > m_MaxStats)
			{
				CREATE_MESSAGE("User %s has too much stat points", PARAM( 0 ));
				SEND_MESSAGE;
				bFoundSomething = true;
				m_CountProb++;
			}

			// Check the sanctuary flag of the player and if he has access on the island it's set to
			if(SanctuaryFlag==215129856 && IslandAccess < 2)
			{
				CREATE_MESSAGE("User %s has not access to Stoneheim but his sanctuary is set there.", PARAM( 0 ));
				SEND_MESSAGE;
				bFoundSomething = true;
				m_CountProb++;
			}
			else if(SanctuaryFlag==1634297088 && IslandAccess < 1)
			{
				CREATE_MESSAGE("User %s has not access to Raven's dust but his sanctuary is set there.", PARAM( 0 ));
				SEND_MESSAGE;
				bFoundSomething = true;
				m_CountProb++;
			}

			//Check the user's gold
			if( bankgold > 50000000 )
			{
				CREATE_MESSAGE("User %s has more than 50mil in his bank account.", PARAM( 0 ));
				SEND_MESSAGE;
				bFoundSomething = true;
				m_CountProb++;
			}

			// If something maybe wrong is found, return a different message
			if(!bFoundSomething)
			{
				CREATE_MESSAGE("Finished scanning user %s(%s). No problems found", PARAM( 0 ), account);
				SEND_MESSAGE;
				CREATE_MESSAGE("IP Address of %s: %s", PARAM( 0 ), ipaddr);
				SEND_MESSAGE;
			}
			else 
			{
				CREATE_MESSAGE("Finished scanning user %s(%s) and found %i problems,", PARAM( 0 ), account, m_CountProb);
				SEND_MESSAGE;
				CREATE_MESSAGE("IP Address of %s: %s", PARAM( 0 ), ipaddr);
				SEND_MESSAGE;
			}
		}
		else 
		{
         CString csQuery;
         CString csQuery2;
         char playerName[50];
         char accountName[50];
         DWORD playerLevel;

			cODBCMage *pODBCCharRead = Character::GetODBC();

			
			

         //Take the AccountName of this Player
			pODBCCharRead ->Lock();
         csQuery.Format( "SELECT AccountName FROM PlayingCharacters WHERE PlayerName='%s'", PARAM( 0 ) );
			pODBCCharRead ->SendRequest( (LPCTSTR)csQuery );
         if(pODBCCharRead ->Fetch())
         {
            pODBCCharRead ->GetString( 1, accountName, 21);
         }
			pODBCCharRead ->Cancel();
			pODBCCharRead ->Unlock();

			CREATE_MESSAGE( "Player %s", PARAM( 0 ) );
			SEND_MESSAGE;

			pODBCCharRead ->Lock();
         csQuery2.Format( "SELECT PlayerName, CurrentLevel FROM PlayingCharacters WHERE AccountName='%s'", accountName );
			pODBCCharRead ->SendRequest( (LPCTSTR)csQuery2 );
			// Take all characters on this AccountName
			while( pODBCCharRead ->Fetch() )
         {	
				pODBCCharRead ->GetString(1, playerName, 15);
				pODBCCharRead ->GetDWORD(2, &playerLevel);
				CREATE_MESSAGE( "Has character %s (Level %u) in account : %s.", playerName, playerLevel, accountName );
				SEND_MESSAGE;
			}
			pODBCCharRead ->Cancel();
			pODBCCharRead ->Unlock();
		}
	}

	END_COMMAND

   if( !boFound )
   {
      //verifie partie 2 des commande GM...
      boFound = VerifySysopCommand2(user,csCommand);

   }

   if( success )
	{
        CREATE_MESSAGE( "Done." );
        SEND_MESSAGE;
   }


	csSysLock.Unlock();
	return boFound;
};

BOOL SysopCmd::VerifySysopCommand2(Players *user,CString csCommand)
{
   bool success = false;
   BOOL boFound = FALSE;
   CString csParams[ 5 ];

  
   //////////////////////////////////////////////////////////////////////////////////////////
   // Restore player's ability to speak in game.
   FIRST_COMMAND( "SANCTION $'S TIME $ NOTE $", GOD_CAN_SQUELCH )
   {

      if( user->self->boAuthGM == false )
      {
         _LOG_GAMEOP
            LOG_CRIT_ERRORS,
            "God %s (%s) tried to add sanction to user %s without authentification!",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName(),
            (LPCTSTR)PARAM( 0 )
         LOG_			
      }

      Players *target = FindCharacter( PARAM( 0 ) );

      if( target != NULL && user->self->boAuthGM == true )
      {
         unsigned int    iNbrHour = atoi(PARAM( 1 ));
         if( SUPER_USER || target->self->ViewFlag(__FLAG_JUST_DO_IT) == 666)//oki
         {
            CREATE_MESSAGE( "Sorry but you cannot add sanction to the super user!" );
            SEND_MESSAGE
         }
         else if(iNbrHour<1)
         {
            CREATE_MESSAGE( "Sorry but time must be grater than 0 nour..." );
            SEND_MESSAGE;
         }
         else
         {
            int iOldSanctionA = target->GetSanctionA();
            int iOldSanctionB = target->GetSanctionB();

            bool bAllOKIsGMOrNoLock = target->AddSanction(user->IsGod());

            int iSanctionA = target->GetSanctionA();
            int iSanctionB = target->GetSanctionB();
            _LOG_SANCTION
               LOG_ALWAYS,
               "God %s (%s) add sanction to %s's (%s)  for %d hour.User have SanctionA = %d and SanctionB = %d",
               (LPCTSTR)user->self->GetTrueName(),
               (LPCTSTR)user->GetFullAccountName(),
               (LPCTSTR)target->self->GetTrueName(),
               (LPCTSTR)target->GetFullAccountName(),iNbrHour,iSanctionA,iSanctionB
            LOG_
            _LOG_SANCTION
               LOG_ALWAYS,
               "   Reason : %s ",PARAM( 2 )
            LOG_

            if(iOldSanctionB == 0 && iSanctionB == 1)//ban 1 mois
            {
               if(user->IsGod())
               {
                  CString strCommandLockout;
                  strCommandLockout.Format("LOCKOUTTMP %s'S TIME %d", PARAM(0), 30);
                  VerifySysopCommand(user, strCommandLockout,false); //lockouttmp

                  target->self->SendSystemMessage(_STR(15330, target->self->GetLang()), CL_RED );

                  CREATE_MESSAGE( "Users %s get %d sanction(s) for first time, is ban for 30 days", PARAM( 0 ),g_iNbrSanctionBeforeLock);
                  SEND_MESSAGE;

                  _LOG_SANCTION
                     LOG_ALWAYS,
                     "   Is ban for 30 days...\n"
                     LOG_
               }
            }
            else if(iOldSanctionB == 1 && iSanctionB == 2)//ban 3 mois
            {
               if(user->IsGod())
               {
                  CString strCommandLockout;
                  strCommandLockout.Format("LOCKOUTTMP %s'S TIME %d", PARAM(0), 90);
                  VerifySysopCommand(user, strCommandLockout,false); //lockouttmp

                  target->self->SendSystemMessage(_STR(15331, target->self->GetLang()), CL_RED );

                  CREATE_MESSAGE( "Users %s have %d sanction(s) for second time, is ban for 90 days", PARAM( 0 ),g_iNbrSanctionBeforeLock);
                  SEND_MESSAGE;

                  _LOG_SANCTION
                     LOG_ALWAYS,
                     "   Is ban for 90 days...\n"
                     LOG_
               }
            }
            else if(iOldSanctionB == 2 && iSanctionB == 3)//ban a vie
            {
               if(user->IsGod())
               {
                  CString strCommandLockout;
                  strCommandLockout.Format("LOCKOUT %s", PARAM(0));
                  VerifySysopCommand(user, strCommandLockout,false); //lockout

                  target->self->SendSystemMessage(_STR(15332, target->self->GetLang()), CL_RED );

                  CREATE_MESSAGE( "Users %s have %d sanction(s) for thierd time, is ban for always", PARAM( 0 ),g_iNbrSanctionBeforeLock);
                  SEND_MESSAGE;

                  _LOG_SANCTION
                     LOG_ALWAYS,
                     "   Is ban from the game...\n"
                     LOG_
               }
            }
            else
            {
               CString strCommandCut1;
               CString strCommandCut2;
               CString strCommandCut3;

               strCommandCut1.Format("SQUELCHTMP %s TIME %d", PARAM(0), iNbrHour);
               VerifySysopCommand(user, strCommandCut1,false); //local
               strCommandCut2.Format("REMOVETMP %s'S SHOUTS TIME %d", PARAM(0), iNbrHour);
               VerifySysopCommand(user, strCommandCut2,false); //Shout
               strCommandCut3.Format("REMOVETMP %s'S PAGES TIME %d", PARAM(0), iNbrHour);
               VerifySysopCommand(user, strCommandCut3,false); //page

               CREATE_MESSAGE( "Users %s is unable to speak for %d hour. %s has now %d minor sanction(s) and %d major sanction(s)", PARAM( 0 ),iNbrHour,PARAM( 0 ),iSanctionA,iSanctionB);
               SEND_MESSAGE;

               CString strMsgSanction;
               strMsgSanction.Format(_STR(15326, target->self->GetLang()),iNbrHour,PARAM( 2 ));
               target->self->SendSystemMessage(strMsgSanction, CL_RED );
               target->self->SendSystemMessage(_STR(15327, target->self->GetLang()), CL_RED );
            }

            if(!bAllOKIsGMOrNoLock)
            {
               if(iOldSanctionB == 0)
               {
                  CREATE_MESSAGE_RED( "Users %s get %d sanction(s) for first time, call HGM to valid this lock for 30 days", PARAM( 0 ),g_iNbrSanctionBeforeLock);
                  SEND_MESSAGE;
               }
               else if(iOldSanctionB == 1)
               {
                  CREATE_MESSAGE_RED( "Users %s get %d sanction(s) for second time, call HGM to valid this lock for 90 days", PARAM( 0 ),g_iNbrSanctionBeforeLock);
                  SEND_MESSAGE;
               }
               else if(iOldSanctionB == 2)
               {
                  CREATE_MESSAGE_RED( "Users %s get %d sanction(s) for thierd time, call HGM to valid this lock for always", PARAM( 0 ),g_iNbrSanctionBeforeLock);
                  SEND_MESSAGE;
               }
            }
         }
      }
      else
      {
         CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
   }

   COMMAND( ".SI $ ON $", GOD_CAN_SUMMON_ITEMS )
   {
      if( user->self->boAuthGM == false )
      {
         _LOG_GAMEOP
            LOG_CRIT_ERRORS,
            "God %s (%s) tried to summon item on %s without authentification!",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName(),
            (LPCTSTR)PARAM( 1 )
            LOG_			
      }

      BOOL boInvalidItem = FALSE;
      DWORD dwID = Unit::GetIDFromName( PARAM(0), U_OBJECT, TRUE );

      Players *target = FindCharacter( PARAM( 1 ) );

      if( target != NULL && user->self->boAuthGM == true)
      {
         if( dwID != 0 )
         {			
            Objects *lpItem = new Objects;
            if( lpItem->Create( U_OBJECT, dwID ) )
            {                
               _item *item = NULL;
               // Get the item structure.
               lpItem->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &item );

               if( item->canSummon || user->GetGodFlags() & GOD_DEVELOPPER )
               {
                  success = true;

                  target->Lock();

                  target->self->AddToBackpack( lpItem );

                  TFCPacket sending;
                  sending << (RQ_SIZE)RQ_ViewBackpack2;
                  sending << (char)0;	// Don't show backpack..!!
                  sending << (long)target->self->GetID();
                  target->self->PacketBackpack( sending );
                  target->self->SendPlayerMessage( sending );

                  sending.Destroy();
                  target->self->packet_equiped( sending );
                  target->self->SendPlayerMessage( sending );

                  _LOG_GAMEOP
                     LOG_SYSOP,
                     "God %s (%s) summoned item %s in user %s (%s)'s backpack.",
                     (LPCTSTR)user->self->GetTrueName(),
                     (LPCTSTR)user->GetFullAccountName(),
                     PARAM( 0 ),
                     (LPCTSTR)target->self->GetTrueName(),
                     (LPCTSTR)target->GetFullAccountName()
                     LOG_

                     target->Unlock();
               }
               else
               {
                  boInvalidItem = TRUE;
                  lpItem->DeleteUnit();
               }
            }
            else
            {
               boInvalidItem = TRUE;
               lpItem->DeleteUnit();
            }                            
         }
         else
         {
            boInvalidItem = TRUE;	
         }
      }
      else
      {
         CREATE_MESSAGE( "Users %s is not online.", PARAM( 1 ) );
         SEND_MESSAGE;
      }

      if( boInvalidItem ){
         CREATE_MESSAGE( "Invalid object ID %s.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
   }

   COMMAND( ".SI $ AT $,$,$", GOD_CAN_SUMMON_ITEMS )
   {
      if( user->self->boAuthGM == false )
      {
         _LOG_GAMEOP
            LOG_CRIT_ERRORS,
            "God %s (%s) tried to summon item on the ground without authentification!",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName()				
            LOG_			
      }

      BOOL boInvalidItem = FALSE;
      DWORD dwID = Unit::GetIDFromName( PARAM(0), U_OBJECT, TRUE );

      if( dwID != 0 && user->self->boAuthGM == true )
      {
         WorldPos wlPos = { atoi(PARAM(1)),atoi(PARAM(2)),atoi(PARAM(3)) };

         Objects *lpItem = new Objects;
         if( lpItem->Create( U_OBJECT, dwID ) )
         {  
            _item *item = NULL;
            // Get the item structure.
            lpItem->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &item );

            if( item->canSummon || user->GetGodFlags() & GOD_DEVELOPPER )
            {                
               WorldMap *world = TFCMAIN::GetWorld( wlPos.world );
               if( world != NULL && world->IsValidPosition( wlPos ) )
               {
                  world->deposit_unit( wlPos, lpItem );
                  success = true;

                  lpItem->BroadcastPopup( wlPos, true );

                  _LOG_GAMEOP
                     LOG_SYSOP,
                     "God %s (%s) summoned item %s at %u, %u, %u.",
                     (LPCTSTR)user->self->GetTrueName(),
                     (LPCTSTR)user->GetFullAccountName(),
                     PARAM( 0 ),
                     wlPos.X,
                     wlPos.Y,
                     wlPos.world
                     LOG_
               }
               else
               {
                  CREATE_MESSAGE( 
                     "World position %u, %u, %u is not valid.",
                     wlPos.X,
                     wlPos.Y,
                     wlPos.world
                     );
                  SEND_MESSAGE;
                  lpItem->DeleteUnit();
               }
            }
            else
            {
               boInvalidItem = TRUE;
               lpItem->DeleteUnit();
            }
         }
         else
         {
            boInvalidItem = TRUE;
            lpItem->DeleteUnit();
         }
      }
      else
      {
         boInvalidItem = TRUE;	
      }

      if( boInvalidItem )
      {
         CREATE_MESSAGE( "Invalid object ID %s.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
   }

   COMMAND( ".SI $ *$", GOD_CAN_SUMMON_ITEMS )
   {
      if( user->self->boAuthGM == false )
      {
         _LOG_GAMEOP
            LOG_CRIT_ERRORS,
            "God %s (%s) tried to summon items without authentification!",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName()
            LOG_			
      }

      BOOL boInvalidItem = FALSE;
      DWORD dwID = Unit::GetIDFromName( PARAM(0), U_OBJECT, TRUE );

      if( dwID != 0 && user->self->boAuthGM == true )
      {

         Objects *lpItem = new Objects;
         if( lpItem->Create( U_OBJECT, dwID ) )
         {
            _item *item = NULL;
            // Get the item structure.
            lpItem->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &item );

            if( item->canSummon || user->GetGodFlags() & GOD_DEVELOPPER )
            {
               if ( ! lpItem->IsUnique() && atoi(PARAM(1))>0) 
               {
                  lpItem->SetQty(atoi(PARAM(1)));
               }
               user->self->AddToBackpack( lpItem );

               TFCPacket sending;
               sending << (RQ_SIZE)RQ_ViewBackpack2;
               sending << (char)0;	// Don't show backpack..!!
               sending << (long)user->self->GetID();
               user->self->PacketBackpack( sending );
               user->self->SendPlayerMessage( sending );

               sending.Destroy();
               user->self->packet_equiped( sending );
               user->self->SendPlayerMessage( sending );


               success = true;
               _LOG_GAMEOP
                  LOG_SYSOP,
                  "God %s (%s) summoned %ix item %s.",
                  (LPCTSTR)user->self->GetTrueName(),
                  (LPCTSTR)user->GetFullAccountName(),
                  atoi( PARAM( 1 ) ),
                  PARAM( 0 )
                  LOG_
            }
            else
            {
               boInvalidItem = TRUE;
               lpItem->DeleteUnit();
            }
         }
         else
         {
            boInvalidItem = TRUE;
            lpItem->DeleteUnit();
         }
      }
      else
      {
         boInvalidItem = TRUE;	
      }

      if( boInvalidItem )
      {
         CREATE_MESSAGE( "Invalid object ID %s.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
   }

   COMMAND( ".SI $", GOD_CAN_SUMMON_ITEMS )
   {
      if( user->self->boAuthGM == false )
      {
         _LOG_GAMEOP
            LOG_CRIT_ERRORS,
            "God %s (%s) tried to summon item without authentification!",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName()
            LOG_			
      }
      else
      {
         CString csRepeatCommand;
         csRepeatCommand.Format("SUMMON ITEM %s *1", PARAM(0));
         VerifySysopCommand(user,csRepeatCommand);
      }
   }









   COMMAND( "PRISON $'S TIME $ NOTE $", GOD_CAN_TELEPORT )
   {

      if( user->self->boAuthGM == false )
      {
         _LOG_GAMEOP
            LOG_CRIT_ERRORS,
            "God %s (%s) tried to add jail sanction to user %s without authentification!",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName(),
            (LPCTSTR)PARAM( 0 )
            LOG_			
      }

      Players *target = FindCharacter( PARAM( 0 ) );

      if( target != NULL && user->self->boAuthGM == true )
      {
         unsigned int    iNbrHour = atoi(PARAM( 1 ));
         if( SUPER_USER || target->self->ViewFlag(__FLAG_JUST_DO_IT) == 666)//oki
         {
            CREATE_MESSAGE( "Sorry but you cannot add jail sanction to the super user!" );
            SEND_MESSAGE
         }
         else if(iNbrHour<1)
         {
            CREATE_MESSAGE( "Sorry but time must be grater than 0 nour..." );
            SEND_MESSAGE;
         }
         else
         {
            int iOldSanctionA = target->GetSanctionA();
            int iOldSanctionB = target->GetSanctionB();

            target->AddSanction(user->IsGod());

            int iSanctionA = target->GetSanctionA();
            int iSanctionB = target->GetSanctionB();
            _LOG_SANCTION
               LOG_ALWAYS,
               "God %s (%s) add jail sanction to %s's (%s)  for %d hour.User have SanctionA = %d and SanctionB = %d",
               (LPCTSTR)user->self->GetTrueName(),
               (LPCTSTR)user->GetFullAccountName(),
               (LPCTSTR)target->self->GetTrueName(),
               (LPCTSTR)target->GetFullAccountName(),iNbrHour,iSanctionA,iSanctionB
               LOG_
               _LOG_SANCTION
               LOG_ALWAYS,
               "   Reason : %s ",PARAM( 2 )
               LOG_


               if(iOldSanctionB == 0 && iSanctionB == 1)//ban 1 mois
               {
                  CString strCommandLockout;
                  strCommandLockout.Format("LOCKOUTTMP %s'S TIME %d", PARAM(0), 30);
                  VerifySysopCommand(user, strCommandLockout,false); //lockouttmp

                  target->self->SendSystemMessage(_STR(15330, target->self->GetLang()), CL_RED );

                  CREATE_MESSAGE( "Users %s get %d sanction(s) for first time, is ban for 30 days", PARAM( 0 ),g_iNbrSanctionBeforeLock);
                  SEND_MESSAGE;

                  _LOG_SANCTION
                     LOG_ALWAYS,
                     "   Is ban for 30 days...\n"
                     LOG_
               }
               else if(iOldSanctionB == 1 && iSanctionB == 2)//ban 3 mois
               {
                  CString strCommandLockout;
                  strCommandLockout.Format("LOCKOUTTMP %s'S TIME %d", PARAM(0), 90);
                  VerifySysopCommand(user, strCommandLockout,false); //lockouttmp

                  target->self->SendSystemMessage(_STR(15331, target->self->GetLang()), CL_RED );

                  CREATE_MESSAGE( "Users %s have %d sanction(s) for second time, is ban for 90 days", PARAM( 0 ),g_iNbrSanctionBeforeLock);
                  SEND_MESSAGE;

                  _LOG_SANCTION
                     LOG_ALWAYS,
                     "   Is ban for 90 days...\n"
                     LOG_
               }
               else if(iOldSanctionB == 2 && iSanctionB == 3)//ban a vie
               {
                  CString strCommandLockout;
                  strCommandLockout.Format("LOCKOUT %s", PARAM(0));
                  VerifySysopCommand(user, strCommandLockout); //lockout

                  target->self->SendSystemMessage(_STR(15332, target->self->GetLang()), CL_RED );

                  CREATE_MESSAGE( "Users %s have %d sanction(s) for thierd time, is ban for always", PARAM( 0 ),g_iNbrSanctionBeforeLock);
                  SEND_MESSAGE;

                  _LOG_SANCTION
                     LOG_ALWAYS,
                     "   Is ban from the game...\n"
                     LOG_
               }
               else
               {

                  target->Lock();

                  //manage la prison pour le user...
                  target->self->SetFlag(__FLAG_NMS_EN_PRISON,1);
                  WorldPos wlPos = { 2960, 352, 0 };
                  target->self->Teleport( wlPos, 0 );
                  
                  //Set le timestamp d'expiration de sa...
                  time_t TimeMsTmp =  time(NULL);
                  //on set le expire for this sanction 
                  DWORD dwExitjail = (DWORD)(TimeMsTmp+(iNbrHour*3600));
                  target->self->SetFlag(__FLAG_PRISON_TIMESTAMP,dwExitjail);
                  target->Unlock();


                  CString strMsgSanction;
                  strMsgSanction.Format(_STR(15328, target->self->GetLang()),iNbrHour,PARAM( 2 ));
                  target->self->SendSystemMessage(strMsgSanction, CL_RED );
                  target->self->SendSystemMessage(_STR(15327, target->self->GetLang()), CL_RED );
               }
         }
      }
      else
      {
         CREATE_MESSAGE( "Users %s is not online.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
   }

   // View the specific value of a flag on a player,
   COMMAND( "SET SANCTION $,$ TO $", GOD_CAN_GIVE_GOD_FLAGS )
   {
      Players *target = FindCharacter( PARAM( 2 ) );

      if( target != NULL )
      {
         int iSanctionOldA = target->GetSanctionA();
         int iSanctionOldB = target->GetSanctionB();
         target->SetSanction(atoi(PARAM( 0 )),atoi(PARAM( 1 )));
         int iSanctionA = target->GetSanctionA();
         int iSanctionB = target->GetSanctionB();

         if(iSanctionA >0)
         {
            time_t TimeMsTmp =  time(NULL);
            target->SetSanctionLastTS(TimeMsTmp+g_iDelayExpireSanction);
         }

         _LOG_SANCTION
            LOG_ALWAYS,
            "God %s (%s) SET sanction to %s's (%s)  BeforeA %d and  Now %d   BeforeB %d and Now %d",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName(),
            (LPCTSTR)target->self->GetTrueName(),
            (LPCTSTR)target->GetFullAccountName(),iSanctionOldA,iSanctionA,iSanctionOldB ,iSanctionB
            LOG_

         CREATE_MESSAGE( "%s's now have %d minor sanction(s) and %d major sanction(s)", PARAM( 2 ), iSanctionA,iSanctionB );
         SEND_MESSAGE;
      }
      else
      {
         CREATE_MESSAGE( "Users %s is not online.", PARAM( 1 ) );
         SEND_MESSAGE;
      }
   }


   // Sets the target player to full PVP.
   COMMAND( "ADD $ NMSGOLD TO $", GOD_CAN_GIVE_GOD_FLAGS )
   {
      Players *target = FindCharacter( PARAM( 1 ) );

      if( target != NULL )
      {
         success = true;

         int iNbrGoldToAdd = atoi( PARAM( 0 ) );
         int iNbrAvait     = target->GetNMSGold();
         int iNewTotal     = iNbrGoldToAdd+iNbrAvait;

         target->SetNMSGold(iNewTotal);
         target->SaveAccount();

         _LOG_ACHAT_NMS
            LOG_ALWAYS,
            "God %s (%s) edited %s's (%s) Add %d NMSGold. Now Player have a total of %d NMS Gold",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName(),
            (LPCTSTR)target->self->GetTrueName(),
            (LPCTSTR)target->GetFullAccountName(),
            iNbrGoldToAdd,
            iNewTotal
            LOG_
      }
      else
      {
         CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
   }

   COMMAND( "REMOVE $ NMSGOLD FROM $", GOD_CAN_GIVE_GOD_FLAGS )
   {
      Players *target = FindCharacter( PARAM( 1 ) );

      if( target != NULL )
      {
         success = true;

         int iNbrGoldToRem = atoi( PARAM( 0 ) );
         int iNbrAvait     = target->GetNMSGold();
         int iNewTotal     = iNbrAvait - iNbrGoldToRem;
         if(iNewTotal <0)
            iNewTotal = 0;

         target->SetNMSGold(iNewTotal);
         target->SaveAccount();

         _LOG_ACHAT_NMS
            LOG_ALWAYS,
            "God %s (%s) edited %s's (%s) Remove %d NMSGold. Now Player have a total of %d NMS Gold",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName(),
            (LPCTSTR)target->self->GetTrueName(),
            (LPCTSTR)target->GetFullAccountName(),
            iNbrGoldToRem,
            iNewTotal
            LOG_
      }
      else
      {
         CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
   }

   COMMAND( "NMSGREMORT EVIL LVL1 $ X $", GOD_CAN_GIVE_GOD_FLAGS )
   {
      Players *target = FindCharacter( PARAM( 0 ) );

      if( target != NULL )
      {
         int iNbrX = atoi( PARAM( 1 ) );
         success = true;

         int iFlagBegore = target->GetNMSGoldSELv1();
         target->SetNMSGoldSELv1(iNbrX);
         int iErr = theApp.NMSGOLD_Remort(target->self,1315, 920, 1 ,1,true);
         if(iErr == -1)
         {
            CREATE_MESSAGE( "Unable to process on this player...");
            SEND_MESSAGE;
         }
         else if(iErr == -2)
         {
            CREATE_MESSAGE( "This player dont have good alignement.");
            SEND_MESSAGE;
         }
         else
         {
            CREATE_MESSAGE( "Successfull");
            SEND_MESSAGE;
         }
         target->SetNMSGoldSELv1(iFlagBegore);
      }
      else
      {
         CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
   }

   COMMAND( "NMSGREMORT EVIL $ X $", GOD_CAN_GIVE_GOD_FLAGS )
   {
      Players *target = FindCharacter( PARAM( 0 ) );

      if( target != NULL )
      {
         int iNbrX = atoi( PARAM( 1 ) );
         success = true;


         int iFlagBegore = target->GetNMSGoldSE();
         target->SetNMSGoldSE(iNbrX);
         int iErr = theApp.NMSGOLD_Remort(target->self,1315, 920, 1 ,1,false);
         if(iErr == -1)
         {
            CREATE_MESSAGE( "Unable to process on this player...");
            SEND_MESSAGE;
         }
         else if(iErr == -2)
         {
            CREATE_MESSAGE( "This player dont have good alignement.");
            SEND_MESSAGE;
         }
         else
         {
            CREATE_MESSAGE( "Successfull");
            SEND_MESSAGE;
         }
         target->SetNMSGoldSE(iFlagBegore);
      }
      else
      {
         CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
   }

   COMMAND( "NMSGREMORT GOOD LVL1 $ X $", GOD_CAN_GIVE_GOD_FLAGS )
   {
      Players *target = FindCharacter( PARAM( 0 ) );

      if( target != NULL )
      {
         int iNbrX = atoi( PARAM( 1 ) );
         success = true;

         int iFlagBegore = target->GetNMSGoldSGLv1();
         target->SetNMSGoldSGLv1(iNbrX);
         int iErr = theApp.NMSGOLD_Remort(target->self,1315, 920, 1 ,0,true);
         if(iErr == -1)
         {
            CREATE_MESSAGE( "Unable to process on this player...");
            SEND_MESSAGE;
         }
         else if(iErr == -2)
         {
            CREATE_MESSAGE( "This player dont have good alignement.");
            SEND_MESSAGE;
         }
         else
         {
            CREATE_MESSAGE( "Successfull");
            SEND_MESSAGE;
         }
         target->SetNMSGoldSGLv1(iFlagBegore);

      }
      else
      {
         CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
   }

   COMMAND( "NMSGREMORT GOOD $ X $", GOD_CAN_GIVE_GOD_FLAGS )
   {
      Players *target = FindCharacter( PARAM( 0 ) );

      if( target != NULL )
      {
         int iNbrX = atoi( PARAM( 1 ) );
         success = true;

         int iFlagBegore = target->GetNMSGoldSG();
         target->SetNMSGoldSG(iNbrX);

         int iErr = theApp.NMSGOLD_Remort(target->self,1315, 920, 1 ,0,false);
         if(iErr == -1)
         {
            CREATE_MESSAGE( "Unable to process on this player...");
            SEND_MESSAGE;
         }
         else if(iErr == -2)
         {

            CREATE_MESSAGE( "This player dont have good alignement.");
            SEND_MESSAGE;
         }
         else
         {
            CREATE_MESSAGE( "Successfull");
            SEND_MESSAGE;
         }
         target->SetNMSGoldSG(iFlagBegore);
      }
      else
      {
         CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
   }

   COMMAND( "NMSGREROLL $", GOD_CAN_GIVE_GOD_FLAGS )
   {
      Players *target = FindCharacter( PARAM( 0 ) );

      if( target != NULL )
      {
         success = true;


         int iFlagBegore = target->GetNMSGoldReroll();
         target->SetNMSGoldReroll(1);
         int iErr = theApp.NMSGOLD_Reroll(target->self,1315, 920, 1);
         if(iErr == -1)
         {
            CREATE_MESSAGE( "Unable to process on this player...");
            SEND_MESSAGE;
         }
         else
         {
            CREATE_MESSAGE( "Successfull");
            SEND_MESSAGE;
         }
         target->SetNMSGoldReroll(iFlagBegore);
      }
      else
      {
         CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
   }

   COMMAND( "NMSGTODECHU $", GOD_CAN_GIVE_GOD_FLAGS )
   {
      Players *target = FindCharacter( PARAM( 0 ) );

      if( target != NULL )
      {
         success = true;

         int iFlagBegore = target->GetNMSGoldToD();
         target->SetNMSGoldToD(1);
         int iErr = theApp.NMSGOLD_PassageDechu(target->self,1315, 920, 1);
         if(iErr == -1)
         {
            CREATE_MESSAGE( "Unable to process on this player...");
            SEND_MESSAGE;
         }
         if(iErr == -2)
         {
            CREATE_MESSAGE( "Already Dechu...");
            SEND_MESSAGE;
         }
         else
         {
            CREATE_MESSAGE( "Successfull");
            SEND_MESSAGE;
         }
         target->SetNMSGoldToD(iFlagBegore);
      }
      else
      {
         CREATE_MESSAGE( "User %s is not online.", PARAM( 0 ) );
         SEND_MESSAGE;
      }
   }

   // View the specific value of a flag on a player,
   COMMAND( "SETGLOBALFLAG $ TO $", GOD_CAN_GIVE_FLAG_TO_HIM)
   {
      
      GiveGlobalFlag(  atoi(PARAM( 0 )), atoi(PARAM( 1 )) );
      CREATE_MESSAGE( "You set global flag %d to %d", atoi(PARAM( 0 )), atoi(PARAM( 1 )));
      SEND_MESSAGE;
   }

   // View the specific value of a flag on a player,
   COMMAND( "VIEWGLOBALFLAG $", GOD_CAN_GIVE_FLAG_TO_HIM)
   {
      CREATE_MESSAGE( "Global flag %d = %d", atoi(PARAM( 0 )),CheckGlobalFlag(atoi(PARAM( 0 )) ));
      SEND_MESSAGE;
   }

   // View the specific value of a flag on a player,
   COMMAND( "NMS COLISEUM $ ON", GOD_CAN_GIVE_FLAG_TO_HIM)
   {
      int iCocoNbr = atoi(PARAM( 0 ));
      if(iCocoNbr == 2 || iCocoNbr == 3 || iCocoNbr == 4)
      {
         if(iCocoNbr == 2)
            theApp.dwGFEnableCoco2 = 1;
         if(iCocoNbr == 3)
            theApp.dwGFEnableCoco3 = 1;
         if(iCocoNbr == 4)
            theApp.dwGFEnableCoco4 = 1;

         CREATE_MESSAGE( "You successfully enable coliseum number %d.",iCocoNbr);
         SEND_MESSAGE;

         RegKeyHandler regKey; 
         if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
         {
            if(iCocoNbr == 2)
               regKey.WriteProfileInt( "NMSColiseum2", theApp.dwGFEnableCoco2);
            if(iCocoNbr == 3)
               regKey.WriteProfileInt( "NMSColiseum3", theApp.dwGFEnableCoco3);
            if(iCocoNbr == 4)
               regKey.WriteProfileInt( "NMSColiseum4", theApp.dwGFEnableCoco4);
         }

         CString strMsg;
         strMsg.Format(_STR(15320, IntlText::GetDefaultLng()),iCocoNbr);

         WorldPos wlPos = { 0, 0, 0 };
         Broadcast::BCServerMessage( wlPos, 0, strMsg, NULL, CL_RED );
      }
      else
      {
         CREATE_MESSAGE( "NMS Coliseum number %d dont exist... only COliseum 2-3-4 exist.", iCocoNbr);
         SEND_MESSAGE;
      }
   }

   // View the specific value of a flag on a player,
   COMMAND( "NMS COLISEUM $ OFF", GOD_CAN_GIVE_FLAG_TO_HIM)
   {
      int iCocoNbr = atoi(PARAM( 0 ));
      if(iCocoNbr == 2 || iCocoNbr == 3 || iCocoNbr == 4)
      {
         if(iCocoNbr == 2)
            theApp.dwGFEnableCoco2 = 0;
         if(iCocoNbr == 3)
            theApp.dwGFEnableCoco3 = 0;
         if(iCocoNbr == 4)
            theApp.dwGFEnableCoco4 = 0;

         CREATE_MESSAGE( "You successfully disable coliseum number %d.",iCocoNbr);
         SEND_MESSAGE;

         RegKeyHandler regKey; 
         if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY ) )
         {
            if(iCocoNbr == 2)
               regKey.WriteProfileInt( "NMSColiseum2", theApp.dwGFEnableCoco2);
            if(iCocoNbr == 3)
               regKey.WriteProfileInt( "NMSColiseum3", theApp.dwGFEnableCoco3);
            if(iCocoNbr == 4)
               regKey.WriteProfileInt( "NMSColiseum4", theApp.dwGFEnableCoco4);
         }

         CString strMsg;
         strMsg.Format(_STR(15321, IntlText::GetDefaultLng()),iCocoNbr);

         WorldPos wlPos = { 0, 0, 0 };
         Broadcast::BCServerMessage( wlPos, 0, strMsg, NULL, CL_RED );
      }
      else
      {
         CREATE_MESSAGE( "NMS Coliseum number %d dont exist... only COliseum 2-3-4 exist.", iCocoNbr);
         SEND_MESSAGE;
      }
   }


   COMMAND( "ACTIVATE SPELL $ ON $", GOD_CAN_EDIT_USER_SKILLS )
   {
      if( user->self->boAuthGM == false )
      {
         _LOG_GAMEOP
            LOG_CRIT_ERRORS,
            "God %s (%s) tried to activate spell %d on %s without authentification!",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName(),
            (LPCTSTR)PARAM( 0 ),
            (LPCTSTR)PARAM( 1 )
            LOG_			
      }

      WORD wSpellID = atoi( PARAM( 0 ) );
      if( wSpellID != 0 )
      {
         Players *target = FindCharacter( PARAM( 1 ) );
         if( target != NULL && user->self->boAuthGM == true )
         {

            Character *ch = static_cast< Character * >( target->self );
            if(ch)
            {
               ch->CastSpellNoCheckFull(wSpellID,ch);
            }
            success = true;

            _LOG_GAMEOP
               LOG_SYSOP,
               "God %s (%s) activate %s's (%s) spell %u.",
               (LPCTSTR)user->self->GetTrueName(),
               (LPCTSTR)user->GetFullAccountName(),
               (LPCTSTR)target->self->GetTrueName(),
               (LPCTSTR)target->GetFullAccountName(),
               wSpellID
               LOG_
         }
         else
         {
            CREATE_MESSAGE( "User %s is not online.", PARAM( 1 ) );
            SEND_MESSAGE;
         }
      }
      else
      {
         CREATE_MESSAGE( "Spell ID isn't valid" );
         SEND_MESSAGE;
      }
   }

   COMMAND( "DISPELL SPELL $ FROM $", GOD_CAN_EDIT_USER_SKILLS )
   {
      if( user->self->boAuthGM == false )
      {
         _LOG_GAMEOP
            LOG_CRIT_ERRORS,
            "God %s (%s) tried to dispell spell %d on %s without authentification!",
            (LPCTSTR)user->self->GetTrueName(),
            (LPCTSTR)user->GetFullAccountName(),
            (LPCTSTR)PARAM( 0 ),
            (LPCTSTR)PARAM( 1 )
            LOG_			
      }

      WORD wSpellID = atoi( PARAM( 0 ) );
      if( wSpellID != 0 )
      {
         Players *target = FindCharacter( PARAM( 1 ) );
         if( target != NULL && user->self->boAuthGM == true )
         {

            target->self->DispellEffects(user->self,target->self,wSpellID);

            success = true;

            _LOG_GAMEOP
               LOG_SYSOP,
               "God %s (%s) dispell %s's (%s) spell %u.",
               (LPCTSTR)user->self->GetTrueName(),
               (LPCTSTR)user->GetFullAccountName(),
               (LPCTSTR)target->self->GetTrueName(),
               (LPCTSTR)target->GetFullAccountName(),
               wSpellID
               LOG_
         }
         else
         {
            CREATE_MESSAGE( "User %s is not online.", PARAM( 1 ) );
            SEND_MESSAGE;
         }
      }
      else
      {
         CREATE_MESSAGE( "Spell ID isn't valid" );
         SEND_MESSAGE;
      }
   }


   END_COMMAND


   if( success )
   {
      CREATE_MESSAGE( "Done." );
      SEND_MESSAGE;
   }

   return boFound;
}

/******************************************************************************/
namespace
/******************************************************************************/
{
	/******************************************************************************/
	//  This function is called to set name asynchronously.
	void AsyncSetName(LPVOID lpData) // Contains the IP of the gameop and the IP of the target.
	/******************************************************************************/
	{
		auto_ptr< SetNameData > nameData( reinterpret_cast< SetNameData * >( lpData ) );

		// Fetch the target of the command.
		Players *lpTarget = NULL;

		// Try 10 times, 250ms to lock the player.
		int d;
		for( d = 0; d < 10; d++ )
		{
			lpTarget = CPlayerManager::GetPlayerResourceFct( nameData->sockTarget );
			if( lpTarget != NULL )
			{
				break;
			}
			Sleep( 25 );
		}

		// If the target exists.
		if( lpTarget != NULL )
		{
			std::string originalName =  lpTarget->self->GetName( _DEFAULT_LNG );
			std::string newName = nameData->csNewName;
			bool printGameopLog = false;

			// Create a Format object.
			TFormat format;
			string csCompletionMessage;

			// Get an ODBC connection
			cODBCMage *lpDB = Character::GetODBC();

			// Lock connection
			lpDB->Lock();

			string csQuery = format( "SELECT PlayerName FROM PlayingCharacters WHERE PlayerName='%s'", nameData->csNewName.c_str());
			lpDB->SendRequest( csQuery.c_str() );        

			// If the new name could NOT be fetched, then allow name change (no duplicates).
			if( !lpDB->Fetch() )
			{
				printGameopLog = true;

				// Set the player's name.
				lpTarget->self->SetName( nameData->csNewName.c_str() );
				csCompletionMessage = format("Name changed to %s.",nameData->csNewName.c_str());

				// Update the OnlineUsers table.
				lpDB->Cancel();
				if(lpDB->SendRequest(format("UPDATE OnlineUsers SET PlayerName='%s' WHERE AccountName='%s'",nameData->csNewName.c_str(),(LPCTSTR)lpTarget->GetAccount())))
               lpDB->Commit();

            // Save player data (commit name change). Allows the name to be reverted on subsequent name changes.
            lpTarget->ForceSave();  //AsynsSet name GM Command rename


            theApp.AddGuildRequest(NULL,NULL,NULL,GUILD_UPDATE_OFFLINE_NAME,lpTarget->self->GetID(),0,0,0,lpTarget->self->GetTrueName(),"");

            CString strName;
            strName.Format("%s",nameData->csNewName.c_str());
            TFCPacket sending;
            sending << (RQ_SIZE)RQ_NameChange;
            sending << strName;
            lpTarget->self->SendPlayerMessage( sending );

         }
			// Otherwise warn the god.
			else
			{           
				csCompletionMessage = format( "The name %s already exists. Name change couldn't be completed.",nameData->csNewName.c_str());
			}
			lpDB->Cancel();
			lpDB->Unlock();

			// Free the player resource.
			CPlayerManager::FreePlayerResourceFct( lpTarget );

			// Get the god's player resource
			Players *lpGod = CPlayerManager::GetPlayerResourceFct( nameData->sockGod );

			// If the god is still online.
			if( lpGod != NULL )
			{
				if( printGameopLog )
				{   
					_LOG_GAMEOP
						LOG_SYSOP,
						"God %s (%s) changed player %s (%s)'s name to %s.",
						(LPCTSTR)lpGod->self->GetName( _DEFAULT_LNG ),
						(LPCTSTR)lpGod->GetFullAccountName(),
						originalName.c_str(),
						(LPCTSTR)lpTarget->GetFullAccountName(),
						newName.c_str()
					LOG_
				}

				// Notify the god.
				lpGod->self->SendSystemMessage( csCompletionMessage.c_str() );

				// Free the player's resource.
				CPlayerManager::FreePlayerResourceFct( lpGod );
			}
			else
			{
				if( printGameopLog )
				{                
					_LOG_GAMEOP 
						LOG_SYSOP,
						"Logged off god changed player %s (%s)'s name to %s.",
						originalName.c_str(),
						(LPCTSTR)lpTarget->GetFullAccountName(),
						newName.c_str()
					LOG_
				}
			}
		}
	}
	/******************************************************************************/
	CString GetOPFlagNameByShortcut(CString csFlagShortCut) 
	/******************************************************************************/
	{
		if (csFlagShortCut.CompareNoCase("noclip") == 0) return "GOD_NO_CLIP";
		if (csFlagShortCut.CompareNoCase("nomonsters") == 0) return "GOD_NO_MONSTERS";
		if (csFlagShortCut.CompareNoCase("teleport") == 0) return "GOD_CAN_TELEPORT";
		if (csFlagShortCut.CompareNoCase("teleportuser") == 0) return "GOD_CAN_TELEPORT_USER";
		if (csFlagShortCut.CompareNoCase("zap") == 0) return "GOD_CAN_ZAP";
		if (csFlagShortCut.CompareNoCase("squelch") == 0) return "GOD_CAN_SQUELCH";
		if (csFlagShortCut.CompareNoCase("removeshouts") == 0) return "GOD_CAN_REMOVE_SHOUTS";
		if (csFlagShortCut.CompareNoCase("summonmonsters") == 0) return "GOD_CAN_SUMMON_MONSTERS";
		if (csFlagShortCut.CompareNoCase("summonitems") == 0) return "GOD_CAN_SUMMON_ITEMS";
		if (csFlagShortCut.CompareNoCase("setuserflag") == 0) return "GOD_CAN_SET_USER_FLAG";
		if (csFlagShortCut.CompareNoCase("edituser") == 0) return "GOD_CAN_EDIT_USER";
		if (csFlagShortCut.CompareNoCase("edituserstat") == 0) return "GOD_CAN_EDIT_USER_STAT";
		if (csFlagShortCut.CompareNoCase("edituserhp") == 0) return "GOD_CAN_EDIT_USER_HP";
		if (csFlagShortCut.CompareNoCase("editusermp") == 0) return "GOD_CAN_EDIT_USER_MANA";
		if (csFlagShortCut.CompareNoCase("edituserxplevel") == 0) return "GOD_CAN_EDIT_USER_XP_LEVEL";
		if (csFlagShortCut.CompareNoCase("editusername") == 0) return "GOD_CAN_EDIT_USER_NAME";
		if (csFlagShortCut.CompareNoCase("edituserappearance") == 0) return "GOD_CAN_EDIT_USER_APPEARANCE_CORPSE";
		if (csFlagShortCut.CompareNoCase("edituserspells") == 0) return "GOD_CAN_EDIT_USER_SPELLS";
		if (csFlagShortCut.CompareNoCase("edituserskills") == 0) return "GOD_CAN_EDIT_USER_SKILLS";
		if (csFlagShortCut.CompareNoCase("edituserbackpack") == 0) return "GOD_CAN_EDIT_USER_BACKPACK";
		if (csFlagShortCut.CompareNoCase("viewuserstat") == 0) return "GOD_CAN_VIEW_USER_STAT";
		if (csFlagShortCut.CompareNoCase("viewuserbackpack") == 0) return "GOD_CAN_VIEW_USER_BACKPACK";
		if (csFlagShortCut.CompareNoCase("viewuserspells") == 0) return "GOD_CAN_VIEW_USER_SPELLS";
		if (csFlagShortCut.CompareNoCase("viewuserskills") == 0) return "GOD_CAN_VIEW_USER_SKILLS";
		if (csFlagShortCut.CompareNoCase("viewuserappearance") == 0) return "GOD_CAN_VIEW_USER_APPEARANCE_CORPSE";
		if (csFlagShortCut.CompareNoCase("lockout") == 0) return "GOD_CAN_LOCKOUT_USER";
		if (csFlagShortCut.CompareNoCase("slay") == 0) return "GOD_CAN_SLAY_USER";
		if (csFlagShortCut.CompareNoCase("emulatemonster") == 0) return "GOD_CAN_EMULATE_MONSTER";
		if (csFlagShortCut.CompareNoCase("invincible") == 0) return "GOD_INVINCIBLE";
		if (csFlagShortCut.CompareNoCase("developper") == 0) return "GOD_DEVELOPPER";
		if (csFlagShortCut.CompareNoCase("shutdown") == 0) return "GOD_CAN_SHUTDOWN";
		if (csFlagShortCut.CompareNoCase("seeaccounts") == 0) return "GOD_CAN_SEE_ACCOUNTS";
		if (csFlagShortCut.CompareNoCase("givegodflags") == 0) return "GOD_CAN_GIVE_GOD_FLAGS";
		if (csFlagShortCut.CompareNoCase("unlimitedshouts") == 0) return "GOD_UNLIMITED_SHOUTS";
		if (csFlagShortCut.CompareNoCase("trueinvisibility") == 0) return "GOD_TRUE_INVISIBILITY";
		if (csFlagShortCut.CompareNoCase("emulatesystem") == 0) return "GOD_CAN_EMULATE_SYSTEM";
		if (csFlagShortCut.CompareNoCase("chatmaster") == 0) return "GOD_CHAT_MASTER";
		if (csFlagShortCut.CompareNoCase("cannotdie") == 0) return "GOD_CANNOT_DIE";
		if (csFlagShortCut.CompareNoCase("runscripts") == 0) return "GOD_CAN_RUN_CLIENT_SCRIPTS";
      if (csFlagShortCut.CompareNoCase("seeall") == 0) return "GOD_SEE_ALL";
      if (csFlagShortCut.CompareNoCase("changesettings") == 0) return "GOD_CAN_CHANGE_SETTINGS";
      
		return csFlagShortCut;
	}
};//namespace