/******************************************************************************
Modify for vs2008 (30/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "Clans.h"
#include "GameDefs.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
WORD Clans::NumberOfClans;

/******************************************************************************/
Clans::Clans()
/******************************************************************************/
{
	MemorizedClans = NULL;
	Create();
}
/******************************************************************************/
Clans::~Clans()
/******************************************************************************/
{
	if(MemorizedClans != NULL)
	{
		delete MemorizedClans;
		MemorizedClans = NULL;
	}
}
/******************************************************************************/
void Clans::Create()
/******************************************************************************/
{
	WORD i;
	// default _NEUTRAL association with all other clans
	MemorizedClans = new char[NumberOfClans];
	
	for(i = 0; i < NumberOfClans; i++)
	{
		MemorizedClans[i] = 0;
	}
}
/******************************************************************************/
char Clans::MutualRelation(WORD clan, DWORD ID)
/******************************************************************************/
{
	// First check players, since players have top priority over clans
	if(MemorizedClans)
	{
		if(clan < NumberOfClans) 
		{
			return MemorizedClans[clan];
		}
	}
	return 0; // If we didn't find any reference to the creature, return a neutral relation
}
/******************************************************************************/
void Clans::SetPlayerRelation(DWORD ID, char AgressivnessPercentage)
/******************************************************************************/
{
	MemorizedPlayers.SetFlag(ID, AgressivnessPercentage);
}
/******************************************************************************/
void Clans::SetClanRelation(WORD clan, char AgressivnessPercentage)
/******************************************************************************/
{
	if(MemorizedClans) 
	{
		if(clan < NumberOfClans) MemorizedClans[clan] = AgressivnessPercentage;
	}
}
/******************************************************************************/
void Clans::ResetAllRelationsWith(DWORD ID)
/******************************************************************************/
{
	MemorizedPlayers.RemoveFlag(ID);
}
/******************************************************************************/
void Clans::SetNumberOfClans(WORD newNumberOfClans)
/******************************************************************************/
{
	NumberOfClans = newNumberOfClans;
}
/******************************************************************************/
WORD Clans::GetNumberOfClans()
/******************************************************************************/
{
	return NumberOfClans;
}
/******************************************************************************/
// This allows initialisation of the different monster clans,
Clans *Clans::InitClans( void )
/******************************************************************************/
{
	Clans *ClanArray = new Clans[ NumberOfClans ];

	return ClanArray;
}