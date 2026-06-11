/******************************************************************************
Modify for vs2008 (31/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "SpellEffect.h"
#include "TFC_MAIN.h"
#include "t4clog.h"

/******************************************************************************/
void SpellEffect::CreateEffectStatus(
 Unit *target,      // Unit to which the status should be sent.
 DWORD effectId,    // A unique identifier
 DWORD time,        // The total of the effect in milliseconds
 DWORD totalDuration,
 _SPELL_STRUCT *lpSpell    // The spell structure (for description, icon etc).
)
/******************************************************************************/
{
    string desc = lpSpell->GetDesc( target->GetLang() );    
    if( desc.empty() )
	{
        return;
    }
    DWORD callerAddr;
    GET_CALLER_ADDR( callerAddr );
    _LOG_DEBUG
        LOG_DEBUG_LVL4,
        "CreateEffectStatus unit %u effect %u time %u, totalDuration=%u called from 0x%x.",
        target->GetID(),
        effectId,
        time,
        totalDuration,
        callerAddr
    LOG_

    TFCPacket sending;
    sending << (RQ_SIZE)RQ_CreateEffectStatus;
    sending << (long)effectId;
    sending << (long)time;
    sending << (long)totalDuration;
    sending << (long)lpSpell->dwIcon;
    sending << desc;

    target->SendPlayerMessage( sending );
}
/******************************************************************************/
//  Removes an effect from the status display
void SpellEffect::DispellEffectStatus(
  Unit *target,      // Unit to which the status should be sent.
  DWORD effectId,     // The Id to dispell.
  int iTmp
)
/******************************************************************************/
{
    DWORD callerAddr;
    GET_CALLER_ADDR( callerAddr );
    _LOG_DEBUG
        LOG_DEBUG_LVL4,
        "DispellEffectStatus unit %u effect %u called from 0x%x.",
        target->GetID(),
        effectId,
        callerAddr
    LOG_
	
/*
// 	FILE *pfd = NULL;
// 	fopen_s(&pfd,"C:\\!!!!!DispellFromWho.txt","a+t");
// 	fprintf_s(pfd,"Dispell ID %d  == From%d\n",effectId,iTmp);
// 	fclose(pfd);
*/

    TFCPacket sending;
    sending << (RQ_SIZE)RQ_DispellEffectStatus;
    sending << (long)effectId;

    target->SendPlayerMessage( sending );
}