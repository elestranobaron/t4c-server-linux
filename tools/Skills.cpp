/******************************************************************************
Modify for vs2008 (31/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "TFC_MAIN.h"
#include "Skills.h"
#include "IntlText.h"
#include "format.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
CPtrArray Skills::c_aSkills;

/******************************************************************************/
LPCTSTR __declspec(dllexport) _SKILL::GetName( WORD wLanguage )
/******************************************************************************/
{
    return IntlText::ParseString( lpszSkillName, wLanguage );
}
/******************************************************************************/
// Registers a skill
//		lpszName:	String containing the name of the skill
//		lpFunc:		Callback function containing the code executing the skill.
//		lpAttrib:	Pointer to a SKILL_ATTRIBUTE structure
//		nHook:		Where the skill is hooked
void Skills::Register(LPCTSTR lpszName, DWORD descId, int nID, LPSKILL_CALLBACK lpFunc, 
					  LPSKILL_ATTRIBUTES lpAttrib, int nHook, LPSKILLPNTFUNC lpPntFunc)
/******************************************************************************/
{
	LPSKILL lpSkill = new SKILL;
	
    lpSkill->SetName( lpszName );
	lpSkill->lpFunc = lpFunc;
	lpSkill->nHook = nHook;
	lpSkill->nSkillID = nID;
	lpSkill->lpAddPntFunc = lpPntFunc;
	lpSkill->lpsaAttrib = lpAttrib;
    lpSkill->desc = descId;

	TRACE( "\r\nRegistering skill ID %u.", lpSkill->nSkillID );

	c_aSkills.SetAtGrow(nID, lpSkill);
}
/******************************************************************************/
// Executes a skill
//	nID:	Skill ID (which skill).
//	nHook:	Which type of intrinsic skill to use.
//	self/medium/target/valueIN/valueOUT:	Standard message.
//	lpusUserSkill:	Pointer to a USER_SKILL.
int Skills::ExecuteSkill(int nID, int nHook, Unit *self, Unit *medium, 
						  Unit *target, void *valueIN, void *valueOUT, LPUSER_SKILL lpusUserSkill)
/******************************************************************************/
{
	LPSKILL lpSkill;	
	int nReturn = SKILL_FAILED;

	if(nID < c_aSkills.GetSize())
	{
		lpSkill = (LPSKILL)c_aSkills.GetAt(nID);
		if(lpSkill)
		{
			nReturn = lpSkill->lpFunc(nHook, self, medium, target, valueIN, valueOUT, lpusUserSkill);
			
			if( nReturn == SKILL_SUCCESSFULL || nReturn == SKILL_FAILED )
			{
				//UINT param[2];
				//param[0] = nID;
				//param[1] = nReturn;			
				//Broadcast::BCast(__EVENT_SKILL_USED, self->GetWL(), _DEFAULT_RANGE, param);
			}
		}
#ifdef _DEBUG
		else
		{
			TRACE(_TEXT("\r\n!Warning! Using unreferenced skill.\r\n"));
			TRACE(_TEXT("Dump: ID %u Hook %u self %08x medium %08x target %08 valueIN %08x valueOUT %08x\r\n"), 
				nID, nHook, self, medium, target, valueIN, valueOUT);
		}		
#endif
	}
#ifdef _DEBUG
	else
	{
		TRACE(_TEXT("\r\n!Warning! Using out-of-bound skill.\r\n"));
		TRACE(_TEXT("Dump: ID %u Hook %u self %08x medium %08x target %08 valueIN %08x valueOUT %08x\r\n"), 
			nID, nHook, self, medium, target, valueIN, valueOUT);
	}
#endif	

	return nReturn;
}
/******************************************************************************/
// Determines if nID is a skill
//	nID:		The skill to query
//  lpnHook:	Pointer to an int which will receive on what is the skill hooked, NULL otherwise
// return:	TRUE, nID is a skill
BOOL Skills::IsSkill(int nID, LPINT lpnHook)
/******************************************************************************/
{	
	if(nID < c_aSkills.GetSize())
	{
		LPSKILL lpSkill = (LPSKILL)c_aSkills.GetAt(nID);
		if(lpSkill)
		{
			if(lpnHook)
			{
				*lpnHook = lpSkill->nHook;
				return TRUE;
			}
		}
	}
	return FALSE;
}
/******************************************************************************/
// This function returns TRUE if skill nID is learnable
//	nID:		ID of the skill to learn
//	uForWho:	Unit which would learn the skill
// return: TRUE, skill is learnable
BOOL Skills::IsSkillLearnable(int nID, Unit *uLearner, CString &reqText)
/******************************************************************************/
{
   if(nID < c_aSkills.GetSize())
   {
      LPSKILL lpSkill = (LPSKILL)c_aSkills.GetAt(nID);
      // Then check for 'nID'
      if(lpSkill)
      {
         LPSKILL_ATTRIBUTES lpAttrib = lpSkill->lpsaAttrib;
         // Form the description string.
         TFormat format;

         reqText = _STR( 7277, uLearner->GetLang() );
         bool prev = false;
         bool nothin = true;
         if( lpAttrib->skLevel != 0 )
         {
            reqText += format( _STR( 7278, uLearner->GetLang() ), lpAttrib->skLevel );;
            prev = true;
            nothin = false;
         }
         if( lpAttrib->skSTR != 0 )
         {
            if( prev ){
               reqText += ", ";
            }                
            reqText += format( _STR( 7279, uLearner->GetLang() ), lpAttrib->skSTR );
            prev = true;
            nothin = false;
         }
         if( lpAttrib->skEND != 0 )
         {
            if( prev )
            {
               reqText += ", ";
            }                
            reqText += format( _STR( 7280, uLearner->GetLang() ), lpAttrib->skEND );
            prev = true;
            nothin = false;
         }
         if( lpAttrib->skAGI != 0 )
         {
            if( prev )
            {
               reqText += ", ";
            }                
            reqText += format( _STR( 7281, uLearner->GetLang() ), lpAttrib->skAGI );
            prev = true;
            nothin = false;
         }
         if( lpAttrib->skINT != 0 )
         {
            if( prev )
            {
               reqText += ", ";
            }                
            reqText += format( _STR( 7282, uLearner->GetLang() ), lpAttrib->skINT );
            prev = true;
            nothin = false;
         }
         if( lpAttrib->skWIS != 0 )
         {
            if( prev ){
               reqText += ", ";
            }                
            reqText += format( _STR( 7283, uLearner->GetLang() ), lpAttrib->skWIS );
            prev = true;
            nothin = false;
         }

         if( lpAttrib->tlskSkillRequired.NbObjects() != 0 )
         {
            if( prev )
            {
               reqText += _STR( 7284, uLearner->GetLang() );
            }
            nothin = false;
            if( lpAttrib->tlskSkillRequired.NbObjects() > 1 )
            {
               reqText += _STR( 7285, uLearner->GetLang() );
            }
            else
            {
               reqText += _STR( 7287, uLearner->GetLang() );
            }
            lpAttrib->tlskSkillRequired.ToHead();
            if( lpAttrib->tlskSkillRequired.QueryNext() )
            {
               LPUSER_SKILL lpRequired = lpAttrib->tlskSkillRequired.Object();
               LPSKILL lpSkill = GetSkill( lpRequired->GetSkillID() );
               if( lpSkill != NULL )
               {
                  reqText += lpSkill->GetName( uLearner->GetLang() );   
               }

               while( lpAttrib->tlskSkillRequired.QueryNext() )
               {
                  reqText += ", ";
                  lpRequired = lpAttrib->tlskSkillRequired.Object();
                  LPSKILL lpSkill = GetSkill( lpRequired->GetSkillID() );
                  if( lpSkill != NULL )
                  {
                     reqText += lpSkill->GetName( uLearner->GetLang() );   
                  }           
               }
            }
         }
         if( nothin )
         {
            reqText += _STR( 7286, uLearner->GetLang() );
         }
         else
         {
            reqText += ".";
         }

         if(lpAttrib->skLevel <= (int)uLearner->GetLevel())
         {
            if( lpAttrib->skAGI <= uLearner->GetTrueAGI() && lpAttrib->skSTR <= uLearner->GetTrueSTR() &&
               lpAttrib->skEND <= uLearner->GetTrueEND() && lpAttrib->skINT <= uLearner->GetTrueINT() &&
               lpAttrib->skWIS <= uLearner->GetTrueWIS())
            {
               BOOL boOK = TRUE;
               LPUSER_SKILL lpRequired;
               LPUSER_SKILL lpUserSkill;
               lpAttrib->tlskSkillRequired.Lock();
               lpAttrib->tlskSkillRequired.ToHead();
               while( lpAttrib->tlskSkillRequired.QueryNext() && boOK )
               {
                  lpRequired = lpAttrib->tlskSkillRequired.Object();

                  lpUserSkill = uLearner->GetSkill( lpRequired->GetSkillID() );
                  // If user has the skill
                  if( lpUserSkill )
                  {
                     // If user doesn't has enough skill points in the required skill.
                     if( lpUserSkill->GetTrueSkillPnts() < lpRequired->GetTrueSkillPnts() )
                     {
                        // user cannot learn skill
                        boOK = FALSE;
                     }
                  }
                  else
                  {
                     // Cannot learn if user doesn't have a required skill.
                     boOK = FALSE;
                  }							
               }
               lpAttrib->tlskSkillRequired.Unlock();

               if( !boOK )
               {
                  _LOG_DEBUG
                     LOG_DEBUG_LVL4,
                     "Skill %u is NOT valid for teaching.",
                     nID
                     LOG_
               }
               else
               {
                  _LOG_DEBUG
                     LOG_DEBUG_LVL4,
                     "Skill %u is valid for teaching.",
                     nID
                     LOG_
               }

               return boOK;

            }
            else
            {
               _LOG_DEBUG
                  LOG_DEBUG_LVL4,
                  "Skill %u is NOT valid for teaching. User: AGI=%u STR=%u END=%u INT=%u WIS=%u.",
                  nID,
                  uLearner->GetTrueAGI(),uLearner->GetTrueSTR(),uLearner->GetTrueEND(),uLearner->GetTrueINT(),
                  uLearner->GetTrueWIS()
                  LOG_
                  _LOG_DEBUG
                  LOG_DEBUG_LVL4,
                  "Skill %u is NOT valid for teaching. Skill: AGI=%u STR=%u END=%u INT=%u WIS=%u.",
                  nID,
                  lpAttrib->skAGI,lpAttrib->skSTR,lpAttrib->skEND,lpAttrib->skINT,
                  lpAttrib->skWIS
                  LOG_

            }
         }
         else
         {
            _LOG_DEBUG
               LOG_DEBUG_LVL4,
               "Skill %u is NOT valid for teaching. User level %u vs required level %u.", nID, uLearner->GetLevel(), lpAttrib->skLevel
               LOG_
         }
      }
   }

   return FALSE;
}
/******************************************************************************/
// Returns a LPSKILL pointer of the skill, or NULL
//	nID:	ID of the skill to get.
// return:	A pointer to the skill queried.
LPSKILL Skills::GetSkill(int nID)
/******************************************************************************/
{
	if(nID < c_aSkills.GetSize())
	{
		return (LPSKILL)(c_aSkills.GetAt(nID));
	}
	return NULL;
}
/******************************************************************************/
//  Returns the skill with the given name.
LPSKILL Skills::GetSkillByName(
 std::string skillName, // The skill name
 WORD wLang             // The language.
)
/******************************************************************************/
{
    int i;
    for( i = 0; i < c_aSkills.GetSize(); i++ )
	{
        LPSKILL lpSkill = reinterpret_cast< LPSKILL >( c_aSkills.GetAt( i ) );
        if( lpSkill != NULL && _stricmp( lpSkill->GetName( wLang ), skillName.c_str() ) == 0 )
		{
            return lpSkill;
        }
    }
    return NULL;
}
/******************************************************************************
// This function returns the ID of the skill of name name
int Skills::GetSkillIDByName(LPCTSTR lpszName)
/******************************************************************************
{
	int nID = 0;

	for(nID = 0; nID < c_aSkills.GetSize(); nID++){			
		if(strcmp(lpszName, ((LPSKILL)(c_aSkills.GetAt(nID)))->lpszSkillName))
			return nID;	
	}

	return -1;
} */
/******************************************************************************/
// Called to train points in a skill.
void Skills::TrainSkill(
 Unit *lpuTrained,			// Unit that trained.
 LPUSER_SKILL lpUserSkill,	// User skill to train.
 DWORD dwNbPoints			// Number of skill points to add, to train.
)
/******************************************************************************/
{
	LPSKILL lpSkill = GetSkill( lpUserSkill->GetSkillID() );

	if(lpSkill)
	{
		// Calls the skill specific training function.
		if( lpSkill->lpAddPntFunc )
		{
			lpSkill->lpAddPntFunc(lpuTrained, lpUserSkill, dwNbPoints);
		}
		
		lpUserSkill->SetSkillPnts( lpUserSkill->GetTrueSkillPnts() + dwNbPoints );
	}
}
/******************************************************************************/
// Creates the class
void Skills::Create( void )
/******************************************************************************/
{
}
/******************************************************************************/
// Destroys skills
void Skills::Destroy( void )
/******************************************************************************/
{
	LPSKILL lpSkill;
	int i;
	
	for(i = 0; i < c_aSkills.GetSize(); i++)
	{
		 lpSkill = (LPSKILL)c_aSkills.GetAt(i);
		 if(lpSkill)
		 {
			lpSkill->Delete();
		}
	}
}
