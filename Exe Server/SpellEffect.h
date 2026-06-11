/******************************************************************************
Modify for vs2008 (06/05/2009)
Add incremental flag on Spell Name bu Nightmare (29/06/2009)
/******************************************************************************/
//      File Name: SpellEffect.h
//      Project:   TFC Server
//      Plateform: Windows NT 4.0 Workstation/Server
//      Creation:  25/6/1998
//      Author:    Francois Leblanc (FL)
/******************************************************************************/
//      Change History
//
//         Date            Ver.      Author         Purpose
//         ----            ----      ------         -------
//         25/6/1998      1.0        FL             Initial developpement
//
//      Description
//          Top class for handling spell effects. Purely virtual, effects must be conceived
//      somewhere else.
/******************************************************************************/
#if !defined(AFX_SPELLEFFECT_H__65AF1060_0C35_11D2_835C_00E02922FA40__INCLUDED_)
#define AFX_SPELLEFFECT_H__65AF1060_0C35_11D2_835C_00E02922FA40__INCLUDED_

#if _MSC_VER >= 1000
	#pragma once
#endif // _MSC_VER >= 1000

#include "Unit.h"
#include "Skills.h"
#include "IntlText.h"

#define ELEMENT_NONE							0
#define ELEMENT_FIRE							1
#define ELEMENT_EARTH							2
#define ELEMENT_AIR								3
#define ELEMENT_WATER							4

#define TARGET_UNIT_ANY							0
#define TARGET_UNIT_OBJECT						1
#define TARGET_UNIT_NPC							2
#define TARGET_UNIT_PC							3
#define TARGET_UNIT_LIVING						4
#define TARGET_SELF								5
#define TARGET_POSITION							6
#define TARGET_UNIT_PC_NONSELF					7
#define TARGET_UNIT_ANY_NONSELF					8
#define TARGET_UNIT_LIVING_NONSELF				9
#define TARGET_UNIT_LIVING_FAVOR_NPC			10
#define TARGET_UNIT_LIVING_FAVOR_NPC_NONSELF	11
#define TARGET_UNIT_LIVING_FAVOR_PC				12
#define TARGET_UNIT_ANY_FAVOR_PC				13
#define TARGET_GROUP_UNIT						14
#define TARGET_GROUP_SELF						15
#define TARGET_GROUP_POSITION					16
#define TARGET_NONGROUP_UNIT					17
#define TARGET_NONGROUP_SELF					18
#define TARGET_NONGROUP_POSITION				19

#define ATTACK_PHYSICAL							1
#define ATTACK_MENTAL       					2

#define SPELL_EFFECT_PROTOTYPE  Unit *self, Unit *medium, Unit *target, WorldPos wlPos, double range
#define SPELL_EFFECT_PARAMS     self, medium, target, wlPos, range

/******************************************************************************/
// Spell structure
class SpellEffect;
typedef struct _SPELL_STRUCT{
//    friend SpellEffect;

    _SPELL_STRUCT( void ){ boPVPcheck = false; boSkillExclusion = true;};

    const char * GetName( WORD wLang )
    {
        if( strSpellName == "")
        {
            return "";
        }
        return IntlText::ParseString( strSpellName.GetBuffer(0), wLang );
    }

    void SetName( const char *lpszName )
	{
      nIncrementedFlag = 0;
      strSpellName.Format("%s",lpszName);

      if( strSpellName.GetLength() > 2 )
      {

         int iPos    = 0;
         bool bOK    = false;
         // If this string contains a language ID.        
         if( strSpellName[ 0 ] == '{' )
         {
            nIncrementedFlag = atoi(strSpellName.GetBuffer(0)+1);
            
            //search the end of ID... MAX 6 pos because flag must me SHORT maximum 65535
            //0 1 2 3 4 5 6
            //{ X X X X X }
            
            for(int i=1;i<7;i++)
            {
               if(strSpellName[i] == '}')
               {
                  bOK = true;
                  iPos = i+1;
                  i = 8;
               }
            }
         }
         if(bOK)
         {
            strSpellName = strSpellName.Right(strSpellName.GetLength()-iPos);
         }
         else
         {
            //erreur de ID, on fais rien, opn reste le NOm telquel
            nIncrementedFlag = 0;
         }
      }
     
   }
   
    string GetDesc( WORD wLang )
    {
        return IntlText::ParseString( strDesc.GetBuffer(0), wLang );
    }
    void SetDesc( const char *lpszDesc )
    {
        strDesc.Format("%s",lpszDesc);        
    }

    WORD	        wSpellID;

    BoostFormula    bfManaCost;
    BoostFormula    bfDuration;
    INDUCED_EXHAUST sInducedExhaust;
    BYTE	        bElementType;
    BYTE            bDamageType;
    BYTE	        bTarget;
    BYTE	        bRange;
    WORD	        wVisualEffect;
    WORD            wRangeVisualEffect;
    bool            attackSpell;
    DWORD           dwIcon;
    BoostFormula    successPercentage;

    BoostFormula    bfTimerFrequency;

    int             nLineOfSight;
    int             nIncrementedFlag;

    bool            boSkillExclusion;
    bool            boPVPcheck;

    SKILL_ATTRIBUTES	saAttrib;

    DWORD    dwNextEffectID;

	TemplateList< SpellEffect > tlEffects;

    void Delete( void )
	{
		if (this != NULL) 
		{
			delete this;
		}
    }
private:
   // Prohibit stack-based instantiations of this class.
   ~_SPELL_STRUCT( void )
   {
   };
   
   // prohibite operator = and copy constructor.
   PROHIBIT_ASSIGNMENT( _SPELL_STRUCT );
   
   DWORD                   dwTextID;
   CString                 strSpellName;
   CString                 strDesc;
} SPELL_STRUCT, *LPSPELL_STRUCT;

#define CREATE_EFFECT_HANDLE( __obj, __uniteffectslot ) __obj *Obj = new __obj;\
    Obj->lpSpell = lpSpell;\
    Obj->dwEffectID = lpSpell->dwNextEffectID;\
    lpSpell->dwNextEffectID += __uniteffectslot;\
    return Obj;
                                                
/******************************************************************************/
// Spell effect class.
class __declspec( dllexport ) SpellEffect  
/******************************************************************************/
{
public:
    virtual BOOL InputParameter( CString csParam, WORD wParamID ) = 0;
	virtual void CallEffect( SPELL_EFFECT_PROTOTYPE ) = 0;

    static void CreateEffectStatus( Unit *target, DWORD effectId, DWORD time, DWORD totalDuration, _SPELL_STRUCT *lpSpell );
    static void DispellEffectStatus( Unit *target, DWORD effectId ,int iTmp = 0);

protected:
    LPSPELL_STRUCT lpSpell;     // Parent spell structure.
    DWORD dwEffectID;           // Effect ID offset.
};


#endif // !defined(AFX_SPELLEFFECT_H__65AF1060_0C35_11D2_835C_00E02922FA40__INCLUDED_)
