#pragma once

#include "Objects.h"
#include "SharedStructures.h"
#include "Directions.h"
#include "Unit.h"
#include "Hive.h"

/******************************************************************************/
class __declspec(dllexport) Minions : public Unit
   /******************************************************************************/
{
public:
   Minions(Unit *pParent);
   virtual ~Minions();

   // Position handle functions
   WorldPos MoveCreature(DIR::MOVE where);

   WorldPos MoveUnit(DIR::MOVE where, BOOL boMoveAbsolute, bool boCompress, bool boBroadcastMove );

   // Combat Structures
   int hit(LPATTACK_STRUCTURE strike, Unit *WhoHit);
   void Death( LPATTACK_STRUCTURE lpBlow, Unit *WhoHit );
   int attacked(LPATTACK_STRUCTURE strike, Unit *Mechant){ return 0; };

   DWORD GetTrueMaxHP();
   void  SetMaxHP(DWORD newMax);
   DWORD GetHP();
   void  SetHP(DWORD newMax, bool boUpdate );
   CString GetName( WORD wLang );
   void SetName(CString newname);
   CString GetRealName();
   void SetLastMoveTime(UINT newTime);
   UINT GetLastMoveTime();

   BOOL CanMove( void );
   void SetCanMove( BOOL boCanMove );



   void VaporizeUnit( bool bLog = true );

private:
   DWORD HP;
   DWORD MaxHP;
   CString m_strName;

   Unit *m_pParent;
   UINT LastMoveTime;
   



   BOOL boCanMove;
};

