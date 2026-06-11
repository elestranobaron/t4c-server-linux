/******************************************************************************
Modify for vs2008 (26/04/2009)
Add Profession initialpos, etc etc by NightMare (27/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "NPC.h"

using namespace std;

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	/******************************************************************************/
	NPC::NPC( std::string iname, std::string iid, bool imove, bool iprivate, bool forceattack ) :
		name( iname ), id( iid ), boCanMove( imove ), boPrivateTalkN( iprivate ), boForceAttack( forceattack )
	/******************************************************************************/
	{
	}
	/******************************************************************************/
	NPC::NPC( const NPC &onpc )
	/******************************************************************************/
	{
		*this = onpc;
	}
	/******************************************************************************/
	NPC::~NPC()
	/******************************************************************************/
	{
	}
	/******************************************************************************/
	// Currently no integrity check.
	bool NPC::IsIntegrityOK()
	/******************************************************************************/
	{
		return true;	
	}
	/******************************************************************************/
	bool NPC::AddInstruction(Instruction *ins, Instruction *relativeIns, InsertionRelation relation)
	/******************************************************************************/
	{
		// Cannot add a NULL instruction.
		ATLASSERT( ins != NULL );
		if( ins == NULL )
		{ 
			_LOG_DEBUG
				LOG_DEBUG_LVL1,
				"NPC::AddInstruction -> Added NULL instruction instance." 
				LOG_
				return false; 
		};

		if( ins->GetId() == InsElseIf || ins->GetId() == InsElse )
		{
			ATLASSERT( relativeIns != NULL );
			if( relativeIns == NULL )
			{
				_LOG_DEBUG
					LOG_DEBUG_LVL1,
					"NPC::AddInstruction -> Added NULL relativeInstruction to Else/ElseIF."
					LOG_
					return false;
			}
			if( relativeIns->GetId() != InsIf || relation != asChild )
			{
				_LOG_DEBUG
					LOG_DEBUG_LVL1,
					"NPC::AddInstruction -> IF and ElseIF instructions cannot only be added as child of IF instructions."
					LOG_ 
					return false;
			}
		}

		// Forward to root instruction.
		bool stopSearch = false;
		return rootInstruction.AddInstruction( ins, relativeIns, relation, stopSearch );
	}
	/******************************************************************************/
	bool NPC::MoveInstruction(Instruction *ins, Instruction *relativeIns, InsertionRelation relation)
	/******************************************************************************/
	{
		bool stopSearch = false;

		if( ins->GetId() == InsElse || ins->GetId() == InsElseIf )
		{
			return false;
		}

		if( rootInstruction.AddInstruction( ins, relativeIns, relation, stopSearch ) )
		{
			stopSearch = false;
			rootInstruction.DeleteInstruction( ins );
			return rootInstruction.AddInstruction( ins, relativeIns, relation, stopSearch );
		}

		return false;
	}
	/******************************************************************************/
	// Deletes an instruction. Forwards it to the root instruction.
	void NPC::DeleteInstruction(Instruction *ins)
	/******************************************************************************/
	{
		// Cannot delete the root instruction.
		ATLASSERT( ins != &rootInstruction );
		rootInstruction.DeleteInstruction( ins );
	}
	/******************************************************************************/
	// Returns the root instruction.
	RootInstruction *NPC::GetRootInstruction( void )
	/******************************************************************************/
	{
		return &rootInstruction;
	}
	/******************************************************************************/
	void NPC::Load(WDAFile &file)
	/******************************************************************************/
	{
		DWORD version;

		file.Read( version );
		file.Read( creatureId );

   	    // Load default values first
	    boCanMove      = TRUE;
	    boPrivateTalkN = TRUE;
       boOverwrite    = FALSE;
       boForceAttack  = FALSE;
	    InitialPos.X = 0;
	    InitialPos.Y = 0;
	    InitialPos.world = 0;

	   switch( version )
		{
          case 0x09:
          case 0x08:
          case 0x07:
          case 0x06:
          // Read ver. 4 npcs
          {
            file.Read( boCanMove );
            file.Read( boPrivateTalkN );
            file.Read( InitialPos.X );
            file.Read( InitialPos.Y );
            file.Read( InitialPos.world );
            file.Read( boOverwrite );
            file.Read( boForceAttack );
          }
          break;
          case 0x05:
		    case 0x04:
			// Read ver. 4 npcs
			{
				file.Read( boCanMove );
				file.Read( boPrivateTalkN );
				file.Read( InitialPos.X );
				file.Read( InitialPos.Y );
				file.Read( InitialPos.world );
				file.Read( boOverwrite );
			}
			break;
			case 0x03:
			// Read ver. 3 npcs
			{
				file.Read( boCanMove );
				file.Read( boPrivateTalkN );
				file.Read( InitialPos.X );
				file.Read( InitialPos.Y );
				file.Read( InitialPos.world );
            }
			break;

			case 0x02:
		    // Read ver. 2 npcs, discard buffer
		    {
			   file.Read( boCanMove );
			   file.Read( boPrivateTalkN );

			   bool buf;
			   for( int xxx = 0; xxx < 512; xxx++ )
			   {
			   	  file.Read( buf );
			   }
			}
			break;
            case 0x01:
		    // Default values
  		    break;
   		    default:
		    // Unknown version?
		    {
			   static WDAFileException ex( "Unknown WDA version while loading npcs.",WDAFileException::ReadError);
			   // Throw a read exception.
			   throw( ex );
		    }
			break;
	    }

		DWORD size, i;
		file.Read( size );
		for( i = 0; i != size; i++ )
		{
			SoldItem item;
			file.Read( item.itemId );
         if(version >= 0x09)
         {
            file.Read( item.priceFormula );
         }
         else
         {
            DWORD priceTmp;
            file.Read( priceTmp );
            char strTmp[100];
            sprintf_s(strTmp,100,"%d",priceTmp);
            item.priceFormula = strTmp;
         }
			itemsSold.push_back( item );
		}

		file.Read( size );
		for( i = 0; i != size; i++ )
		{
			BoughtItem item;
			DWORD sellType;
			file.Read( sellType );
			item.sellType = (SellTypes)sellType;
			file.Read( item.minPrice );
			file.Read( item.maxPrice );
         if(version >= 0x08)
         {
            file.Read( item.minPriceF );
            file.Read( item.maxPriceF );
         }
         else
         {
            item.minPriceF = "";
            item.maxPriceF = "";
         }
			itemsBought.push_back( item );
		}

		file.Read( size );
		for( i = 0; i != size; i++ )
		{
			TrainedSkill skill;
			file.Read( skill.skillId );
			file.Read( skill.maxSkillPnts );
			file.Read( skill.price );
			trainedSkills.push_back( skill );
		}

		file.Read( size );
		for( i = 0; i != size; i++ )
		{
			TaughtSkill skill;
			file.Read( skill.skillId );
			file.Read( skill.skillPnts );
			file.Read( skill.price );
			taughtSkills.push_back( skill );
		}

	    switch( version )
	    {
         case 0x09:
         case 0x08:
         case 0x07:
         case 0x06:
			case 0x05:
			// Read ver. 5 npcs
			{
	           file.Read( size );  //NMNMNMNMNMNM a valider pour les formule dans les wda.....
		       for( i = 0; i != size; i++ )
			   {
			      TaughtFormule formule;
			      file.Read( formule.formuleId );
			      file.Read( formule.price );
			      taughtFormules.push_back( formule );
		       }
	       }
	       break;
       }

       switch( version )
       {
          case 0x09:
          case 0x08:
          case 0x07:
          {
             file.Read( size );
             for( i = 0; i != size; i++ )
             {
                SoldItem item;
                file.Read( item.itemId );
                if(version >= 0x09)
                {
                   file.Read( item.priceFormula );
                }
                else
                {
                   DWORD priceTmp;
                   file.Read( priceTmp );
                   char strTmp[100];
                   sprintf_s(strTmp,100,"%d",priceTmp);
                   item.priceFormula = strTmp;
                }
                itemsPoints.push_back( item );
             }
          }
          break;
       }
       

		// Load the instruction tree.
		rootInstruction.Load( file );	
	}
	/******************************************************************************/
	// Assignement operator.
	NPC &NPC::operator = (const NPC &onpc) // The NPC copy.
	/******************************************************************************/
	{
		name = onpc.name;
		id = onpc.id;
	    
	   boCanMove		= onpc.boCanMove;
     	boPrivateTalkN	= onpc.boPrivateTalkN;
     	creatureId		= onpc.creatureId;
	   InitialPos		= onpc.InitialPos;
     	itemsSold		= onpc.itemsSold;
	   itemsBought		= onpc.itemsBought;
	   trainedSkills	= onpc.trainedSkills;
	   taughtSkills	= onpc.taughtSkills;
      taughtFormules = onpc.taughtFormules;
      itemsPoints		= onpc.itemsPoints;

		rootInstruction.Copy( const_cast< RootInstruction * >( &onpc.rootInstruction ) );

		return *this;
	}
} // NPC_Editor

