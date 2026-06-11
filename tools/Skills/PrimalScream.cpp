
// primalscream.cpp: implementation of the Resurect class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "PrimalScream.h"
#include "../GameDefs.h"
#include "../intltext.h"
#include "../ObjectListing.h"
#include "../t4clog.h"
#include "../TFC Server.h"
extern CTFCServerApp theApp;

typedef struct _CALL_PSINF{
   DWORD dwRayon;
   DWORD dwCurrentEffect;
} CALL_PSINF, *LPCALL_PSINF;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PrimalScream::PrimalScream()
{
	s_saAttrib.skLevel = 12;
	s_saAttrib.skAGI = 0;
	s_saAttrib.skSTR = 38;
	s_saAttrib.skEND = 0;
	s_saAttrib.skINT = 0;
	s_saAttrib.skWIS = 0;
	s_saAttrib.skWIL = 0;
	s_saAttrib.skLCK = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
void PrimalScream::Destroy( void )
//////////////////////////////////////////////////////////////////////////////////////////
//  Destroys peek.
// 
//////////////////////////////////////////////////////////////////////////////////////////
{
}
LPSKILLPNTFUNC PrimalScream::lpOnAddPnts = NULL;

#define PRIMALSCREAM_SKILL   lpusUserSkill->GetSkillPnts( self )

//////////////////////////////////////////////////////////////////////////////////////////
// Timer callback when immobilisation can be re-used
void PrimalScream::ExhaustRemovallCallback(EFFECT_FUNC_PROTOTYPE )
{		
	self->RemoveFlag( __FLAG_PRIMALSCREAM_EXHAUST );
}

//////////////////////////////////////////////////////////////////////////////////////////
void PrimalScream::PrimalScreamCallback(EFFECT_FUNC_PROTOTYPE )

{	
   if( wMessageID == MSG_OnTimer )
   {
      LPCALL_PSINF lpRayon = (LPCALL_PSINF)lpEffectData;

      // TODO : Agro les mobs dans un rayon de 'dAgroRay'
      //  ----------- CODE AGRO ICI -------------
      // The spell's center area.
      Unit *pFondTarget = NULL;
      WorldPos wlCenter = self->GetWL();
      WorldMap *wlWorld = TFCMAIN::GetWorld( wlCenter.world );
      if(!wlWorld)
         return ;

      //xcree la zone a scanner selon la position de la source...
      int nMinX = wlCenter.X - lpRayon->dwRayon;
      nMinX = nMinX < 0 ? 0 : nMinX;

      int nMinY = wlCenter.Y - lpRayon->dwRayon;
      nMinY = nMinY < 0 ? 0 : nMinY;

      int nMaxX = wlCenter.X + lpRayon->dwRayon;
      nMaxX = nMaxX >= (int)wlWorld->GetMAXX() ? (int)wlWorld->GetMAXX() - 1 : nMaxX;

      int nMaxY = wlCenter.Y + lpRayon->dwRayon;
      nMaxY = nMaxY >= (int)wlWorld->GetMAXY() ? (int)wlWorld->GetMAXY() - 1 : nMaxY;

      //Boucle pour toute la zone...
      for(int x = nMinX; x < nMaxX; x++ )
      {
         for(int y = nMinY; y < nMaxY; y++ )
         {
            // Get relative X and Y
            int relX = ( x - wlCenter.X );
            int relY = ( y - wlCenter.Y );

            // Find the ray length of this point.
            double dblRay = sqrt( double(relX * relX + relY * relY) );

            // If the ray is not within the spell's range.
            if( dblRay >= lpRayon->dwRayon )
            {
               continue;
            }
            WorldPos wlTopUnitPos = { x, y, wlCenter.world };

            // Get the unit standing on this spot.
            pFondTarget = wlWorld->ViewTopUnit( wlTopUnitPos );

            // If a no unit was found at this position.
            if( pFondTarget == NULL  || pFondTarget == self)
            {
               continue;
            }

            // Do not target hives.
            if( pFondTarget->GetType() == U_HIVE    || 
                pFondTarget->GetType() == U_OBJECT  || 
                pFondTarget->GetType() == U_MINIONS || 
                pFondTarget->GetType() == U_PC      || 
               pFondTarget->GetAppearance() == 0      )
            {
               continue;
            }

            pFondTarget->Lock();
            //oki donc on a une targer valide, on peu la setter en attack contre nous...
            if(pFondTarget->CanAttack())
            {
               pFondTarget->SetTarget(self);
               pFondTarget->Do(fighting);
            }
            pFondTarget->Unlock();
         }
      }

      if ( self->GetPrimalScreamCycle() != 0) 
      {
         self->SetPrimalScreamCycle(self->GetPrimalScreamCycle() - 1);

         LPCALL_PSINF lpCallRayon = new CALL_PSINF;
         lpCallRayon->dwRayon = lpRayon->dwRayon;
         
         CREATE_EFFECT( self, 
                        MSG_OnTimer, 
                        EFFECT_PRIMALSCREAM, 
                        PrimalScreamCallback, 
                        lpCallRayon, 
                        1000 MILLISECONDS TDELAY, // Timer frequency
                        1000, // Duration
                        __SKILL_PRIMAL_SCREAM,
                        0  );

      }
   }
   else if( wMessageID == MSG_OnDestroy )
   {
      // Delete the AddFlag structure associated to the effect.
      LPCALL_PSINF lpCallRayon = (LPCALL_PSINF)lpEffectData;
      if (lpCallRayon != NULL)
      {
         delete lpCallRayon;
         lpCallRayon = NULL;
      }
    }

}


//////////////////////////////////////////////////////////////////////////////////////////
// Critical strike main function
// Return: int, SKILL_* return parameter.
//////////////////////////////////////////////////////////////////////////////////////////
int PrimalScream::Func( DWORD dwReason,			// Hook which was used to call the skill.
	                     Unit *self,				// Unit using the skill.
	                     Unit *medium,				// Unused.
	                     Unit *target,				// Target of attack.
	                     void *valueIN,				// Unused.
	                     void *valueOUT,			// Unused.
	                     LPUSER_SKILL lpusUserSkill // Current skill strength of the user.
                       )
 
{    
   if(theApp.dwEquilibrageNewSkillEnable == 0)
   {
      return SKILL_NO_FEEDBACK;
   }

	// this skill is on used only.
	if( !( dwReason & HOOK_USE ) ) {
        return SKILL_NO_FEEDBACK;
	}

   // Set target equal to self.
   target = self;


	Character *lpCharS = static_cast< Character * >( self );
	double dCompSuccess = 0;
	double dFinalSuccess = 0;
	double dAgroRay = 0;
	int iManaCost = 0;

    // If the player can use immobilization.
    if( self->ViewFlag( __FLAG_PRIMALSCREAM_EXHAUST ) == 0 ) 
    {
		self->Lock();

		// La competence ne peut pas échouer
		dFinalSuccess = 100;

		// Coup en mana (fixe)		
		iManaCost = 24;
		
		if(lpCharS->GetMana() < iManaCost) 
      {
			// not enought mana
			self->SendInfoMessage( _STR( 15387 , self->GetLang() ) ,0x000080);
			// unlock
			self->Unlock();
			// return 
			return SKILL_NO_FEEDBACK;
		}
		lpCharS->SetMana(lpCharS->GetMana() - iManaCost, TRUE); // on consomme la mana du caster...

		// Test de la competence (Passe toujours pour l'instant, voir si je met un test après les tests)
		int iSuccess = rnd( 0, 100 );

		if(  iSuccess <= (int)dFinalSuccess && dFinalSuccess >0) 
      {
         // AGRO DES MOBS DANS UN RAYON DE skill/20 borné entre 2 (0 pts) et 10 (200 pts)
         dAgroRay = (double)PRIMALSCREAM_SKILL / 20.0;
         if (dAgroRay < 2) 
            dAgroRay = 2;
         if (dAgroRay > 10) 
            dAgroRay = 10;


         // Setup the add flag
         LPCALL_PSINF lpCallRayon = new CALL_PSINF;
         lpCallRayon->dwRayon         = dAgroRay;

         self->RemoveEffect( EFFECT_PRIMALSCREAM );
         self->SetPrimalScreamCycle(5);
         CREATE_EFFECT( self, 
                        MSG_OnTimer, 
                        EFFECT_PRIMALSCREAM, 
                        PrimalScreamCallback, 
                        lpCallRayon, 
                        100 MILLISECONDS TDELAY, // Timer frequency
                        100, // Duration
                        __SKILL_PRIMAL_SCREAM,
                        0  );

         //Show a tiny effect :)
         Broadcast::BCSpellEffect( self->GetWL(), _DEFAULT_RANGE,30347, self->GetID(), 
                                   0, self->GetWL(),self->GetWL(),GetNextGlobalEffectID(),0);

         /*
			


			// TODO : Agro les mobs dans un rayon de 'dAgroRay'
			//  ----------- CODE AGRO ICI -------------
         // The spell's center area.
         Unit *pFondTarget = NULL;
         WorldPos wlCenter = self->GetWL();
         WorldMap *wlWorld = TFCMAIN::GetWorld( wlCenter.world );
         if(!wlWorld)
            return SKILL_NO_FEEDBACK;

         //xcree la zone a scanner selon la position de la source...
         int nMinX = wlCenter.X - dAgroRay;
         nMinX = nMinX < 0 ? 0 : nMinX;

         int nMinY = wlCenter.Y - dAgroRay;
         nMinY = nMinY < 0 ? 0 : nMinY;

         int nMaxX = wlCenter.X + dAgroRay;
         nMaxX = nMaxX >= (int)wlWorld->GetMAXX() ? (int)wlWorld->GetMAXX() - 1 : nMaxX;

         int nMaxY = wlCenter.Y + dAgroRay;
         nMaxY = nMaxY >= (int)wlWorld->GetMAXY() ? (int)wlWorld->GetMAXY() - 1 : nMaxY;

         //Boucle pour toute la zone...
         for(int x = nMinX; x < nMaxX; x++ )
         {
            for(int y = nMinY; y < nMaxY; y++ )
            {
               // Get relative X and Y
               int relX = ( x - wlCenter.X );
               int relY = ( y - wlCenter.Y );

               // Find the ray length of this point.
               double dblRay = sqrt( double(relX * relX + relY * relY) );

               // If the ray is not within the spell's range.
               if( dblRay >= dAgroRay )
               {
                  continue;
               }
               WorldPos wlTopUnitPos = { x, y, wlCenter.world };

               // Get the unit standing on this spot.
               pFondTarget = wlWorld->ViewTopUnit( wlTopUnitPos );

               // If a no unit was found at this position.
               if( pFondTarget == NULL  || pFondTarget == self)
               {
                  continue;
               }
               
               // Do not target hives.
               if( pFondTarget->GetType() == U_HIVE    || 
                   pFondTarget->GetType() == U_OBJECT  || 
                   pFondTarget->GetType() == U_MINIONS || 
                   pFondTarget->GetType() == U_PC      || 
                   pFondTarget->GetAppearance() == 0      )
               {
                  continue;
               }

               pFondTarget->Lock();
               //oki donc on a une targer valide, on peu la setter en attack contre nous...
               pFondTarget->SetTarget(self);
               pFondTarget->Do(fighting);
               pFondTarget->Unlock();
            }
         }
         */
		}

		// EXHAUST DU LANCEUR Calling RemoveEffect triggers the effect (in this case), so, we must call RemoveEffect *before* calling the SetFlag =)
		self->RemoveEffect( EFFECT_PRIMALSCREAM_EXHAUST );
		self->SetFlag(__FLAG_PRIMALSCREAM_EXHAUST, (UINT)PrimalScream::ExhaustRemovallCallback );

		CREATE_EFFECT(
			self, 
			MSG_OnTimer, 
			EFFECT_PRIMALSCREAM_EXHAUST, 
			ExhaustRemovallCallback, 
			NULL, 
			10 SECONDS TDELAY, // Timer frequency
			10 SECONDS TDELAY, // Duration
			__SKILL_PRIMAL_SCREAM,
			0
		);

      self->Unlock();
	}
   else
   {
      self->SendSystemMessage( _STR(15358, self->GetLang()) );
}
   return SKILL_NO_FEEDBACK;
}

		



	