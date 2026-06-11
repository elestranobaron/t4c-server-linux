/******************************************************************************
Modify for vs2008 (26/04/2009)
ADD PROfession and Initialpos by Nightmare 928/06/2009)
/******************************************************************************/
#ifndef __NPC__
#define __NPC__

#include "../WDAFile.h"
#include "Instruction.h"
#include "NPC_Editor.h"
#include "RootInstruction.h"
#include <string>

/******************************************************************************/
namespace NPC_Editor
/******************************************************************************/
{
	class NPC
	{
	public:
		NPC( std::string name, std::string id, bool boCanMove = true, bool boPrivateTalk = false,bool forceattack = false );
		NPC( const NPC &onpc );
		~NPC();
		struct SoldItem
		{
			bool operator ==( const SoldItem &i )
			{
				return( itemId == i.itemId && priceFormula == i.priceFormula );
			}
			std::string itemId;
         std::string priceFormula;
			//DWORD price;
		};
		struct BoughtItem
		{
			bool operator ==( const BoughtItem &i )
			{
				return( sellType == i.sellType && minPrice == i.minPrice && maxPrice == i.maxPrice && minPriceF == i.minPriceF && maxPriceF == i.maxPriceF);
			}
			SellTypes sellType;
			DWORD minPrice;
			DWORD maxPrice;
         std::string minPriceF;
         std::string maxPriceF;
		};
		struct TrainedSkill
		{
			bool operator ==( const TrainedSkill &i )
			{
				return( skillId == i.skillId && maxSkillPnts == i.maxSkillPnts && price == i.price );
			}
			DWORD skillId;
			DWORD maxSkillPnts;
			DWORD price;
		};
		struct TaughtSkill
		{
			bool operator ==( const TaughtSkill &i )
			{
				return( skillId == i.skillId && price == i.price );
			}
			DWORD skillId;
			DWORD skillPnts;
			DWORD price;
		};
    
        struct TaughtFormule
        {
           bool operator ==( const TaughtFormule &i )
           {
               return( formuleId == i.formuleId && price == i.price );
           }
           DWORD formuleId;
           DWORD price;
        };

        struct WorldPos
        {
           WorldPos() : X(0), Y(0), world(0){}
        
           bool operator==( const WorldPos &r ) const
           {
               return X     == r.X && 
                      Y     == r.Y && 
                      world == r.world;
           }

           int X, Y, world;
        };

		std::string GetName() const 
		{
			return name; 
		}
		void SetName(const std::string &thename)
		{
			name = thename; 
		}  
		std::string GetId() const 
		{
			return id; 
		}
		void SetId(const std::string &theid)
		{
			id = theid; 
		}
	   std::string GetCreatureId() const
		{
			return creatureId;
		}
		void SetCreatureId(const std::string &thecreatureId)
		{
			creatureId = thecreatureId; 
		}
		bool GetCanMove() const
		{
			return boCanMove; 
		}
		void SetCanMove( const bool bState ) 
		{
			boCanMove = bState; 
		}
		bool GetPrivateTalk() const
		{
			return boPrivateTalkN; 
		}
		void SetPrivateTalk( const bool bState )
		{
			boPrivateTalkN = bState; 
		} 

      bool GetForceAttack() const
      {
         return boForceAttack; 
      }
      void SetForceAttack( const bool bState )
      {
         boForceAttack = bState; 
      } 


      WorldPos GetInitialPos() const 
      { 
         return InitialPos; 
      }
      void SetInitialPos(const WorldPos &pos)
      {	
         InitialPos = pos; 
      }

      bool GetOverwrite() const
      {
         return boOverwrite; 
      }

		bool IsIntegrityOK();  
		bool AddInstruction( Instruction *ins, Instruction *relativeIns, InsertionRelation relation );
		bool MoveInstruction( Instruction *instruction, Instruction *relativeIns, InsertionRelation relation );
		void DeleteInstruction( Instruction *ins );            
		void Load( WDAFile &file );
		void SetSoldItemList( const std::list< SoldItem > &itemList )
      {
			itemsSold = itemList;
		}
		void GetSoldItemList( std::list< SoldItem >&itemList )
		{
			itemList = itemsSold;
		}
		void SetBoughtItemList( const std::list< BoughtItem > &itemList )
		{
			itemsBought = itemList;
		}
		void GetBoughtItemList( std::list< BoughtItem > &itemList )
		{
			itemList = itemsBought;
		}
		void GetTrainedSkillList( std::list< TrainedSkill > &skillList )
		{
			skillList = trainedSkills;
		}
		void SetTrainedSkillList( const std::list< TrainedSkill > &skillList )
		{
			trainedSkills = skillList;
		}
		void GetTaughtSkillList( std::list< TaughtSkill > &skillList )
		{
			skillList = taughtSkills;
		}
      void SetTaughtSkillList( const std::list< TaughtSkill > &skillList )
      {
         taughtSkills = skillList;
      }
      void GetTaughtFormuleList( std::list< TaughtFormule > &formuleList )
      {
         formuleList = taughtFormules;
      }
      void SetTaughtFormuleList( const std::list< TaughtFormule > &formuleList )
      {
         taughtFormules = formuleList;
      }
      void SetPointsItemList( const std::list< SoldItem > &itemList )
      {
         itemsPoints = itemList;
      }
      void GetPointsItemList( std::list< SoldItem >&itemList )
      {
         itemList = itemsPoints;
      }
		NPC &operator = ( const NPC &onpc );
		RootInstruction *GetRootInstruction();

	private:
		std::string name;
		std::string id;
		std::string creatureId;
      WorldPos InitialPos;
		bool boPrivateTalkN;
		bool boCanMove;
      bool boOverwrite;
      bool boForceAttack;
		RootInstruction rootInstruction;
      std::list< SoldItem > itemsPoints;
		std::list< SoldItem > itemsSold;
		std::list< BoughtItem > itemsBought;
		std::list< TrainedSkill > trainedSkills;
		std::list< TaughtSkill > taughtSkills;
      std::list< TaughtFormule > taughtFormules;
	};
}

#endif // __NPC__