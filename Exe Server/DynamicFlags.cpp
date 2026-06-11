/******************************************************************************
Modify for vs2008 (30/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "DynamicFlags.h"
#include "TFC Server.h"
#include "QuestFlagsListing.h"
#include "RegKeyHandler.h"

extern CTFCServerApp theApp;

/******************************************************************************/
DynamicFlags::DynamicFlags()
/******************************************************************************/
{
}
/******************************************************************************/
DynamicFlags::~DynamicFlags()
/******************************************************************************/
{
}


/******************************************************************************/
//  Returns the value of a given flag ID.
long DynamicFlags::ViewFlag(unsigned long dwFlagID,long lDefaultValue)
{
   // Insures exception-safe auto function locking.
   CAutoLock cAutoLock( &csThreadLock );

   FlagMap::iterator i;

   i = mFlagMap.find( dwFlagID );
   // If the key wasn't found.
   if( i == mFlagMap.end() )
   {
      // Return default value.
      return lDefaultValue;
   }       

   // Otherwise return the key's value.
   return (*i).second;
}


/******************************************************************************/
//  Sets a flag value.
void DynamicFlags::SetFlag( unsigned long dwFlagID,long lFlagValue)
{
   /*
   if( dwFlagID > 10000000 )
   {
      DWORD debugStack[ 10 ];
      int stackLevel = WalkStack( debugStack, 10 );
      if( stackLevel >= 10 )
      {
         stackLevel = 5;
      }

      std::string stackTrace;
      char buf[ 256 ];
      int i;
      for( i = 0; i < stackLevel; i++ )
      {
         _snprintf_s( buf, 256, "0x%x ", debugStack[ i ] );
         stackTrace += buf;
      }
   }
   */

    // Insures exception-safe auto function locking.
    CAutoLock cAutoLock( &csThreadLock );
    
    // If the flag's value is null.
    if( lFlagValue == 0 )
    {
       // Remove the flag.
       RemoveFlag( dwFlagID );
    }
    else
    {
       // Set the flag's value.
       mFlagMap[ dwFlagID ] = lFlagValue;
    }
}
/******************************************************************************/
//  Removes a flag
void DynamicFlags::RemoveFlag(unsigned long dwFlagID)
{
   // Insures exception-safe auto function locking.
   CAutoLock cAutoLock( &csThreadLock );

   FlagMap::iterator i;

   i = mFlagMap.find( dwFlagID );
   // If the key wasn't found.
   if( i == mFlagMap.end() )
   {
      // Return.
      return;
   }       

   // Otherwise remove the flag from the map.
   mFlagMap.erase( i );
}

/******************************************************************************/
//  This function will copy all current flags into a flag holder vector.
void DynamicFlags::GetFlags(FlagCont &vFlags)
{
   // Insures exception-safe auto function locking.
   CAutoLock cAutoLock( &csThreadLock );

   // Copy the flags.
   std::copy( mFlagMap.begin(), mFlagMap.end(), std::back_inserter( vFlags ) );	
}

/******************************************************************************/
//  Destroy all flags.
void DynamicFlags::DestroyFlags( void )
{
    // Insures exception-safe auto function locking.
    CAutoLock cAutoLock( &csThreadLock );
    mFlagMap.clear();
}