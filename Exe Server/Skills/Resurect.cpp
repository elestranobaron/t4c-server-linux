
// Resurect.cpp: implementation of the Resurect class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Resurect.h"
#include "../GameDefs.h"
#include "../IntlText.h"
#include "../ObjectListing.h"
#include "../T4CLog.h"
#include "../TFC Server.h"
extern CTFCServerApp theApp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Resurect::Resurect()
{
   s_saAttrib.skLevel = 17;
   s_saAttrib.skAGI = 0;
   s_saAttrib.skSTR = 0;
   s_saAttrib.skEND = 0;
   s_saAttrib.skINT = 0;
   s_saAttrib.skWIS = 50;
   s_saAttrib.skWIL = 0;
   s_saAttrib.skLCK = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
void Resurect::Destroy( void )
//////////////////////////////////////////////////////////////////////////////////////////
//  Destroys peek.
// 
//////////////////////////////////////////////////////////////////////////////////////////
{
}

LPSKILLPNTFUNC Resurect::lpOnAddPnts = NULL;

#define RESURECT_SKILL   lpusUserSkill->GetSkillPnts( self )

//////////////////////////////////////////////////////////////////////////////////////////
int Resurect::Func
//////////////////////////////////////////////////////////////////////////////////////////
// Critical strike main function
// 
(
 DWORD dwReason,			// Hook which was used to call the skill.
 Unit *self,				// Unit using the skill.
 Unit *medium,				// Unused.
 Unit *target,				// Target of attack.
 void *valueIN,			// Unused.
 void *valueOUT,			// Unused.
 LPUSER_SKILL lpusUserSkill // Current skill strength of the user.
 )
 // Return: int, SKILL_* return parameter.
 //////////////////////////////////////////////////////////////////////////////////////////
{    
   const int ResurectRange = 3;
   BOOL bResurect = FALSE;


   // Requires both target and self to be present.
   if( !target || !self ) 
      return SKILL_NO_FEEDBACK;

   if(target->ViewFlag(__FLAG_NMS_EN_PRISON) ==1)
      return SKILL_NO_FEEDBACK;  //la persourne source ets en prison skill pas d'effet


   // Skill only works on OTHER characters.
   if( self->GetType() != U_PC || target->GetType() != U_PC || self == target )
   {
      self->SendSystemMessage( _STR( 23, self->GetLang() ) );        
      return SKILL_NO_FEEDBACK;
   }

   if(target->ViewFlag(__FLAG_NMS_PLAYER_DEATH) <=0)
   {
      self->SendInfoMessage( _STR( 15014 , self->GetLang() ) ,0x000080);
      return SKILL_NO_FEEDBACK;
   }

   if(theApp.dwEquilibrageSkillNewFormulaEnable == 0)
   {

      Character *lpCharS = static_cast< Character * >( self );
      Character *lpCharT = static_cast< Character * >( target );


      int iTrueWis = self->GetTrueWIS();
      if(iTrueWis >1000)
         iTrueWis = 1000;

      int iExaust = 10000-(iTrueWis*10);

      int iManaCost = (lpCharS->GetMaxMana()/2) + ((lpCharS->GetMaxMana()/2)-((iTrueWis*(lpCharS->GetMaxMana()/2))/1000));

      if(lpCharS->GetMana() < iManaCost)
      {
         self->SendInfoMessage( _STR( 15016 , self->GetLang() ) ,0x000080);
         return SKILL_NO_FEEDBACK;
      }

      target->Lock();
      self->Lock();

      if( self->GetType() == U_PC )
      {
         Character *ch = static_cast< Character * >( self );
         ch->StopAutoCombat();
      }

      // Calculate the range between the two players.
      int nXdiff = abs( self->GetWL().X - target->GetWL().X );
      int nYdiff = abs( self->GetWL().Y - target->GetWL().Y );
      int nRange = ::sqrt( double(nXdiff * nXdiff + nYdiff * nYdiff) );

      if( nRange <= ResurectRange  )
      {
         //lpCharS->_CastSpellDirect(11145,target);
         Broadcast::BCSpellEffect( lpCharS->GetWL(), 30, 30132, lpCharS->GetID(), 0, lpCharS->GetWL(),lpCharS->GetWL(),GetNextGlobalEffectID(),0);
         lpCharS->SetMana(0,TRUE); // recupere 1005 de la mana du caster...



         self->DealExhaust(iExaust,iExaust,iExaust);

         // If robbed was successful.
         int iSuccess = rnd( 0, 10000 );

         int iLevelInfluence = 0;
         int dwlevelrange = self->GetLevel() - target->GetLevel();

         if( dwlevelrange >= 100 )
            iLevelInfluence = 15;
         else if( dwlevelrange >= 50 )
            iLevelInfluence = 10;		
         else if( dwlevelrange > 25 )
            iLevelInfluence = 5;
         else if (dwlevelrange >= -25 && dwlevelrange <= 25 )
            iLevelInfluence = 0;						
         else if( dwlevelrange <= -100 )
            iLevelInfluence = -15;
         else if( dwlevelrange <= -50 )
            iLevelInfluence = -10;
         else if( dwlevelrange < -25 )
            iLevelInfluence = -5;

         int iLevelSAG = 0;
         if(self->GetTrueWIS() > 900)
            iLevelSAG = 70;
         else if(self->GetTrueWIS() > 800)
            iLevelSAG = 60;
         else if(self->GetTrueWIS() > 700)
            iLevelSAG = 50;
         else if(self->GetTrueWIS() > 600)
            iLevelSAG = 40;
         else if(self->GetTrueWIS() > 500)
            iLevelSAG = 30;
         else if(self->GetTrueWIS() > 300)
            iLevelSAG = 20;
         else if(self->GetTrueWIS() > 200)
            iLevelSAG = 10;



         if(  iSuccess <= (RESURECT_SKILL*(30+iLevelInfluence+iLevelSAG)))
         {
            bResurect = TRUE;
         }
         else
         {
            self->SendInfoMessage( _STR( 15017, self->GetLang() ),0x000080 );
         }
      }
      else
      {
         self->SendSystemMessage( _STR( 24, self->GetLang() ) );
      }



      self->Unlock();
      target->Unlock();

      if(bResurect)
      {
         lpCharT->NMResurect(TRUE);
         self->SetFlag(__FLAG_NMS_RESURECT_SKILL,self->ViewFlag(__FLAG_NMS_RESURECT_SKILL)+1);
      }
   }
   else
   {
      // Ratio de wis = ResurectWisRatioNum / ResurectWisRatioDenum
      const double ResurectWisRatioNum = 5.0;
      const double ResurectWisRatioDenum = 100.0;
      Character *lpCharS = static_cast< Character * >( self );
      Character *lpCharT = static_cast< Character * >( target );

      //Calcul du lvlRatio
      int iTargetLevel = target->GetLevel(); if (iTargetLevel <= 0) iTargetLevel = 1;
      int iSelfLevel = self->GetLevel(); if (iSelfLevel <= 0) iSelfLevel = 1;

      double dLvlRatio = (double)iTargetLevel / (double)iSelfLevel;

      if( ((Character*)self)->GetGodFlags() & GOD_DEVELOPPER )
      {
         CString csMessage1;
         csMessage1.Format( "[\"debug\"] Resurect Skill. (iTargetLevel: {%d}, iSelfLevel: {%d}, dLvlRatio : {%lf} )",iTargetLevel,iSelfLevel,dLvlRatio);
         self->SendSystemMessage( csMessage1 );
      }

      //int iExaust = 10000-(iTrueWis*10);
      int iExaust = 1000;

      target->Lock();
      self->Lock();

      //Desactiver le combat quand le skill est utilisù
      if( self->GetType() == U_PC )
      {
         Character *ch = static_cast< Character * >( self );
         ch->StopAutoCombat();
      }

      // Calculate the range between the two players.
      int nXdiff = abs( self->GetWL().X - target->GetWL().X );
      int nYdiff = abs( self->GetWL().Y - target->GetWL().Y );
      int nRange = ::sqrt( double(nXdiff * nXdiff + nYdiff * nYdiff) );

      int iManaCost = 0;

      double dCompSuccess = 0;
      double dWisSuccess = 0;
      double dFinalSuccess = 0;
      int iSuccess = 0;

      if( nRange <= ResurectRange  ) 
      {

         // Effect du spell
         //lpCharS->_CastSpellDirect(11145,target);
         Broadcast::BCSpellEffect( lpCharS->GetWL(), 30, 30132, lpCharS->GetID(), 0, lpCharS->GetWL(),lpCharS->GetWL(),GetNextGlobalEffectID(),0);
         self->DealExhaust(iExaust,iExaust,iExaust);

         // success wis = Wis * 5 / 100
         dWisSuccess = (double)self->GetWIS() * ( ResurectWisRatioNum / ResurectWisRatioDenum );
         // success comp 
         dCompSuccess = (double)RESURECT_SKILL / 4.0;
         // succes final (%rùussite)
         dFinalSuccess = (dWisSuccess + dCompSuccess) / dLvlRatio;


         // Coup en mana ( mana*(100-%reussite)/100)		
         iManaCost = lpCharS->GetMaxMana() * (100 - dFinalSuccess) / 100;
         // On ne retire jamais moins de 10% de la mana
         if(iManaCost < lpCharS->GetMaxMana()*10/100.0)  iManaCost = (int)(lpCharS->GetMaxMana()*10/100.0);

         if(lpCharS->GetMana() < iManaCost) 
         {
            // not enought mana
            self->SendInfoMessage( _STR( 15016 , self->GetLang() ) ,0x000080);
            // unlock
            self->Unlock();
            target->Unlock();
            // return 
            return SKILL_NO_FEEDBACK;
         }
         lpCharS->SetMana(lpCharS->GetMana() - iManaCost, TRUE); // on consomme la mana du caster...


         // Test de la competence
         iSuccess = rnd( 0, 100 );

         if(  iSuccess <= (int)dFinalSuccess && dFinalSuccess >0) 
         {
            bResurect = TRUE;
         }
         // La competence n'est pas passùe
         else 
         {
            self->SendInfoMessage( _STR( 15017, self->GetLang() ),0x000080 );
         }

         if( ((Character*)self)->GetGodFlags() & GOD_DEVELOPPER )
         {
            CString csMessage;
            csMessage.Format( "[\"Resurect\" \"v2\"] Resurect Skill. (iSuccess: {%d}, ({%10.3lf}wisSuccess + {%10.3lf}compSuccess) / {%10.3lf}lvlRatio = {%10.3lf}dFinalSuccess ) = Mana:{%d}",
                              iSuccess ,
                              dWisSuccess ,
                              dCompSuccess ,
                              dLvlRatio,
                              dFinalSuccess,
                              iManaCost);
            self->SendSystemMessage( csMessage );
         }
      }
      else 
      {
         self->SendSystemMessage( _STR( 24, self->GetLang() ) );
      }

      // recup de l'or
      int lostGold;
      int oldGold = self->GetGold();

      lostGold = self->ViewFlag(__FLAG_DEATH_LOST_GOLD, 0); // On rùcupùre l'or perdu par la cible
      if(lostGold > 0)
      {
         // On rùcupùre un pourcentage d'or, si on a ressucitù ù 100% on rùcupùre 50% de l'or. (on peut monter ù un bonus de 200% pour rùcuperer tout l'or)
         if (dLvlRatio < 200) 
         {
            lostGold *= dLvlRatio / 200 ;
         }
      }
      

      self->Unlock();
      target->Unlock();

      if(bResurect)
      {
         // On donne un %age de l'or perdu par la target au soigneur.
         if(lostGold > 0)
         {
            self->Lock();
            self->SetGold( oldGold + lostGold );
            self->Unlock();
         }
         

         lpCharT->NMResurect(TRUE);
         self->SetFlag(__FLAG_NMS_RESURECT_SKILL, self->ViewFlag(__FLAG_NMS_RESURECT_SKILL) + 1);
      }

   }
   return SKILL_NO_FEEDBACK;
}