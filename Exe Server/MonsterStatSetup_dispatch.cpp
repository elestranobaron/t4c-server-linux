#include "stdafx.h"

void MonsterStatSetup_GameOps(void);
void MonsterStatSetup_NPCs(void);
void MonsterStatSetup_NPCAddOn(void);
void MonsterStatSetup_Arakas(void);
void MonsterStatSetup_RavensDust(void);
void MonsterStatSetup_Remort(void);
void MonsterStatSetup_Stoneheim(void);
void MonsterStatSetup_WindHowl(void);

void MonsterStatDestroy_GameOps(void);
void MonsterStatDestroy_NPCs(void);
void MonsterStatDestroy_NPCAddOn(void);
void MonsterStatDestroy_Arakas(void);
void MonsterStatDestroy_RavensDust(void);
void MonsterStatDestroy_Remort(void);
void MonsterStatDestroy_Stoneheim(void);
void MonsterStatDestroy_WindHowl(void);

void MonsterStatSetup(void)
{
	MonsterStatSetup_GameOps();
	MonsterStatSetup_NPCs();
	MonsterStatSetup_NPCAddOn();
	MonsterStatSetup_Arakas();
	MonsterStatSetup_RavensDust();
	MonsterStatSetup_Remort();
	MonsterStatSetup_Stoneheim();
	MonsterStatSetup_WindHowl();
}

void MonsterStatDestroy(void)
{
	MonsterStatDestroy_GameOps();
	MonsterStatDestroy_NPCs();
	MonsterStatDestroy_NPCAddOn();
	MonsterStatDestroy_Arakas();
	MonsterStatDestroy_RavensDust();
	MonsterStatDestroy_Remort();
	MonsterStatDestroy_Stoneheim();
	MonsterStatDestroy_WindHowl();
}
