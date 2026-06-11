/******************************************************************************
Modify for vs2008 (26/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "Doors.h"
#include "../SkillListing.h"
#include "../IntlText.h"

/******************************************************************************/
Doors::Doors()
/******************************************************************************/
{}
/******************************************************************************/
Doors::~Doors()
/******************************************************************************/
{}
/******************************************************************************/
// Initialisation
void Doors::OnInitialise(UNIT_FUNC_PROTOTYPE) // prototype
/******************************************************************************/
{
	ObjectStructure::OnInitialise( UNIT_FUNC_PARAM ); // first call basic initialisation

	// then add 'door specific' initialisation	
	self->SetFlag(__FLAG_BLOCKING,  3 );			// Door is blocking by default
	
	// Set the door's blocking.
	self->SetBlock( __BLOCK_CAN_FLY_OVER );

	if(lock.key)
	{
		self->SetFlag(__FLAG_DOOR_LOCKED, 1);		// locked by default		
		//SetStaticFlag(__FLAG_DOOR_KEY,  lock.key);	// Key to open the door
	}
}		
/******************************************************************************/
// Uses a door, opens it
void Doors::OnUse(UNIT_FUNC_PROTOTYPE) // prototype
/******************************************************************************/
{	
	ObjectStructure::OnUse( UNIT_FUNC_PARAM );

	Lock();

	DWORD &itemUsed = *(DWORD *)( valueOUT );
	itemUsed = TRUE;

    BOOL boLockPick = FALSE;
    
    // ValueIN contains a pointer to a BOOL stating if door should be picked-locked.
    if( valueIN != NULL )
	{
        boLockPick = *(LPBOOL)( valueIN );
    }

	BOOL boOpen = TRUE;
	if(self->ViewFlag(__FLAG_OPENED_DOOR) != 0) // if opened
	{ 						
		
	}
	else
	{
		boOpen = TRUE;						
		if( lock.key != 0 )
		{
			// Locked, cannot open by default
			boOpen = FALSE;

			// Check if player has the key. medium being the door user.							
			TemplateList<Unit> *lptlBackpack = medium->GetBackpack();                    

			if( lptlBackpack != NULL )
			{
                // If user picks lock
                if( boLockPick )
				{
                    boOpen = TRUE;
                    CString csText( _STR( 3, medium->GetLang() ) );

					TFCPacket sending;
					sending << (RQ_SIZE)RQ_ServerMessage;
					sending << (short)30;
					sending << (short)3;
					sending << csText;
					sending << (long)CL_BLUE_LIGHT;
					medium->Lock();
					medium->SendPlayerMessage( sending );
					medium->Unlock();
                }
                // Otherwise find the key.
                else
				{
					// Seach for item in backpack
					lptlBackpack->Lock();

					lptlBackpack->ToHead();
					while( lptlBackpack->QueryNext() && !boOpen )
					{
                        Objects *lpItem = static_cast< Objects * >( lptlBackpack->Object() );
						// If item was found in the backpack
						if( lpItem->GetStaticReference() == lock.key )
						{
							                            
                            // Allow door to open.
							boOpen = TRUE;
							
							// Send message stating we used the item.
							CString csText;										
							csText.Format( _STR( 2, medium->GetLang() ), lptlBackpack->Object()->GetName( medium->GetLang() ) );

							TFCPacket sending;
							sending << (RQ_SIZE)RQ_ServerMessage;
							sending << (short)30;
							sending << (short)3;
							sending << csText;
							sending << (long)CL_BLUE_LIGHT;
							medium->Lock();
							medium->SendPlayerMessage( sending );
							medium->Unlock();

                            int nCharges = lpItem->ViewFlag( __FLAG_CHARGES );
                            if( nCharges != 0 && ( nCharges - 1 ) != 0 )
							{
                                nCharges--;
                                if( nCharges > 0 )
								{
                                    lpItem->SetFlag( __FLAG_CHARGES, nCharges );
                                }
                            }
							else
							{
                                CString str;
                                str.Format( 
                                    _STR( 7205, medium->GetLang() ),
                                    lpItem->GetName( medium->GetLang() )
                                );
                                medium->SendSystemMessage( str );

                                lpItem->Remove();
                                if( lpItem->GetQty() == 0 )
								{
                                    // Destroy the item.
                                    lptlBackpack->Remove();
                                    lpItem->DeleteUnit();
                                }
                            }
						}
					}
					lptlBackpack->Unlock();
                }
			}
			// If door still isn't opened.
			if( !boOpen )
			{
				CString csText = _STR( 4, medium->GetLang() );

				// Send message stating door is locked.
				TFCPacket sending;
				sending << (RQ_SIZE)RQ_ServerMessage;
				sending << (short)30;
				sending << (short)3;
				sending << csText;
				sending << (long)CL_BLUE_LIGHT;
				medium->Lock();
				medium->SendPlayerMessage( sending );
				medium->Unlock();
			}
		}
		// Open door if it could be opened.
		if( boOpen )
		{
			WorldMap *wl = TFCMAIN::GetWorld(self->GetWL().world);
			if(wl)
			{
				wl->RemoveBlockingUnit(self->GetWL(), self);
			}

			self->SetFlag(__FLAG_OPENED_DOOR, 1);
			self->SetAppearance(self->GetAppearance() + 1);

            Broadcast::BCObjectChanged( self->GetWL(), _DEFAULT_RANGE_CHANGE,
                self->GetAppearance(),
                self->GetID(),0
            );

            TFCTimerManager::AddTimer( 100, 10 SECONDS, AutoClose, self, TRUE );
			   TFCTimerManager::AddTimer( 101, 60 SECONDS, FinalClose, self, TRUE );
		}
	}

    Unlock();
}
/******************************************************************************/
//  Timer callback for auto-closing doors.
void Doors::AutoClose(TIMERCALLBACK_PROTOTYPE) // Callback prototype
/******************************************************************************/
{
    Unit *lpDoor = (Unit *)lpData;

    lpDoor->Lock();

    BOOL boOpen = TRUE;
    if( lpDoor->ViewFlag(__FLAG_OPENED_DOOR ) != 0 ) // if opened
    { 						
       WorldMap *wl = TFCMAIN::GetWorld( lpDoor->GetWL().world );
       if(wl)
       {
          //verifie si la zone ets libre sinon on tp le gars


          if( wl->SetBlockingUnit( lpDoor->GetWL(), lpDoor ) )
          {
             lpDoor->RemoveFlag(__FLAG_OPENED_DOOR); // DOOR is not opened, (close)
             lpDoor->SetAppearance( lpDoor->GetAppearance() - 1 );
             Broadcast::BCObjectChanged( lpDoor->GetWL(), _DEFAULT_RANGE_CHANGE,lpDoor->GetAppearance(),lpDoor->GetID(),0);
          }
          else
          {
             TFCTimerManager::AddTimer( 100, 10 SECONDS, AutoClose, lpDoor, TRUE );
          }
       }					

    }
    lpDoor->Unlock();
}
/******************************************************************************/
//  Timer callback for auto-closing doors.
void Doors::FinalClose(TIMERCALLBACK_PROTOTYPE) // Callback prototype
/******************************************************************************/
{
   Unit *lpDoor = (Unit *)lpData;

   lpDoor->Lock();

   BOOL boOpen = TRUE;
   if( lpDoor->ViewFlag(__FLAG_OPENED_DOOR ) != 0 ) // if opened
   { 						
      WorldMap *wl = TFCMAIN::GetWorld( lpDoor->GetWL().world );
      if(wl)
      {
         wl->SetBlockingUnit( lpDoor->GetWL(), lpDoor );
         lpDoor->RemoveFlag(__FLAG_OPENED_DOOR); // DOOR is not opened, (closed )

         lpDoor->SetAppearance( lpDoor->GetAppearance() - 1 );

         Broadcast::BCObjectChanged( lpDoor->GetWL(), _DEFAULT_RANGE_CHANGE,lpDoor->GetAppearance(),lpDoor->GetID(),0);
      }					

   }

   lpDoor->Unlock();
}
/******************************************************************************/
ObjectStructure *Doors::CreateObject( void )
/******************************************************************************/
{
	return new Doors;
}