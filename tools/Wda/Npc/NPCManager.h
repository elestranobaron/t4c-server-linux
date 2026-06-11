/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#ifndef __NPCManager__
#define __NPCManager__

#include "NPC.h"
#include "../WDAFile.h"
#include <string>
#include <map>

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	class NPCManager
	{
	public:
		~NPCManager();
	    NPC *CreateNPC( std::string name, std::string id );
	    NPC *GetNPC( std::string id );
		void DeleteNPC( std::string id );
		bool RenameNPC( std::string oldId, std::string newId );
		void Load( WDAFile &file );   
		void Initialize();
		void GetNPCList( std::list< NPC * > &npcList ) const;
		static NPCManager *GetInstance();
	
	private:
		typedef std::map<std::string, NPC *> NpcMap;
		NPCManager();
		NpcMap npcList;
	};
}

#endif // __NPCManager__