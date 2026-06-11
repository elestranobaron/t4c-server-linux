/******************************************************************************
Modify for vs2008 (26/04/2009)
Add SPAWN Items  by Nightmare (28/06/2009)
/******************************************************************************/
#include "stdafx.h"
#include "ItemActionEffect.h"
#include "../WorldMap.h"
#include "../TFC_MAIN.h"

/******************************************************************************/
REGISTER_SPELL_EFFECT( ITEMACTION, ItemActionEffect::NewFunc, ITEMACTION_EFFECT, __noop );

/******************************************************************************/
ItemActionEffect::ItemActionEffect()
/******************************************************************************/
{
	bError = NOT_LOADED;
	dwItemID = 0;
	bResType = 0;
	wlDestinationPos.X = 0;
	wlDestinationPos.Y = 0;
	wlDestinationPos.world = 0;
	bmUserDefinedPosition = 0;
}
/******************************************************************************/
ItemActionEffect::~ItemActionEffect()
/******************************************************************************/
{
}
/******************************************************************************/
BOOL ItemActionEffect::InputParameter(
 CString csParam,   // Parameter value.
 WORD wParamID      // Parameter ID.
)
/******************************************************************************/
{
    const int ResType = 1;
	const int ItemID = 2;
	const int ConjurePosX = 3;
	const int ConjurePosY = 4;
	const int ConjurePosW = 5;

	BOOL boReturn = TRUE;

	switch(wParamID)
	{
		case ResType:
			if(csParam.CompareNoCase("take") == 0)
			{
				bResType = TAKE;
			}
			else if (csParam.CompareNoCase("give")==0)
			{
				bResType = GIVE;
			}
			else if (csParam.CompareNoCase("spawn")==0)
			{
				bResType = SPAWN;
			}
			else
			{
				boReturn = FALSE;
			}
			break;
		case ItemID:
			csItemName = csParam;
			break;
		case ConjurePosX:
			if (bfPosX.SetFormula(csParam) == FALSE) 
			{
				boReturn = FALSE;
			} 
			else 
			{
				bmUserDefinedPosition |= 0x01;
			}
			break;
		case ConjurePosY:
			if (bfPosY.SetFormula(csParam) == FALSE) 
			{
				boReturn = FALSE;
			} 
			else 
			{
				bmUserDefinedPosition |= 0x02;
			}
			break;
		case ConjurePosW:
			if (bfPosW.SetFormula(csParam) == FALSE) 
			{
				boReturn = FALSE;
			} 
			else 
			{
				bmUserDefinedPosition |= 0x04;
			}
			break;
		default:
			boReturn = FALSE;
    }
    return boReturn;
}
/******************************************************************************/
// Log errors
void ItemActionEffect::HandleError(DWORD dwDaItemID, BYTE bDaResType, CString csDaItemName, Unit *self, Unit *target, BYTE bmUserDefinedPosition)
/******************************************************************************/
{
	_LOG_DEBUG
		LOG_DEBUG_HIGH,
		"\r\nError while casting ItemActionEffect. \
		 \r\nItemID: %d, ActionType: %d, ItemBuffer: %s \
		 \r\nCurrent target: %08X self: %08X \
		 \r\nDestination Parameteters: %s",
		 dwDaItemID,
		 bDaResType,
		 csDaItemName,
		 target,
		 self,
		 bmUserDefinedPosition != 0x00 && bmUserDefinedPosition != 0x07 ? "Incomplete" : "(X,Y,W) Ok"
	LOG_
}
/******************************************************************************/
// Take or give an item
void ItemActionEffect::CallEffect(SPELL_EFFECT_PROTOTYPE)
/******************************************************************************/
{
   TRACE("***ItemActionEffect\n");
	if(bError == NOT_LOADED)
	{
		dwItemID = Unit::GetIDFromName( csItemName, U_OBJECT, TRUE );
		bError = LOADED;
		if(dwItemID == 0 || (bmUserDefinedPosition != 0x00 && bmUserDefinedPosition != 0x07)){ //bmUserDefinedPosition must be 0 (no specific position formula defined) or 7 (X, Y and W formulas defined)
			bError = FAILED;
		}
	}

	if(bError == FAILED)
	{
		HandleError(dwItemID, bResType, csItemName, self, target, bmUserDefinedPosition);
	}

	if(bError == LOADED)
	{
		switch(bResType)
		{
			case TAKE:
				{
					if( target != NULL ) 
					{
						try 
						{
							TakeItemFunc(dwItemID, target);
						}
						catch (...) 
						{
							HandleError(dwItemID, bResType, csItemName, self, target, bmUserDefinedPosition);
						}
					}
					else 
					{
						HandleError(dwItemID, bResType, csItemName, self, target, bmUserDefinedPosition);
					}
				}
				break;
			case GIVE:
				{
					if( (target != NULL) && (self != NULL) ) 
					{
						try 
						{
							__GiveItem( self, target, dwItemID, false, true, true ,true);
						}
						catch (...) 
						{
							HandleError(dwItemID, bResType, csItemName, self, target, bmUserDefinedPosition);
						}
					}
					else
					{
						HandleError(dwItemID, bResType, csItemName, self, target, bmUserDefinedPosition);
					}
				}
				break;
         case SPAWN:
            {
               WorldPos wlDestination;
               if (bmUserDefinedPosition == 0x07)  // user specified formulas for X, Y and W.
               {
                  wlDestination.X = bfPosX.GetBoost(self, target);
                  wlDestination.Y = bfPosY.GetBoost(self, target);
                  wlDestination.world = bfPosW.GetBoost(self, target);
               } 
               else 
               { // no user defined formulas, lets use the spell casting position.
                  wlDestination = wlPos;
               }
               try 
               {
                  Objects *lpItem = new Objects;
                  if( lpItem->Create( U_OBJECT, dwItemID ) )
                  {
                     _item *item = NULL;
                     // Get the item structure.
                     lpItem->SendUnitMessage( MSG_OnGetUnitStructure, NULL, NULL, NULL, NULL, &item );
                  
                     WorldMap *world = TFCMAIN::GetWorld( wlDestination.world );
                     if( world != NULL && world->IsValidPosition( wlDestination ) )
                     {
                        world->deposit_unit( wlDestination, lpItem );
                        lpItem->BroadcastPopup( wlDestination, true );
                     } 
                     else 
                     {
                        lpItem->DeleteUnit();
                        HandleError(dwItemID, bResType, csItemName, self, target, bmUserDefinedPosition);
                     }
                  } 
                  else 
                  {
                     lpItem->DeleteUnit();
                     HandleError(dwItemID, bResType, csItemName, self, target, bmUserDefinedPosition);
                  }
               } 
               catch (...) 
               {
                  HandleError(dwItemID, bResType, csItemName, self, target, bmUserDefinedPosition);
               }
            }
            break;
		   default:
		   break;
		}
	}
}
/******************************************************************************/
// Create a SpellEffect object.
SpellEffect *ItemActionEffect::NewFunc(LPSPELL_STRUCT lpSpell) // The spell structure for any spell effect registration.
/******************************************************************************/
{
  CREATE_EFFECT_HANDLE( ItemActionEffect, 0 )
}