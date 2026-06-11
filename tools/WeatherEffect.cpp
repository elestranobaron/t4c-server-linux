/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "stdafx.h"
#include "WeatherEffect.h"
#include "Random.h"
#include "T4CLog.h"

/******************************************************************************/
WeatherEffect::WeatherEffect()
/******************************************************************************/
{
}
/******************************************************************************/
WeatherEffect::~WeatherEffect()
/******************************************************************************/
{
}
/******************************************************************************/
// Return the only instance of the class
WeatherEffect* WeatherEffect::GetInstance( void )
/******************************************************************************/
{
    static WeatherEffect m_pInstance;
    return &m_pInstance;
}
/******************************************************************************/
// Set a weather effect to on/off
void WeatherEffect::SetEffectState( short bNewState, DWORD weatherEffect )
/******************************************************************************/
{
	switch( weatherEffect )
	{
		// Rain
		case WEATHER_RAIN:
		{
			// Check if it's snowing
			if( bSnow == true )
			{
				// if it's the case, switch the Snow off
				SwitchEffectState( WEATHER_SNOW );
			}

			bRain = false;
			if( bNewState )
			{
				// Switch the rain on
				bRain = true;
			}

			WorldPos wlPos = { 0, 0, 0 };
			Broadcast::BCWeatherMsg( wlPos, 0, WEATHER_RAIN, bNewState );
			
			break;
		}
		case WEATHER_SNOW:
		{
			// Check if it's rainning
			if( bRain == true )
			{
				// if it's the case, switch the Rain off
				SwitchEffectState( WEATHER_RAIN );
			}

			bSnow = false;
			if( bNewState )
			{
				// Switch the rain on
				bSnow = true;
			}

			WorldPos wlPos = { 0, 0, 0 };
			Broadcast::BCWeatherMsg( wlPos, 0, WEATHER_SNOW, bNewState );
			
			break;
		}
		case WEATHER_FOG:
		{
			bFog = false;
			if( bNewState )
			{
				// Switch the fog on
				bFog = true;
			}

			// Broadcast the new state
			// Since fog in cavern/dungeon is allowed
			WorldPos wlPos = { 0, 0, 0 };
			Broadcast::BCWeatherMsg( wlPos, 0, WEATHER_FOG, bNewState );

			break;
		}
		default:
			return;
	}	
}
/******************************************************************************/
// Switch an effect (on->off or off->on)
void WeatherEffect::SwitchEffectState( DWORD weatherEffect )
/******************************************************************************/
{
	switch( weatherEffect )
	{
		case WEATHER_RAIN:
		{
			bRain = !bRain;
			break;
		}
		case WEATHER_SNOW:
		{
			bSnow = !bSnow;
			break;
		}
		case WEATHER_FOG:
		{
			bFog = !bFog;
			break;
		}
		default:
			return;
	}
}
/******************************************************************************/
// Return true if effect is activated, else otherwise
bool WeatherEffect::GetEffectState( DWORD weatherEffect )
/******************************************************************************/
{
	switch( weatherEffect )
	{
		case WEATHER_RAIN:
		{
			return bRain;
		}
		case WEATHER_SNOW:
		{
			return bSnow;
		}
		case WEATHER_FOG:
		{
			return bFog;
		}
	}
	return false;
}
/******************************************************************************/
// Disable all the effects
void WeatherEffect::DisableAllEffects()
/******************************************************************************/
{
	WorldPos wlPos = { 0, 0, 0 };

	bRain = false;	
	Broadcast::BCWeatherMsg( wlPos, 0, WEATHER_RAIN, (short) bRain );

	bSnow = false;
	Broadcast::BCWeatherMsg( wlPos, 0, WEATHER_SNOW, (short) bSnow );

	bFog  = false;
	Broadcast::BCWeatherMsg( wlPos, 0, WEATHER_FOG, (short) bFog );
}
/******************************************************************************/
// Return true if all the effects are switched off
bool WeatherEffect::AreEffectsOff()
/******************************************************************************/
{
	return !( bRain || bSnow || bFog );
}
/******************************************************************************/
// Check the new pos of the player and determines if effects must continue or not
void WeatherEffect::CheckWeatherState( Unit* self, WorldPos newPos )
/******************************************************************************/
{
	// If there are no effects ON, stop here
	if( AreEffectsOff() == true )
	{
		return;
	}

	// Now check the pos
	if( newPos.world == 1 || newPos.world == 2 )
	{
		// Do not show RAIN or SNOW effect in the caverns/dungeons (fog is allowed)
		if( bRain )
		{
			SendWeatherState( self, WEATHER_RAIN, false );			
		}

		if( bSnow )
		{
			SendWeatherState( self, WEATHER_SNOW, false );
		}

		return;
	}

	// If this area is under an effect, broadcast it
	if( ( newPos.X >= 200 && newPos.X <= 300 ) || ( newPos.Y >= 600 && newPos.Y <= 700 ) )
	{
		if( bRain )
			SendWeatherState( self, WEATHER_RAIN, bRain );

		if( bSnow )
			SendWeatherState( self, WEATHER_SNOW, bSnow );

		if( bFog )	
			SendWeatherState( self, WEATHER_FOG, bFog );	

	}

	return;
}
/******************************************************************************/
// Send to the player the state of an effect
void WeatherEffect::SendWeatherState( Unit* self, DWORD weatherEffect, short bValue )
/******************************************************************************/
{	
	TFCPacket sending;

	// Prepare then send the packet	
	sending << (RQ_SIZE) RQ_WeatherMsg;
	sending << (short) bValue;
	sending << (long) weatherEffect;
	self->SendPlayerMessage( sending );

	// Free
	sending.Destroy();
}
