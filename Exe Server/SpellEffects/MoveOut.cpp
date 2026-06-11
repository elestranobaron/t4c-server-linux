// MoveOut.cpp: implementation of the MoveOut class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MoveOut.h"
#include "../TFC_MAIN.h"


#define SAMEDIR( __text )  csParam.CompareNoCase( __text ) == 0
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

REGISTER_SPELL_EFFECT( MOVEOUT, MoveOut::NewFunc, MOVEOUT_EFFECT, __noop );


MoveOut::MoveOut()
{
}

MoveOut::~MoveOut()
{
}

//////////////////////////////////////////////////////////////////////////////////////////
BOOL MoveOut::InputParameter(CString csParam,WORD wParamID)
{
   const int Direction = 1;
   const int Success   = 2;
   switch( wParamID )
   {
      case Direction:
         if(      SAMEDIR( "north" ))			dwDirection = 0;
         else if( SAMEDIR( "northeast" ))		dwDirection = 1;
         else if( SAMEDIR( "east" ))         dwDirection = 2;
         else if( SAMEDIR( "southeast" ))	   dwDirection = 3;
         else if( SAMEDIR( "south" ))        dwDirection = 4;
         else if( SAMEDIR( "southwest" ))    dwDirection = 5;
         else if( SAMEDIR( "west" ))         dwDirection = 6;
         else if( SAMEDIR( "northwest" ))    dwDirection = 7;
         else return FALSE;
      break;
      case Success:
         if( !successPercentage.SetFormula( csParam ) )
            return FALSE;
      break;
      
      default:
         return FALSE;
   }
   return TRUE;
}

void MoveOut::CallEffect( SPELL_EFFECT_PROTOTYPE )
{

   FILE *pfD = NULL;
   if( target != NULL )
   {
      static Random rnd;
      int iSuccess = successPercentage.GetBoost( self, target, 0, 0, range );
      if( rnd( 0, 100 ) <= iSuccess &&  iSuccess > 0)
      {
         if(dwDirection == 0)
         {
            target->MoveUnit(DIR::north, false, true, true );
            target->MoveUnit(DIR::north, false, true, true );
         }
         else if(dwDirection == 1)
         {
            target->MoveUnit(DIR::northeast, false, true, true );
            target->MoveUnit(DIR::northeast, false, true, true );
         }
         else if(dwDirection == 2)
         {
            target->MoveUnit(DIR::east, false, true, true );
            target->MoveUnit(DIR::east, false, true, true );
         }
         else if(dwDirection == 3)
         {
            target->MoveUnit(DIR::southeast, false, true, true );
            target->MoveUnit(DIR::southeast, false, true, true );
         }
         else if(dwDirection == 4)
         {
            target->MoveUnit(DIR::south, false, true, true );
            target->MoveUnit(DIR::south, false, true, true );
         }
         else if(dwDirection == 5)
         {
            target->MoveUnit(DIR::southwest, false, true, true );
            target->MoveUnit(DIR::southwest, false, true, true );
         }
         else if(dwDirection == 6)
         {
            target->MoveUnit(DIR::west, false, true, true );
            target->MoveUnit(DIR::west, false, true, true );
         }
         else if(dwDirection == 7)
         {
            target->MoveUnit(DIR::northwest, false, true, true );
            target->MoveUnit(DIR::northwest, false, true, true );
         }
      }
   }
}

//////////////////////////////////////////////////////////////////////////////////////////
SpellEffect *MoveOut::NewFunc(LPSPELL_STRUCT lpSpell)
{
    CREATE_EFFECT_HANDLE( MoveOut, 0 )
}
