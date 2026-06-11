/******************************************************************************
Modify for vs2008 (26/04/2009)
******************************************************************************/
#include "StdAfx.h"

#include <NPC.H>
#include "ExitGate.h"
#include "ArenaMob50.h"
#include "ArenaMob200.h"
#include "ArenaMob100.h"
#include "ArenaMob475.h"
#include "ArenaMob450.h"
#include "ArenaMob40.h"
#include "ArenaMob375.h"
#include "ArenaMob350.h"
#include "ArenaMob325.h"
#include "ArenaMob300.h"
#include "ArenaMob425.h"
#include "ArenaMob250.h"
#include "ArenaMob225.h"
#include "ArenaMob400.h"
#include "ArenaMob190.h"
#include "ArenaMob180.h"
#include "ArenaMob170.h"
#include "ArenaMob160.h"
#include "ArenaMob150.h"
#include "ArenaMob140.h"
#include "ArenaMob130.h"
#include "ArenaMob120.h"
#include "ArenaMob110.h"
#include "DoktorSpine.h"
#include "ArenaMob275.h"
#include "ColosseumClerk.h"
#include "ArenaMob500.h"
#include "ColosseumOwner.h"
#include "DrVonPyre.h"
#include "MadDoorEntrance.h"
#include "MadDoorExit.h"
#include "ArenaMob90.h"
#include "ArenaMob80.h"
#include "ArenaMob70.h"
#include "ArenaMob60.h"
#include "MadMadrigan.h"
#include "MOBCrazedNurse.h"
#include "MOBDerangedOrderly.h"
#include "MOBLoon.h"
#include "MOBMadMan.h"
#include "MOBMadPatient.h"
#include "MOBRavingLunatic.h"
#include "PortalA2.h"
#include "PortalB4.h"
#include "PortalC1.h"
#include "PortalC5.h"
#include "PortalH3.h"
#include "PortalJ1.h"
#include "PortalJ3.h"
#include "PortalJ4.h"
#include "WardenVortimer.h"
#include "MakrshPtanghSpawner.h"
#include "MakrshPtangh2.h"
#include "MakrshPtangh.h"
#include "MakrshPortal.h"
#include "MakrshPortal2.h"
#include "MakrshPortal3.h"
#include "MakrshPortal4.h"
#include "MakrshPortal5.h"
#include "Archibald.h"
#include "ForestWanderer.h"
#include "MOBRunawayPatient.h"
#include "ColosseumClerkXP.h"
#include "ArenaMobXP50.h"
#include "ArenaMobXP200.h"
#include "ArenaMobXP100.h"
#include "ArenaMobXP475.h"
#include "ArenaMobXP450.h"
#include "ArenaMobXP40.h"
#include "ArenaMobXP375.h"
#include "ArenaMobXP350.h"
#include "ArenaMobXP325.h"
#include "ArenaMobXP300.h"
#include "ArenaMobXP425.h"
#include "ArenaMobXP250.h"
#include "ArenaMobXP225.h"
#include "ArenaMobXP400.h"
#include "ArenaMobXP190.h"
#include "ArenaMobXP180.h"
#include "ArenaMobXP170.h"
#include "ArenaMobXP160.h"
#include "ArenaMobXP150.h"
#include "ArenaMobXP140.h"
#include "ArenaMobXP130.h"
#include "ArenaMobXP120.h"
#include "ArenaMobXP110.h"
#include "ArenaMobXP275.h"
#include "ArenaMobXP500.h"
#include "ArenaMobXP90.h"
#include "ArenaMobXP80.h"
#include "ArenaMobXP70.h"
#include "ArenaMobXP60.h"
#include "Arena2MobXP50.h"
#include "Arena2MobXP200.h"
#include "Arena2MobXP100.h"
#include "Arena2MobXP475.h"
#include "Arena2MobXP450.h"
#include "Arena2MobXP40.h"
#include "Arena2MobXP375.h"
#include "Arena2MobXP350.h"
#include "Arena2MobXP325.h"
#include "Arena2MobXP300.h"
#include "Arena2MobXP425.h"
#include "Arena2MobXP250.h"
#include "Arena2MobXP225.h"
#include "Arena2MobXP400.h"
#include "Arena2MobXP190.h"
#include "Arena2MobXP180.h"
#include "Arena2MobXP170.h"
#include "Arena2MobXP160.h"
#include "Arena2MobXP150.h"
#include "Arena2MobXP140.h"
#include "Arena2MobXP130.h"
#include "Arena2MobXP120.h"
#include "Arena2MobXP110.h"
#include "Arena2MobXP275.h"
#include "Arena2MobXP500.h"
#include "Arena2MobXP90.h"
#include "Arena2MobXP80.h"
#include "Arena2MobXP70.h"
#include "Arena2MobXP60.h"
#include "Arena3MobXP50.h"
#include "Arena3MobXP200.h"
#include "Arena3MobXP100.h"
#include "Arena3MobXP475.h"
#include "Arena3MobXP450.h"
#include "Arena3MobXP40.h"
#include "Arena3MobXP375.h"
#include "Arena3MobXP350.h"
#include "Arena3MobXP325.h"
#include "Arena3MobXP300.h"
#include "Arena3MobXP425.h"
#include "Arena3MobXP250.h"
#include "Arena3MobXP225.h"
#include "Arena3MobXP400.h"
#include "Arena3MobXP190.h"
#include "Arena3MobXP180.h"
#include "Arena3MobXP170.h"
#include "Arena3MobXP160.h"
#include "Arena3MobXP150.h"
#include "Arena3MobXP140.h"
#include "Arena3MobXP130.h"
#include "Arena3MobXP120.h"
#include "Arena3MobXP110.h"
#include "Arena3MobXP275.h"
#include "Arena3MobXP500.h"
#include "Arena3MobXP90.h"
#include "Arena3MobXP80.h"
#include "Arena3MobXP70.h"
#include "Arena3MobXP60.h"
#include "Arena4MobXP50.h"
#include "Arena4MobXP200.h"
#include "Arena4MobXP100.h"
#include "Arena4MobXP475.h"
#include "Arena4MobXP450.h"
#include "Arena4MobXP40.h"
#include "Arena4MobXP375.h"
#include "Arena4MobXP350.h"
#include "Arena4MobXP325.h"
#include "Arena4MobXP300.h"
#include "Arena4MobXP425.h"
#include "Arena4MobXP250.h"
#include "Arena4MobXP225.h"
#include "Arena4MobXP400.h"
#include "Arena4MobXP190.h"
#include "Arena4MobXP180.h"
#include "Arena4MobXP170.h"
#include "Arena4MobXP160.h"
#include "Arena4MobXP150.h"
#include "Arena4MobXP140.h"
#include "Arena4MobXP130.h"
#include "Arena4MobXP120.h"
#include "Arena4MobXP110.h"
#include "Arena4MobXP275.h"
#include "Arena4MobXP500.h"
#include "Arena4MobXP90.h"
#include "Arena4MobXP80.h"
#include "Arena4MobXP70.h"
#include "Arena4MobXP60.h"

/******************************************************************************/
// NPC object definitions.

ExitGate								EXITGATE;
ArenaMob50                              ARENAMOB50;
ArenaMob200                             ARENAMOB200;
ArenaMob100                             ARENAMOB100;
ArenaMob475                             ARENAMOB475;
ArenaMob450                             ARENAMOB450;
ArenaMob40                              ARENAMOB40;
ArenaMob375                             ARENAMOB375;
ArenaMob350                             ARENAMOB350;
ArenaMob325                             ARENAMOB325;
ArenaMob300                             ARENAMOB300;
ArenaMob425                             ARENAMOB425;
ArenaMob250                             ARENAMOB250;
ArenaMob225                             ARENAMOB225;
ArenaMob400                             ARENAMOB400;
ArenaMob190                             ARENAMOB190;
ArenaMob180                             ARENAMOB180;
ArenaMob170                             ARENAMOB170;
ArenaMob160                             ARENAMOB160;
ArenaMob150                             ARENAMOB150;
ArenaMob140                             ARENAMOB140;
ArenaMob130                             ARENAMOB130;
ArenaMob120                             ARENAMOB120;
ArenaMob110                             ARENAMOB110;
DoktorSpine                             DOKTORSPINE;
ArenaMob275                             ARENAMOB275;
ColosseumClerk                          COLOSSEUMCLERK;
ArenaMob500                             ARENAMOB500;
ColosseumOwner                          COLOSSEUMOWNER;
DrVonPyre                               DRVONPYRE;
MadDoorEntrance                         MADDOORENTRANCE;
MadDoorExit                             MADDOOREXIT;
ArenaMob90                              ARENAMOB90;
ArenaMob80                              ARENAMOB80;
ArenaMob70                              ARENAMOB70;
ArenaMob60                              ARENAMOB60;
MadMadrigan                             MADMADRIGAN;
MOBCrazedNurse                          MOBCRAZEDNURSE;
MOBDerangedOrderly                      MOBDERANGEDORDERLY;
MOBLoon                                 MOBLOON;
MOBMadMan                               MOBMADMAN;
MOBMadPatient                           MOBMADPATIENT;
MOBRavingLunatic                        MOBRAVINGLUNATIC;
PortalA2                                PORTALA2;
PortalB4                                PORTALB4;
PortalC1                                PORTALC1;
PortalC5                                PORTALC5;
PortalH3                                PORTALH3;
PortalJ1                                PORTALJ1;
PortalJ3                                PORTALJ3;
PortalJ4                                PORTALJ4;
WardenVortimer                          WARDENVORTIMER;
MakrshPtanghSpawner                     MAKRSHPTANGHSPAWNER;
MakrshPtangh2                           MAKRSHPTANGH2;
MakrshPtangh                            MAKRSHPTANGH;
MakrshPortal                            MAKRSHPORTAL;
MakrshPortal2                           MAKRSHPORTAL2;
MakrshPortal3                           MAKRSHPORTAL3;
MakrshPortal4                           MAKRSHPORTAL4;
MakrshPortal5                           MAKRSHPORTAL5;
Archibald                               ARCHIBALD;
ForestWanderer                          FORESTWANDERER;
MOBRunawayPatient                       MOBRUNAWAYPATIENT;
ColosseumClerkXP                          COLOSSEUMCLERKXP;
ArenaMobXP50                              ARENAMOBXP50;
ArenaMobXP200                             ARENAMOBXP200;
ArenaMobXP100                             ARENAMOBXP100;
ArenaMobXP475                             ARENAMOBXP475;
ArenaMobXP450                             ARENAMOBXP450;
ArenaMobXP40                              ARENAMOBXP40;
ArenaMobXP375                             ARENAMOBXP375;
ArenaMobXP350                             ARENAMOBXP350;
ArenaMobXP325                             ARENAMOBXP325;
ArenaMobXP300                             ARENAMOBXP300;
ArenaMobXP425                             ARENAMOBXP425;
ArenaMobXP250                             ARENAMOBXP250;
ArenaMobXP225                             ARENAMOBXP225;
ArenaMobXP400                             ARENAMOBXP400;
ArenaMobXP190                             ARENAMOBXP190;
ArenaMobXP180                             ARENAMOBXP180;
ArenaMobXP170                             ARENAMOBXP170;
ArenaMobXP160                             ARENAMOBXP160;
ArenaMobXP150                             ARENAMOBXP150;
ArenaMobXP140                             ARENAMOBXP140;
ArenaMobXP130                             ARENAMOBXP130;
ArenaMobXP120                             ARENAMOBXP120;
ArenaMobXP110                             ARENAMOBXP110;
ArenaMobXP275                             ARENAMOBXP275;
ArenaMobXP500                             ARENAMOBXP500;
ArenaMobXP90                              ARENAMOBXP90;
ArenaMobXP80                              ARENAMOBXP80;
ArenaMobXP70                              ARENAMOBXP70;
ArenaMobXP60                              ARENAMOBXP60;
Arena2MobXP50                              ARENA2MOBXP50;
Arena2MobXP200                             ARENA2MOBXP200;
Arena2MobXP100                             ARENA2MOBXP100;
Arena2MobXP475                             ARENA2MOBXP475;
Arena2MobXP450                             ARENA2MOBXP450;
Arena2MobXP40                              ARENA2MOBXP40;
Arena2MobXP375                             ARENA2MOBXP375;
Arena2MobXP350                             ARENA2MOBXP350;
Arena2MobXP325                             ARENA2MOBXP325;
Arena2MobXP300                             ARENA2MOBXP300;
Arena2MobXP425                             ARENA2MOBXP425;
Arena2MobXP250                             ARENA2MOBXP250;
Arena2MobXP225                             ARENA2MOBXP225;
Arena2MobXP400                             ARENA2MOBXP400;
Arena2MobXP190                             ARENA2MOBXP190;
Arena2MobXP180                             ARENA2MOBXP180;
Arena2MobXP170                             ARENA2MOBXP170;
Arena2MobXP160                             ARENA2MOBXP160;
Arena2MobXP150                             ARENA2MOBXP150;
Arena2MobXP140                             ARENA2MOBXP140;
Arena2MobXP130                             ARENA2MOBXP130;
Arena2MobXP120                             ARENA2MOBXP120;
Arena2MobXP110                             ARENA2MOBXP110;
Arena2MobXP275                             ARENA2MOBXP275;
Arena2MobXP500                             ARENA2MOBXP500;
Arena2MobXP90                              ARENA2MOBXP90;
Arena2MobXP80                              ARENA2MOBXP80;
Arena2MobXP70                              ARENA2MOBXP70;
Arena2MobXP60                              ARENA2MOBXP60;
Arena3MobXP50                              ARENA3MOBXP50;
Arena3MobXP200                             ARENA3MOBXP200;
Arena3MobXP100                             ARENA3MOBXP100;
Arena3MobXP475                             ARENA3MOBXP475;
Arena3MobXP450                             ARENA3MOBXP450;
Arena3MobXP40                              ARENA3MOBXP40;
Arena3MobXP375                             ARENA3MOBXP375;
Arena3MobXP350                             ARENA3MOBXP350;
Arena3MobXP325                             ARENA3MOBXP325;
Arena3MobXP300                             ARENA3MOBXP300;
Arena3MobXP425                             ARENA3MOBXP425;
Arena3MobXP250                             ARENA3MOBXP250;
Arena3MobXP225                             ARENA3MOBXP225;
Arena3MobXP400                             ARENA3MOBXP400;
Arena3MobXP190                             ARENA3MOBXP190;
Arena3MobXP180                             ARENA3MOBXP180;
Arena3MobXP170                             ARENA3MOBXP170;
Arena3MobXP160                             ARENA3MOBXP160;
Arena3MobXP150                             ARENA3MOBXP150;
Arena3MobXP140                             ARENA3MOBXP140;
Arena3MobXP130                             ARENA3MOBXP130;
Arena3MobXP120                             ARENA3MOBXP120;
Arena3MobXP110                             ARENA3MOBXP110;
Arena3MobXP275                             ARENA3MOBXP275;
Arena3MobXP500                             ARENA3MOBXP500;
Arena3MobXP90                              ARENA3MOBXP90;
Arena3MobXP80                              ARENA3MOBXP80;
Arena3MobXP70                              ARENA3MOBXP70;
Arena3MobXP60                              ARENA3MOBXP60;
Arena4MobXP50                              ARENA4MOBXP50;
Arena4MobXP200                             ARENA4MOBXP200;
Arena4MobXP100                             ARENA4MOBXP100;
Arena4MobXP475                             ARENA4MOBXP475;
Arena4MobXP450                             ARENA4MOBXP450;
Arena4MobXP40                              ARENA4MOBXP40;
Arena4MobXP375                             ARENA4MOBXP375;
Arena4MobXP350                             ARENA4MOBXP350;
Arena4MobXP325                             ARENA4MOBXP325;
Arena4MobXP300                             ARENA4MOBXP300;
Arena4MobXP425                             ARENA4MOBXP425;
Arena4MobXP250                             ARENA4MOBXP250;
Arena4MobXP225                             ARENA4MOBXP225;
Arena4MobXP400                             ARENA4MOBXP400;
Arena4MobXP190                             ARENA4MOBXP190;
Arena4MobXP180                             ARENA4MOBXP180;
Arena4MobXP170                             ARENA4MOBXP170;
Arena4MobXP160                             ARENA4MOBXP160;
Arena4MobXP150                             ARENA4MOBXP150;
Arena4MobXP140                             ARENA4MOBXP140;
Arena4MobXP130                             ARENA4MOBXP130;
Arena4MobXP120                             ARENA4MOBXP120;
Arena4MobXP110                             ARENA4MOBXP110;
Arena4MobXP275                             ARENA4MOBXP275;
Arena4MobXP500                             ARENA4MOBXP500;
Arena4MobXP90                              ARENA4MOBXP90;
Arena4MobXP80                              ARENA4MOBXP80;
Arena4MobXP70                              ARENA4MOBXP70;
Arena4MobXP60                              ARENA4MOBXP60;

/******************************************************************************/
// NPC registration.

INIT_NPC_DATA

	 REGISTER_NPC( COLOSSEUMCLERKXP )
	 REGISTER_NPC( EXITGATE ) 
    REGISTER_NPC( ARENAMOB50 )
    REGISTER_NPC( ARENAMOB200 )
    REGISTER_NPC( ARENAMOB100 )
    REGISTER_NPC( ARENAMOB475 )
    REGISTER_NPC( ARENAMOB450 )
    REGISTER_NPC( ARENAMOB40 )
    REGISTER_NPC( ARENAMOB375 )
    REGISTER_NPC( ARENAMOB350 )
    REGISTER_NPC( ARENAMOB325 )
    REGISTER_NPC( ARENAMOB300 )
    REGISTER_NPC( ARENAMOB425 )
    REGISTER_NPC( ARENAMOB250 )
    REGISTER_NPC( ARENAMOB225 )
    REGISTER_NPC( ARENAMOB400 )
    REGISTER_NPC( ARENAMOB190 )
    REGISTER_NPC( ARENAMOB180 )
    REGISTER_NPC( ARENAMOB170 )
    REGISTER_NPC( ARENAMOB160 )
    REGISTER_NPC( ARENAMOB150 )
    REGISTER_NPC( ARENAMOB140 )
    REGISTER_NPC( ARENAMOB130 )
    REGISTER_NPC( ARENAMOB120 )
    REGISTER_NPC( ARENAMOB110 )
    REGISTER_NPC( DOKTORSPINE )
    REGISTER_NPC( ARENAMOB275 )
    REGISTER_NPC( COLOSSEUMCLERK )
    REGISTER_NPC( ARENAMOB500 )
    REGISTER_NPC( COLOSSEUMOWNER )
    REGISTER_NPC( DRVONPYRE )
    REGISTER_NPC( MADDOORENTRANCE )
    REGISTER_NPC( MADDOOREXIT )
    REGISTER_NPC( ARENAMOB90 )
    REGISTER_NPC( ARENAMOB80 )
    REGISTER_NPC( ARENAMOB70 )
    REGISTER_NPC( ARENAMOB60 )
    REGISTER_NPC( MADMADRIGAN )
    REGISTER_NPC( MOBCRAZEDNURSE )
    REGISTER_NPC( MOBDERANGEDORDERLY )
    REGISTER_NPC( MOBLOON )
    REGISTER_NPC( MOBMADMAN )
    REGISTER_NPC( MOBMADPATIENT )
    REGISTER_NPC( MOBRAVINGLUNATIC )
    REGISTER_NPC( PORTALA2 )
    REGISTER_NPC( PORTALB4 )
    REGISTER_NPC( PORTALC1 )
    REGISTER_NPC( PORTALC5 )
    REGISTER_NPC( PORTALH3 )
    REGISTER_NPC( PORTALJ1 )
    REGISTER_NPC( PORTALJ3 )
    REGISTER_NPC( PORTALJ4 )
    REGISTER_NPC( WARDENVORTIMER )
    REGISTER_NPC( MAKRSHPTANGHSPAWNER )
    REGISTER_NPC( MAKRSHPTANGH2 )
    REGISTER_NPC( MAKRSHPTANGH )
    REGISTER_NPC( MAKRSHPORTAL )
    REGISTER_NPC( MAKRSHPORTAL2 )
    REGISTER_NPC( MAKRSHPORTAL3 )
    REGISTER_NPC( MAKRSHPORTAL4 )
    REGISTER_NPC( MAKRSHPORTAL5 )
    REGISTER_NPC( ARCHIBALD )
    REGISTER_NPC( FORESTWANDERER )
    REGISTER_NPC( MOBRUNAWAYPATIENT )
    REGISTER_NPC( ARENAMOBXP50 )
    REGISTER_NPC( ARENAMOBXP200 )
    REGISTER_NPC( ARENAMOBXP100 )
    REGISTER_NPC( ARENAMOBXP475 )
    REGISTER_NPC( ARENAMOBXP450 )
    REGISTER_NPC( ARENAMOBXP40 )
    REGISTER_NPC( ARENAMOBXP375 )
    REGISTER_NPC( ARENAMOBXP350 )
    REGISTER_NPC( ARENAMOBXP325 )
    REGISTER_NPC( ARENAMOBXP300 )
    REGISTER_NPC( ARENAMOBXP425 )
    REGISTER_NPC( ARENAMOBXP250 )
    REGISTER_NPC( ARENAMOBXP225 )
    REGISTER_NPC( ARENAMOBXP400 )
    REGISTER_NPC( ARENAMOBXP190 )
    REGISTER_NPC( ARENAMOBXP180 )
    REGISTER_NPC( ARENAMOBXP170 )
    REGISTER_NPC( ARENAMOBXP160 )
    REGISTER_NPC( ARENAMOBXP150 )
    REGISTER_NPC( ARENAMOBXP140 )
    REGISTER_NPC( ARENAMOBXP130 )
    REGISTER_NPC( ARENAMOBXP120 )
    REGISTER_NPC( ARENAMOBXP110 )
    REGISTER_NPC( ARENAMOBXP275 )
    REGISTER_NPC( ARENAMOBXP500 )
    REGISTER_NPC( ARENAMOBXP90 )
    REGISTER_NPC( ARENAMOBXP80 )
    REGISTER_NPC( ARENAMOBXP70 )
    REGISTER_NPC( ARENAMOBXP60 )
    REGISTER_NPC( ARENA2MOBXP50 )
    REGISTER_NPC( ARENA2MOBXP200 )
    REGISTER_NPC( ARENA2MOBXP100 )
    REGISTER_NPC( ARENA2MOBXP475 )
    REGISTER_NPC( ARENA2MOBXP450 )
    REGISTER_NPC( ARENA2MOBXP40 )
    REGISTER_NPC( ARENA2MOBXP375 )
    REGISTER_NPC( ARENA2MOBXP350 )
    REGISTER_NPC( ARENA2MOBXP325 )
    REGISTER_NPC( ARENA2MOBXP300 )
    REGISTER_NPC( ARENA2MOBXP425 )
    REGISTER_NPC( ARENA2MOBXP250 )
    REGISTER_NPC( ARENA2MOBXP225 )
    REGISTER_NPC( ARENA2MOBXP400 )
    REGISTER_NPC( ARENA2MOBXP190 )
    REGISTER_NPC( ARENA2MOBXP180 )
    REGISTER_NPC( ARENA2MOBXP170 )
    REGISTER_NPC( ARENA2MOBXP160 )
    REGISTER_NPC( ARENA2MOBXP150 )
    REGISTER_NPC( ARENA2MOBXP140 )
    REGISTER_NPC( ARENA2MOBXP130 )
    REGISTER_NPC( ARENA2MOBXP120 )
    REGISTER_NPC( ARENA2MOBXP110 )
    REGISTER_NPC( ARENA2MOBXP275 )
    REGISTER_NPC( ARENA2MOBXP500 )
    REGISTER_NPC( ARENA2MOBXP90 )
    REGISTER_NPC( ARENA2MOBXP80 )
    REGISTER_NPC( ARENA2MOBXP70 )
    REGISTER_NPC( ARENA2MOBXP60 )
    REGISTER_NPC( ARENA3MOBXP50 )
    REGISTER_NPC( ARENA3MOBXP200 )
    REGISTER_NPC( ARENA3MOBXP100 )
    REGISTER_NPC( ARENA3MOBXP475 )
    REGISTER_NPC( ARENA3MOBXP450 )
    REGISTER_NPC( ARENA3MOBXP40 )
    REGISTER_NPC( ARENA3MOBXP375 )
    REGISTER_NPC( ARENA3MOBXP350 )
    REGISTER_NPC( ARENA3MOBXP325 )
    REGISTER_NPC( ARENA3MOBXP300 )
    REGISTER_NPC( ARENA3MOBXP425 )
    REGISTER_NPC( ARENA3MOBXP250 )
    REGISTER_NPC( ARENA3MOBXP225 )
    REGISTER_NPC( ARENA3MOBXP400 )
    REGISTER_NPC( ARENA3MOBXP190 )
    REGISTER_NPC( ARENA3MOBXP180 )
    REGISTER_NPC( ARENA3MOBXP170 )
    REGISTER_NPC( ARENA3MOBXP160 )
    REGISTER_NPC( ARENA3MOBXP150 )
    REGISTER_NPC( ARENA3MOBXP140 )
    REGISTER_NPC( ARENA3MOBXP130 )
    REGISTER_NPC( ARENA3MOBXP120 )
    REGISTER_NPC( ARENA3MOBXP110 )
    REGISTER_NPC( ARENA3MOBXP275 )
    REGISTER_NPC( ARENA3MOBXP500 )
    REGISTER_NPC( ARENA3MOBXP90 )
    REGISTER_NPC( ARENA3MOBXP80 )
    REGISTER_NPC( ARENA3MOBXP70 )
    REGISTER_NPC( ARENA3MOBXP60 )
    REGISTER_NPC( ARENA4MOBXP50 )
    REGISTER_NPC( ARENA4MOBXP200 )
    REGISTER_NPC( ARENA4MOBXP100 )
    REGISTER_NPC( ARENA4MOBXP475 )
    REGISTER_NPC( ARENA4MOBXP450 )
    REGISTER_NPC( ARENA4MOBXP40 )
    REGISTER_NPC( ARENA4MOBXP375 )
    REGISTER_NPC( ARENA4MOBXP350 )
    REGISTER_NPC( ARENA4MOBXP325 )
    REGISTER_NPC( ARENA4MOBXP300 )
    REGISTER_NPC( ARENA4MOBXP425 )
    REGISTER_NPC( ARENA4MOBXP250 )
    REGISTER_NPC( ARENA4MOBXP225 )
    REGISTER_NPC( ARENA4MOBXP400 )
    REGISTER_NPC( ARENA4MOBXP190 )
    REGISTER_NPC( ARENA4MOBXP180 )
    REGISTER_NPC( ARENA4MOBXP170 )
    REGISTER_NPC( ARENA4MOBXP160 )
    REGISTER_NPC( ARENA4MOBXP150 )
    REGISTER_NPC( ARENA4MOBXP140 )
    REGISTER_NPC( ARENA4MOBXP130 )
    REGISTER_NPC( ARENA4MOBXP120 )
    REGISTER_NPC( ARENA4MOBXP110 )
    REGISTER_NPC( ARENA4MOBXP275 )
    REGISTER_NPC( ARENA4MOBXP500 )
    REGISTER_NPC( ARENA4MOBXP90 )
    REGISTER_NPC( ARENA4MOBXP80 )
    REGISTER_NPC( ARENA4MOBXP70 )
    REGISTER_NPC( ARENA4MOBXP60 )


END_INIT_NPC_DATA

/******************************************************************************/
// NPC creation.

INIT_NPC_CREATION

	CREATE_NPC( COLOSSEUMCLERKXP ) 
   
	CREATE_NPC( EXITGATE ) 
    CREATE_NPC( ARENAMOB50 )
    CREATE_NPC( ARENAMOB200 )
    CREATE_NPC( ARENAMOB100 )
    CREATE_NPC( ARENAMOB475 )
    CREATE_NPC( ARENAMOB450 )
    CREATE_NPC( ARENAMOB40 )
    CREATE_NPC( ARENAMOB375 )
    CREATE_NPC( ARENAMOB350 )
    CREATE_NPC( ARENAMOB325 )
    CREATE_NPC( ARENAMOB300 )
    CREATE_NPC( ARENAMOB425 )
    CREATE_NPC( ARENAMOB250 )
    CREATE_NPC( ARENAMOB225 )
    CREATE_NPC( ARENAMOB400 )
    CREATE_NPC( ARENAMOB190 )
    CREATE_NPC( ARENAMOB180 )
    CREATE_NPC( ARENAMOB170 )
    CREATE_NPC( ARENAMOB160 )
    CREATE_NPC( ARENAMOB150 )
    CREATE_NPC( ARENAMOB140 )
    CREATE_NPC( ARENAMOB130 )
    CREATE_NPC( ARENAMOB120 )
    CREATE_NPC( ARENAMOB110 )
    CREATE_NPC( DOKTORSPINE )
    CREATE_NPC( ARENAMOB275 )
    CREATE_NPC( COLOSSEUMCLERK )
    CREATE_NPC( ARENAMOB500 )
    CREATE_NPC( COLOSSEUMOWNER )
    CREATE_NPC( DRVONPYRE )
    CREATE_NPC( MADDOORENTRANCE )
    CREATE_NPC( MADDOOREXIT )
    CREATE_NPC( ARENAMOB90 )
    CREATE_NPC( ARENAMOB80 )
    CREATE_NPC( ARENAMOB70 )
    CREATE_NPC( ARENAMOB60 )
    CREATE_NPC( MADMADRIGAN )
    CREATE_NPC( MOBCRAZEDNURSE )
    CREATE_NPC( MOBDERANGEDORDERLY )
    CREATE_NPC( MOBLOON )
    CREATE_NPC( MOBMADMAN )
    CREATE_NPC( MOBMADPATIENT )
    CREATE_NPC( MOBRAVINGLUNATIC )
    CREATE_NPC( PORTALA2 )
    CREATE_NPC( PORTALB4 )
    CREATE_NPC( PORTALC1 )
    CREATE_NPC( PORTALC5 )
    CREATE_NPC( PORTALH3 )
    CREATE_NPC( PORTALJ1 )
    CREATE_NPC( PORTALJ3 )
    CREATE_NPC( PORTALJ4 )
    CREATE_NPC( WARDENVORTIMER )
    CREATE_NPC( MAKRSHPTANGHSPAWNER )
    CREATE_NPC( MAKRSHPTANGH2 )
    CREATE_NPC( MAKRSHPTANGH )
    CREATE_NPC( MAKRSHPORTAL )
    CREATE_NPC( MAKRSHPORTAL2 )
    CREATE_NPC( MAKRSHPORTAL3 )
    CREATE_NPC( MAKRSHPORTAL4 )
    CREATE_NPC( MAKRSHPORTAL5 )
    CREATE_NPC( ARCHIBALD )
    CREATE_NPC( FORESTWANDERER )
    CREATE_NPC( MOBRUNAWAYPATIENT )
    CREATE_NPC( ARENAMOBXP50 )
    CREATE_NPC( ARENAMOBXP200 )
    CREATE_NPC( ARENAMOBXP100 )
    CREATE_NPC( ARENAMOBXP475 )
    CREATE_NPC( ARENAMOBXP450 )
    CREATE_NPC( ARENAMOBXP40 )
    CREATE_NPC( ARENAMOBXP375 )
    CREATE_NPC( ARENAMOBXP350 )
    CREATE_NPC( ARENAMOBXP325 )
    CREATE_NPC( ARENAMOBXP300 )
    CREATE_NPC( ARENAMOBXP425 )
    CREATE_NPC( ARENAMOBXP250 )
    CREATE_NPC( ARENAMOBXP225 )
    CREATE_NPC( ARENAMOBXP400 )
    CREATE_NPC( ARENAMOBXP190 )
    CREATE_NPC( ARENAMOBXP180 )
    CREATE_NPC( ARENAMOBXP170 )
    CREATE_NPC( ARENAMOBXP160 )
    CREATE_NPC( ARENAMOBXP150 )
    CREATE_NPC( ARENAMOBXP140 )
    CREATE_NPC( ARENAMOBXP130 )
    CREATE_NPC( ARENAMOBXP120 )
    CREATE_NPC( ARENAMOBXP110 )
    CREATE_NPC( ARENAMOBXP275 )
    CREATE_NPC( ARENAMOBXP500 )
    CREATE_NPC( ARENAMOBXP90 )
    CREATE_NPC( ARENAMOBXP80 )
    CREATE_NPC( ARENAMOBXP70 )
    CREATE_NPC( ARENAMOBXP60 )
    CREATE_NPC(ARENA2MOBXP50 )
    CREATE_NPC(ARENA2MOBXP200 )
    CREATE_NPC(ARENA2MOBXP100 )
    CREATE_NPC(ARENA2MOBXP475 )
    CREATE_NPC(ARENA2MOBXP450 )
    CREATE_NPC(ARENA2MOBXP40 )
    CREATE_NPC(ARENA2MOBXP375 )
    CREATE_NPC(ARENA2MOBXP350 )
    CREATE_NPC(ARENA2MOBXP325 )
    CREATE_NPC(ARENA2MOBXP300 )
    CREATE_NPC(ARENA2MOBXP425 )
    CREATE_NPC(ARENA2MOBXP250 )
    CREATE_NPC(ARENA2MOBXP225 )
    CREATE_NPC(ARENA2MOBXP400 )
    CREATE_NPC(ARENA2MOBXP190 )
    CREATE_NPC(ARENA2MOBXP180 )
    CREATE_NPC(ARENA2MOBXP170 )
    CREATE_NPC(ARENA2MOBXP160 )
    CREATE_NPC(ARENA2MOBXP150 )
    CREATE_NPC(ARENA2MOBXP140 )
    CREATE_NPC(ARENA2MOBXP130 )
    CREATE_NPC(ARENA2MOBXP120 )
    CREATE_NPC(ARENA2MOBXP110 )
    CREATE_NPC(ARENA2MOBXP275 )
    CREATE_NPC(ARENA2MOBXP500 )
    CREATE_NPC(ARENA2MOBXP90 )
    CREATE_NPC(ARENA2MOBXP80 )
    CREATE_NPC(ARENA2MOBXP70 )
    CREATE_NPC(ARENA2MOBXP60 )
    CREATE_NPC(ARENA3MOBXP50 )
    CREATE_NPC(ARENA3MOBXP200 )
    CREATE_NPC(ARENA3MOBXP100 )
    CREATE_NPC(ARENA3MOBXP475 )
    CREATE_NPC(ARENA3MOBXP450 )
    CREATE_NPC(ARENA3MOBXP40 )
    CREATE_NPC(ARENA3MOBXP375 )
    CREATE_NPC(ARENA3MOBXP350 )
    CREATE_NPC(ARENA3MOBXP325 )
    CREATE_NPC(ARENA3MOBXP300 )
    CREATE_NPC(ARENA3MOBXP425 )
    CREATE_NPC(ARENA3MOBXP250 )
    CREATE_NPC(ARENA3MOBXP225 )
    CREATE_NPC(ARENA3MOBXP400 )
    CREATE_NPC(ARENA3MOBXP190 )
    CREATE_NPC(ARENA3MOBXP180 )
    CREATE_NPC(ARENA3MOBXP170 )
    CREATE_NPC(ARENA3MOBXP160 )
    CREATE_NPC(ARENA3MOBXP150 )
    CREATE_NPC(ARENA3MOBXP140 )
    CREATE_NPC(ARENA3MOBXP130 )
    CREATE_NPC(ARENA3MOBXP120 )
    CREATE_NPC(ARENA3MOBXP110 )
    CREATE_NPC(ARENA3MOBXP275 )
    CREATE_NPC(ARENA3MOBXP500 )
    CREATE_NPC(ARENA3MOBXP90 )
    CREATE_NPC(ARENA3MOBXP80 )
    CREATE_NPC(ARENA3MOBXP70 )
    CREATE_NPC(ARENA3MOBXP60 )
    CREATE_NPC(ARENA4MOBXP50 )
    CREATE_NPC(ARENA4MOBXP200 )
    CREATE_NPC(ARENA4MOBXP100 )
    CREATE_NPC(ARENA4MOBXP475 )
    CREATE_NPC(ARENA4MOBXP450 )
    CREATE_NPC(ARENA4MOBXP40 )
    CREATE_NPC(ARENA4MOBXP375 )
    CREATE_NPC(ARENA4MOBXP350 )
    CREATE_NPC(ARENA4MOBXP325 )
    CREATE_NPC(ARENA4MOBXP300 )
    CREATE_NPC(ARENA4MOBXP425 )
    CREATE_NPC(ARENA4MOBXP250 )
    CREATE_NPC(ARENA4MOBXP225 )
    CREATE_NPC(ARENA4MOBXP400 )
    CREATE_NPC(ARENA4MOBXP190 )
    CREATE_NPC(ARENA4MOBXP180 )
    CREATE_NPC(ARENA4MOBXP170 )
    CREATE_NPC(ARENA4MOBXP160 )
    CREATE_NPC(ARENA4MOBXP150 )
    CREATE_NPC(ARENA4MOBXP140 )
    CREATE_NPC(ARENA4MOBXP130 )
    CREATE_NPC(ARENA4MOBXP120 )
    CREATE_NPC(ARENA4MOBXP110 )
    CREATE_NPC(ARENA4MOBXP275 )
    CREATE_NPC(ARENA4MOBXP500 )
    CREATE_NPC(ARENA4MOBXP90 )
    CREATE_NPC(ARENA4MOBXP80 )
    CREATE_NPC(ARENA4MOBXP70 )
    CREATE_NPC(ARENA4MOBXP60 )

END_INIT_NPC_CREATION
