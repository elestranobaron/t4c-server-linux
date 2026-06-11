/******************************************************************************
Modify for vs2008 (29/04/2009)
/******************************************************************************/
#if !defined(AFX_INTLTEXT_H__46FDF684_11B4_11D2_836E_00E02922FA40__INCLUDED_)
#define AFX_INTLTEXT_H__46FDF684_11B4_11D2_836E_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "Unit.h"
#include "GenRef.h"
#include "Random.h"

/******************************************************************************/
#define NB_SUPPORTED_LNG    7
#define LNG_FRENCH          0
#define LNG_ENGLISH         1
#define LNG_ITALIAN         2
#define LNG_PORTUGESE       3
#define LNG_SPANISH         4
#define LNG_GERMAN          5
#define LNG_KOREAN          6

/******************************************************************************/
class __declspec( dllexport ) IntlText  
/******************************************************************************/
{
public:        
	~IntlText( void )
	{
		IntlText::Destroy();
    }
    static void LoadLngDB( CString csFile );
	static BOOL IsLngOK( void );            // Determines if at least one language is loaded!     
	static bool CheckLng( WORD wLanguage ); // Ajout du système multilingue
	
	// Returns a string given its ID and a language.
	static CString &GetString( DWORD dwID, WORD wLanguage, CString &csSource );
    static CString &AppendString( DWORD dwID, WORD wLanguage, CString &csSource );

    // Returns a string given its ID and a language.
    static const char *GetString( DWORD dwID, WORD wLanguage, const char *szDefault = "" );
    static WORD GetDefaultLng( void );
    static void SendPlayerMessage( Unit *lpUnit, DWORD dwID );

    // Parses an 'intlstring' (ie: [id]default ) and returns a valid string.    
    static const char *ParseString( const char *szString, WORD wLanguage );

private:
	IntlText( void )
	{
		IntlText::Create();
    };

    static void Create( void );
    static void Destroy( void );            // Frees the language resources.

    struct STR_ID
	{
        CString csString;
        DWORD dwStringID;
    };
    struct LNG_STR
	{
        CString *lpcsStrings;
        DWORD dwMaxStrings;
    };
    static int GetChar( FILE *pFile );
    static void SeekOneBefore( FILE *pFile );
    static void SeekOneBeforeEnd( FILE *pFile );

    static BOOL ParseLngFile( FILE *pFile, WORD wLangID, TemplateList <STR_ID> &tlStrings );
    static BOOL FetchNumber( FILE *pFile, DWORD &dwNum, WORD &wLine );
    static BOOL FetchString( FILE *pFile, CString &csString, WORD &wLine );
    static BOOL ToChar( FILE *pFile, char ch, const char *invalid, WORD &wLine );

    static WORD wDefaultLng;
    static LNG_STR lpLang[ NB_SUPPORTED_LNG ];  
    
    static IntlText m_Instance;
};

/******************************************************************************/
#define _DEFAULT_LNG                            IntlText::GetDefaultLng()
#define _STR( __resource, __lng )               IntlText::GetString( __resource, __lng )
#define _TELL_PLAYER( __player, __resource )    IntlText::SendPlayerMessage( __player, __resource );
#define _DEFAULT_STR( __resource )              IntlText::GetString( __resource, _DEFAULT_LNG )

#endif // !defined(AFX_INTLTEXT_H__46FDF684_11B4_11D2_836E_00E02922FA40__INCLUDED_)
