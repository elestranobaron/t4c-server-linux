/******************************************************************************
Modify for vs2008 (26/04/2009)
Add Check for Invisibility2 by Nightmare (27/06/2009)
******************************************************************************/
#include "stdafx.h"
#include "TFC Server.h"
#include "WorldMap.h"
#include "SharedStructures.h"
#include "random.h"
#include "Broadcast.h"
#include "GameDefs.h"
#include "Objects.h"
#include "Creatures.h"
#include "TFC_MAIN.h"
#include "Clans.h"
#include "MonsterStructure.h"
#include "T4CLog.h"
#include "Hive.h"
#include "IntlText.h"
#include "NPCmacroScriptLng.h"
#ifdef _WIN32
#include <mmsystem.h>
#endif

extern CTFCServerApp theApp;

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
#define X_HIVE_CHECK	15
#define Y_HIVE_CHECK	30
#define MAXIMAL_SCAN_RANGE 80
#define DIVIDER 2
#define TOP_RANDOM_MOB	50000
#define POPUP( pos, appear )

static unsigned char bitmask[8];
static const int YListSize = 512;

extern Clans *CreatureClans;
extern TemplateList <SubmitNearUnit> IsNearSubmission;
extern TemplateList <Unit> CreatureList;
extern TFC_MAIN *TFCServer;
extern Random rnd;

BOOL LOCK;
BOOL FirstTime = TRUE;

/******************************************************************************/
WorldMap::WorldMap()
/******************************************************************************/
{
	// This variables tells WorldMap that the object was locked by an internal function.
	world = 0;
	MAXX = 0;
	MAXY = 0;
	bitmask[0] = 0x80;
	bitmask[1] = 0x40;
	bitmask[2] = 0x20;
	bitmask[3] = 0x10;
	bitmask[4] = 0x08;
	bitmask[5] = 0x04;
	bitmask[6] = 0x02;
	bitmask[7] = 0x01;	
}
/******************************************************************************/
WorldMap::~WorldMap()
/******************************************************************************/
{
	unsigned short i, j = 0;	
	for(i = 0; i < (MAXX >> DIVIDER); i++)
	{
		for(j = 0; j < (MAXY >> DIVIDER); j++)
		{		
			try
			{
				if( WorldUnits[i][j] != NULL )
				{
					delete WorldUnits[i][j];
					WorldUnits[i][j] = NULL;
				}
			}
			catch(...)
			{

			}
		}		
		if (WorldUnits[i] != NULL)
		{
			delete [] WorldUnits[i];
			WorldUnits[i] = NULL;
		}
	}

	if (WorldUnits != NULL)
	{
		delete [] WorldUnits;
		WorldUnits = NULL;
	}
	if (map_buffy != NULL)
	{
		delete map_buffy;
		map_buffy = NULL;
	}

   if (map_buffyOri != NULL)
   {
      delete map_buffyOri;
      map_buffyOri = NULL;
   }

   

   /*
	for(i = 0; i < MAXY; i++)
	{		
		EffectList[i].ToHead();
		while(EffectList[i].QueryNext())
		{
			EffectList[i].DeleteAbsolute();
		}
	}
	if (EffectList != NULL)
	{
		delete []EffectList;
		EffectList = NULL;
	}
   */

	TRACE(_T("d"));
}
/******************************************************************************/
// This function creates the world :)
void WorldMap::CreateMap(unsigned short X, unsigned short Y, unsigned short wld, unsigned char *buffy, unsigned char *buffyOri)
/******************************************************************************/
{
	MAXX = X;
	MAXY = Y;
	world = wld;

	TRACE(_T("%u-%u %u-%u %u-%u"), MAXX, X, MAXY, Y, world, wld);
	
	// Creates a double array of capuchon pointers
	WorldUnits = new Capuchon **[(MAXX >> DIVIDER)];

	unsigned short i, j;
	for(i = 0; i < (MAXX >> DIVIDER); i++)
	{
		WorldUnits[i] = new Capuchon *[(MAXY >> DIVIDER)];
	}

	for(i = 0; i < (MAXX >> DIVIDER); i++)
	{
		for(j = 0; j < (MAXY >> DIVIDER); j++)
		{
			WorldUnits[i][j] = NULL;
		}
	}

	// temporarly
	EffectList = new TemplateList<AreaSensitiveEffect> [MAXY];

	map_buffy    = buffy;
   map_buffyOri = buffyOri;
}
/******************************************************************************/
//  Returns the top unit on a tentacule.
Unit *WorldMap::ViewTopUnit(WorldPos wlPos)
/******************************************************************************/
{
	Lock();

	Unit *lpUnit = NULL;

	if( WorldUnits[ wlPos.X >> DIVIDER ][ wlPos.Y >> DIVIDER ] )
	{
		lpUnit = WorldUnits[ wlPos.X >> DIVIDER ][ wlPos.Y >> DIVIDER ]->ViewTopUnit( wlPos.X, wlPos.Y );
	}

	Unlock();
	return lpUnit;
}
/******************************************************************************/
// This function drops an object on top of a square
void WorldMap::deposit_unit(WorldPos where, Unit *obj, DWORD dwLinkID)
/******************************************************************************/
{	
	int XPos = where.X >> DIVIDER;
	int YPos = where.Y >> DIVIDER;
	int noBlockChange=0;
	
	if( obj->GetType() == U_OBJECT ) 
	{
		_item *lpItem = NULL;
		obj->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItem );

      //Arealink, mais plus supporter dans editeur donc ne devrait jamais arriver
		//If item to be dropped on floor is an area teleporter :
		if( lpItem != NULL && lpItem->itemStructureId == 14 ) 
		{
			WorldMap *lpWorld = TFCMAIN::GetWorld( where.world );
			if( lpWorld )
			{
				WorldPos wlPos;
				wlPos.world = lpItem->armor.End;
				wlPos.X = lpItem->weapon.Str;
				wlPos.Y = lpItem->weapon.Agi;

				lpWorld->add_effect( where.X, where.Y, where.X,where.Y,__AREA_TELEPORT,&wlPos,sizeof( WorldPos ));				
			}
		}
		else  //if item to be dropped on floor is a normal object 
		{
			// Object is at the new position
			Lock();
			where.world = world;	
			obj->SetWL(where);
			
			BYTE underBlock = QueryAreaType( where );
			// If there are no capuchon here, create one
			if( WorldUnits[XPos][YPos] == NULL )
			{
				WorldUnits[XPos][YPos] = new Capuchon;
			}
			else
			{
				// Query the under block of the top unit on that block.
				Unit *u = WorldUnits[XPos][YPos]->ViewTopUnit(where.X, where.Y );
				if( u != NULL )
				{
					underBlock = u->GetUnderBlock();
					if ( u->GetType() == U_PC ) 
                  noBlockChange=1;
				}
			}
			
			// Then add the object on the capuchon
			WorldUnits[XPos][YPos]->AddUnit(where.X, where.Y, obj);

         if(theApp.dwTimedUnitEnable)
         {
            sWorldObjectSpec* pNewObj= new sWorldObjectSpec;
            pNewObj->pUnit       = obj;
            pNewObj->dwLockID    = dwLinkID;
            pNewObj->dwDropTime  =  timeGetTime();
            uIDPosCompo uID;
            uID.sID.ushX = where.X;
            uID.sID.ushY = where.Y;
            uID.sID.dwID = obj->GetID();
            m_WorldObjectMap[uID.i64ID] = pNewObj;
         }
         

			// Save the blocking the unit cast.
			obj->SetUnderBlock( underBlock );
    
			// Set the blocking of the unit
			if (!noBlockChange) 
			{ 
				SetBlocking( where, obj->GetBlock() ); 
			}

			Unlock();
		}
	}
	else 
	{
		// NOt an OBJECT
		Lock();
		where.world = world;	
		obj->SetWL(where);
		BYTE underBlock = QueryAreaType( where );
		// If there are no capuchon here, create one
		if( WorldUnits[XPos][YPos] == NULL )
		{
			WorldUnits[XPos][YPos] = new Capuchon;
		}
		else
		{
			// Query the under block of the top unit on that block.
			Unit *u = WorldUnits[XPos][YPos]->ViewTopUnit(where.X, where.Y );
			if( u != NULL )
			{
				underBlock = u->GetUnderBlock();					
				if ( u->GetType() == U_PC ) 
				{
					noBlockChange = 1;
				}
			}
		}
		// Then add the object on the capuchon
		WorldUnits[XPos][YPos]->AddUnit(where.X, where.Y, obj);
      // Save the blocking the unit cast.
		obj->SetUnderBlock( underBlock ); 
		// Set the blocking of the unit
		if (!noBlockChange) 
		{
			SetBlocking( where, obj->GetBlock() );
		}
		Unlock();
	}
}
/******************************************************************************/
// This function returns TRUE if square has a block flag
BOOL WorldMap::IsBlocking(WorldPos where, Unit *obj, signed short XO, signed short YO)
/******************************************************************************/
{    
    BOOL boReturn;
	Lock();
	boReturn = internalIsBlocking(where, obj, XO, YO);

	Unlock();

	return boReturn;
}
/******************************************************************************/
// Sets the blocking flag of a square
void WorldMap::SetBlocking(WorldPos where, unsigned char which)
/******************************************************************************/
{	
    if( !IsValidPosition( where ) )
	{
		return;
    }
    
    unsigned long offset = (where.Y * (MAXX / 2)) + (where.X / 2);

    if(!(where.X % 2))
	{
		// if pair
        map_buffy[offset] &= 0x0F;
        map_buffy[offset] |= (which << 4);
	}
	else
	{
        map_buffy[offset] &= 0xF0; 
        map_buffy[offset] |= (which);
	}
}
/******************************************************************************/
// Sets the blocking flag of the area covered by a unit
BOOL WorldMap::SetBlockingUnit(WorldPos where, Unit *obj)
/******************************************************************************/
{
	UINT y;
	UINT uHeight = obj->ViewFlag( __FLAG_BLOCKING );
	WorldPos wlBlockPos = where;

	//nBlockSpace = nBlockSpace ? nBlockSpace
	BOOL boOK = TRUE;
	for(y = where.Y; y > where.Y - uHeight; y--)
	{
		wlBlockPos.Y = y;
		if( internalIsBlocking( wlBlockPos ) )
		{
			boOK = FALSE;
		}
	}

	// Only set blocking unit if we can
	if( boOK )
	{
		obj->SetUnderBlock( QueryAreaType( where ) );
		for(y = where.Y; y > where.Y - uHeight; y--)
		{
			wlBlockPos.Y = y;			
			SetBlocking(wlBlockPos, obj->GetBlock() );
		}		
	}

	return boOK;
}
/******************************************************************************/
// Removes the blocking flag from a unit
void WorldMap::RemoveBlockingUnit(WorldPos where, Unit *obj)
/******************************************************************************/
{
	int y;
	WorldPos wlBlockPos = where;
	for(y = where.Y; y > where.Y - obj->ViewFlag(__FLAG_BLOCKING); y--)
	{
		wlBlockPos.Y = y;
		SetBlocking(wlBlockPos, obj->GetUnderBlock() );
	}
}
/******************************************************************************/
// Removes all the blocking flags from the different units at this pos
void WorldMap::RemoveAllBlockingUnits(WorldPos where, Unit *obj )
/******************************************************************************/
{
	int y;
	WorldPos wlBlockPos = where;
	for(y = where.Y; y > where.Y - obj->ViewFlag(__FLAG_BLOCKING); y--)
	{
		wlBlockPos.Y = y;
		SetBlocking(wlBlockPos, __BLOCK_NONE );
	}
}
/******************************************************************************/
// Verifies the validity of a unit, whether the unit should be packetted or not.
bool WorldMap::VerifyUnitPacketting(Unit *lpUnit, Unit *target, WorldPos wlPos)        
/******************************************************************************/
{
    // never show hives!
	if( lpUnit->GetType() != U_HIVE ) 
	{
		if( !lpUnit->GetInvisibleQuery()->SendPacketTo( target ) )
		{
			// Do not add the player.
			return false;
		}
        
        return true;
	}
    return false;
}
/******************************************************************************/
void WorldMap::VerifyStackPacketting
//  Verifies a stack to find any units to send.
//
// WARNING: This function will virtually create all units which will be added
//          to the tlItemsFound list! You need to dereference all units
//          with DeleteUnit.
// 
(
    WorldPos wlPos,                     // Position of the stack.
    Capuchon *lpStack,                  // The stack.        
    TemplateList< Unit > &tlItemsFound, // List of items to send to player.
    Unit *target
)        
/******************************************************************************/
{
    Lock();
    
    vector< Unit * > vTentacule;
    
    // Fetch the whole tentacule.
    lpStack->ViewTentacule( wlPos.X, wlPos.Y, vTentacule );
 
    // Scroll through each units in the tentacule.
    vector< Unit * >::iterator i;

    for( i = vTentacule.begin(); i != vTentacule.end(); i++ )
	{
        Unit *lpUnit = (*i);
        
        // A unit on a tentacule should never be NULL
        ASSERT( lpUnit != NULL );
        if( lpUnit != NULL )
		{
            // Verifies that unit is suitable for sending to the client.
            if( VerifyUnitPacketting( lpUnit, target, wlPos ) )
			{
                TRACE( "\r\nFound an item!" );
                // Increase the unit's creation reference count.
                // This is used to avoid the unit being deleted before we used it completly.
                lpUnit->CreateVirtualUnit();
		    	// Add the unit to the list of items to send to clients.
                tlItemsFound.AddToTail( lpUnit );
            }
        }
    }

    Unlock();
}


/******************************************************************************/
// This functions puts "peripheral objects" (square of (RANGE*2xRANGE*2) inside a packet to 
// be shipped to client
BOOL WorldMap::packet_peripheral_units(WorldPos where, unsigned char srange, DIR::MOVE dir, TFCPacket &sending, Unit *self )
/******************************************************************************/
{			 	
   //NM
	const int xrange = 24; //18; //12
	const int yrange = 32; //26; //18

	WORD scanX = 0;
	WORD scanY = 0;
	WORD loX, hiX, loY, hiY;
	WORD i, j;

	Unit *obj;
	WorldPos ObjectWL;
	
	switch(dir)
	{ 
		case DIR::north:
			scanY = (where.Y - yrange); 
			loX = (where.X - xrange < 0) ? 0 : where.X - xrange;
			hiX = (where.X + xrange >= MAXX) ? MAXX - 1 : where.X + xrange;
			break;
		case DIR::east:		
			scanX = (where.X + xrange); 
			loY = (where.Y - yrange < 0) ? 0 : where.Y - yrange;
			hiY = (where.Y + yrange >= MAXY) ? MAXY - 1 : where.Y + yrange;
			break;
		case DIR::south:	
			scanY = (where.Y + yrange); 
			loX = (where.X - xrange < 0) ? 0 : where.X - xrange;
			hiX = (where.X + xrange >= MAXX) ? MAXX - 1 : where.X + xrange;	
			break;
		case DIR::west:		
			scanX = (where.X - xrange); 
			loY = (where.Y - yrange < 0) ? 0 : where.Y - yrange;
			hiY = (where.Y + yrange >= MAXY) ? MAXY - 1 : where.Y + yrange;		
			break;
		case DIR::northeast:
			scanX = (where.X + xrange); 
			scanY = (where.Y - yrange); 
			loY = (where.Y - yrange < 0) ? 0 : where.Y - yrange;
			hiY = (where.Y + yrange >= MAXY) ? MAXY - 1 : where.Y + yrange;	
			loX = (where.X - xrange < 0) ? 0 : where.X - xrange;
			hiX = (where.X + xrange >= MAXX) ? MAXX - 1 : where.X + xrange;	
			break;	
		case DIR::southeast:	
			scanX = (where.X + xrange); 
			scanY = (where.Y + yrange); 
			loY = (where.Y - yrange < 0) ? 0 : where.Y - yrange;
			hiY = (where.Y + yrange >= MAXY) ? MAXY - 1 : where.Y + yrange;	
			loX = (where.X - xrange < 0) ? 0 : where.X - xrange;
			hiX = (where.X + xrange >= MAXX) ? MAXX - 1 : where.X + xrange;							
			break;	
		case DIR::southwest:
			scanX = (where.X - xrange); 
			scanY = (where.Y + yrange); 
			loY = (where.Y - yrange < 0) ? 0 : where.Y - yrange;
			hiY = (where.Y + yrange >= MAXY) ? MAXY - 1 : where.Y + yrange;	
			loX = (where.X - xrange < 0) ? 0 : where.X - xrange;
			hiX = (where.X + xrange >= MAXX) ? MAXX - 1 : where.X + xrange;
			break;	
		case DIR::northwest:	
			scanX = (where.X - xrange); 
			scanY = (where.Y - yrange); 
			loY = (where.Y - yrange < 0) ? 0 : where.Y - yrange;
			hiY = (where.Y + yrange >= MAXY) ? MAXY - 1 : where.Y + yrange;	
			loX = (where.X - xrange < 0) ? 0 : where.X - xrange;
			hiX = (where.X + xrange >= MAXX) ? MAXX - 1 : where.X + xrange;							
			break;	
	}

	Lock();

	TemplateList <Unit> tlFoundItems;

	if(scanX < MAXX && scanX >= 0 && scanY >= 0 && scanY < MAXY)
	{
		TRACE(_T("\r\nScanX=%u ScanY=%u   "), scanX, scanY);

		if(scanX)
		{
			// if a X scan was asked	
			// Scans all capuchon in range
			for(j = (loY >> DIVIDER); j < (hiY >> DIVIDER) + 1; j++)
			{
				// if there's a capuchon then send all top units on the square
				if(WorldUnits[scanX >> DIVIDER][j])
				{
					for(i = 0; i < 4; i++)
					{
                        // Get the wlPos of the stack.
                        WorldPos wlPos = {
                            scanX, ( j << DIVIDER ) + i, world
                        };
                        VerifyStackPacketting( wlPos, WorldUnits[scanX >> DIVIDER][j], tlFoundItems, self );
			        }
				}
			}
		}
		// if a Y scan was asked
		if(scanY)
		{
			for(i = ((loX) >> DIVIDER); i < ((hiX) >> DIVIDER) + 1; i++)
			{
				// if there's a capuchon then send all top units on the square
				if(WorldUnits[i][scanY >> DIVIDER])
				{
					for(j = 0; j < 4; j++)
					{
                        // Get wlPos of the stack
                        WorldPos wlPos = {
                            ( i << DIVIDER ) + j, scanY, world
                        };
                        VerifyStackPacketting( wlPos, WorldUnits[i][scanY >> DIVIDER ], tlFoundItems, self );
					}
				}
			}
		}	
	}
    Unlock();

	if( tlFoundItems.NbObjects() > 0 )
	{
		sending << (RQ_SIZE)RQ_SendPeriphericObjects; // send objects
		sending << (short) tlFoundItems.NbObjects();
		tlFoundItems.ToHead();
		while( tlFoundItems.QueryNext() )
		{
			obj = tlFoundItems.Object();
			ObjectWL = obj->GetWL();		
			sending << (short)(ObjectWL.X);
			sending << (short)(ObjectWL.Y);

            obj->PacketUnitInformation( sending );
            
            obj->DeleteUnit();
		}
        return true;
	}
	return false;
}
/******************************************************************************/
// Creates a unit and puts it on a square
Unit *WorldMap::create_world_unit(char UnitType, UINT which, WorldPos WL, Unit *Provided, BOOL ForceCreation)
/******************************************************************************/
{	
	if(WL.X < MAXX && WL.Y < MAXY)
	{
		Lock();
		// if a capuchon wasn't found, create it
		//TRACE(_T("Creating a unit at (%u, %u)		"), WL.X, WL.Y);
		if(!WorldUnits[WL.X >> DIVIDER][WL.Y >> DIVIDER]) {
			WorldUnits[WL.X >> DIVIDER][WL.Y >> DIVIDER] = new Capuchon;
		}

		BYTE block;
		unsigned long offset = (WL.Y * (MAXX / 2)) + (WL.X / 2);

		if(!(WL.X % 2))
		{
			// if pair
			block = (map_buffy[offset] & 0xF0) >> 4;
		}
		else
		{
			block = (map_buffy[offset] & 0x0F); // impair
		}

		WL.world = world;
		if(WL.X < MAXX && WL.Y < MAXY)
		{
			//TRACE(_T("***%u***"), block);
			switch(UnitType)
			{		
				case U_PC:
            case U_MINIONS:
					if(1/*block == __BLOCK_NONE || ForceCreation || block == __AREA_BUILDING*/)
					{
						deposit_unit( WL, Provided );
						Unlock();
						return Provided;
					}
					else
					{
						Unlock(); 
                  return NULL; 
					}
					break;
				case U_NPC:
					TRACE( "block=%u  ", block );
					if( block == __AREA_BUILDING )
					{
						TRACE( " ..??" );
					}

					if( block == __BLOCK_NONE || ForceCreation || 
                   block == __AREA_BUILDING ||
						 block == __SAFE_HAVEN || 
                   block == __INDOOR_SAFE_HAVEN || 
                   block == __FULL_PVP_CANNOT_REALLY_DIE_DROP_ORROB ||
                   block == __FULL_PVP_CANNOT_REALLY_DIE_DROP_ORROB_CAST_SPELL ||
                   block == __ARENAGAME_FULL_PVP|| 
                   block == __ARENAGAME_BT_FULL_PVP||
                   block == __ARENAGAME_RT_FULL_PVP)
					{
						if(!Provided){
							Creatures *new_mob;
							new_mob = new Creatures;
							if(	new_mob->Create(U_NPC, which) )
							{
								new_mob->SetWL(WL);
								deposit_unit( WL, new_mob );
							}
							else
							{
								new_mob->DeleteUnit();
								new_mob = NULL;
							}
							Unlock();
							return new_mob;
						}
						else
						{
							deposit_unit( WL, Provided );
							Unlock();
							return Provided;
						}
					}
					else{
						Unlock();
						return NULL;
					}
					break;
				case U_OBJECT:
					if(1/*block == __BLOCK_NONE || ForceCreation || block == __AREA_BUILDING*/)
					{
						if(!Provided)
						{
							Objects *new_item;
							new_item = new Objects;
							if( new_item->Create(UnitType, which) )
							{
								new_item->SetUnderBlock( QueryAreaType( WL ) );
								new_item->SetWL(WL);
								WorldUnits[WL.X >> DIVIDER][WL.Y >> DIVIDER]->AddUnit(WL.X % 4, WL.Y % 4, new_item);
								if(new_item->ViewFlag(__FLAG_BLOCKING)) 
								{
									SetBlockingUnit(WL, new_item);
								}
								Unlock();
								return new_item;
							}
							else
							{
								_LOG_DEBUG LOG_DEBUG_LVL1, "Failed to create object BaseRefID %u! (at WorldMap::create_world_unit())", which LOG_
								Unlock();
								new_item->DeleteUnit();
								return NULL;
							}
						}
						else
						{
							WorldUnits[WL.X >> DIVIDER][WL.Y >> DIVIDER]->AddUnit(WL.X % 4, WL.Y % 4, Provided);
							if(Provided->ViewFlag(__FLAG_BLOCKING)) 
							{
								SetBlockingUnit(WL, Provided);
							}
							Unlock();
							return Provided;
						}
					} 
					else
					{
						Unlock();
						return NULL;
					}
					break;
				default: 
					{
						if(WorldUnits[WL.X >> DIVIDER][WL.Y >> DIVIDER]->IsEmpty())
						{
							if (WorldUnits[WL.X >> DIVIDER][WL.Y >> DIVIDER] != NULL)
							{
								delete WorldUnits[WL.X >> DIVIDER][WL.Y >> DIVIDER];
								WorldUnits[WL.X >> DIVIDER][WL.Y >> DIVIDER] = NULL;
							}
						}
						Unlock();
						return NULL;
					}
			}		
		}
		else
		{
			if(WorldUnits[WL.X >> DIVIDER][WL.Y >> DIVIDER]->IsEmpty())
			{
				if (WorldUnits[WL.X >> DIVIDER][WL.Y >> DIVIDER] != NULL)
				{
					delete WorldUnits[WL.X >> DIVIDER][WL.Y >> DIVIDER];
					WorldUnits[WL.X >> DIVIDER][WL.Y >> DIVIDER] = NULL;
				}
			}
			Unlock();
			return NULL;
		}
		Unlock();
	}
	return NULL;
}
/******************************************************************************/
// Adds an effect to a particular area in the world which will affect units getting in it
void WorldMap::add_effect(unsigned int minx, unsigned int miny, unsigned int maxx, unsigned int maxy, unsigned short effect, LPVOID params, int paramsize)
/******************************************************************************/
{
	maxx++;
	maxy++;

	Lock();
	AreaSensitiveEffect *tmp_effect;
	UINT y;
	for(y = miny; y < maxy; y++)
	{
		tmp_effect = new AreaSensitiveEffect;
		tmp_effect->Create(minx, maxx, effect, params, paramsize);
		EffectList[y].Lock();
		EffectList[y].AddToHead(tmp_effect);		
		EffectList[y].Unlock();
	}
	Unlock();
}
/******************************************************************************/
// Checks for any area effects and executes any effects
void WorldMap::QueryEffects(WorldPos where, Unit *who_steps)
{	
	AreaSensitiveEffect *temp_effect;
	EffectList[where.Y].Lock();
	EffectList[where.Y].ToHead();

	while(EffectList[where.Y].QueryNext()){
		temp_effect = EffectList[where.Y].Object();
		temp_effect->QueryEffect(where, who_steps);
	}
	EffectList[where.Y].Unlock();

}
/******************************************************************************/
// This function returns a position near, but out of sight, of a player
WorldPos WorldMap::GetLocalRandomPos(WorldPos where, BYTE range)
/******************************************************************************/
{
	WorldPos pos;

	if( range == 0 )
	{
		switch(rnd(0, 3))
		{
			case 0:
				pos.X = where.X + rnd(-16, 16);
				pos.Y = where.Y + rnd(-24, -16);
				break;
			case 1:
				pos.X = where.X + rnd(-16, 16);
				pos.Y = where.Y + rnd(16, 24);
				break;
			case 2:
				pos.X = where.X + rnd(-19, -11);
				pos.Y = where.Y + rnd(-20, 20);
				break;
			case 3:
				pos.X = where.X + rnd(11, 19);
				pos.Y = where.Y + rnd(-20, 20);
				break;
		}
	}
	else
	{
		pos.X = where.X + rnd( -range, range );
		pos.Y = where.Y + rnd( -range, range );
	}
	pos.world = world;
	if(pos.X >= 0 && pos.X < MAXX && pos.Y >= 0 && pos.Y < MAXY)
	{
		return pos;
	}

	pos.X = pos.Y = pos.world = 0;
	return pos;
}
/******************************************************************************/
// Summons a monster.
bool WorldMap::SummonMonster
(
 Unit *new_mob,    // Monster structure to summon
 BOOL boSummonAbsolute // TRUE if summon should appear at all cost.
)
/******************************************************************************/
{
   WorldPos mobWL = new_mob->GetWL();

   if( !boSummonAbsolute )
   {
      SubmitNearUnit *nearObj = new SubmitNearUnit;
      nearObj->pos = mobWL;
      nearObj->NoticeDeletion = FALSE;

      nearObj->Submission = SubmitNearUnit::ToCreate;
      nearObj->target = static_cast<Creatures *>(new_mob);

      TRACE( "Controling near submission!" );
      IsNearSubmission.Lock();
      IsNearSubmission.AddToTail(nearObj);
      IsNearSubmission.Unlock();
      TRACE( "Released near submission!!" );

      return true;
   }

   // Try to find a valid spot near the summon area.
   WorldPos wlPos = FindValidSpot( mobWL, 3 ,true);

   if( wlPos.X == -1 || wlPos.Y == -1 || wlPos.world == -1 )
   {
      return false;
   }
   Unit *ThisCreature = create_world_unit(U_NPC, 0, wlPos, new_mob);

   if( ThisCreature == NULL )
   {
      return false;
   }

   if( ThisCreature->SystemDestroy() )
   {
      WorldPos wlDest = { -1, -1, -1 };
      ThisCreature->SetUpperLimit( wlDest );
      ThisCreature->SetLowerLimit( wlDest );
   }
   else
   {
      MonsterStructure *lpMonsterStructure = NULL; 
      ThisCreature->SendUnitMessage( MSG_OnGetUnitStructure, ThisCreature, NULL, NULL, NULL, &lpMonsterStructure );

      WorldPos wlUpperLimit = mobWL;
      wlUpperLimit.X += lpMonsterStructure->wHiXrange;
      wlUpperLimit.Y += lpMonsterStructure->wHiYrange;
      WorldPos wlLowerLimit = mobWL;
      wlLowerLimit.X -= lpMonsterStructure->wLoXrange;
      wlLowerLimit.Y -= lpMonsterStructure->wLoYrange;

      ThisCreature->SetUpperLimit(wlUpperLimit);
      ThisCreature->SetLowerLimit(wlLowerLimit);
   }

   new_mob->SetWL( wlPos );
   EXHAUST newExhaust = {0,0,0};
   ThisCreature->SetExhaust(newExhaust);
   ThisCreature->SetIdleTime(TFCMAIN::GetRound() + _MONSTER_LIFE_SPAN);            
   TFCMAIN::AddMonster( ThisCreature );

   return true;
}
/******************************************************************************/
// Determines if a player is in view of the given position
BOOL WorldMap::IsNearPlayer(WorldPos wlPos)
/******************************************************************************/
{
	unsigned int loX, loY, hiX, hiY;
	unsigned int loXoff, loYoff, hiXoff, hiYoff;
	char range = 12;
	BOOL boFound = FALSE;

	if((wlPos.X - range) < 0)
	{
		loX		= 0;
		loXoff	= 0;
	}
	else
	{
		loX		= (wlPos.X - range) >> DIVIDER;
		loXoff	= 4 - ( (wlPos.X - range) % 4 );
	}

	if((wlPos.X + range) >= MAXX)
	{
		hiX		= ((MAXX - 1) >> DIVIDER);
		hiXoff	= ( MAXX - 1 ) % 4;
	}
	else
	{
		hiX		= (wlPos.X + range) >> DIVIDER;
		hiXoff	= (wlPos.X + range) % 4;
	}

	if((wlPos.Y - range) < 0)
	{
		loY		= 0;
		loYoff	= 0;
	}
	else
	{
		loY		= (wlPos.Y - range) >> DIVIDER;
		loYoff	= 4 - ( (wlPos.Y - range) % 4 );
	}

	if((wlPos.Y + range) >= MAXY)
	{
		hiY		= ((MAXY - 1) >> DIVIDER);
		hiYoff	= ( MAXY - 1 ) % 4;
	}
	else
	{
		hiY		= (wlPos.Y + range) >> DIVIDER;
		hiYoff	= (wlPos.Y + range) % 4;
	}

	int innerX, innerY, outerX, outerY;
	WORD X, Y;
	WORD i = 3, j = 4;
	Unit *obj;	
		
	// scans all capuchon
	Lock();
	for(X = loX; X < hiX; X++)
	{
		// If at last X capuchon
		if( X == hiX )
		{
			outerX = hiXoff;
		}
		else
		{
			outerX = 4;
		}

		// If at first X capuchon
		if( X == loX )
		{
			innerX = loXoff;
		}
		else
		{
			innerX = 0;
		}

		for(Y = loY; Y < hiY; Y++)
		{
			// If at last Y capuchon
			if( Y == hiY )
			{
				outerY = hiYoff;
			}
			else
			{
				outerY = 4;
			}

			if( Y == loY )
			{
				innerY = hiYoff;
			}
			else
			{
				innerY = 0;
			}

			// if a capuchon was found			
			if(WorldUnits[X][Y])
			{
				// then scan the capuchon
				for(i = innerX; i < outerX; i++)
				{
					
					for(j = innerY; j < outerY; j++)
					{
						loYoff = 0;
						obj = WorldUnits[X][Y]->ViewTopUnit( i, j );
						// Found a scanned object
						if(obj)
						{
							if( obj->GetType() == U_PC )
							{
								boFound = TRUE;
							}
						}
					}
				}
			}		
		}		
	}
	Unlock();

	return boFound;
}
/******************************************************************************/
// Uses an item on a square
void WorldMap::UseItem( WorldPos where, DWORD dwID, Unit *self, Unit *target )
/******************************************************************************/
{
	Lock();
	
	if( where.X < MAXX && where.Y < MAXY )
	{
		Unit *lpUnit = NULL;
		
        // If the tentacule exists.
		if(WorldUnits[where.X >> DIVIDER][where.Y >> DIVIDER])
		{
			lpUnit = WorldUnits[where.X >> DIVIDER][where.Y >> DIVIDER]->ViewUnitDeep( where.X, where.Y, dwID );
		}
		// If a unit was found.
        if( lpUnit != NULL )
		{
			// Can only use objects..!
            if( lpUnit->GetType() == U_OBJECT)
			{
                // Unit disturbed!!
                lpUnit->SendUnitMessage( MSG_OnDisturbed, lpUnit, self, self );   
                DWORD itemUsed = 0;
				lpUnit->SendUnitMessage( MSG_OnUse, lpUnit, self, self, NULL, &itemUsed );
            }
		}
	}

	Unlock();
}
/******************************************************************************/
// Puts all objects within a range of RANGE*2xRANGE*2 into a packet
int WorldMap::packet_inview_units(WorldPos where, TFCPacket &sending, unsigned char range, Unit *self )
/******************************************************************************/
{
	Lock();

	TRACE(_T("--%u--%u--%u--%u %u,"), where.X, where.Y, range, MAXX, MAXY);
	
	unsigned int loX, loY, hiX, hiY;
	if((where.X - range) < 0) {
		loX = 0;
	}
	else
	{
		loX = (where.X - range) >> DIVIDER;
	}

	if((where.X + range) > MAXX) 
	{
		hiX = ((MAXX - 1) >> DIVIDER);
	}
	else 
	{
		hiX = (where.X + range) >> DIVIDER;
	}

	if((where.Y - range) < 0)
	{
		loY = 0;
	}
	else
	{
		loY = (where.Y - range) >> DIVIDER;
	}

	if((where.Y + range) > MAXY)
	{
		hiY = ((MAXY - 1) >> DIVIDER);
	}
	else
	{
		hiY = (where.Y + range) >> DIVIDER;
	}

	WORD X, Y;
	WORD i, j;
	int next = 500;
	int nb_obj = 0;
	
	TRACE(_T("Getting objects from (%u, %u)-(%u,%u)"), loX, loY, hiX, hiY);

	TemplateList <Unit> tlFoundItems;

	// scans all capuchon
	for(X = loX; X < hiX; X++)
	{
		for(Y = loY; Y < hiY; Y++)
		{
			// if a capuchon was found
			if(WorldUnits[X][Y])
			{
				// then scan the capuchon
				for(i = 0; i < 4; i++)
				{
					for(j = 0; j < 4; j++)
					{						
                        // Get the wlPos of the stack.
                        WorldPos wlPos = {
                            X + i, Y + j, world
                        };
                        // Verify this stack
                        VerifyStackPacketting( wlPos, WorldUnits[X][Y], tlFoundItems, self );
					}
				}
			}
		}
	}
    Unlock();
   
    // If items were found
    if( tlFoundItems.NbObjects() > 0 )
	{
        Unit *lpUnit;

        sending << (short)__EVENT_OBJECT_APPEARED_LIST;
        sending << (short)tlFoundItems.NbObjects();	// This packet will the rest of the found items.		

		// Add each objects in the packet.
        tlFoundItems.ToHead();
		while( tlFoundItems.QueryNext() )
		{
            lpUnit = tlFoundItems.Object();            

			// Add the object to the current packet.
			WorldPos ObjectWL = lpUnit->GetWL();
			sending << (short)(ObjectWL.X);
			sending << (short)(ObjectWL.Y);					

         // If this unit is a puppet
         if( lpUnit->IsPuppet() )
         {
            // Packets its puppet information and send it to the user.
            TFCPacket pPacket;
            lpUnit->PacketPuppetInfo( pPacket );
            self->SendPlayerMessage( pPacket );
         }

         lpUnit->PacketUnitInformation( sending );

         // Free virtually created unit in VerifyStackPacketting.
         lpUnit->DeleteUnit();
      }
	}
	return tlFoundItems.NbObjects();
}

void WorldMap::valid_link_expire()
{
   
   Lock();

   if(m_WorldObjectMap.size() >0)
   {
      sWorldObjectSpec *pData = NULL;
      std::map<__int64, sWorldObjectSpec*>::iterator it = m_WorldObjectMap.begin();
      while(it != m_WorldObjectMap.end()) 
      {
         pData = (*it).second;
         if(pData)
         {
            DWORD dwElapsed = timeGetTime() -  pData->dwDropTime;
            if(pData->dwLockID && dwElapsed > theApp.dwTimedUnitLockExpire)
               pData->dwLockID = 0;

            if(dwElapsed > theApp.dwTimedUnitExpire)
            {
               //on wipe l'object
               Unit *pUnit = pData->pUnit;
               it = m_WorldObjectMap.erase( it );
               pUnit->VaporizeUnit();
               
            }
            else
               it++;
         }
         else
         {
            it = m_WorldObjectMap.erase( it );
         }
      }
   }
   Unlock();
}


bool WorldMap::valid_link_id( WorldPos where, DWORD ID ,DWORD dwGetUserID)
{
   Lock();

   bool bOK = true;
   uIDPosCompo uID;
   uID.sID.ushX = where.X;
   uID.sID.ushY = where.Y;
   uID.sID.dwID = ID;

   sWorldObjectSpec *pData = NULL;

   std::map<__int64, sWorldObjectSpec*>::iterator it = m_WorldObjectMap.find(uID.i64ID);
   if (it != m_WorldObjectMap.end()) 
   {
      pData = (*it).second;
      if(pData && pData->dwLockID >0)
      {
         if(pData->dwLockID == dwGetUserID)
            bOK = true;
         else
            bOK = false;
      }
   }
   Unlock();

   return bOK;
}

/******************************************************************************/
// Removes a unit from a square
void WorldMap::remove_world_unit( WorldPos where, DWORD ID )
{
   Lock();

   // check if there's a capuchon
   if(WorldUnits[where.X >> DIVIDER][where.Y >> DIVIDER])
   {
      // Then removes the unit from the capuchon
      Unit *lpFoundUnit = WorldUnits[where.X >> DIVIDER][where.Y >> DIVIDER]->FetchUnitDeep( where.X, where.Y, ID );

      bool boQueryEmptyWorldUnits = true;

      // If it found the object.
      if( lpFoundUnit )
      {
         BYTE bObjBlock = lpFoundUnit->GetUnderBlock();
         if( lpFoundUnit->GetType() != U_OBJECT )
         {
            // Restore old blocking
            SetBlocking( where, bObjBlock );
         }
         else
         { 
            //CV:ITEM_TS --> Efface l<item dans remove_world_unit 
            if(theApp.dwTimedUnitEnable)
            {
               uIDPosCompo uID;
               uID.sID.ushX = where.X;
               uID.sID.ushY = where.Y;
               uID.sID.dwID = ID;
               std::map<__int64, sWorldObjectSpec*>::iterator it = m_WorldObjectMap.find(uID.i64ID);
               if (it != m_WorldObjectMap.end()) 
               {
                  m_WorldObjectMap.erase( it );
               }
            }
            
         }
      }
      else
      {
         // If no unit was found, well now its complicated! We *must* find it
         // Search for the unit in nearby areas.
         Unit *lpUnit = FindNearUnit( where, ID );

         // If it was found
         if( lpUnit != NULL )
         {
            _LOG_DEBUG
               LOG_DEBUG_LVL4,
                  "Unit %u couln't be removed for specific remove_world_unit at %u, %u, %u, re-trying",
                  ID,
                  where.X,
                  where.Y,
                  world
               LOG_

            // FindNearUnit should have adjusted lpUnit's world pos.
            remove_world_unit( lpUnit->GetWL(), lpUnit->GetID() );                
            // Do not query empty world units, the called remove_world_unit will do it.
            boQueryEmptyWorldUnits = false;
         }
         else
         {
            // Otherwise issue a warning
            _LOG_DEBUG
               LOG_DEBUG_LVL1,
                  "Couldn't find unit %u to remove from (%u, %u, %u)", 
                  ID,
                  where.X, 
                  where.Y, 
                  world
               LOG_
         }
      }

      if( boQueryEmptyWorldUnits )
      {
         // If capuchon is now empty
         if(WorldUnits[where.X >> DIVIDER][where.Y >> DIVIDER]->IsEmpty())
         {
            // Free it.
            if (WorldUnits[where.X >> DIVIDER][where.Y >> DIVIDER] != NULL)
            {
               delete WorldUnits[where.X >> DIVIDER][where.Y >> DIVIDER];
               WorldUnits[where.X >> DIVIDER][where.Y >> DIVIDER] = NULL;
            }
         }
      }
   }
   else
   {
      // If no capuchon was found.
      _LOG_DEBUG
         LOG_DEBUG_LVL4,
            "No capuchon for remove_world_unit at %u, %u, %u for ID %u. Retrying.",
            where.X, where.Y, world, ID
         LOG_

      // Find unit near the capuchon.
      Unit *lpUnit = FindNearUnit( where, ID );

      // If it was found
      if( lpUnit != NULL )
      {
         // FindNearUnit should have adjusted lpUnit's world pos.
         remove_world_unit( lpUnit->GetWL(), lpUnit->GetID() );                
      }
      else
      {
         // Otherwise issue a warning
         _LOG_DEBUG
            LOG_DEBUG_LVL1,
               "Couldn't find unit %u to remove from (%u, %u, %u)", 
               ID,
               where.X, 
               where.Y, 
               world
            LOG_
      }
   }

   Unlock();
}
/******************************************************************************/
// Moves a world object from one place to another
BOOL WorldMap::move_world_unit(WorldPos from, WorldPos to, DWORD ID, char direction, BOOL god, bool boBroadcastMove )
/******************************************************************************/
{				
	BOOL return_value = TRUE;	
   WorldPos wlDestination = to;
	Unit *lpUnit;
	CString logmsg;

#ifdef __ENABLE_LOG
	if(__LOG > 95){
		logmsg.Format("Moving unit ID#%u from (%u, %u wl %u) to (%u, %u wl %u)", ID, from.X, from.Y, from.world, to.X, to.Y, to.world);
		__LOG((LPCTSTR)logmsg);
	}
#endif
	
   if(to.X < MAXX && to.Y < MAXY && to.X >= 0 && to.Y >= 0 )
   {
      Lock();

      // If there is a capuchon where the object is
      if( WorldUnits[from.X >> DIVIDER][from.Y >> DIVIDER] )
      {

         // Gets the object from the capuchon
         int done = 0;

         // View the unit from the capuchon.
         lpUnit = WorldUnits[ from.X >> DIVIDER ][ from.Y >> DIVIDER ]->ViewUnitDeep( from.X, from.Y, ID );

         // If the unit exists.
         if( lpUnit != NULL )
         {
            // If the 'to' worldpos is blocked.
            if( internalIsBlocking( to, lpUnit ) && !god )
            {
               wlDestination = from;
               return_value = FALSE;
            }
            else
            {
               // If unit doesn't move, don't bother moving it!
               if( to.X != from.X || to.Y != from.Y )
               {
                  // Finally, remove the unit from it's previous place
                  remove_world_unit( from, ID );
                  // deposit unit to its new location
                  deposit_unit( to, lpUnit );
               }
            }

            Unlock();
            if( boBroadcastMove && return_value)
            {
               TFCPacket sending;
               sending << (RQ_SIZE)__EVENT_OBJECT_MOVED;
               sending << (short)wlDestination.X;
               sending << (short)wlDestination.Y;     // where the player has moved (is now)

               lpUnit->PacketUnitInformation( sending );		    		
               // NMNMNM 22
               Broadcast::BCast( wlDestination, _DEFAULT_RANGE, sending, lpUnit->GetInvisibleQuery() );	
            }

            return return_value;
         }
      }
      Unlock();
   }
   return FALSE;
}
/******************************************************************************/
// Scans all hostiles and decides what action to do according to the 'units' allegeance
Unit *WorldMap::ScanHostiles(WorldPos where, char range, Unit *ThisCreature)
/******************************************************************************/
{
	unsigned int loX, loY, hiX, hiY;
	unsigned int loXoff, loYoff, hiXoff, hiYoff;

	if((where.X - range) < 0)
	{
		loX		= 0;
		loXoff	= 0;
	}
	else
	{
		loX		= (where.X - range) >> DIVIDER;
		loXoff	= 4 - ( (where.X - range) % 4 );
	}

	if((where.X + range) >= MAXX)
	{
		hiX		= ((MAXX - 1) >> DIVIDER);
		hiXoff	= ( MAXX - 1 ) % 4;
	}
	else
	{
		hiX		= (where.X + range) >> DIVIDER;
		hiXoff	= (where.X + range) % 4;
	}

	if((where.Y - range) < 0)
	{
		loY		= 0;
		loYoff	= 0;
	}
	else
	{
		loY		= (where.Y - range) >> DIVIDER;
		loYoff	= 4 - ( (where.Y - range) % 4 );
	}

	if((where.Y + range) >= MAXY)
	{
		hiY		= ((MAXY - 1) >> DIVIDER);
		hiYoff	= ( MAXY - 1 ) % 4;
	}
	else
	{
		hiY		= (where.Y + range) >> DIVIDER;
		hiYoff	= (where.Y + range) % 4;
	}

	int innerX, innerY, outerX, outerY;
	WORD X, Y;
	WORD i = 3, j = 4;
	Unit *obj;	
	int next = 500;
	int nb_obj = 0;
	DWORD ourID = ThisCreature->GetID();
	TemplateList <Unit> PossibleTargets;
	//TRACE(_T("Scanning for creatures in (%u, %u)-(%u,%u)", where.X - range, where.Y - range, where.X + range, where.Y + range);

#ifdef __ENABLE_LOG
	if(__LOG > 90)
	{
		CString logmsg;
		logmsg.Format("NPCID %u, '%s', scans for hostiles at (%u, %u wl %u)", ourID, ThisCreature->GetName(), where.X, where.Y, world);
		__LOG((LPCTSTR)logmsg);
	}
#endif
	BOOL boBerserk;
	if( ThisCreature->ViewFlag( __FLAG_BERSERK ) != 0 )
	{
		boBerserk = TRUE;
	}
	else
	{
		boBerserk = FALSE;
	}
	
	// scans all capuchon
	Lock();
	for(X = loX; X < hiX; X++)
	{
		// If at last X capuchon
		if( X == hiX )
		{
			outerX = hiXoff;
		}
		else
		{
			outerX = 4;
		}
		// If at first X capuchon
		if( X == loX )
		{
			innerX = loXoff;
		}
		else
		{
			innerX = 0;
		}

		for(Y = loY; Y < hiY; Y++)
		{
			// If at last Y capuchon
			if( Y == hiY )
			{
				outerY = hiYoff;
			}
			else
			{
				outerY = 4;
			}

			if( Y == loY )
			{
				innerY = hiYoff;
			}
			else
			{
				innerY = 0;
			}

			// if a capuchon was found			
			if(WorldUnits[X][Y])
			{
				// then scan the capuchon
				for(i = innerX; i < outerX; i++)
				{
					for(j = innerY; j < outerY; j++)
					{
						loYoff = 0;
						obj = WorldUnits[X][Y]->ViewTopUnit(i,j);
                        try
						{
							// Found a scanned object
							if(obj)
							{
								//Only attacks creatures or players.. ;)
								if(obj->GetType() != U_OBJECT && obj->GetType() != U_HIVE && obj->GetType() != U_MINIONS && obj->GetID() != ourID && ThisCreature->GetBond() != obj)
								{
									WORD OtherClan = obj->GetClan();
									DWORD OtherID = obj->GetID();
									WORD ThisClan = ThisCreature->GetClan();							

									BOOL BeNatural = TRUE;
									int politic = 0;
									BOOL BeAgressive = FALSE;

									BOOL boEvadeView = FALSE;

									bool isInvisible = false;
									// If this unit is a creature, verify with evade view.
									if( ThisCreature->GetType() == U_NPC )
									{
										// Do evade view test with creature's int.
										MonsterStructure *mob;
										ThisCreature->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &mob );

										// If monster's int loses its evade view test.
										if( rnd.testvs( mob->INT, obj->ViewFlag( __FLAG_EVASIVNESS ) ) < 0 )
										{
											boEvadeView = TRUE;																		
										}
									}


                           //CV: Check si Friendly
                           DWORD dwFactionID  = obj->ViewFlag( __FLAG_PJ_VS_MONSTER_FRIENDLY );
                           DWORD dwMonsterFID = ThisCreature->GetFriendlyID();
                           if(dwFactionID >0 && dwFactionID <31 && dwMonsterFID > 0)
                           {
                              DWORD dwFlagID      = 3000000+dwMonsterFID;
                              DWORD dwFactionMask = PLFactionMask(dwFactionID);
                              if( CheckGlobalFlag(dwFlagID) & dwFactionMask)
                                 boEvadeView = TRUE; //AMI on ne le set pas dans la liste des ennemie possible
                           }

									if( !boEvadeView )
									{
										// Checks to see if they are in the same clan
										//TRACE(_T("\r\n\r\nTEST CLAN %u VS CLAN %u!!\r\n\r\n") , OtherClan, ThisClan);

                              
                              

										if(ThisClan != OtherClan || !ThisClan)
										{
											// Returns the mutual relation between us and the encountered unit
											if(ThisClan < Clans::GetNumberOfClans() && OtherClan < Clans::GetNumberOfClans())							
											{
												politic = CreatureClans[ThisClan].MutualRelation(OtherClan, OtherID);
											}

											// if clan is aressive towards us
											politic += ThisCreature->GetAgressivness();

											// If target is invisible, diminish aggressivity.
											if( obj->ViewFlag( __FLAG_INVISIBILITY ) != 0 || obj->ViewFlag( __FLAG_INVISIBILITY2 ) != 0 || obj->ViewFlag( __FLAG_HIDDEN ) )
											{
												politic -= (abs(politic) * 50) / 100;
											}

											//TRACE(_T("Politic=%d, "), politic);
											// If clan politics succeeded, then we are to be agressive
											if(rnd(1, 100) < politic || boBerserk )
											{
												BeAgressive = TRUE;
												TRACE(_T("Agressive"));
												// Increase the unit's creation reference count.
												obj->CreateVirtualUnit();
												PossibleTargets.AddToTail(obj);	
											}
										}
									}
								}
							}
                        }
						catch(...)
						{
							_LOG_DEBUG
                       LOG_CRIT_ERRORS,
                       "Crash in ScanHostiles handling unit 0x%x.",
                       obj
                     LOG_
                     throw;
                  }
					}
				}
			}		
		}		
	}
	Unlock();

	Unit *NearestFoe = NULL;

	PossibleTargets.ToHead();
	UINT lastray = 0xFFFFFFFF;
	UINT newray;
	WorldPos CreatureWL;
	// If it has scanned any unliked foes, then check who is the nearest
	while(PossibleTargets.QueryNext())
	{
		CreatureWL = PossibleTargets.Object()->GetWL();
		// ray = x^2 + y^2 (we don't care about the square root)
		CreatureWL.X *= CreatureWL.X;
		CreatureWL.Y *= CreatureWL.Y;
		newray = CreatureWL.X + CreatureWL.Y;
		if(newray < lastray)
		{
			lastray = newray;
			NearestFoe = PossibleTargets.Object();
		}
		
        // If the unit was eradicated from memory.
        if( PossibleTargets.Object()->DeleteUnit() )
		{
            // If the deleted unit was the found nearest foe.
            if( NearestFoe == PossibleTargets.Object() )
			{
                // Well, the nearest foe is dead.
                NearestFoe = NULL;
            }
        }
        PossibleTargets.Remove();
	}

	// if it found a unit to attack
	if(NearestFoe)
	{
		TRACE(_T("Found a foe of type %u\r\n"), NearestFoe->GetType());
		ThisCreature->Do(fighting);		
		ThisCreature->SetTarget(NearestFoe);		
	}else{
		ThisCreature->SetTarget(ThisCreature->GetBond());
		ThisCreature->Do(wandering,"ScanHostiles");
		// CreatureWL.X = CreatureWL.Y = CreatureWL.world = -1;
		// ThisCreature->SetDestination(CreatureWL);
	}

	// Return the nearest enemy.
	return NearestFoe;
}
/******************************************************************************/
int WorldMap::GetMAXX()
/******************************************************************************/
{
	return static_cast< int >( MAXX );
}
/******************************************************************************/
int WorldMap::GetMAXY()
/******************************************************************************/
{
//	TRACE(_T("MAXX=%u", MAXY);
	return static_cast< int >( MAXY );
}
/******************************************************************************/
// Queries the area type of a given position
BYTE WorldMap::QueryAreaType(WorldPos wlPos) // The position to query
/******************************************************************************/
{
	DWORD offset = (wlPos.Y * (MAXX / 2)) + (wlPos.X / 2);
	BYTE block = __BLOCK_NONE;
	if( IsValidPosition( wlPos ) )
	{		
		if(!( wlPos.X % 2))
		{
			// if pair
			block = (map_buffy[offset] & 0xF0) >> 4;
		}
		else
		{
			block = (map_buffy[offset] & 0x0F); // impair
		}
 	} 
	return block;
}

BYTE WorldMap::QueryAreaTypeMap(WorldPos wlPos) // The position to query
/******************************************************************************/
{
   DWORD offset = (wlPos.Y * (MAXX / 2)) + (wlPos.X / 2);
   BYTE block = __BLOCK_NONE;
   if( IsValidPosition( wlPos ) )
   {		
      if(!( wlPos.X % 2))
      {
         // if pair
         block = (map_buffyOri[offset] & 0xF0) >> 4;
      }
      else
      {
         block = (map_buffyOri[offset] & 0x0F); // impair
      }
   } 
   return block;
}

/******************************************************************************/
// This function returns TRUE if square has a block flag
BOOL WorldMap::internalIsBlocking(WorldPos where, Unit *lpMoving, signed short XO, signed short YO)
/******************************************************************************/
{	    
    // Always block for invalid positions.
    if( !IsValidPosition( where ) )
	{
        return TRUE;
    }

    unsigned long offset = (where.Y * (MAXX / 2)) + (where.X / 2);
	unsigned char block;
	
    if(!(where.X % 2))
	{
		// if pair
		block = (map_buffy[offset] & 0xF0) >> 4;
    }
	else
	{
		block = (map_buffy[offset] & 0x0F); // impair
    }

	switch(block)
	{		 
		case __BLOCK_DEEP_WATER:
			case __BLOCK_CAN_FLY_OVER:
				case __BLOCK_ABSOLUTE: 
					return TRUE;

		case __AREA_FULL_PVP:
			case __SAFE_HAVEN: 
				case __INDOOR_SAFE_HAVEN:
					case __BLOCK_NONE:
						case __BLOCK_FORCE_FIELD:
							return FALSE;
	  
	 	case __BLOCK_SHALLOW_WATER:
			// If a unit was specified
			if( lpMoving != NULL )
			{
				// If unit is flying, do not block.
				if( lpMoving->ViewFlag( __FLAG_FLY ) != 0 || lpMoving->ViewFlag( __FLAG_WALK_ON_WATER ) != 0 )
				{
					return FALSE;
				}
			}	
			return TRUE;
	}
	return FALSE;
}
/******************************************************************************/
// Finds a nearby unit using it's ID. It requires X and Y coordinates so it won't scan
// the entire world but rather the 2 Capuchons (making it an area of 3x3 capuchons)
Unit *WorldMap::FindNearUnit(WorldPos where, DWORD ID)
/******************************************************************************/
{
    Unit *lpUnit = NULL;
    
    if( !IsValidPosition( where ) )
	{
        return NULL;
    }

	POINT capFrom = {(where.X >> DIVIDER) - 2, (where.Y >> DIVIDER) - 2};
	POINT capTo   = {(where.X >> DIVIDER) + 2, (where.Y >> DIVIDER) + 2};

	// Checks so the scanning won't go out of bounds
	capFrom.x = capFrom.x < 0 ? 0 : capFrom.x;
	capFrom.y = capFrom.y < 0 ? 0 : capFrom.y;

	capTo.x = capTo.x >= MAXX >> DIVIDER ? (MAXX >> DIVIDER) - 1 : capTo.x;
	capTo.y = capTo.y >= MAXY >> DIVIDER ? (MAXY >> DIVIDER) - 1 : capTo.y;

	long x, y;
	WORD i, j;	

	Lock();

	// scans x and y capuchons
	for(x = capFrom.x; x < capTo.x; x++)
	{
		for(y = capFrom.y; y < capTo.y; y++)
		{
			// if the capuchon exists
			if(WorldUnits[x][y])
			{
				// then scan the internal capuchon
				for(i = 0; i < 4; i++)
				{
					for(j = 0; j < 4; j++)
					{
                        // Try to find unit inside tentacule.
						lpUnit = WorldUnits[x][y]->ViewUnitDeep( i, j, ID );

                        // If the right unit was found.
                        if( lpUnit != NULL )
						{
                            WorldPos wlUpdatedUnitPos = {( x << DIVIDER ) + i,( y << DIVIDER ) + j,world};
                            
							TRACE( "\r\nResetting unit's pos to %u, %u, %u.", wlUpdatedUnitPos.X,wlUpdatedUnitPos.Y, wlUpdatedUnitPos.world );
                            // Make the for loops break.
                            j = i = 0xFFFE;
							x = y = 0xFFFE;
                            // Set the unit's world pos.
                            lpUnit->SetWL( wlUpdatedUnitPos );
						}
					}
				}
			}
		}
	}

	Unlock();

	return lpUnit;
}

DWORD WorldMap::GetInViewObjectsUnits(WorldPos wlWhere, BYTE bRange,DWORD *pdwID, WorldPos *pWL)
{
   DWORD dwNbrItems = 0;
   if(bRange < MAXIMAL_SCAN_RANGE)
   {
      unsigned int loX=10, loY=11, hiX=12, hiY=13;

      if((wlWhere.X - bRange) < 0) loX = 0;
      else loX = (wlWhere.X - bRange) >> DIVIDER;

      if((wlWhere.X + bRange) >= MAXX) hiX = ((MAXX - 1) >> DIVIDER);
      else hiX = (wlWhere.X + bRange) >> DIVIDER;

      if((wlWhere.Y - bRange) < 0) loY = 0;
      else loY = (wlWhere.Y - bRange) >> DIVIDER;

      if((wlWhere.Y + bRange) >= MAXY) hiY = ((MAXY - 1) >> DIVIDER);
      else hiY = (wlWhere.Y + bRange) >> DIVIDER;

      WORD X, Y;
      WORD i, j;

      // scans all capuchon
      Lock();
      for(X = loX; X < hiX; X++)
      {
         for(Y = loY; Y < hiY; Y++)
         {
            // if a capuchon was found
            if(WorldUnits[X][Y])
            {
               // then scan the capuchon
               for(i = 0; i < 4; i++)
               {
                  for(j = 0; j < 4; j++)
                  {
                     WorldPos wlPos = {X + i, Y + j, world};

                     // Verify this stack
                     Capuchon *lpStack = WorldUnits[X][Y];
                     vector< Unit * > vTentacule;

                     // Fetch the whole tentacule.
                     lpStack->ViewTentacule( wlPos.X, wlPos.Y, vTentacule );
                     vector< Unit * >::iterator i;
                     for( i = vTentacule.begin(); i != vTentacule.end(); i++ )
                     {
                        Unit *lpUnit = (*i);

                        // A unit on a tentacule should never be NULL
                        if( lpUnit != NULL )
                        {
                           if(lpUnit->GetType() == U_OBJECT && dwNbrItems < 499)
                           {
                              pdwID[dwNbrItems] = lpUnit->GetID();
                              pWL  [dwNbrItems] = lpUnit->GetWL();
                              dwNbrItems++;
                           }
                           else
                           {
							  
                              if(dwNbrItems >=499)
                              {
                                 Unlock();
                                 return dwNbrItems;
                              }
                           }
                        }
                     }
                  }
               }
            }
         }		
      }
      Unlock();
   }

   return dwNbrItems;
}

DWORD WorldMap::GetZoneObjectsUnits(int iX1,int iY1,int iX2,int iY2,int iMaxElement,DWORD *pdwID, WorldPos *pWL)
{
   DWORD dwNbrItems = 0;
   unsigned int loX = iX1>> DIVIDER;
   unsigned int loY = iY1>> DIVIDER;
   unsigned int hiX = iX2>> DIVIDER;
   unsigned int hiY = iY2>> DIVIDER;

   WORD X, Y;
   WORD i, j;

   // scans all capuchon
   Lock();
   for(X = loX; X < hiX; X++)
   {
      for(Y = loY; Y < hiY; Y++)
      {
         // if a capuchon was found
         if(WorldUnits[X][Y])
         {
            // then scan the capuchon
            for(i = 0; i < 4; i++)
            {
               for(j = 0; j < 4; j++)
               {
                  WorldPos wlPos = {X + i, Y + j, world};

                  // Verify this stack
                  Capuchon *lpStack = WorldUnits[X][Y];
                  vector< Unit * > vTentacule;

                  // Fetch the whole tentacule.
                  lpStack->ViewTentacule( wlPos.X, wlPos.Y, vTentacule );
                  vector< Unit * >::iterator i;
                  for( i = vTentacule.begin(); i != vTentacule.end(); i++ )
                  {
                     Unit *lpUnit = (*i);

                     // A unit on a tentacule should never be NULL
                     if( lpUnit != NULL )
                     {
                        if(lpUnit->GetType() == U_OBJECT && dwNbrItems < iMaxElement-1)
                        {

                           _item *lpItem = NULL;
                           lpUnit->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &lpItem );
                           if( lpItem != NULL )
                           {
                              if( lpItem->dwDropFlags & CANNOT_GET_ITEM ) 
                              {

                              }
                              else
                              {
                                 pdwID[dwNbrItems] = lpUnit->GetID();
                                 pWL  [dwNbrItems] = lpUnit->GetWL();
                                 dwNbrItems++;
                              }
                           }
                        }
                        else
                        {

                           if(dwNbrItems >=iMaxElement-1)
                           {
                              Unlock();
                              return dwNbrItems;
                           }
                        }
                     }
                  }
               }
            }
         }
      }		
   }
   Unlock();

   return dwNbrItems;
}

void  WorldMap::RemoveZoneObjectsUnits(int iX1,int iY1,int iX2,int iY2)
{
   DWORD dwNbrItems = 0;
   unsigned int loX = iX1>> DIVIDER;
   unsigned int loY = iY1>> DIVIDER;
   unsigned int hiX = iX2>> DIVIDER;
   unsigned int hiY = iY2>> DIVIDER;

   WORD X, Y;
   WORD i, j;

   // scans all capuchon
   Lock();
   for(X = loX; X < hiX; X++)
   {
      for(Y = loY; Y < hiY; Y++)
      {
         // if a capuchon was found
         if(WorldUnits[X][Y])
         {
            // then scan the capuchon
            for(i = 0; i < 4; i++)
            {
               for(j = 0; j < 4; j++)
               {
                  WorldPos wlPos = {X + i, Y + j, world};

                  // Verify this stack
                  Capuchon *lpStack = WorldUnits[X][Y];
                  vector< Unit * > vTentacule;

                  // Fetch the whole tentacule.
                  lpStack->ViewTentacule( wlPos.X, wlPos.Y, vTentacule );
                  vector< Unit * >::iterator i;
                  for( i = vTentacule.begin(); i != vTentacule.end(); i++ )
                  {
                     Unit *lpUnit = (*i);

                     // A unit on a tentacule should never be NULL
                     if( lpUnit != NULL )
                     {

                        if(lpUnit->GetType() == U_OBJECT)
                        {
                          lpUnit->VaporizeUnit(false);
                        }
                     }
                  }
               }
            }
         }
      }		
   }
   Unlock();
   return;
}


/******************************************************************************/
// This function gets a list of units in the area of the player withing bRange range.
TemplateList <Unit> *WorldMap::GetLocalUnits
(
 WorldPos wlWhere,  // Center of spot to search
 BYTE bRange,       // Range of search
 BOOL boLiving      // Only return living things (_PCs and _NPCs) 
)
/******************************************************************************/
{
	if(bRange < MAXIMAL_SCAN_RANGE)
	{
		unsigned int loX=10, loY=11, hiX=12, hiY=13;

		if((wlWhere.X - bRange) < 0) 
		{
			loX = 0;
		}
		else
		{
			loX = (wlWhere.X - bRange) >> DIVIDER;
		}

		if((wlWhere.X + bRange) >= MAXX)
		{
			hiX = ((MAXX - 1) >> DIVIDER);
		}
		else
		{
			hiX = (wlWhere.X + bRange) >> DIVIDER;
		}

		if((wlWhere.Y - bRange) < 0)
		{
			loY = 0;
		}
		else
		{
			loY = (wlWhere.Y - bRange) >> DIVIDER;
		}

		if((wlWhere.Y + bRange) >= MAXY)
		{
			hiY = ((MAXY - 1) >> DIVIDER);
		}
		else
		{
			hiY = (wlWhere.Y + bRange) >> DIVIDER;
		}

		WORD X, Y;
		WORD i, j;
		Unit *obj;	
		BOOL boFetchUnit = TRUE;

		TemplateList <Unit> *tlFoundUnits = new TemplateList <Unit>;

		// scans all capuchon
		Lock();
		for(X = loX; X < hiX; X++)
		{
			for(Y = loY; Y < hiY; Y++)
			{
				// if a capuchon was found
				if(WorldUnits[X][Y])
				{
					// then scan the capuchon
					for(i = 0; i < 4; i++)
					{
						for(j = 0; j < 4; j++)
						{	
							obj = WorldUnits[X][Y]->ViewTopUnit(i,j);
							// Found a scanned object
							if(obj)
							{
								// if boLiving was specified, do not fetch objects
								boFetchUnit = TRUE;
								if(boLiving)
								{
									if(obj->GetType() == U_OBJECT)
									{
										boFetchUnit = FALSE;
									}
								}

								if(boFetchUnit)
								{
									tlFoundUnits->AddToTail(obj);
								}														
							}
						}
					}
				}
			}		
		}
		Unlock();

		return tlFoundUnits;
	}
	return NULL;
}
/******************************************************************************/
// Finds a valid non-blocking spot withing wRange of the given wlCenter.
WorldPos WorldMap::FindValidSpot
(
 WorldPos wlCenter, // Center area
 WORD wRange,		// Max range from which the spot can be found.
 BOOL bAllowPiles
)
// Return: WorldPos, the found spot. { -1, -1, -1 } if no spot could be found.
/******************************************************************************/
{
	WorldPos wlFoundPos = { -1, -1, -1 };
	BOOL boFound = FALSE;	
	
	//if (bAllowPiles) return wlCenter;//BLBLBL
	Lock();

	WorldPos wlCheckPos = { 0, 0, world };
	int nRay = 1;	// Begins at ray 1

	// Does squares around the center.
	while( !boFound && nRay < wRange )
	{
		// Check lower line.
		//
		//       C
		//      ...
		wlCheckPos.Y = wlCenter.Y + nRay;		
		wlCheckPos.X = wlCenter.X - nRay;
		while( wlCheckPos.X < wlCenter.X + nRay + 1 && !boFound )
		{
			if( !IsBlocking( wlCheckPos ) )
			{
				TRACE( "\r\nFound pos %u, %u, %u", wlCheckPos.X, wlCheckPos.Y, wlCheckPos.world );
				
				//BLBLBL on vérifie si y a pas une unité déjà sur la case où on veut dropper :
				if (bAllowPiles || (WorldUnits[ wlCheckPos.X >> DIVIDER ][ wlCheckPos.Y >> DIVIDER ] == NULL) )
				{
					// Found pos!
					wlFoundPos = wlCheckPos;
					boFound = TRUE;
				}
			}
			wlCheckPos.X++;
		}
		
		// Check upper line.
		//      ...
		//       C
		//      !!!
		wlCheckPos.Y = wlCenter.Y - nRay;		
		wlCheckPos.X = wlCenter.X - nRay;
		while( wlCheckPos.X < wlCenter.X + nRay + 1 && !boFound )
		{
			if( !IsBlocking( wlCheckPos ) )
			{
				//BLBLBL on vérifie si y a pas une unité déjà sur la case où on veut dropper :
				if (bAllowPiles || ( WorldUnits[ wlCheckPos.X >> DIVIDER ][ wlCheckPos.Y >> DIVIDER ] == NULL ) )
				{
					// Found pos!
					wlFoundPos = wlCheckPos;
					boFound = TRUE;
				}
			}
			wlCheckPos.X++;
		}
		
		// Check left side
		//       !!!
		//       .C
		//       !!!
		wlCheckPos.X = wlCenter.X - nRay;
		wlCheckPos.Y = wlCenter.Y - nRay + 1;
		while( wlCheckPos.Y < wlCenter.Y + nRay && !boFound )
		{
			if( !IsBlocking( wlCheckPos ) )
			{
				//BLBLBL on vérifie si y a pas une unité déjà sur la case où on veut dropper :
				if (bAllowPiles || ( WorldUnits[ wlCheckPos.X >> DIVIDER ][ wlCheckPos.Y >> DIVIDER ] == NULL ) )
				{
					// Found pos!
					wlFoundPos = wlCheckPos;
					boFound = TRUE;
				}
			}			
			wlCheckPos.Y++;
		}
		
		// Check right side
		//		  !!!
		//        !C.
		//        !!!
		wlCheckPos.X = wlCenter.X + nRay;
		wlCheckPos.Y = wlCenter.Y - nRay + 1;
		while( wlCheckPos.Y < wlCenter.Y + nRay && !boFound )
		{
			if( !IsBlocking( wlCheckPos ) )
			{
				//BLBLBL on vérifie si y a pas une unité déjà sur la case où on veut dropper :
				if (bAllowPiles || ( WorldUnits[ wlCheckPos.X >> DIVIDER ][ wlCheckPos.Y >> DIVIDER ] == NULL ) )
				{
					// Found pos!
					wlFoundPos = wlCheckPos;
					boFound = TRUE;
				}
			}			
			wlCheckPos.Y++;
		}
		
		nRay++;
	}

	Unlock();

	TRACE( "\r\nFound position %u, %u, %u", wlFoundPos.X, wlFoundPos.Y, wlFoundPos.world );

	return wlFoundPos;
}
/******************************************************************************/
// Returns the hive at a given position
Unit *WorldMap::GetHiveAt(int nX, int nY)
/******************************************************************************/
{
	Unit *lpHive = NULL;
	// If a capuchon exists
	if( WorldUnits[ nX >> DIVIDER][ nY >> DIVIDER] != NULL )
	{
		// Try to find the hive.
		lpHive = WorldUnits[ nX >> DIVIDER ][ nY >> DIVIDER ]->SearchUnitType( nX, nY, U_HIVE );
	}
	return lpHive;
}
/******************************************************************************/
// Verify for any triggered monster hives.
void WorldMap::VerifyPeripheralHives(WorldPos wlPos, DIR::MOVE dirPOV)
/******************************************************************************/
{	
	int nXLine, nYLine;
	int nMinX, nMaxX, nMinY, nMaxY;
	nXLine = nYLine = nMinX = nMaxX = nMinY = nMaxY = 0;

	// Check for any hives for monster popup.
	switch( dirPOV )
	{
		case DIR::north:
			nYLine = wlPos.Y - Y_HIVE_CHECK;
			nMinX  = wlPos.X - X_HIVE_CHECK;
			nMaxX  = wlPos.X + X_HIVE_CHECK;
			break;
		case DIR::northeast:		
			// north
			nYLine = wlPos.Y - Y_HIVE_CHECK;
			nMinX  = wlPos.X - X_HIVE_CHECK;
			nMaxX  = wlPos.X + X_HIVE_CHECK;
			// east
			nXLine = wlPos.X + X_HIVE_CHECK;
			nMinY  = wlPos.Y - Y_HIVE_CHECK;
			nMaxY  = wlPos.Y + Y_HIVE_CHECK;
			break;
		case DIR::east:
			nXLine = wlPos.X + X_HIVE_CHECK;
			nMinY  = wlPos.Y - Y_HIVE_CHECK;
			nMaxY  = wlPos.Y + Y_HIVE_CHECK;
			break;
		case DIR::southeast:
			// south
			nYLine = wlPos.Y + Y_HIVE_CHECK;
			nMinX  = wlPos.X - X_HIVE_CHECK;
			nMaxX  = wlPos.X + X_HIVE_CHECK;
			// east
			nXLine = wlPos.X + X_HIVE_CHECK;
			nMinY  = wlPos.Y - Y_HIVE_CHECK;
			nMaxY  = wlPos.Y + Y_HIVE_CHECK;
			break;
		case DIR::south:
			nYLine = wlPos.Y + Y_HIVE_CHECK;
			nMinX  = wlPos.X - X_HIVE_CHECK;
			nMaxX  = wlPos.X + X_HIVE_CHECK;
			break;
		case DIR::southwest:
			// south
			nYLine = wlPos.Y + Y_HIVE_CHECK;
			nMinX  = wlPos.X - X_HIVE_CHECK;
			nMaxX  = wlPos.X + X_HIVE_CHECK;
			// west
			nXLine = wlPos.X - X_HIVE_CHECK;
			nMinY  = wlPos.Y - Y_HIVE_CHECK;
			nMaxY  = wlPos.Y + Y_HIVE_CHECK;
			break;
		case DIR::west:
			nXLine = wlPos.X - X_HIVE_CHECK;
			nMinY  = wlPos.Y - Y_HIVE_CHECK;
			nMaxY  = wlPos.Y + Y_HIVE_CHECK;
			break;
		case DIR::northwest:
			// north
			nYLine = wlPos.Y - Y_HIVE_CHECK;
			nMinX  = wlPos.X - X_HIVE_CHECK;
			nMaxX  = wlPos.X + X_HIVE_CHECK;
			// west
			nXLine = wlPos.X - X_HIVE_CHECK;
			nMinY  = wlPos.Y - Y_HIVE_CHECK;
			nMaxY  = wlPos.Y + Y_HIVE_CHECK;
			break;
	}
	nMinX = nMinX < 0 ? 0 : nMinX;
	nMinY = nMinY < 0 ? 0 : nMinY;
	nMaxX = nMaxX >= MAXX ? MAXX - 1 : nMaxX;
	nMaxY = nMaxY >= MAXY ? MAXY - 1 : nMaxY;	

    if( nYLine >= 0 && nYLine < MAXY && nXLine >= 0 && nXLine < MAXX )
	{
	    int i;
	    Lock();
	    // If a Y scan line is requested.
	    if( nYLine != 0 )
		{
		    // Scan each X of yline.
		    for( i = nMinX; i <= nMaxX; i++ )
			{ 
			    Hive *lpHive = static_cast< Hive * >( GetHiveAt( i, nYLine ) );
			    // If the hive was found
			    if( lpHive != NULL )
				{
				    // Query if it pops up.
				    Unlock();
				    lpHive->QueryPopup();
				    Lock();
			    }
		    }
	    }
	    // If a X scan line is requests.
	    if( nXLine != 0 )
		{
		    // Scan each Y of xline
		    for( i = nMinY; i <= nMaxY; i++ )
			{
			    Hive *lpHive = static_cast< Hive * >( GetHiveAt( nXLine, i ) );
			    // If hive was found
			    if( lpHive != NULL )
				{
				    // Query if it pops up.
				    Unlock();
				    lpHive->QueryPopup();
				    Lock();
			    }
		    }
	    }
	    Unlock();
    }
}
/******************************************************************************/
// Veryfies all the hives in view of position.
void WorldMap::VerifyInviewHives(WorldPos wlPos)
/******************************************************************************/
{
	int nMinX, nMaxX, nMinY, nMaxY;
	int nX, nY;
	
	nMinX = wlPos.X - X_HIVE_CHECK;
	nMaxX = wlPos.X + X_HIVE_CHECK;
	nMinY = wlPos.Y - Y_HIVE_CHECK;
	nMaxY = wlPos.Y + Y_HIVE_CHECK;

	nMinX = nMinX < 0 ? 0 : nMinX;
	nMinY = nMinY < 0 ? 0 : nMinY;
	nMaxX = nMaxX >= MAXX ? MAXX - 1 : nMaxX;
	nMaxY = nMaxY >= MAXY ? MAXY - 1 : nMaxY;

	// Scan all areas within square.
	Lock();
	for( nX = nMinX; nX < nMaxX; nX++ )
	{
		for( nY = nMinY; nY < nMaxY; nY++ )
		{ 
			Hive *lpHive = static_cast< Hive * >( GetHiveAt( nX, nY ) );
			// If hive was found
			if( lpHive != NULL )
			{
				// Query if it pops up.
				Unlock();
				lpHive->QueryPopup();
				Lock();
			}		
		}
	}
	Unlock();
}
/******************************************************************************/
// Determines if an area type should be a 'collision course' blocking.
BOOL WorldMap::IsCollideArea(BYTE bAreaType) // The area type;
/******************************************************************************/
{
    switch( bAreaType )
	{
		// case __BLOCK_NONE:          return FALSE;
		case __BLOCK_ABSOLUTE:      return TRUE;
		case __BLOCK_CAN_FLY_OVER:  return TRUE;
		case __BLOCK_FORCE_FIELD:   return TRUE;
		// case __BLOCK_DEEP_WATER:    return FALSE;
		// case __BLOCK_SHALLOW_WATER: return FALSE;
		// case __AREA_BUILDING:       return FALSE;
		// case __SAFE_HAVEN:          return FALSE;
		// case __INDOOR_SAFE_HAVEN    return FALSE;
	}

    return FALSE;
}
/******************************************************************************/
// Determines the course starting from Pos1 to Pos2 and stop as soon as it ecounters a blocking.
BOOL WorldMap::GetCollisionPos
(
 WorldPos wlPos1,       // First position
 WorldPos wlPos2,       // 
 WorldPos *lpReturnPos, // 
 Unit **lpCollideUnit,   // 
 bool boTargetPC,        // TRUE if lineofsight targets PCs.
 bool boTargetNPC       // TRUE if lineofsight targets NPCs.
)
/******************************************************************************/
{
    POPUP( wlPos1, __OBJGROUP_POUCH );
    POPUP( wlPos2, __OBJGROUP_POT_GREEN );

    BOOL boCollide = FALSE;

    // By default, return the target position (ex: when no collision).
    *lpReturnPos = wlPos2;
    *lpCollideUnit = NULL;

    // If both positions are valid.
    if( IsValidPosition( wlPos1 ) && IsValidPosition( wlPos2 ) && !(wlPos1 == wlPos2) )
	{
		int xDiff = abs(wlPos2.X - wlPos1.X);
		int yDiff = abs(wlPos2.Y - wlPos1.Y);
		int stepIncrement, startupCoord, steps=1;
		double rectCoeficient;

        WorldPos wlCollidePos;				

		if (xDiff >= yDiff) 
		{
			stepIncrement = wlPos1.X < wlPos2.X ? 1 : -1;
			rectCoeficient = wlPos1.Y < wlPos2.Y ? (double)yDiff/xDiff : (double)-yDiff/xDiff;
			startupCoord = wlPos1.Y;

			while (wlPos1.X != wlPos2.X)
			{								
				wlPos1.X += stepIncrement;
				wlPos1.Y = (startupCoord + rectCoeficient*steps++);

				if ( IsCollideArea( QueryAreaType( wlPos1 ) ) ) 
				{
					boCollide = true;
                    Unit *lpBlockingUnit = NULL;
                    Lock();
                    // then scan the internal capuchon to check if it hit a unit.
                    if( WorldUnits[ wlPos1.X >> DIVIDER ][ wlPos1.Y >> DIVIDER ] )
					{
						lpBlockingUnit = WorldUnits[ wlPos1.X >> DIVIDER ][ wlPos1.Y >> DIVIDER ]->ViewTopUnit( wlPos1.X, wlPos1.Y );
                    }
                    Unlock();                   
                    // If the unit is a character and we can't target characters.
                    if( lpBlockingUnit != NULL && !boTargetPC && lpBlockingUnit->GetType() == U_PC )
					{
						// Don't stop here.
						boCollide = false;                        
					}
					else 
					{
						// If the unit is an npc and we can't target creatures.
						if( lpBlockingUnit != NULL && !boTargetNPC && lpBlockingUnit->GetType() == U_NPC )
						{
							// Don't stop here.
							boCollide = false;
						}
						else
						{
							// set return values
							*lpCollideUnit = lpBlockingUnit;
							wlCollidePos = wlPos1;
							POPUP( wlPos1, __OBJGROUP_BEER_MUG_TOO_FULL );

							// set condition to leave the while loop
							wlPos1.X = wlPos2.X;
						}
					}
				}
				else 
				{
					POPUP( wlPos1, __OBJGROUP_SCROLL_BLUE ); 
				}
			}
		}
		else 
		{
			stepIncrement = wlPos1.Y < wlPos2.Y ? 1 : -1;
			rectCoeficient = wlPos1.X < wlPos2.X ? (double)xDiff/yDiff : (double)-xDiff/yDiff;
			startupCoord = wlPos1.X;

			while (wlPos1.Y != wlPos2.Y)
			{
				wlPos1.Y += stepIncrement;
				wlPos1.X = (startupCoord + rectCoeficient*steps++);

				if ( IsCollideArea( QueryAreaType( wlPos1 ) ) ) 
				{
					boCollide = true;
                    Unit *lpBlockingUnit = NULL;
                    Lock();
                    // then scan the internal capuchon to check if it hit a unit.
                    if( WorldUnits[ wlPos1.X >> DIVIDER ][ wlPos1.Y >> DIVIDER ] )
					{
						lpBlockingUnit = WorldUnits[ wlPos1.X >> DIVIDER ][ wlPos1.Y >> DIVIDER ]->ViewTopUnit( wlPos1.X, wlPos1.Y );
                    }
                    Unlock();                   
                    // If the unit is a character and we can't target characters.
                    if( lpBlockingUnit != NULL && !boTargetPC && lpBlockingUnit->GetType() == U_PC )
					{
						// Don't stop here.
						boCollide = false;                        
					}
					else 
					{
						// If the unit is an npc and we can't target creatures.
						if( lpBlockingUnit != NULL && !boTargetNPC && lpBlockingUnit->GetType() == U_NPC )
						{
							// Don't stop here.
							boCollide = false;
						}
						else
						{
							// set return values
							*lpCollideUnit = lpBlockingUnit;
							wlCollidePos = wlPos1;						

							POPUP( wlPos1, __OBJGROUP_BEER_MUG_TOO_FULL );

							// set condition to leave the while loop
							wlPos1.Y = wlPos2.Y;
						}
					}
				}
				else 
				{
					POPUP( wlPos1, __OBJGROUP_SCROLL_BLUE ); 
				}
			}
		}

        // If in the end, we collided.
        if( boCollide )
		{
			// Fill the collide pos.
            *lpReturnPos = wlCollidePos;
        }
    }
    return boCollide;
}