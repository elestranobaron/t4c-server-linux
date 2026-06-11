/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "Recall.h"

/******************************************************************************/
REGISTER_SPELL_EFFECT( RECALL_EFFECT, Recall::NewFunc, RECALL_EFFECT, __noop );

/******************************************************************************/
Recall::Recall()
/******************************************************************************/
{
}
/******************************************************************************/
Recall::~Recall()
/******************************************************************************/
{
}
/******************************************************************************/
// Input parameter
BOOL Recall::InputParameter(
 CString csParam,   // Parameter
 WORD wParamID      // Paremeter ID.
)
/******************************************************************************/
{
    const int ParamX = 1;
    const int ParamY = 2;
    const int ParamWorld = 3;

    BOOL boReturn = TRUE;

    switch( wParamID )
	{
		case ParamX:
			boReturn = recallX.SetFormula( csParam );
			break;
		case ParamY:
			boReturn = recallY.SetFormula( csParam );
			break;
		case ParamWorld:
			boReturn = recallW.SetFormula( csParam );
			break;
		default:
			boReturn = FALSE;
			break;
	}

    return boReturn;
}
/******************************************************************************/
// Does the recall effect
void Recall::CallEffect(SPELL_EFFECT_PROTOTYPE) // The spell data.
/******************************************************************************/
{
   TRACE("***Recall\n");
    if( target != NULL )
	{
        WorldPos wlPos;

        wlPos.X     = recallX.GetBoost( self, target );
        wlPos.Y     = recallY.GetBoost( self, target );
        wlPos.world = recallW.GetBoost( self, target );

        target->Teleport( wlPos, 0 );
    }
}
/******************************************************************************/
// Creates an instance of Recall effect.
SpellEffect *Recall::NewFunc(LPSPELL_STRUCT lpSpell) // The spell structure
/******************************************************************************/
{
   CREATE_EFFECT_HANDLE( Recall, 0 );    
}