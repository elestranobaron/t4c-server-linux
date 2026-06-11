/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "SysMsgEffect.h"

/******************************************************************************/
REGISTER_SPELL_EFFECT( SYSMSG, SysMsgEffect::NewFunc, SYSMSG_EFFECT, __noop );

/******************************************************************************/
SysMsgEffect::SysMsgEffect()
/******************************************************************************/
{
	m_bError = NOT_LOADED;
	m_bResType = 0;
}
/******************************************************************************/
SysMsgEffect::~SysMsgEffect()
/******************************************************************************/
{
}
/******************************************************************************/
BOOL SysMsgEffect::InputParameter(
 CString csParam,   // Parameter value.
 WORD wParamID      // Parameter ID.
)
/******************************************************************************/
{
    const int ResType = 1;
	const int Msg = 2;

	BOOL boReturn = TRUE;

	switch(wParamID)
	{
		case ResType:
			if(csParam.CompareNoCase("private") == 0)
			{
				m_bResType = PRIVATE;
			}
			else if (csParam.CompareNoCase("global")==0)
			{
				m_bResType = GLOBAL;
			}
			else
			{
				boReturn = FALSE;
			}
			break;
		case Msg:
			m_csMsg = csParam;
			break;
	}
	return boReturn;
}
/******************************************************************************/
// Log errors
void SysMsgEffect::HandleError(BYTE bDaResType, CString csDaMsg)
/******************************************************************************/
{
	_LOG_DEBUG
		LOG_DEBUG_LVL1,
		"Error while casting SysMsgEffect.\r\nMsg Type: %d, Msg: %s",
		bDaResType,
		csDaMsg
	LOG_
}
/******************************************************************************/
void SysMsgEffect::CallEffect(SPELL_EFFECT_PROTOTYPE)
/******************************************************************************/
{
   TRACE("***SysMsgEffect\n");
	if(m_bError == NOT_LOADED)
	{
		m_bError = LOADED;
	}

	if(m_bError == FAILED)
	{
		HandleError(m_bResType, m_csMsg);
	}

	if(m_bError == LOADED)
	{
		switch(m_bResType)
		{
			case GLOBAL:
				{
					GLOBAL_SYSTEM_MESSAGE( BoostFormula::TranslateStringFormula(m_csMsg, self, target) );
				}
				break;
			case PRIVATE:
				{
					if(target->GetType() == U_PC ) 
					{
						PRIVATE_SYSTEM_MESSAGE( BoostFormula::TranslateStringFormula(m_csMsg, self, target) );
					}
					else 
					{
						HandleError(m_bResType, m_csMsg);
					}
				}
				break;
			default:
				break;
		}
	}
}
/******************************************************************************/
// Create a SpellEffect object.
SpellEffect *SysMsgEffect::NewFunc(LPSPELL_STRUCT lpSpell) // The spell structure for any spell effect registration.
/******************************************************************************/
{
  CREATE_EFFECT_HANDLE( SysMsgEffect, 0 )
}