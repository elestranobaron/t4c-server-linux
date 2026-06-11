/******************************************************************************
Modify for vs2008 (06/05/2009)
/******************************************************************************/
#if !defined(AFX_WEATHEREFFECT_H__50C1F85A_093D_4E1E_A32E_198D020DE6C5__INCLUDED_)
#define AFX_WEATHEREFFECT_H__50C1F85A_093D_4E1E_A32E_198D020DE6C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Unit.h"
#include "Character.h"
#include "SharedStructures.h"
#include "TFC_Main.h"

#define WEATHER_RAIN	0x01
#define WEATHER_SNOW	0x02
#define WEATHER_FOG		0x03

class __declspec(dllexport) WeatherEffect  
{
public:
	WeatherEffect();
	virtual ~WeatherEffect();

	static WeatherEffect* GetInstance( void );

	struct WeatherMapData
	{
		WORD		wWorldID;
		
		WORD		wMapX;
		WORD		wMapY;

		BYTE*		bData;
		COLORREF**	Effects;

		bool		bLoaded;
	};
	
	void SwitchEffectState( DWORD weatherEffect );
	void SetEffectState( short bNewState, DWORD weatherEffect );
	bool GetEffectState( DWORD weatherEffect );

	void DisableAllEffects();
	bool AreEffectsOff();

	void CheckWeatherState( Unit* self, WorldPos newPos );
	
	void SendWeatherState( Unit* self, DWORD weatherEffect, short bValue );

private:
	bool	bRain; // Is rain on
	bool	bSnow; // Is snow on
	bool	bFog;  // Is fog on	

	std::vector< WeatherMapData > vWeatherMaps;	
};

#endif // !defined(AFX_WEATHEREFFECT_H__50C1F85A_093D_4E1E_A32E_198D020DE6C5__INCLUDED_)
