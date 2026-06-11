/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "Keyword.h"
#include "InstructionFactory.h"

using namespace std;

/******************************************************************************/
namespace
/******************************************************************************/
{
    string AnGetName()
	{
        return GetAppString( IDS_INST_KEYWORD );
    }
    string AnGetHelp()
	{
        return GetAppString( IDS_INST_KEYWORD_HELP );
    }    
};
/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	/******************************************************************************/
	Keyword::Keyword()	: CompositeInstruction( AnGetName(), AnGetHelp(), InsKeyword )
	/******************************************************************************/
	{
		kwRelation = Or;

		// Always add a default SayText to keywords.
		Instruction *sayText = InstructionFactory::GetInstance()->CreateInstruction( InsSayText );
		subInstructions.push_back( sayText );
	}
	/******************************************************************************/
	Keyword::~Keyword()
	/******************************************************************************/
	{
	}
	/******************************************************************************/
	void Keyword::LoadImp(WDAFile &file)
	/******************************************************************************/
	{
		DWORD rel = 0, size = 0, i;
		BYTE kwType;

		// Since the keywords will be loaded from the file, destroy the default SayText.
		/*
		//NMNMNMNMNMNMNMNMNM peut etre mieux dememe a voir mais sa plante pas comme lautres...
		Résolution d'un problème au niveau de incrémentation de K
		list< Instruction * >::iterator k; 
		for( k = subInstructions.begin(); k != subInstructions.end(); k++ )
		{
			if (*k != NULL)
			{
				delete *k;
				*k = NULL;
			}
			k = subInstructions.erase( k );
		}*/


		list< Instruction * >::iterator k;
		for( k = subInstructions.begin(); k != subInstructions.end(); )
		{
			if (*k != 0) //NMNMNMNMNMNMNMNMNM changer avant sa (*k < 0)
			{
				delete *k;
				k = subInstructions.erase( k );
			}
			else 
			{
				++k;
			}
		}

		file.Read( kwType );
		file.Read( rel );
		file.Read( size );

		if( kwType == 1 )
		{
			SetName( GetAppString( IDS_INITIAL_KEYWORD_NAME ) );
			SetHelpText( GetAppString( IDS_INITIAL_KEYWORD_HELP ) );
		}
		else if( kwType == 2 )
		{
			SetName( GetAppString( IDS_DEFAULT_KEYWORD_NAME ) );
			SetHelpText( GetAppString( IDS_DEFAULT_KEYWORD_HELP ) );
		}
		else if( kwType == 3 )	
		{
			SetName( GetAppString( IDS_BYE_KEYWORD_NAME ) );
			SetHelpText( GetAppString( IDS_BYE_KEYWORD_HELP ) );
		}
		else if( kwType == 4 )	
		{
			SetName( GetAppString( IDS_ONDEATH_KEYWORD_NAME ) );
			SetHelpText( GetAppString( IDS_ONDEATH_KEYWORD_HELP ) );
		}
		else if( kwType == 5 )	
		{
			SetName( GetAppString( IDS_ONATTACK_KEYWORD_NAME ) );
			SetHelpText( GetAppString( IDS_ONATTACK_KEYWORD_HELP ) );
		}
		else if( kwType == 6 )	
		{
			SetName( GetAppString( IDS_ONATTACKED_KEYWORD_NAME ) );
			SetHelpText( GetAppString( IDS_ONATTACKED_KEYWORD_HELP ) );
		}
      else if( kwType == 7 )	
      {
         SetName( GetAppString( IDS_ONATTACKHIT_KEYWORD_NAME ) );
         SetHelpText( GetAppString( IDS_ONATTACKHIT_KEYWORD_HELP ) );
      }
      else if( kwType == 8 )	
      {
         SetName( GetAppString( IDS_ONHIT_KEYWORD_NAME ) );
         SetHelpText( GetAppString( IDS_ONHIT_KEYWORD_HELP ) );
      }
      else if( kwType == 9 )	
      {
         SetName( GetAppString( IDS_ONPOPUP_KEYWORD_NAME ) );
         SetHelpText( GetAppString( IDS_ONPOPUP_KEYWORD_HELP ) );
      }
      kwRelation = (KeywordRelations)rel;
		for( i = 0; i != size; i++ )
		{
			string kw;
			file.Read( kw );
			keywords.push_back( kw );
		}
	}
	/******************************************************************************/
	// Clone a keyword
	Instruction *Keyword::Clone( void )
	/******************************************************************************/
	{
		Keyword *kw = new Keyword;

		kw->Copy( this );
		kw->keywords = this->keywords; 
		kw->kwRelation = this->kwRelation;

		return kw;
	}
	/******************************************************************************/
	// Updates the displayed name according to the keyword's content.
	void Keyword::UpdateName( void )
	/******************************************************************************/
	{
		// Do not update the default or init keyword's name.
		if( IsDefaultInitKw() )
		{
			return;
		}

		if( keywords.size() == 0 )
		{
			SetName( AnGetName() );
			return;
		}

		string newName;
		int curCount = 0;
		list< string >::iterator i;
		for( i = keywords.begin(); i != keywords.end(); i++ )
		{
			newName += *i;

			if( ++curCount < keywords.size() )
			{
				newName += ", ";
			}
		}
		SetName( newName );
	}
	/******************************************************************************/
	// Sets a keyword and updates the keyword's name
	void Keyword::SetKw(const list< string > &kws) // The new list of keywords.
	/******************************************************************************/
	{
		keywords = kws;

    UpdateName();
}

} // NPC_Editor

