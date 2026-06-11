/******************************************************************************
Modify for vs2008 (30/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "FormatPlayerName.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
void FormatPlayerName::Format( CString *name )
/******************************************************************************/
{
    ASSERT( name != NULL );
    if( name == NULL )
	{
        return;
    }

    name->TrimLeft();
    name->TrimRight();

    CString newStr;
    
    int i;
    int len = name->GetLength();
    int nbSpaces = 0;
    for( i = 0; i < len; i++ )
	{
        TCHAR val = name->GetAt( i );
        if( val == ' ' )
		{
            nbSpaces++;
        }
		else
		{
            nbSpaces = 0;
        }
        if( nbSpaces < 2 )
		{
            newStr += val;
        }                       
    }

    *name = newStr;
}