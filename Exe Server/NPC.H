/******************************************************************************
Modify for vs2008 (06/05/2009)
Linux monolithic build: per-region CreateNPC_* via T4C_CREATE_NPC_FUNC_NAME.
/******************************************************************************/
#include <GameDefs.h>
#include <MonsterStructure.h>
#include <NPCstructure.h>
#include <Unit.h>
#include "Random.h"

#ifndef T4C_CREATE_NPC_FUNC_NAME
#define T4C_CREATE_NPC_FUNC_NAME CreateNPC
#endif

void MonsterStatSetup();
void MonsterStatDestroy();
void RegisterNCP( void );

#ifdef _WIN32

void CreateNPC();

#define INIT_NPC_DATA_PART1 Random rnd; static AFX_EXTENSION_MODULE NPCsDLL;\
void RegisterNPC( void );\
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)\
{\
	UNREFERENCED_PARAMETER(lpReserved);\
	switch(dwReason) {\
    case DLL_PROCESS_ATTACH: \
			if (!AfxInitExtensionModule(NPCsDLL, hInstance))\
				return 0;

#define INIT_NPC_DATA_PART2 TRACE0("NPC DLL Initializing!\n");\
			TFCMAIN::RegisterUnitStartupFunction( MonsterStatSetup );\
			RegisterNPC();\
		break;\
    case DLL_THREAD_ATTACH: break;\
	case DLL_THREAD_DETACH: break;\
    case DLL_PROCESS_DETACH: \
			TRACE0("NPC DLL Terminating!\n");\
			MonsterStatDestroy();\
			AfxTermExtensionModule(NPCsDLL);\
		break;\
    }\
return 1;\
}\
void RegisterNPC( void ){

#define INIT_NPC_DATA INIT_NPC_DATA_PART1 INIT_NPC_DATA_PART2

#define REGISTER_NPC( ObjectName )	Unit::RegisterUnitMessageHandler( 30000, &ObjectName, #ObjectName, U_NPC, TRUE );

#define	END_INIT_NPC_DATA	};

#define INIT_NPC_CREATION	void CreateNPC(){

#define CREATE_NPC( ObjectName )	ObjectName.Create();

#define END_INIT_NPC_CREATION }

#else /* Linux */

#define INIT_NPC_DATA \
	namespace { \
		Random rnd; \
		void RegisterNPC_Impl(void); \
		struct NPCAutoRegister { \
			NPCAutoRegister() { RegisterNPC_Impl(); } \
		}; \
		static NPCAutoRegister g_npc_auto_register; \
		void RegisterNPC_Impl(void) {

#define END_INIT_NPC_DATA \
		} \
	}

#define REGISTER_NPC( ObjectName ) Unit::RegisterUnitMessageHandler( 30000, &ObjectName, #ObjectName, U_NPC, TRUE, TRUE );

#define INIT_NPC_CREATION void T4C_CREATE_NPC_FUNC_NAME() {

#define CREATE_NPC( ObjectName ) ObjectName.Create();

#define END_INIT_NPC_CREATION }

#endif
