/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "NPCManager.h"
#include "InstructionFactory.h"
#include "Keyword.h"

using namespace std;

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	/******************************************************************************/
	NPCManager::NPCManager()
	/******************************************************************************/
	{
	}
	/******************************************************************************/
	NPCManager::~NPCManager()
	/******************************************************************************/
	{
	}
	/******************************************************************************/
	// Creates an NPC with a name and an ID. If the ID of the NPC already exists,
	// the function returns NULL, otherwise it returns the newly created NPC.
	NPC *NPCManager::CreateNPC(string name, string id)
	/******************************************************************************/
	{
		// NPC already exists
		NpcMap::iterator i = npcList.find( id );
		if( i != npcList.end() )
		{
			_LOG_DEBUG
				LOG_DEBUG_LVL1,
				"NPCManager::CreateNPC -> The NPC ID already exists."
			LOG_ 
			return NULL;
		}

		NPC *newNpc = new NPC( name, id );

		InstructionFactory *factory = InstructionFactory::GetInstance();

		Instruction *initKw        = factory->CreateInstruction( InsKeyword );
		Instruction *defKw         = factory->CreateInstruction( InsKeyword );
		Instruction *byeKw			= factory->CreateInstruction( InsKeyword );
		Instruction *byeIns			= factory->CreateInstruction( InsBreakConversation );
		Instruction *OnDeathKw		= factory->CreateInstruction( InsKeyword );
		Instruction *OnAttackKw		= factory->CreateInstruction( InsKeyword );
		Instruction *OnAttackedKw	= factory->CreateInstruction( InsKeyword );
		Instruction *OnAttackHitKw	= factory->CreateInstruction( InsKeyword );
		Instruction *OnHitKw	   	= factory->CreateInstruction( InsKeyword );
		Instruction *OnPopupKw		= factory->CreateInstruction( InsKeyword );
    
    	// The instruction factory must return valid instruction instances.
    	ATLASSERT( initKw != NULL && defKw != NULL && byeKw != NULL );
    	if( initKw == NULL || defKw == NULL || byeKw == NULL )
		{
			_LOG_DEBUG
				LOG_DEBUG_LVL1,
				"NPCManager::CreateNPC -> Got NULL default or initial keyword."
			LOG_
			if (newNpc != NULL)
			{
				delete newNpc;
				newNpc = NULL;
			}
			return NULL;
		}

		// The initial keyword
		initKw->SetName( GetAppString( IDS_INITIAL_KEYWORD_NAME ) );
		initKw->SetHelpText( GetAppString( IDS_INITIAL_KEYWORD_HELP ) );

		// The default keyword.
		defKw->SetName( GetAppString( IDS_DEFAULT_KEYWORD_NAME ) );
		defKw->SetHelpText( GetAppString( IDS_DEFAULT_KEYWORD_HELP ) );

        
		// The bye keyword
		byeKw->SetName( GetAppString( IDS_BYE_KEYWORD_NAME ) );    
		byeKw->SetHelpText( GetAppString( IDS_BYE_KEYWORD_HELP ) );

		// The On Death event
		OnDeathKw->SetName( GetAppString( IDS_ONDEATH_KEYWORD_NAME ) );
		OnDeathKw->SetHelpText( GetAppString( IDS_ONDEATH_KEYWORD_HELP ) );

		// The On Attack event
		OnAttackKw->SetName( GetAppString( IDS_ONATTACK_KEYWORD_NAME ) );
		OnAttackKw->SetHelpText( GetAppString( IDS_ONATTACK_KEYWORD_HELP ) );

		// The On Attacked event
		OnAttackedKw->SetName( GetAppString( IDS_ONATTACKED_KEYWORD_NAME ) );
		OnAttackedKw->SetHelpText( GetAppString( IDS_ONATTACKED_KEYWORD_HELP ) );

		// The On AttackHit event
		OnAttackHitKw->SetName( GetAppString( IDS_ONATTACKHIT_KEYWORD_NAME ) );
		OnAttackedKw->SetHelpText( GetAppString( IDS_ONATTACKHIT_KEYWORD_HELP ) );

		// The On Hit event
		OnHitKw->SetName( GetAppString( IDS_ONHIT_KEYWORD_NAME ) );
		OnHitKw->SetHelpText( GetAppString( IDS_ONHIT_KEYWORD_HELP ) );

		// The On Popup event
		OnPopupKw->SetName( GetAppString( IDS_ONPOPUP_KEYWORD_NAME ) );
		OnPopupKw->SetHelpText( GetAppString( IDS_ONPOPUP_KEYWORD_HELP ) );
        
		// Add the default and initial keywords to the NPC.
		newNpc->AddInstruction( initKw, NULL,   asChild );
		newNpc->AddInstruction( defKw,  initKw, asNext );
		newNpc->AddInstruction( byeKw, NULL, asChild );
		newNpc->AddInstruction( byeIns, byeKw, asChild );
		newNpc->AddInstruction( OnDeathKw, NULL, asChild );
		newNpc->AddInstruction( OnAttackKw, NULL, asChild );
		newNpc->AddInstruction( OnAttackedKw, NULL, asChild );
		newNpc->AddInstruction( OnAttackHitKw, NULL, asChild );
		newNpc->AddInstruction( OnHitKw, NULL, asChild );
		newNpc->AddInstruction( OnPopupKw, NULL, asChild );


		// Register the Bye keyword
		NPC_Editor::Keyword *kw = static_cast< NPC_Editor::Keyword * >( byeKw );
		std::list< std::string > kwList;

		kwList.push_back( GetAppString( IDS_BYE_KEYWORD_NAME ).c_str() );
		kw->SetKw( kwList );

		// Register the OnDeath keyword
		NPC_Editor::Keyword *death = static_cast< NPC_Editor::Keyword* >( OnDeathKw );
		std::list< std::string > keywordsDeath;

		keywordsDeath.push_back( GetAppString( IDS_ONDEATH_KEYWORD_NAME ).c_str() );
		death->SetKw( keywordsDeath );

		// Register the OnAttack keyword
		NPC_Editor::Keyword *attack = static_cast< NPC_Editor::Keyword* >( OnAttackKw );
		std::list< std::string > keywordsOnAttack;

		keywordsOnAttack.push_back( GetAppString( IDS_ONATTACK_KEYWORD_NAME ).c_str() );
		attack->SetKw( keywordsOnAttack );

		// Register the OnAttacked keyword
		NPC_Editor::Keyword *attacked = static_cast< NPC_Editor::Keyword* >( OnAttackedKw );
		std::list< std::string > keywordsOnAttacked;

		keywordsOnAttacked.push_back( GetAppString( IDS_ONATTACKED_KEYWORD_NAME ).c_str() );
		attacked->SetKw( keywordsOnAttacked );

		// Register the OnAttackHit keyword
		NPC_Editor::Keyword *attackHit = static_cast< NPC_Editor::Keyword* >( OnAttackHitKw );
		std::list< std::string > keywordsOnAttackHit;

		keywordsOnAttackHit.push_back( GetAppString( IDS_ONATTACKHIT_KEYWORD_NAME ).c_str() );
		attackHit->SetKw( keywordsOnAttackHit );

		// Register the OnHit keyword
		NPC_Editor::Keyword *hit = static_cast< NPC_Editor::Keyword* >( OnHitKw );
		std::list< std::string > keywordsOnHit;

		keywordsOnHit.push_back( GetAppString( IDS_ONHIT_KEYWORD_NAME ).c_str() );
		hit->SetKw( keywordsOnHit );

		// Register the OnPopup keyword
		NPC_Editor::Keyword *popup = static_cast< NPC_Editor::Keyword* >( OnPopupKw );
		std::list< std::string > keywordsOnPopup;

		keywordsOnPopup.push_back( GetAppString( IDS_ONPOPUP_KEYWORD_NAME ).c_str() );
		popup->SetKw( keywordsOnPopup );

		// Set the NPC's name to the NPC's id.
		newNpc->SetName( name );

		npcList[ id ] = newNpc;

		return newNpc;
	}
	/******************************************************************************/
	// Returns the NPC instance that has the given id. The function returns NULL
	// if no NPC of the given ID is found.
	NPC *NPCManager::GetNPC(string id)
	/******************************************************************************/
	{
		NpcMap::iterator i = npcList.find( id );
		if( i == npcList.end() )
		{
			_LOG_DEBUG
				LOG_DEBUG_LVL1,
				"NPCManager::GetNPC -> The supplied NPC ID does not exist."
			LOG_ 
			return NULL;
		}

		return (*i).second;
	}
	/******************************************************************************/
	// Remove the NPC that has the given id. It deletes the NPC * instance as well.
	void NPCManager::DeleteNPC(string id)
	/******************************************************************************/
	{
		NpcMap::iterator i = npcList.find( id );
		if( i == npcList.end() )
		{
			_LOG_DEBUG
				LOG_DEBUG_LVL1,
				"NPCManager::DeleteNPC -> The supplied NPC ID does not exist."
			LOG_ 
			return;
		}
		if ((*i).second != NULL)
		{
			delete (*i).second;
			(*i).second = NULL;
		}
		npcList.erase( i );
	}
	/******************************************************************************/
	// Loads the NPCs from a WDA file.
	void NPCManager::Load(WDAFile &file)
	/******************************************************************************/
	{
		DWORD version = 0;
		DWORD npcQty = 0;

		file.Read( version );

		if( version != 0x01 )
		{
			return;
		}

		file.Read( npcQty );

		DWORD i;
		for( i = 0; i < npcQty; i++ )
		{
			string name, id;
			// Read the name and Id
			file.Read( name );
			file.Read( id );

			// Create the new
			NPC *newNpc = new NPC( name, id );

			// Load the NPC from the file.
			newNpc->Load( file );

			// Add to NPC list.
			if ( npcList.find(id) != npcList.end() ) 
			{
				_LOG_DEBUG
					LOG_DEBUG_LVL1,
					"Found duplicate NPC ID %s, overriding previous one.",
					id.c_str()
				LOG_
			}
			npcList[ id ] = newNpc;
		}
	}
	/******************************************************************************/
	// Renames the NPC associated with the oldId and sets it the newId. If the
	// newID already exists or if the oldId doesn't exist, it returns false.
	bool NPCManager::RenameNPC(string oldId, string newId)
	/******************************************************************************/
	{
		NpcMap::iterator i = npcList.find( oldId );
		if( i == npcList.end() ){
			_LOG_DEBUG
				LOG_DEBUG_LVL1,
				"NPCManager::RenameNPC -> The supplied NPC ID does not exist."
			LOG_ 
			return false;
		}

		NpcMap::iterator j = npcList.find( newId );
		if( j != npcList.end() ){
			_LOG_DEBUG
				LOG_DEBUG_LVL1,
				"NPCManager::RenameNPC -> The NPC ID already exists."
			LOG_ 
			return false;
		}

		NPC *oldNpc = (*i).second;

		npcList.erase( i );

		oldNpc->SetId( newId );
		npcList[ newId ] = oldNpc;

		return true;
	}
	/******************************************************************************/
	// Returns the npc manager instance.
	NPCManager *NPCManager::GetInstance( void )
	/******************************************************************************/
	{
		static NPCManager instance;
		return &instance;
	}
	/******************************************************************************/
	// Returns the list of NPCs in the supplied list.
	void NPCManager::GetNPCList(std::list< NPC * > &inpcList) const // Container into which to dump the list of NPCs.
	/******************************************************************************/
	{
		NpcMap::const_iterator i;
		for( i = npcList.begin(); i != npcList.end(); i++ )
		{
			inpcList.push_back( (*i).second );        
		}
	}
} // NPC_Editor

