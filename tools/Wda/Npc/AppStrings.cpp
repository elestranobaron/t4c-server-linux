/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "Stdafx.h"
#include "AppStrings.h"

/******************************************************************************/
std::string GetAppString( DWORD stringId )
/******************************************************************************/
{
	// Return some strings for special types of IDs.
	switch( stringId )
	{   
		case IDS_INITIAL_KEYWORD_NAME:		return "1KW";
		case IDS_INITIAL_KEYWORD_HELP:		return "2KW";
		case IDS_DEFAULT_KEYWORD_NAME:		return "3KW";
		case IDS_DEFAULT_KEYWORD_HELP:		return "4KW";
		case IDS_BYE_KEYWORD_NAME:			   return "5KW";
		case IDS_BYE_KEYWORD_HELP:			   return "6KW";
		case IDS_ONDEATH_KEYWORD_NAME:		return "OnDeath";
		case IDS_ONDEATH_KEYWORD_HELP:		return "8KW";
		case IDS_ONATTACK_KEYWORD_NAME:		return "OnAttack";
		case IDS_ONATTACK_KEYWORD_HELP:		return "10KW";
		case IDS_ONATTACKED_KEYWORD_NAME:	return "OnAttacked";
		case IDS_ONATTACKED_KEYWORD_HELP:	return "12KW";
      case IDS_ONATTACKHIT_KEYWORD_NAME:	return "OnAttackHit";
      case IDS_ONATTACKHIT_KEYWORD_HELP:	return "14KW";
      case IDS_ONHIT_KEYWORD_NAME:	   	return "OnHit";
      case IDS_ONHIT_KEYWORD_HELP:	   	return "16KW";
      case IDS_ONPOPUP_KEYWORD_NAME:		return "OnPopup";
      case IDS_ONPOPUP_KEYWORD_HELP:		return "18KW";
	}
	return "";
}
