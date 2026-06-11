/******************************************************************************
Modify for vs2008 (26/04/2009)
Add many new stats... ,GOLD ,LUCK, MANA, MAX_MANA, HP, etc etc by Nightmare (28/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "SetStatEffect.h"
#include "../TFC_MAIN.h"
#include "../Broadcast.h"
#include "../format.h"

/******************************************************************************/
#define ISEQUAL( a ) csParam.CompareNoCase( a ) == 0
#define GETSETVAL cSetVal.GetBoost( self, target, 0, 0, range )
REGISTER_SPELL_EFFECT( SETSTAT, SetStatEffect::NewFunc, SETSTAT_EFFECT, __noop );

/******************************************************************************/
SetStatEffect::SetStatEffect()
/******************************************************************************/
{
   wSetType = 0;
}
/******************************************************************************/
SetStatEffect::~SetStatEffect()
/******************************************************************************/
{
}
/******************************************************************************/
BOOL SetStatEffect::InputParameter(
 CString csParam,   // Parameter value.
 WORD wParamID      // Parameter ID.
)
/******************************************************************************/
{
	BOOL boReturn = TRUE;

	const int StatToSet = 1;
	const int SetValue = 2;
	const int Success = 3;

	switch( wParamID )
	{
		case StatToSet:
			if( ISEQUAL( "strength" ) )				wSetType = STR;
			else if( ISEQUAL( "endurance" ) )		wSetType = END;
			else if( ISEQUAL( "agility" ) ) 		wSetType = AGI;
			else if( ISEQUAL( "wisdom" ) )			wSetType = WIS;
			else if( ISEQUAL( "intelligence" ) )	wSetType = INT;
			else if( ISEQUAL( "appearance" ) )		wSetType = APPEARANCE;
			else if( ISEQUAL( "air" ) )				wSetType = AIR_POWER;
			else if( ISEQUAL( "dark" ) )			wSetType = DARK_POWER;
			else if( ISEQUAL( "earth" ) )			wSetType = EARTH_POWER;
			else if( ISEQUAL( "fire" ) )			wSetType = FIRE_POWER;
			else if( ISEQUAL( "light" ) )			wSetType = LIGHT_POWER;
			else if( ISEQUAL( "water" ) )			wSetType = WATER_POWER;
			else if( ISEQUAL( "air resist" ) )		wSetType = AIR_RESIST;
			else if( ISEQUAL( "dark resist" ) )		wSetType = DARK_RESIST;
			else if( ISEQUAL( "earth resist" ) )	wSetType = EARTH_RESIST;
			else if( ISEQUAL( "fire resist" ) )		wSetType = FIRE_RESIST;
			else if( ISEQUAL( "light resist" ) )	wSetType = LIGHT_RESIST;
			else if( ISEQUAL( "water resist" ) )	wSetType = WATER_RESIST;
			else if( ISEQUAL( "luck" ) )			wSetType = LUCK;
			else if( ISEQUAL( "mana" ) )			wSetType = MANA;
			else if( ISEQUAL( "max mana" ) )	    wSetType = MAX_MANA;
			else if( ISEQUAL( "hp" ) )			    wSetType = HP;
			else if( ISEQUAL( "max hp" ) )			wSetType = MAX_HP;
			else if( ISEQUAL( "level" ) )			wSetType = LEVEL;
			else if( ISEQUAL( "xp" ) )			    wSetType = XP;
			else if( ISEQUAL( "gender" ) )			    wSetType = GENDER;
			else if( ISEQUAL( "statpoints (give)" ) )	wSetType = STATPOINTS_GIVE;
			else if( ISEQUAL( "statpoints (set)" ) )	wSetType = STATPOINTS_SET;
			else if( ISEQUAL( "skillpoints (give)" ) )	wSetType = SKILLPOINTS_GIVE;
			else if( ISEQUAL( "skillpoints (set)" ) )	wSetType = SKILLPOINTS_SET;
			else if( ISEQUAL( "skillpoints (use)" ) )	wSetType = SKILLPOINTS_USE;
			else									wSetType = 0; //BLBLBLBL 0 au lieu de -1
			break;
		case SetValue:
			if(!cSetVal.SetFormula(csParam))
			{
				boReturn = FALSE;
			}
			break;
		case Success:
			if( !successPercent.SetFormula(csParam) )
			{
				boReturn = FALSE;
			}
			break;
	}

	return boReturn;
}
/******************************************************************************/
// Take or give an item
void SetStatEffect::CallEffect(SPELL_EFFECT_PROTOTYPE)
/******************************************************************************/
{
   TRACE("***SetStatEffect\n");
	static Random rnd;

	int iSuccess = successPercent.GetBoost( self, target, 0, 0, range );
	if ( rnd(0, 100) <= iSuccess && iSuccess >0 ) 
	{
		Character *ch = static_cast< Character * >( target );
		TFCPacket sending;
		CString csLogTxt;
		csLogTxt = "SetStat Effect called. ";
		csLogTxt += self->GetName( _DEFAULT_LNG );
		csLogTxt += " setted ";
		csLogTxt += target->GetName( _DEFAULT_LNG );
		csLogTxt += "'s ";

		switch( wSetType )
		{
			case STR:
				target->SetSTR( GETSETVAL );
				csLogTxt += "strength";
				break;
			case END:
				target->SetEND( GETSETVAL );
				csLogTxt += "endurance";
				break;
			case AGI:
				target->SetAGI( GETSETVAL );
				csLogTxt += "agility";
				break;
			case WIS:
				target->SetWIS( GETSETVAL );
				csLogTxt += "wisdom";
				break;
			case INT:
				target->SetINT( GETSETVAL );
				csLogTxt += "intelligence";
				break;
			case APPEARANCE:
				target->SetAppearance( GETSETVAL );
				Broadcast::BCObjectChanged( target->GetWL(), _DEFAULT_RANGE_CHANGE,
					target->GetAppearance(),
					target->GetID(),0
					);
				csLogTxt += "appearance";
				break;
			case AIR_POWER:
				target->SetAirPower( GETSETVAL );
				csLogTxt += "air";
				break;
			case DARK_POWER:
				target->SetDarkPower( GETSETVAL );
				csLogTxt += "dark";
				break;
			case EARTH_POWER:
				target->SetEarthPower( GETSETVAL );
				csLogTxt += "earth";
				break;
			case FIRE_POWER:
				target->SetFirePower( GETSETVAL );
				csLogTxt += "fire";
				break;
			case LIGHT_POWER:
				target->SetLightPower( GETSETVAL );
				csLogTxt += "air";
				break;
			case WATER_POWER:
				target->SetWaterPower( GETSETVAL );
				csLogTxt += "water";
				break;
			case AIR_RESIST:
				target->SetAirResist( GETSETVAL );
				csLogTxt += "air resist";
				break;
			case DARK_RESIST:
				target->SetDarkResist( GETSETVAL );
				csLogTxt += "dark resist";
				break;
			case EARTH_RESIST:
				target->SetEarthResist( GETSETVAL );
				csLogTxt += "earth resist";
				break;
			case FIRE_RESIST:
				target->SetFireResist( GETSETVAL );
				csLogTxt += "fire resist";
				break;
			case LIGHT_RESIST:
				target->SetLightResist( GETSETVAL );
				csLogTxt += "air resist";
				break;
			case WATER_RESIST:
				target->SetWaterResist( GETSETVAL );
				csLogTxt += "water resist";
				break;
			case LUCK:
				target->SetLCK( GETSETVAL );
				csLogTxt += "luck";
				break;
			case MANA:
				target->SetMana( GETSETVAL, TRUE );
				csLogTxt += "mana";
				break;
			case MAX_MANA:
				target->SetMaxMana( GETSETVAL );
            	if( target->GetType() == U_PC )
				{
                	ch->PacketStatus( sending );
	                ch->SendPlayerMessage( sending );
    	        }
				csLogTxt += "max mana";
				break;
			case HP:
				target->SetHP( GETSETVAL , TRUE );
				csLogTxt += "hp";
				break;
			case MAX_HP:
				target->SetMaxHP( GETSETVAL );
	            if( target->GetType() == U_PC )
				{
                	ch->PacketStatus( sending );
	                ch->SendPlayerMessage( sending );
    	        }
				csLogTxt += "max hp";
				break;
			case LEVEL:
				if( GETSETVAL != 0)
				{
					target->SetLevel( GETSETVAL );
					csLogTxt += "level";
				}else 
					csLogTxt += "level [change was forbidden]";
			break;
    		case XP:
		        {
	            	int amount = cSetVal.GetBoost( self, target );
		            target->SetXP(target->GetXP()+amount);
		            TFormat format;
		            target->SendInfoMessage( format(_STR(15044, target->GetLang()),amount),0x0570D5 );
	               csLogTxt += "xp";
	    	    }
				break;
			case GENDER:
				if( GETSETVAL == 0)
				{
					target->SetGender( GETSETVAL );
					csLogTxt += "gender";
				}
				else if( GETSETVAL == 1)
				{
					target->SetGender( GETSETVAL );
					csLogTxt += "gender";
				}
				else
				{
					target->SetGender( 0 );
					csLogTxt += "gender [default to 0]";
				}
				break;
			case STATPOINTS_GIVE:
				ch->SetStatPoints( ch->GetStatPoints() + GETSETVAL );
				csLogTxt += "statpoints (give)";
				break;
			case STATPOINTS_SET:
				ch->SetStatPoints( GETSETVAL );
				csLogTxt += "statpoints (set)";
				break;
			case SKILLPOINTS_GIVE:
				ch->GiveSkillPoints( GETSETVAL );
				csLogTxt += "skillpoints (give)";
				break;
			case SKILLPOINTS_SET:
				ch->UseSkillPnts( ch->GetSkillPoints() );
				ch->GiveSkillPoints( GETSETVAL );
				csLogTxt += "skillpoints (set)";
				break;
			case SKILLPOINTS_USE:
				if( ch->GetSkillPoints() >= GETSETVAL )
				{
					ch->UseSkillPnts( GETSETVAL );
					csLogTxt += "skillpoints (use)";
				}
				else csLogTxt += "skillpoints (use) [change was forbidden]";
				break;
		}

		csLogTxt += " to ";
		csLogTxt += (wchar_t)GETSETVAL;

		_LOG_GAMEOP
			LOG_SYSOP, 
			(char *)(LPCTSTR)csLogTxt 
		LOG_
	}
}
/******************************************************************************/
// Create a SpellEffect object.
SpellEffect *SetStatEffect::NewFunc(LPSPELL_STRUCT lpSpell) // The spell structure for any spell effect registration.
/******************************************************************************/
{
  CREATE_EFFECT_HANDLE( SetStatEffect, 0 )
}