/******************************************************************************
Modify for vs2008 (29/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "IntlText.h"
#include "T4CLog.h"
#include "RegKeyHandler.h"
#include "tfc_main.h"
#include "Random.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/******************************************************************************/
extern CTFCServerApp theApp;
extern Random rnd;
IntlText::LNG_STR  IntlText::lpLang[ NB_SUPPORTED_LNG ];
WORD               IntlText::wDefaultLng;
IntlText           IntlText::m_Instance;

/******************************************************************************/
#define SAME_STR( __str1, __str2 )  _stricmp( (LPCTSTR)__str1, (LPCTSTR)__str2 ) == 0


/******************************************************************************/
//  Returns the next character in the parsed file.
int IntlText::GetChar( FILE *pFile )
/******************************************************************************/
{
   int ch = fgetc( pFile );
   if( ch == EOF )
   {
      return EOF;
   }
   return (char)ch;
}
/******************************************************************************/
//  Seek the parse one step before current.
void IntlText::SeekOneBefore( FILE *pFile )
/******************************************************************************/
{
   fseek( pFile, -1, SEEK_CUR );
}
/******************************************************************************/
//  Seek the parse one step before end.
void IntlText::SeekOneBeforeEnd( FILE *pFile )
/******************************************************************************/
{
   fseek( pFile, -1, SEEK_END );
}
/******************************************************************************/
// Initializes the string buffer.
void IntlText::Create( void )
/******************************************************************************/
{
   int i;
   // Sets all languages to NULL
   for( i = 0; i < NB_SUPPORTED_LNG; i++ )
   {
      lpLang[ i ].lpcsStrings = NULL;
   }

   wDefaultLng = 0xFFFF;   // No default language.
}
/******************************************************************************/
// Frees string resources
void IntlText::Destroy( void )
/******************************************************************************/
{
   int i;
   // Unmaps all views
   for( i = 0; i < NB_SUPPORTED_LNG; i++ )
   {
      if( lpLang[ i ].lpcsStrings != NULL )
      {
         delete [] lpLang[ i ].lpcsStrings;
         lpLang[ i ].lpcsStrings = NULL;
      }        
   }
}
/******************************************************************************/
BOOL IntlText::ToChar(
                      FILE *pFile,
                      char ch,       // Char to advance to.
                      const char *no,// Chars which shouldn't be encountered when search for ch.
                      WORD &wLine    // Line count for debugging.
                      )
                      /******************************************************************************/
{    
   char cCh;
   if( strchr( no, '\1' ) )
   {
      cCh = GetChar(pFile);        
      if( cCh == '\n' ) wLine++;
      while( cCh != EOF && cCh != ch && ( cCh == ' ' || cCh == '\r' || cCh == '\n' || cCh == '\t' ) )
      {
         cCh = GetChar(pFile);
         if( cCh == '\n' ) wLine++;            
      }

      if( cCh == EOF || ( cCh != ' ' && cCh != '\r' && cCh != '\n' && cCh != '\t' ) && cCh != ch )
      {
         return FALSE;
      }
   }
   else
   {
      cCh = GetChar(pFile);
      if( cCh == '\n' ) wLine++;

      //TRACE( "%u,", cCh );
      while( cCh != EOF && cCh != ch && strchr( no, cCh ) == NULL )
      {
         cCh = GetChar(pFile);     
         if( cCh == '\n' ) wLine++;
      }
   }    
   if( cCh == EOF /*|| strchr( no, cCh ) != NULL */)
   {
      return FALSE;
   }

   return TRUE;
}
/******************************************************************************/
// Fetches the next number in the specified file.
BOOL IntlText::FetchNumber(
                           FILE *pFile,
                           DWORD &dwNumber,   // DWORD in which to put number.
                           WORD &wLine        // Line count for debugging.
                           )
                           /******************************************************************************/
{    
   char ch = GetChar(pFile);
   if( ch == '\n' ) wLine++;
   // Eleminate white spaces.
   while( ch != EOF && ch == ' ' )
   {
      ch = GetChar(pFile);
      if( ch == '\n' ) wLine++;
   }

   dwNumber = 0;
   while( ch != EOF && isdigit( ch ) )
   {
      dwNumber *= 10;
      dwNumber += ch - '0';
      ch = GetChar(pFile);
      if( ch == '\n' ) wLine++;
   }

   // Reposition to last char.
   SeekOneBefore(pFile);
   if( ch == EOF )
   {
      return FALSE;
   }

   return TRUE;
}
/******************************************************************************/
// Fetches the next string in the file
BOOL IntlText::FetchString(
                           FILE *pFile,
                           CString &csString, // The string which will contain the string.
                           WORD &wLine        // Line count for debugging.
                           )
                           /******************************************************************************/
{
   BOOL boError;
   BOOL boEndString;    
   BOOL boEndChunck;
   char ch;    

   boError = FALSE;
   boEndString = FALSE;

   CString csChunck;

   while( !boEndString )
   {
      csChunck = "";

      boEndChunck = FALSE;

      // Fetch chunck        
      while( !boEndChunck )
      {
         ch = GetChar(pFile);
         if( ch == '\n' ) wLine++;

         switch( ch )
         {
         case EOF:
            boError = TRUE;
            boEndChunck = TRUE;
            break;
         case '"':
            // If "" paste a single "
            if( GetChar(pFile) == '"' )
            {
               csChunck += '"';
            }
            // Otherwise end chunck.
            else
            {
               boEndChunck = TRUE;
            }
            break;
         default:
            // Any other char should be added to chunck
            csChunck += ch;           
            break;
         }                      
      }

      if( ch != EOF )
      {
         // Try to find another chunck
         ch = GetChar(pFile);
         if( ch == '\n' ) wLine++;
         while( ch == ' ' || ch == '\r' || ch == '\n' )
         {
            ch = GetChar(pFile);
            if( ch == '\n' ) wLine++;
         }

         // Is ch the beginning of a new chunck?
         if( ch != '"' )
         {
            if( ch == EOF )
            {
               SeekOneBeforeEnd(pFile);
            }
            // If not, end string
            boEndString = TRUE;
         }

         // Add chuck to string
         csString += csChunck;
      }
      else
      {
         boError = TRUE;
      }
   }

   return !boError;
}
/******************************************************************************/
// Parses the language file and loads all its strings.
BOOL IntlText::ParseLngFile( 
                            FILE *pFile,
                            WORD wLangID,   // Language ID of the file being loaded.
                            TemplateList< STR_ID > &tlStrings
                            )
                            /******************************************************************************/
{
   int nError = 0;
   DWORD dwStringID;
   CString csString;
   WORD wLine = 1;

   // While it finds string ID blocks.
   while( ToChar( pFile, '(', ")\"", wLine ) && nError == 0 )
   {
      // Fetch string ID.
      if( FetchNumber( pFile, dwStringID, wLine ) )
      {

         // Make sure parenthesis is closed to avoid conflicts
         if( ToChar( pFile, ')', "(\"", wLine ) )
         {
            // Go to string definition
            if( ToChar( pFile, '"', "()\1", wLine ) )
            {
               csString = "";
               // Fetch string.
               if( FetchString( pFile, csString, wLine ) )
               {
                  // Add string to string to list.
                  STR_ID *lpStrID = new STR_ID;
                  lpStrID->csString = csString;
                  lpStrID->dwStringID = dwStringID;
                  tlStrings.AddToTail( lpStrID );
               }
               else
               {
                  nError = 4;
               }
            }
            else
            {
               nError = 3;
            }
         }
         else
         {
            nError = 2;
         }
      }
      else
      {
         nError = 1;
      }
   }

   if( nError != 0 )
   {
      CString csError;
      switch( nError )
      {
      case 1: csError.Format( "Language file error fetching string ID on line %u.", wLine ); break;
      case 2: csError.Format( "Language file error: closing parenthesis not found on line %u.", wLine ); break;
      case 3: csError.Format( "Language file error trying to find string on line %u.", wLine ); break;
      case 4: csError.Format( "Language file error fetching string on line %u.", wLine ); break;
      default: csError.Format( "An unkown language file error occured on line %u.", wLine ); break;
      }
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         (char *)(LPCTSTR)csError
         LOG_

         return FALSE;
   }

   return TRUE;
}
/******************************************************************************/
// Loads a language into the database
void IntlText::LoadLngDB(CString csFile) // File containing the language strings.
/******************************************************************************/
{
   BOOL boLoaded = FALSE;
   CString csLoadedFile;
   WORD wLangID;
   FILE *fFile;

   // If file could not be opened.
   csLoadedFile = csFile;
   fopen_s(&fFile, (LPCTSTR)csLoadedFile, "rb" );

   if( fFile == NULL )
   {
      // Try adding the server's binary path.
      csLoadedFile = theApp.sPaths.csBinaryPath;
      csLoadedFile += csFile;
      TRACE( "Trying %s.", (LPCTSTR)csLoadedFile );
      fopen_s(&fFile, (LPCTSTR)csLoadedFile, "rb" );  
      // If it again failed to open.
      if( fFile == NULL )
      {
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
           "Could not load language file %s.",
            (LPCTSTR)csFile
            LOG_
            // Nothing else to do here.
            return;
      }
   }

   BOOL boError;
   CString csError;
   CString csLang;
   CString csDefault;
   bool boDefault = false;

   char ch;
   ch = GetChar(fFile);
   while( ch != EOF && ch == ' ' )
   {
      ch = GetChar(fFile);
   }

   while( ch != EOF && isalpha( ch ) )
   {
      csLang += ch;
      ch = GetChar(fFile);
   }

   wLangID = 0xFFFF;
   if( SAME_STR( csLang, "french" ) || SAME_STR( csLang, "francais" ) )       wLangID = LNG_FRENCH;
   else if( SAME_STR( csLang, "english" ) )											   wLangID = LNG_ENGLISH;
   else if( SAME_STR( csLang, "italian" ) || SAME_STR( csLang, "italiano" ) )	wLangID = LNG_ITALIAN;
   else if( SAME_STR( csLang, "portugese" ) )							            wLangID = LNG_PORTUGESE;
   else if( SAME_STR( csLang, "spanish" ) || SAME_STR( csLang, "espanol" ) )	wLangID = LNG_SPANISH;
   else if( SAME_STR( csLang, "german" ) || SAME_STR( csLang, "deutsh" ) )    wLangID = LNG_GERMAN;
   else if( SAME_STR( csLang, "korean" ) )								            wLangID = LNG_KOREAN;

   // Fetch the word default.
   ch = GetChar(fFile);
   while( ch != EOF && ch == ' ' )
   {
      ch = GetChar(fFile);
   }

   while( ch != EOF && isalpha( ch ) )
   {
      csDefault += ch;
      ch = GetChar(fFile);
   }

   // If it was found, set this language to default.
   if( SAME_STR( csDefault, "default" ) )
   {
      boDefault = true;
   }

   if( wLangID != 0xFFFF )
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL3,
         "Loading language %s, file %s, lngID%u",
         (LPCTSTR)csLang,
         (LPCTSTR)csFile,
         wLangID
         LOG_

         if( wLangID < NB_SUPPORTED_LNG )
         {
            if( lpLang[ wLangID ].lpcsStrings == NULL )
            {
               if( boDefault )
               {
                  if( wDefaultLng != 0xFFFF )
                  {
                     _LOG_DEBUG
                        LOG_DEBUG_LVL2,
                        "A default language has already been set, language %s in file %s will not be default",
                        (LPCTSTR)csLang,
                        (LPCTSTR)csFile
                        LOG_
                  }
                  else
                  {
                     wDefaultLng = wLangID;

                     _LOG_DEBUG
                        LOG_DEBUG_LVL2,
                        "Setting default language to %s.",
                        (LPCTSTR)csLang
                        LOG_
                  }
               }

               TemplateList < STR_ID > tlStrings;
               if( ParseLngFile( fFile, wLangID, tlStrings ) )
               {
                  TRACE( "\r\nFound %u strings!", tlStrings.NbObjects() );

                  // Find highest string ID.
                  DWORD dwHighestID = 0;
                  tlStrings.ToHead();
                  while( tlStrings.QueryNext() )
                  {
                     if( tlStrings.Object()->dwStringID > dwHighestID )
                     {
                        dwHighestID = tlStrings.Object()->dwStringID;
                     }
                  }

                  // Allocate space for that much strings for that language.
                  lpLang[ wLangID ].lpcsStrings = new CString[ dwHighestID + 1 ];
                  lpLang[ wLangID ].dwMaxStrings = dwHighestID + 1;

                  // Setup the strings
                  tlStrings.ToHead();
                  while( tlStrings.QueryNext() )
                  {
                     STR_ID *lpStrID = tlStrings.Object();

                     if( lpLang[ wLangID ].lpcsStrings[ lpStrID->dwStringID ].IsEmpty() )
                     {
                        // Set string.
                        lpLang[ wLangID ].lpcsStrings[ lpStrID->dwStringID ] = lpStrID->csString;
                     }
                     else
                     {
                        _LOG_DEBUG
                           LOG_DEBUG_LVL2,
                           "String ID %u in language file %s has already been defined. Using only the first definition.",
                           lpStrID->dwStringID,
                           (LPCTSTR)csLoadedFile
                           LOG_
                     }
                  }
               }
               else
               {
                  boError = TRUE;
                  csError.Format( "Error parsing language file %s. Language will be invalid.", (LPCTSTR)csLoadedFile );
               }

               tlStrings.AnnihilateList();
            }
            else
            {
               boError = TRUE;
               csError.Format( "Language %s in file %s has already been loaded.", (LPCTSTR)csLang, (LPCTSTR)csLoadedFile );
            }
         }
         else
         {
            boError = TRUE;
            csError.Format( "Unsupported language %s in file %s .", (LPCTSTR)csLang, (LPCTSTR)csLoadedFile );
         }
   }
   else
   {
      boError = TRUE;
      csError.Format( "Unsupported language %s in file %s .", (LPCTSTR)csLang, (LPCTSTR)csLoadedFile );
   }

   if( boError )
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL2,
         (char *)(LPCTSTR)csError
         LOG_
   }
   else
   {
      _LOG_DEBUG
         LOG_DEBUG_LVL1,
         "Loading language file %s .", 
         (LPCTSTR)csLoadedFile
         LOG_
   }

   fclose( fFile );        
}
/******************************************************************************/
// Verifies if at least one language was loaded and set the default language.
BOOL IntlText::IsLngOK( void )
/******************************************************************************/
{    
   BOOL boOK = FALSE;
   int i;
   for( i = 0; i < NB_SUPPORTED_LNG; i++ )
   {
      // If language exists
      if( lpLang[ i ].lpcsStrings != NULL )
      {
         // If no default language.
         if( wDefaultLng == 0xFFFF )
         {
            // Set default language
            wDefaultLng = i;

            _LOG_DEBUG
               LOG_DEBUG_LVL2,
               "Setting language ID%u as default language.",
               i
               LOG_
         }
         boOK = TRUE;
      }
   }
   return boOK;
}
/******************************************************************************/
// Ajout du système multilingue.
// Vérifie si la langue choisie par l'utilisateur et valide sinon on prend
// celle par defaut.
bool IntlText::CheckLng( WORD wLanguage )
/******************************************************************************/
{
   if ( (wLanguage < NB_SUPPORTED_LNG) && (lpLang[ wLanguage ].lpcsStrings != NULL) ) 
   { 
      return true;
   }
   return false;
}
/******************************************************************************/
// Returns the string defined by dwResource in the desired language
CString &IntlText::GetString(
                             DWORD dwResource,  // String resource
                             WORD wLanguage,    // Language ID
                             CString &csSource  // Source string in which text will be put.
                             )
                             /******************************************************************************/
{    
   // If language is supported
   if( wLanguage < NB_SUPPORTED_LNG )
   {
      // If language wasn't loaded
      if( lpLang[ wLanguage ].lpcsStrings == NULL )
      {
         // Use default language
         wLanguage = wDefaultLng;
      }
   }
   else
   {
      // Use default language
      wLanguage = wDefaultLng;
   }
   // If dwResource is lower then the highest string ID.
   if( dwResource < lpLang[ wLanguage ].dwMaxStrings )
   {
      csSource = lpLang[ wLanguage ].lpcsStrings[ dwResource ];
   }
   else
   {
      csSource = "";
   }

   return csSource;
}
/******************************************************************************/
// Appends a resource string into an existing CString.
CString &IntlText::AppendString(
                                DWORD dwResource,  // The resource string ID to load.
                                WORD wLanguage,    // The language from which to load the string.
                                CString &csSource  // The original CString;
                                )
                                /******************************************************************************/
{
   // If language is supported
   if( wLanguage < NB_SUPPORTED_LNG )
   {
      // If language wasn't loaded
      if( lpLang[ wLanguage ].lpcsStrings == NULL )
      {
         // Use default language
         wLanguage = wDefaultLng;
      }
   }
   else
   {
      // Use default language
      wLanguage = wDefaultLng;
   }
   // If dwResource is lower then the highest string ID.
   if( dwResource < lpLang[ wLanguage ].dwMaxStrings )
   {
      csSource += lpLang[ wLanguage ].lpcsStrings[ dwResource ];
   }

   return csSource;
}
/******************************************************************************/
// Returns the string buffer of the resource.
LPCTSTR IntlText::GetString(
                            DWORD dwResource,  // The resource ID
                            WORD wLanguage,     // The language
                            const char *szDefault
                            )
                            /******************************************************************************/
{
   LPCTSTR lpszStr = NULL;

   if( dwResource == 0 )
   {
      return szDefault;
   }

   // If language is supported
   if( wLanguage < NB_SUPPORTED_LNG )
   {
      // If language wasn't loaded
      if( lpLang[ wLanguage ].lpcsStrings == NULL )
      {
         // Use default language
         wLanguage = wDefaultLng;
      }
   }
   else
   {
      // Use default language
      wLanguage = wDefaultLng;
   }
   // If dwResource is lower then the highest string ID.
   if( dwResource < lpLang[ wLanguage ].dwMaxStrings )
   {
      lpszStr = (LPCTSTR)lpLang[ wLanguage ].lpcsStrings[ dwResource ];
   }
   else
   {
      lpszStr = szDefault;
   }

   return lpszStr;
}
/******************************************************************************/
// Returns the default server language.
WORD IntlText::GetDefaultLng( void )
/******************************************************************************/
{
   return wDefaultLng;
}
/******************************************************************************/
// Sends a server message to a player.
void IntlText::SendPlayerMessage(
                                 Unit *lpUnit,      // The Unit structure to send message to.
                                 DWORD dwID         // ID of the message to send.
                                 )
                                 /******************************************************************************/
{
   if( lpUnit != NULL )
   {
      TFCPacket sending;
      CString csMessage;

      csMessage = GetString( dwID, lpUnit->GetLang(), csMessage );

      sending << (RQ_SIZE)RQ_ServerMessage;
      sending << (short)30;
      sending << (short)3;
      sending << csMessage;
      sending << (long)CL_BLUE_LIGHT;
      lpUnit->SendPlayerMessage( sending );
   }
}
/******************************************************************************/
// Parses an 'intlstring' (in the form of: [id]default_text ) and returns a valid string.  
const char *IntlText::ParseString(
                                  const char *szString,  // The intl string.
                                  WORD wLanguage        // The language in which to search the parsed string ID from. 
                                  )
                                  /******************************************************************************/
{
   if( szString == NULL )
   {
      return "";
   }

   int  nStrLen = strlen( szString );
   const char *szText = szString;

   if( nStrLen > 2 )
   {
      // If this string contains a language ID.        
      if( szString[ 0 ] == '[' )
      {
         int i = 1;
         DWORD dwID = 0;
         bool boDone = false;

         if( !isdigit( szString[ 1 ] ) )
         {
            boDone = true;
         }

         // Parse to get the ID.            
         while( !boDone )
         {
            dwID *= 10;
            dwID += szString[ i ] - '0';

            ++i;
            if( i < nStrLen )
            {
               if( !isdigit( szString[ i ] ) )
               {
                  boDone = true;            
               }
            }
            else
            {
               boDone = true;
            }                
         }            

         // If the ID corresponds to a set language string.
         const char *szNameID = GetString( dwID, wLanguage );
         if( szNameID[ 0 ] != '\0' )
         {
            // Returns the string found in the language file.
            return szNameID;
         }
         // Otherwise increment szName to remove the [id] header.
         if( ++i < nStrLen )
         {
            // Returns the string's default value.
            szText = &szString[ i ];
         }
         else
         {
            // Otherwise return a null string.
            return "";
         }
      }
   }

   return szText;
}