// Peek.cpp: implementation of the Peek class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "../IntlText.h"
#include "Peek.h"
#include "../GameDefs.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Peek::Peek()
{
	s_saAttrib.skLevel = 5;
	s_saAttrib.skAGI = 30;
	s_saAttrib.skSTR = 0;
	s_saAttrib.skEND = 0;
	s_saAttrib.skINT = 0;
	s_saAttrib.skWIS = 0;
	s_saAttrib.skWIL = 0;
	s_saAttrib.skLCK = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
void Peek::Destroy( void )
//////////////////////////////////////////////////////////////////////////////////////////
//  Destroys peek.
// 
//////////////////////////////////////////////////////////////////////////////////////////
{
}

LPSKILLPNTFUNC Peek::lpOnAddPnts = NULL;

#define PEEKSKILL   lpusUserSkill->GetSkillPnts( self )

//////////////////////////////////////////////////////////////////////////////////////////
int Peek::Func
//////////////////////////////////////////////////////////////////////////////////////////
// Critical strike main function
// 
(
 DWORD dwReason,			// Hook which was used to call the skill.
 Unit *self,				// Unit using the skill.
 Unit *medium,				// Unused.
 Unit *target,				// Target of attack.
 void *valueIN,				// LPATTACK_STRUCTURE, current blow.
 void *valueOUT,			// Unused.
 LPUSER_SKILL lpusUserSkill // Current skill strength of the user.
)
// Return: int, SKILL_* return parameter.
//////////////////////////////////////////////////////////////////////////////////////////
{
    const int PeekRange = 2;
    int nResult = SKILL_NO_FEEDBACK;
    
    // Requires both target and self to be present.
    if( !target || !self ) return SKILL_NO_FEEDBACK;

     // Skill only works on characters.
    if( target->GetType() != U_PC || self == target ){
        self->SendSystemMessage( _STR( 23, self->GetLang() ) );        
        return SKILL_NO_FEEDBACK;
    }
    
    EXHAUST sExhaust = self->GetExhaust();
    if( sExhaust.attack > TFCMAIN::GetRound() ){
        self->SendSystemMessage( _STR( 11528, self->GetLang() ) );
        return SKILL_NO_FEEDBACK;
    }

 	// Return if there is a wall or any building between players
	{
		WorldPos tempPos;
		WorldMap *wlWorld = TFCMAIN::GetWorld( self->GetWL().world );
	    Unit *lpCollisionUnit = NULL;
		if (wlWorld->GetCollisionPos( self->GetWL(), target->GetWL(), &tempPos, &lpCollisionUnit, false, false )) {
	        self->SendSystemMessage( _STR( 24, self->GetLang() ) );
			return SKILL_NO_FEEDBACK;
		}
	}


	target->Lock();

   Players *lpPlayer = static_cast< Character *>( target )->GetPlayer();
   if(target->GetType() == U_PC && lpPlayer->IsGod())
   {
      //On ne vole pas un anims...
      self->RemoveFlag( __FLAG_ROBBING );
      self->SendSystemMessage( _STR( 36, self->GetLang() ) );
      target->Unlock();
      return SKILL_NO_FEEDBACK;
   }

    if( self->GetType() == U_PC ){
        Character *ch = static_cast< Character * >( self );
        ch->StopAutoCombat();
    }

    // Calculate the range between the two players.
    int nXdiff = abs( self->GetWL().X - target->GetWL().X );
    int nYdiff = abs( self->GetWL().Y - target->GetWL().Y );
    int nRange = ::sqrt( double(nXdiff * nXdiff + nYdiff * nYdiff) );

    // If the target is within range.
    if( nRange <= PeekRange ){
        bool boSuccess = false;
        const int HidingBonus = 0;
        
		int iSuccess = ( 25 + PEEKSKILL / 2 + self->GetINT() / 5 * PEEKSKILL / 100 + self->GetAGI() / 3 * PEEKSKILL / 100 + HidingBonus ) - ( target->GetINT() / 10 + target->GetAGI() / 3) ;
        if( rnd( 0, 100 ) <= iSuccess && iSuccess >0)
		{
            boSuccess = true;
        }        

        if( boSuccess ){
            if( self->GetType() == U_PC ){
                Character *ch = static_cast< Character * >( self );

                char canRob = 0;
                if( ch->GetSkill( __SKILL_ROB ) != NULL ){
                    // Now in robbing mode!
                    canRob = 1;                    
                }

				self->SetFlag( __FLAG_ROBBING, target->GetID() );

                Character *targetCh = static_cast< Character * >( target );

                TFCPacket sending;
            
    		    // Query the target's backpack.
                sending << (RQ_SIZE)RQ_Rob;
	    	    sending << (char)canRob;
		        sending << (long)target->GetID();		    		    
                sending << target->GetName( self->GetLang() );
                targetCh->PacketRobBackpack( self, sending );
            
                // Send it to the peeking player.
                self->SendPlayerMessage( sending );
            
                nResult = SKILL_PERSONNAL_FEEDBACK_SUCCESSFULL;

                self->SetTarget( target );
            }
            self->SetFlag(__FLAG_NMS_COUPOEIL_SKILL,self->ViewFlag(__FLAG_NMS_COUPOEIL_SKILL)+1);
        }else{
            self->SendSystemMessage( _STR( 31, self->GetLang() ) );

            // If the player got caught
			int iSuccess = (100 -((25 +PEEKSKILL / 2 +self->GetINT() / 5 * PEEKSKILL / 100 + self->GetAGI() / 3 * PEEKSKILL / 100 +HidingBonus) -(target->GetINT() / 10 +target->GetAGI() / 3))) / 2;
            if( rnd( 0, 100 ) <= iSuccess && iSuccess >0)
			{
                target->SendSystemMessage( _STR( 30, target->GetLang() ) );
                
            }
        }
    }else{
        self->SendSystemMessage( _STR( 24, self->GetLang() ) );
    }

    target->Unlock();
    return nResult;
}