// Invisibility2.cpp: implementation of the Invisibility2 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Invisibility2.h"
#include "../TFC_MAIN.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

REGISTER_SPELL_EFFECT( INVIS2, Invisibility2::NewFunc, INVISIBILITY2_EFFECT, __noop );


Invisibility2::Invisibility2()
{
   popupVisualEffect = 0;
}

Invisibility2::~Invisibility2()
{

}

namespace{
   struct InvisEFFECT{
      DWORD popupEffect;
      DWORD effectID;
      DWORD spellID;
   };
};


//////////////////////////////////////////////////////////////////////////////////////////
BOOL Invisibility2::InputParameter(CString csParam,WORD wParamID)
{
   const int Success = 1;
   const int PopupEffect = 2;
   switch( wParamID )
   {
      case Success:
         if( !successPercentage.SetFormula( csParam ) )
            return FALSE;
      break;
      case PopupEffect:
         popupVisualEffect = atoi( csParam );
      break;
      default:
         return FALSE;
   }
   return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////////////
void Invisibility2::BroadcastInvisibility(Unit *self)
{
   WorldMap *wl = TFCMAIN::GetWorld( self->GetWL().world );
   
   if( wl == NULL )
   {
      return;
   }
   
   // Get all units within the default range.
   TemplateList< Unit > *unitList = wl->GetLocalUnits( self->GetWL(), 30, FALSE );
   
   if( unitList == NULL )
   {
      return;
   }
   
   TFCPacket sendingInvi2;
   sendingInvi2 << (RQ_SIZE)RQ_UnitUpdate;
   self->PacketUnitInformation( sendingInvi2 );
   Broadcast::BCast( self->GetWL(), _DEFAULT_RANGE, sendingInvi2 );

   //Boucle pour virer les unit a ceux qui on pas Detect

   unitList->ToHead();
   while( unitList->QueryNext() )
   {
      Unit *un = unitList->Object();
      
      // If the unit does not have detect invisible.
      if( un->ViewFlag( __FLAG_DETECT_INVISIBILITY ) == 0 )
      {
         // Make it dissappear.
         TFCPacket sending;
         // Make the player dissappear
         sending << (RQ_SIZE)__EVENT_OBJECT_REMOVED;
         sending << (char)0;
         sending << (long)self->GetID();
         
         un->SendPlayerMessage( sending );
      }
      else
      {
         
      }
   }
   
   delete unitList;
}

//////////////////////////////////////////////////////////////////////////////////////////
void Invisibility2::InvisibilityRemoval(EFFECT_FUNC_PROTOTYPE)
{
   FILE *pft = NULL;
   // Timer make the call to the function.
   if( wMessageID == MSG_OnTimer )
   {
      InvisEFFECT *invisEffect = (InvisEFFECT *)lpEffectData;
      
      // Remove invisibility
      self->RemoveFlag( __FLAG_INVISIBILITY2 );
      
      WorldPos wlPos = self->GetWL();
      
      DispellEffectStatus( self, invisEffect->spellID ,11);
      
      // Display the popup        
      self->BroadcastPopup( wlPos );
      
      if( invisEffect->popupEffect != 0 )
      {
         Broadcast::BCSpellEffect( wlPos, _DEFAULT_RANGE,invisEffect->popupEffect,self->GetID(),self->GetID(),wlPos,wlPos,GetNextGlobalEffectID(),0);
      }
   }
   else if( wMessageID == MSG_OnDispell )//now destroy effect directly here
   {
      // If the spell got dispelled
      InvisEFFECT *invisEffect = (InvisEFFECT *)lpEffectData;

      // Remove invisibility
      self->RemoveFlag( __FLAG_INVISIBILITY2 );


      WorldPos wlPos = self->GetWL();

      // Display the popup
      self->BroadcastPopup( wlPos );

      if( invisEffect->popupEffect != 0 )
      {
         Broadcast::BCSpellEffect( wlPos, _DEFAULT_RANGE,invisEffect->popupEffect,self->GetID(),self->GetID(),wlPos,wlPos,GetNextGlobalEffectID(),0);
      }

      DispellEffectStatus( self, invisEffect->spellID ,12);
      
      // Remove the effect.
      // self->Remove_Effect( dwEffect );        

      if (invisEffect != NULL)
      {
         delete invisEffect;
         invisEffect = NULL;
      }
   }
   else if( wMessageID == MSG_OnSavePlayer )
   {
      // Player exited
      // Fetch the pointers
      InvisEFFECT *invisEffect = (InvisEFFECT *)lpEffectData;
      LPDATA_SAVE lpDataSave = (LPDATA_SAVE)lpUserData;
      // Prepare buffer to save on player
      lpDataSave->bBufferSize = sizeof( InvisEFFECT );
      lpDataSave->lpbData = new BYTE[ sizeof( InvisEFFECT ) ];        
      memcpy( lpDataSave->lpbData, invisEffect, sizeof( InvisEFFECT ) );        
   }
   else if( wMessageID == MSG_OnLoadPlayer )
   {
      // Player loads
      LPUNIT_EFFECT lpUnitEffect = (LPUNIT_EFFECT)lpEffectData;
      LPDATA_SAVE lpDataSave = (LPDATA_SAVE)lpUserData;
      
      InvisEFFECT *invisEFFECT = new InvisEFFECT;
      
      // Fetch the LPFLAG_ADDING;
      if( lpDataSave->bBufferSize == sizeof( InvisEFFECT ) )
      {
         memcpy( invisEFFECT, lpDataSave->lpbData, sizeof( InvisEFFECT ) );
         // Get the spell
         LPSPELL_STRUCT lpSpell = SpellMessageHandler::GetSpell( invisEFFECT->spellID );
         
         if( lpSpell != NULL )
         {
            // Create an effect status update for the client.
            CreateEffectStatus(
               self,
               invisEFFECT->spellID,
               lpUnitEffect->dwTotalDuration == 0xFFFFFFFF ? 0xFFFFFFFF : ( lpUnitEffect->dwTimer - TFCMAIN::GetRound() ) * 50,
               lpUnitEffect->dwTotalDuration,
               lpSpell
               );
         }
      }
      
      // Attach the flag adding structure to the effect.
      lpUnitEffect->lpData = invisEFFECT;
   }
   else if( wMessageID == MSG_OnDestroy )
   {
      // Effect is getting destroyed.
      // Delete the AddFlag structure associated to the effect.
      InvisEFFECT *invisEffect = (InvisEFFECT *)lpEffectData;
      if (invisEffect != NULL)
      {
         delete invisEffect;
         invisEffect = NULL;
      }
   }
}
//////////////////////////////////////////////////////////////////////////////////////////
void Invisibility2::CallEffect( SPELL_EFFECT_PROTOTYPE )
{
    TRACE("***Invisibility2\n");
    if( target == NULL )
    {
        return;
    }

    // If the spell fails.
    static Random rnd;
	 int iSuccess = successPercentage.GetBoost( self, target, 0, 0, range ) ;
    if( rnd( 0, 100 ) <=  iSuccess && iSuccess > 0)
    {
		// Get duration now.
		DWORD dwDuration = lpSpell->bfDuration.GetBoost( self, target );
		if( dwDuration == 0 )
			return;
	   
      //si un effet est deja present on la flush, et on cree la nouvelle...
      LPUNIT_EFFECT pEffectTst = target->GetEffect( dwEffectID );
      if( pEffectTst != NULL )
      {
         InvisEFFECT *lpEffectData = (InvisEFFECT*)pEffectTst->lpData;
         target->RemoveEffect( pEffectTst->dwEffect ,false);      
         if (lpEffectData != NULL)
         {
            delete lpEffectData;
            lpEffectData = NULL;
         } 
      }
		// Remove the previous effect
		//target->RemoveEffect( dwEffectID );


		// Add the invisibility flag
		target->SetFlag( __FLAG_INVISIBILITY2, 1 );
		BroadcastInvisibility( target );
		// Setup the add flag
		InvisEFFECT *invisEffect = new InvisEFFECT;
		invisEffect->popupEffect = popupVisualEffect;
		invisEffect->spellID = lpSpell->wSpellID;
		invisEffect->effectID = dwEffectID;
		// Create an effect status update for the client.
		CreateEffectStatus(target,lpSpell->wSpellID,dwDuration,dwDuration,lpSpell);
		CREATE_EFFECT(target,MSG_OnTimer,dwEffectID,InvisibilityRemoval,invisEffect,dwDuration MILLISECONDS TDELAY,dwDuration,lpSpell->wSpellID,__FLAG_INVISIBILITY2);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
SpellEffect *Invisibility2::NewFunc(LPSPELL_STRUCT lpSpell)
{
    REGISTER_EFFECT( lpSpell->dwNextEffectID, InvisibilityRemoval );

    CREATE_EFFECT_HANDLE( Invisibility2, 1 )
}
