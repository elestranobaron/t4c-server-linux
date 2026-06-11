/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "Sign.h"

/******************************************************************************/
Sign::Sign()
/******************************************************************************/
{}
/******************************************************************************/
Sign::~Sign()
{}
/******************************************************************************/
// Initialisation
void Sign::OnInitialise(UNIT_FUNC_PROTOTYPE) // prototype
/******************************************************************************/
{
	ObjectStructure::OnInitialise( UNIT_FUNC_PARAM ); // first call basic initialisation

    self->SetFlag( __FLAG_USE_RANGE, 30 ); // Signs can be used anywhere.

    self->SetPrivateTalk( TRUE );
}		
/******************************************************************************/
// Using a book causes it to display its text, therefore it has a very special OnUse function..
void Sign::OnUse( UNIT_FUNC_PROTOTYPE )
/******************************************************************************/
{
	ObjectStructure::OnUse( UNIT_FUNC_PARAM );
	
	DWORD &itemUsed = *(DWORD *)( valueOUT );

	// Displays its super text! .. how ..
	if(!text.csText.IsEmpty())
	{
        WORD lang = _DEFAULT_LNG;
        if( medium->GetType() == U_PC )
		{
            lang = static_cast< Character * >( medium )->GetLang();
        }

        //medium->SetTarget( self );

        medium->SendSystemMessage(CString( IntlText::ParseString( text.csText, lang ) ));

		itemUsed = TRUE;
	}
}
/******************************************************************************/
ObjectStructure *Sign::CreateObject( void )
/******************************************************************************/
{
	return new Sign;
}