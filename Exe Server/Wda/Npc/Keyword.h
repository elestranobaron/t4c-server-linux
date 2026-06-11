/******************************************************************************
Modify for vs2008 (26/04/2009)
Remove NPC Keyword AttackHit, etc etc by Nightmare (28/06/2009)
/******************************************************************************/
#ifndef __Keyword__
#define __Keyword__

#include "CompositeInstruction.h"
#include "NPC_Editor.h"
#include "../WDAFile.h"

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	class Keyword	: public CompositeInstruction
	{
	public:
		Keyword();
		~Keyword();
		KeywordRelations GetKwRelation() const 
		{
			return kwRelation; 
		}
		void SetKwRelation(const KeywordRelations &thekwRelation)
		{
			kwRelation = thekwRelation; 
		}
		void GetKw( std::list< std::string > &kws )
		{
			kws = keywords;
		}
		void SetKw( const std::list< std::string > &kws );    
        void LoadImp( WDAFile &file );    
		bool IsDefaultKw()
		{
			return( GetHelpText() == GetAppString( IDS_DEFAULT_KEYWORD_HELP ) );
		}
		bool IsInitKw()
		{
			return( GetHelpText() == GetAppString( IDS_INITIAL_KEYWORD_HELP ) );
		}
		bool IsByeKw()
		{
			return( GetHelpText() == GetAppString( IDS_BYE_KEYWORD_HELP ) );
		}
		bool IsDeathKw()
		{
			return( GetName() == GetAppString( IDS_ONDEATH_KEYWORD_NAME ) );
		}
		bool IsAttackedKw()
		{
			return( GetName() == GetAppString( IDS_ONATTACKED_KEYWORD_NAME ) );
		}
		bool IsAttackKw()
		{
			return( GetName() == GetAppString( IDS_ONATTACK_KEYWORD_NAME ) );
		}
		///* NMNMNM NPC Keyword ADD
		bool IsAttackHitKw()
		{
			return( GetName() == GetAppString( IDS_ONATTACKHIT_KEYWORD_NAME ) );
		}
		bool IsHitKw()
		{
			return( GetName() == GetAppString( IDS_ONHIT_KEYWORD_NAME ) );
		}
		bool IsPopupKw()
		{
			return( GetName() == GetAppString( IDS_ONPOPUP_KEYWORD_NAME ) );
		}
		//*/
		bool IsDefaultInitKw()
		{
			return( IsDefaultKw() || IsInitKw() );
		}
		bool IsTriggerKw()
		{
			return( IsDeathKw() || IsAttackedKw() || IsAttackKw() || IsAttackHitKw() || IsHitKw() || IsPopupKw() );
		}
	
	protected:
		Instruction *Clone();
	
	private:
		void UpdateName();
		std::list<std::string>  keywords; 
		KeywordRelations        kwRelation; 
	};      
}
#endif // __Keyword__