// TFC_MAIN.cpp: implementation of the TFC_MAIN class.
// Add profession, GuildMaster,AHMaster by Nightmare (29/06/2009)
// Add Send Damage fct
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "TFC Server.h"
#include "TFC_MAIN.h"
#include "TFC ServerDlg.h"
#include "TFCPacket.h"
#include "SharedStructures.h"
#include "Broadcast.h"
#include "Directions.h"
#include "Creatures.h"
#include "Skills.h"
#include "Professions.h"
#include "QuestBook.h"
#include "GuildMaster.h"
#include "AuctionMaster.h"
#include "ODBCMage.h"
#include "GameDefs.h"
#include "RegKeyHandler.h"
#include "AutoConfig.h"
#include "format.h"
#include "ThreadMonitor.h"

#ifdef _DEBUG
	#undef THIS_FILE
	static char THIS_FILE[]=__FILE__;
	#define new DEBUG_NEW
#endif

/******************************************************************************/
void ObjectListing( void );
void SkillRegistration( void );
void RegisterObjects( void );

/******************************************************************************/
static HANDLE hHeartHandle = NULL;
static DWORD  round = 0;
extern TemplateList< Unit> AddCreatureList;
extern CTFCServerApp theApp;
TemplateList <TFCMAIN::_FUNC> TFCMAIN::tlFuncList;
//TemplateList <Players> UsersList; // Auxiliary users list
TemplateList <Unit> UnitsToCreate;
TFC_MAIN* TFCServer;
Random rnd;

/******************************************************************************/
WorldMap *TFCMAIN::GetWorld(unsigned short which)
/******************************************************************************/
{
	if(which < TFCServer->world_number) 
	{
		return &TFCServer->World[which];
	}
	else
	{
		return NULL;
	}
}
/******************************************************************************/
CString TFCMAIN::GetHomeDir()
/******************************************************************************/
{
	return theApp.sPaths.csBinaryPath;
}
/******************************************************************************/
DWORD TFCMAIN::GetColosseum()
/******************************************************************************/
{
	return theApp.sColosseum;
}

DWORD TFCMAIN::GetColosseum2()
{
   return theApp.dwGFEnableCoco2;
}

DWORD TFCMAIN::GetColosseum3()
{
   return theApp.dwGFEnableCoco3;
}

DWORD TFCMAIN::GetColosseum4()
{
   return theApp.dwGFEnableCoco4;
}

/******************************************************************************/
DWORD TFCMAIN::GetDoppelganger()
/******************************************************************************/
{
	return theApp.sDoppelganger;
}
/******************************************************************************/
DWORD TFCMAIN::GetMaxRemorts()
/******************************************************************************/
{
	return theApp.sMaxRemorts;
}

unsigned long TFCMAIN::GetPLFactionMask(unsigned long ulVal)
{
   switch(ulVal)
   {
      case  0: return PL_FACTION_NONE;
      case  1: return PL_FACTION_01;
      case  2: return PL_FACTION_02;
      case  3: return PL_FACTION_03;
      case  4: return PL_FACTION_04;
      case  5: return PL_FACTION_05;
      case  6: return PL_FACTION_06;
      case  7: return PL_FACTION_07;
      case  8: return PL_FACTION_08;
      case  9: return PL_FACTION_09;
      case 10: return PL_FACTION_10;
      case 11: return PL_FACTION_11;
      case 12: return PL_FACTION_12;
      case 13: return PL_FACTION_13;
      case 14: return PL_FACTION_14;
      case 15: return PL_FACTION_15;
      case 16: return PL_FACTION_16;
      case 17: return PL_FACTION_17;
      case 18: return PL_FACTION_18;
      case 19: return PL_FACTION_19;
      case 20: return PL_FACTION_20;
      case 21: return PL_FACTION_21;
      case 22: return PL_FACTION_22;
      case 23: return PL_FACTION_23;
      case 24: return PL_FACTION_24;
      case 25: return PL_FACTION_25;
      case 26: return PL_FACTION_26;
      case 27: return PL_FACTION_27;
      case 28: return PL_FACTION_28;
      case 29: return PL_FACTION_29;
      case 30: return PL_FACTION_30;
      case 31: return PL_FACTION_31;
   }
   return PL_FACTION_NONE;
}

void TFCMAIN::AddCastSpellPos(Unit *pSelf,WorldPos wlPos,DWORD dwSpellID,DWORD dwFreq,DWORD dwRepeat)
{
   CAutoLock autoCastSpellLock( &theApp.g_aLockCastSpell );

   sSpellCastPosition *pNewCSP = new sSpellCastPosition;

   pNewCSP->pCaster        = pSelf;
   pNewCSP->wl             = wlPos;
   pNewCSP->dwSpellID      = dwSpellID;
   pNewCSP->dwFreq         = dwFreq;
   pNewCSP->dwFreqCnt      = 0;
   pNewCSP->dwNbrRepeatCnt = dwRepeat;

   int iPos = theApp.c_aCastSpellPos.GetSize();
   if(iPos < 0)
      iPos = 0;
   theApp.c_aCastSpellPos.SetAtGrow(iPos, pNewCSP );
}

void TFCMAIN::ProcessCastSpellPos()
{
   CAutoLock autoCastSpellLock( &theApp.g_aLockCastSpell );

   sSpellCastPosition *pCSP = new sSpellCastPosition;

   for(int a=0;a<theApp.c_aCastSpellPos.GetSize();a++)
   {
      pCSP = (sSpellCastPosition *)theApp.c_aCastSpellPos.GetAt(a);

      pCSP->dwFreqCnt++;
      //look si le temps ets venu de caster
      BOOL bSpellOK = TRUE;
      if(pCSP->dwFreqCnt > pCSP->dwFreq)
      {
         //on dois caster on reset les compteur et decremente le total cast counter
         pCSP->dwFreqCnt = 0;
         pCSP->dwNbrRepeatCnt--;

         //cast le spell
         
         if( pCSP->pCaster != NULL)
         {
            Character *ch = static_cast< Character * >( pCSP->pCaster );
            if(ch)
            {
               if(ch->GetPlayer() && ch->GetPlayer()->in_game)
                  ch->CastSpellPosSystem(pCSP->dwSpellID,pCSP->wl);
               else 
                  bSpellOK = FALSE;
            }
            else 
               bSpellOK = FALSE;
         }
         else 
            bSpellOK = FALSE;
      }

      if(pCSP->dwNbrRepeatCnt <= 0 || !bSpellOK)
      {
         //complete this autocast we delete it
         delete pCSP;
         pCSP = NULL;
         theApp.c_aCastSpellPos.RemoveAt(a);
         a--;
      }
   }
}

long TFCMAIN::GlobalFlagView(unsigned long ulFlagID)
{
   return theApp.dfGlobalFlags.ViewFlag(ulFlagID);
}

void TFCMAIN::GlobalFlagSet (unsigned long ulFlagID, long lValue, BOOL bSave)
{
   theApp.dfGlobalFlags.SetFlag(ulFlagID,lValue);
   if(!bSave)
      return;
   for(int i=0;i<theApp.m_aGlobalFlag.GetSize();i++)
   {
      if(theApp.m_aGlobalFlag[i].uiID == ulFlagID)
      {
         RegKeyHandler regKey;
         if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GLOBAL_FLAG_SAVE_KEY ) )
         {
            CString strTmp;
            strTmp.Format("GlobalFlagID%05d",ulFlagID);
            regKey.WriteProfileInt( strTmp.GetBuffer(0), lValue);
            return;
         }
      }
   }
   //Flag ed friendlg on save systematiquement
   if(ulFlagID > 3000000 && ulFlagID < 3001000)
   {
      RegKeyHandler regKey;
      if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GLOBAL_FLAG_SAVE_KEY ) )
      {
         CString strTmp;
         strTmp.Format("GlobalFlagID%07d",ulFlagID);
         regKey.WriteProfileInt( strTmp.GetBuffer(0), lValue);
         return;
      }
   }
}

void TFCMAIN::BroadcastFriendlyChange (WorldPos wlPos, DWORD dwMonsterID)
{
   WorldMap *wl = TFCMAIN::GetWorld( wlPos.world );

   if( wl == NULL )
      return;

   TemplateList< Unit > *unitList = wl->GetLocalUnits( wlPos, 70, FALSE );
   if( unitList == NULL )
      return;
  
   unitList->ToHead();
   while( unitList->QueryNext() )
   {
      Unit *un = unitList->Object();

      // If the unit does not have detect invisible.
      if( un->GetFriendlyID() == dwMonsterID )
      {
         WorldPos ObjectWL = un->GetWL();		
         
         TFCPacket sending;
         sending << (RQ_SIZE)__EVENT_OBJECT_MOVED;
         sending << (short)(ObjectWL.X);
         sending << (short)(ObjectWL.Y);
         un->PacketUnitInformation( sending );		    		
         Broadcast::BCast( wlPos, _DEFAULT_RANGE, sending, un->GetInvisibleQuery() );	
      }
   }
   delete unitList;
}

void TFCMAIN::GlobalFlagDel (unsigned long ulFlagID)
{
   theApp.dfGlobalFlags.RemoveFlag(ulFlagID);
}



/******************************************************************************/
DWORD TFCMAIN::GetMaxCharactersPerAccount()
/******************************************************************************/
{
	return theApp.sMaxCharactersPerAccount;
}
/******************************************************************************/
// Adds a monster for auto-life
void TFCMAIN::AddMonster(Unit *theCreature)
/******************************************************************************/
{
   if(theCreature)
   {
	   AddCreatureList.Lock();
	   AddCreatureList.AddToHead(theCreature);
	   AddCreatureList.Unlock();

	   theCreature->BroadcastPopup( theCreature->GetWL() );

	   theCreature->SendUnitMessage( MSG_OnPopup, theCreature, NULL, NULL, NULL, NULL );
   }
}


/******************************************************************************/
WORD TFCMAIN::GetMaxWorlds()
/******************************************************************************/
{
	return TFCServer->world_number;
}
/******************************************************************************/
void TFCMAIN::RegisterUnitStartupFunction( UNITSTARTUPFUNC lpFunc )
/******************************************************************************/
{
	_FUNC *sFunc = new _FUNC;
	sFunc->lpFunc = lpFunc;
	tlFuncList.AddToTail( sFunc );
}
/******************************************************************************/
void TFCMAIN::CallUnitStartupFunctions( void )
/******************************************************************************/
{ 
	_FUNC *sFunc;
	tlFuncList.ToHead();
	while( tlFuncList.QueryNext() )
	{
		sFunc = tlFuncList.Object();
		sFunc->lpFunc();
		tlFuncList.DeleteAbsolute();
	}
}
/******************************************************************************/
// This thread ticks the rounds each 50ms
UINT CALLBACK HeartBeat(LPVOID nil)
/******************************************************************************/
{
   CAutoThreadMonitor tmMonitor("HeartBeat");
   DWORD initialTime = GetRunTime();
   DWORD dummyRound = 0;
   DWORD currentTime = 0;

   while( 1 )
   {
      // Heartbeats each 50ms
      Sleep( 50 );

      // Do incrementation in unsigned
      //DWORD 
      dummyRound = round;
      dummyRound++;

		// Each 20 rounds (1 second)
      if( ( dummyRound % 20 ) == 0 )
      {
         currentTime = GetRunTime();
         // If GetRunTime() looped
         if( currentTime < initialTime )
         {
            // Reset the initial time.
            initialTime = currentTime;
         }
         else
         {
            // Rectify the current round according to the actual
            // elapsed time. Accounts for the time lost processing
            // the heartbeat.
            dummyRound = ( currentTime - initialTime ) / 50;
         }
      }

      // Set new round to this value
      InterlockedExchange( reinterpret_cast< long * >( &round ), static_cast< long >( dummyRound ) );

      // Increment clock
      TFCTime::IncTime();
   }
}
/******************************************************************************/
//  Starts the heartbeat.
void TFCMAIN::StartBeat( void )
/******************************************************************************/
{
	if( hHeartHandle == NULL )
	{
      UINT threadId;
      hHeartHandle = (HANDLE)_beginthreadex(NULL, 0, HeartBeat, NULL, 0, &threadId ); //OK: thread qui calcule le temps general du serveur...

      // Set above priority to heartbeat increments.
      SetThreadPriority( hHeartHandle, THREAD_PRIORITY_HIGHEST );
   }
}
/******************************************************************************/
// Returns the current round
unsigned long TFCMAIN::GetRound( void )
/******************************************************************************/
{
	return round;
}
/******************************************************************************/
// For attacking
int TFCMAIN::Attack(Unit *Attacker, Unit *Target, bool &blockedPath )
/******************************************************************************/
{
    blockedPath = false;
    MultiLock( Attacker, Target );
    
	WorldPos wlAttPos = Attacker->GetWL();	
	WorldPos wlTarPos = Target->GetWL();

	WorldMap *wlWorld = GetWorld( wlAttPos.world );
	BOOL boAttack = TRUE;

    // If a collision is detected.
    WorldPos blockPos = { 0, 0, 0 };
    Unit *collideUnit = NULL;
    if( wlWorld->GetCollisionPos( wlAttPos, wlTarPos, &blockPos, &collideUnit ) )
    {
       // If the target unit isn't the target and the block pos isn't the target pos.
       if( collideUnit != Target && ( blockPos.X != wlTarPos.X || blockPos.Y != wlTarPos.Y ) )
       {
          if( Attacker->GetType() == U_NPC )
          {
             Attacker->SetTarget( NULL );
             Attacker->Do( wandering ,"Attack");
          }
          blockedPath = true;
          Attacker->Unlock();
          Target->Unlock();
          return 0;
       }
    }

	if( Target != Attacker && Target->GetType() != U_OBJECT && Target->GetType() != U_HIVE && Target->GetType() != U_MINIONS && boAttack )
	{
		if( wlWorld != NULL )
		{
			// If any of them is in a safe-haven. If either the attacker or the attacked is on a safe-haven
			// If any of them is in a safe-haven.
         if( GAME_RULES::InSafeHaven( Attacker, Target ) && (Attacker->GetType() == U_PC && Target->GetType() == U_PC))
         {
				if( Attacker->GetType() == U_PC )
				{
					static_cast< Character * >( Attacker )->StopAutoCombat();
				}

				TFCPacket sending;
				sending << (RQ_SIZE)RQ_ServerMessage;
				sending << (short)30;
				sending << (short)3;
				CString csText = _STR( 22, Attacker->GetLang() );
				sending << csText;
				sending << (long) CL_BLUE_LIGHT;
				Attacker->SendPlayerMessage( sending );
				// Do not attack
				boAttack = FALSE;
			}
		}
		UINT uiRet = GAME_RULES::NMPVPCanAttack( Attacker, Target );
		if( uiRet > 0 )
		{
			if(uiRet == 1)
				Attacker->SendInfoMessage(_STR( 15037 , Attacker->GetLang() ),0x0570D5);
			else if(uiRet == 2)
				Attacker->SendInfoMessage(_STR( 15038 , Attacker->GetLang() ),0x0570D5);
			else if(uiRet == 3)
				Attacker->SendInfoMessage(_STR( 15039 , Attacker->GetLang() ),0x0570D5);
			else if(uiRet == 98)
				Attacker->SendInfoMessage(_STR( 15040 , Attacker->GetLang() ),0x0570D5);
         else if(uiRet == 99)
            Attacker->SendInfoMessage(_STR( 15345 , Attacker->GetLang() ),0x0570D5);
         else if(uiRet == 1000)
            Attacker->SendInfoMessage(_STR( 15511 , Attacker->GetLang() ),0x0570D5);
			else 
				Attacker->SendInfoMessage(_STR( 15041 , Attacker->GetLang() ),0x0570D5);
			boAttack = FALSE;
		}
		
		// Attacker->Lock();
		// Target->Lock();

		// If player/unit isn't stun and in attack range            
		if( !Attacker->ViewFlag( __FLAG_STUN ) && boAttack && abs( wlAttPos.X - wlTarPos.X ) <= _DEFAULT_TOUCH_RANGE &&
			abs( wlAttPos.Y - wlTarPos.Y ) <= _DEFAULT_TOUCH_RANGE)
		{
			int attackID; // which attack was cast
			int dodgeID;  // what was dodge
			int result;	  // result of the attack (death?)
			DWORD DodgeSkill, AttackSkill;

			ATTACK_STRUCTURE Blow;
			memset( &Blow, 0, sizeof( ATTACK_STRUCTURE ) );

			// Get the attack first, since we determine the attack skill at runtime 
			// (monster may have multiple attacks with multiple attack skills)
			Blow.lDodgeSkill = Target->GetDODGE();
			Blow.lDodgeSkill = Blow.lDodgeSkill > 0 ? Blow.lDodgeSkill : 1;

			DodgeSkill = Blow.lDodgeSkill;

			attackID = Attacker->attack(&Blow, Target );
			Blow.TrueStrike = Blow.Strike;    // Set true strike.

			double dblPreACstrike = Blow.Strike;

			// DodgeSkill = Target->ViewFlag(__FLAG_DODGE_SKILL);
			// DodgeSkill = DodgeSkill > 0 ? DodgeSkill : 1;

			AttackSkill	= Attacker->GetATTACK();
			TRACE(_T("AttackSkill %d"), AttackSkill);
			AttackSkill = AttackSkill > 0 ? AttackSkill : 1;
			// If attack skill wasn't set by the 'attack' function
			if(!Blow.lAttackSkill)
			{
				Blow.lAttackSkill = AttackSkill;
			}

			//TRACE(_T("\r\nAttackSkill %u vs DodgeSkill %u (exhaust %u, %u)\r\n"), AttackSkill, DodgeSkill, Blow.exhaust.attack, Blow.exhaust.move);
			TRACE( _T("\r\nAttackerPos( %u, %u ) vs DefenderPos( %u, %u )\r\n"), 
				Attacker->GetWL().X,
				Attacker->GetWL().Y,
				Target->GetWL().X,
				Target->GetWL().Y
			);
			TRACE( _T("\r\nThe attacker is %s and the target %s\r\n"),
				Attacker->GetName( _DEFAULT_LNG ),
				Target->GetName( _DEFAULT_LNG )
			);

			// Attacker->DealExhaust( Blow.exhaust.attack, Blow.exhaust.mental, Blow.exhaust.move );

			// If precision wasn't set by one of the called functions..
			if(!Blow.Precision)
			{
				Blow.Precision = GAME_RULES::GetBlowPrecision(AttackSkill, DodgeSkill, Attacker->GetAGI(), Target->GetAGI() );
			}

			// If target can attack
			if( Target->CanAttack() )
			{
				if( Target->GetType() == U_NPC )
				{
					// Force unit into fight
				    Target->Do( fighting );
				    Target->SetTarget( Attacker );
                }
			}

			if(Blow.Precision > 0 || Target->ViewFlag(__FLAG_STUN)) 
			{
				// If attack is previously hidden.
				if( Attacker->ViewFlag( __FLAG_HIDDEN ) )
				{
					// Hit more !
                    Blow.Strike = Blow.Strike * ( 149 + rnd.roll( dice( 1, 50 ) ) ) / 100;
				}

                // Dispell any invisibility on the attack.
                Attacker->DispellInvisibility();

				// Got hit, so cannot be stun anymore
				if( Target->ViewFlag( __FLAG_STUN ) )
				{
 				   //Moen_OK : Modif du calcul O_o
					// Hit more !
					if(theApp.dwEquilibrageSkillNewFormulaEnable == 0)
               {
						Blow.Strike *= 15; // * 1.5, in integers..
						Blow.Strike /= 10;
               }
               else
               {
                  Blow.Strike *= 15; // * 1.5, in integers..
                  Blow.Strike /= 10;
               }
					// Removes the stun timer possibily associated with it					
					Target->RemoveFlag(__FLAG_STUN);
				}
				
				// If attacker is berserk
				int nBerserk = Attacker->ViewFlag( __FLAG_BERSERK );
				if( nBerserk > 0 )
				{
					// Hit more !
					double dblDam = Blow.Strike * 2;
					Blow.Strike *= ( ( nBerserk / 25 ) + 1 );
					Blow.Strike += dblDam;
				}

				// Attack hit! This time, query without a target to enable "skill" checking
				Attacker->attack_hit(&Blow, Target);

    			// Target is move-frozen for 500 ms.
				Target->DealExhaust( 0, 0, 500 );
				// analyse, I don't care

				double dlbSkillStrike = Blow.Strike;
			
				dodgeID = Target->attacked(&Blow, Attacker);

				TRACE(_T("Attack of %d(defense)!!"), Blow.Strike);

				if(Blow.Strike < 0)
				{
					Blow.Strike = 0;
				}

				double dblPostACstrike = Blow.Strike; // Save POST-AC strike value
				
				// Proccess triggered skills and god flags and get final damage value.
				result = Target->hit(&Blow, Attacker);

             bool bDamageSent = false;    
             if( Attacker->GetType() == U_PC )
             {
                Players *lpPlayer = static_cast< Character *>( Attacker )->GetPlayer();
                if( lpPlayer != NULL )
                {
                   lpPlayer->m_DPSCounter +=(int)Blow.Strike;                  
                   if( lpPlayer->GetGodFlags() & GOD_DEVELOPPER )
                   {
                      TFormat format;
                      Attacker->SendSystemMessage(
                         format(
						 "Attack hits %s for : %.2f true damages, %.2f with skills damages, postAc %.2f, %.2f final damage.",
                         (LPCTSTR)Target->GetName(_DEFAULT_LNG),
                         dblPreACstrike,
						 dlbSkillStrike,
                         dblPostACstrike,
                         Blow.Strike
                         )
                         );
                   }

                   if((int)Blow.Strike >0 && theApp.dwSendDamageHealingSystem == 1)
                   {
                      DWORD color = CL_HEAL_DAMAGE_1;
                      CString strDamage;
                      strDamage.Format("-%d",(int)Blow.Strike);
                      TFCPacket sending;
                      sending << (RQ_SIZE)RQ_DamageUnit;
                      sending << (long)Target->GetID();
                      sending << strDamage;
                      sending << (long)color;
                      lpPlayer->self->SendPlayerMessage( sending );
                   }
                }
             }
            if( Target->GetType() == U_PC)
            {
               Players *lpPlayer = static_cast< Character *>( Target )->GetPlayer();
               if( lpPlayer != NULL )
               {
                  if( lpPlayer->GetGodFlags() & GOD_DEVELOPPER )
                  {
                     TFormat format;
                     Target->SendSystemMessage(
                        format(
                        "You were hit by %s for %.2f damages (%.2f post-AC damages, %.2f final damage).",
                        (LPCTSTR)Attacker->GetName(_DEFAULT_LNG),
                        dblPreACstrike,
                        dblPostACstrike,
                        Blow.Strike
                        )
                        );
                  }

                  if((int)Blow.Strike >0 && theApp.dwSendDamageHealingSystem == 1)
                  {
                     DWORD color = CL_HEAL_DAMAGE_2;
                     CString strDamage;
                     strDamage.Format("-%d",(int)Blow.Strike);
                     TFCPacket sending;
                     sending << (RQ_SIZE)RQ_DamageUnit;
                     sending << (long)Target->GetID();
                     sending << strDamage;
                     sending << (long)color;
                     Target->SendPlayerMessage( sending );
                  }
               }
            }


#ifdef __ENABLE_LOG
				if(__LOG > 60)
				{
					CString logmsg;
					logmsg.Format("Unit ID#%u (%s) attacks unit ID#%u (%s) for %u damage.",
						Attacker->GetID(), 
						Attacker->GetName(),
						Target->GetID(),
						Target->GetName(), 
						Blow.Strike
					);
					__LOG((LPCTSTR)logmsg);
				}
#endif

				if( result != -1 )
				{
					TRACE( "\r\nBroadcasting attack" );
                    TRACE( "\r\nAttackerID=%u. TargetID=%u.", Attacker->GetID(), Target->GetID() );

                    char CombatMode = 0;
                    if(Target->GetType() == U_PC && Attacker->GetType() == U_PC)
                    {
                       Character *ThisCharacter = static_cast<Character *>( Target );
                       if(ThisCharacter)
                          CombatMode = ThisCharacter->GetNMCombatMode();
                    }

                    Broadcast::BCAttack( Attacker->GetWL(), _DEFAULT_RANGE,
                                         Attacker->GetID(),
                                         Target->GetID(),
                                         (Target->GetHP() * 100 / Target->GetMaxHP()),
                                         CombatMode,
                                         Attacker->GetWL(),
                                         Target->GetWL(),
                                         Attacker->GetInvisibleQuery()
                    );

				}
				else
				{ // Kill me, do anything you want
	               char CombatMode = 0;
	               if(Target->GetType() == U_PC && Attacker->GetType() == U_PC)
	               {
	                  Character *ThisCharacter = static_cast<Character *>( Target );
	                  if(ThisCharacter)
	                     CombatMode = ThisCharacter->GetNMCombatMode();
	               }
               
	               Broadcast::BCAttack( Attacker->GetWL(), _DEFAULT_RANGE,
	                                    Attacker->GetID(),
	                                    Target->GetID(),
	                                    (Target->GetHP() * 100 / Target->GetMaxHP()),
	                                    CombatMode,
	                                    Attacker->GetWL(),
	                                    Target->GetWL(),
	                                    Attacker->GetInvisibleQuery()
	                                    );
               
                  bool display = true;
                  if( Target->GetType() == U_PC )
                  {
                     Character *pc = static_cast< Character * >( Target );
                     Players *pl = static_cast< Players * >( pc->GetPlayer() );
                     if( pl != NULL && ( pl->GetGodFlags() & GOD_CANNOT_DIE ) )
                     {
                        display = false;
                     }
                  }

                  if( display )
                  {
                     Broadcast::BCObjectChanged( Target->GetWL(), _DEFAULT_RANGE_CHANGE,
                        Target->GetCorpse(),
                        Target->GetID(),1,
                        Attacker->GetInvisibleQuery()
                        );
                  }
				}
				
//				Attacker->Unlock();
//				Target->Unlock();			
			}
			else
			{
				Broadcast::BCMiss( Attacker->GetWL(), _DEFAULT_RANGE,
                    Attacker->GetID(),
                    Target->GetID(),
                    Attacker->GetWL(),
                    Target->GetWL(),
                    Attacker->GetInvisibleQuery()
                );
			}
						
			TRACE( "\r\nUnlocking units." );
			Attacker->Unlock();
			Target->Unlock();

			return 20; // dunno why actually.. this is 1 second exhaust			
		}
    }
	else
	{
		// Reset target if target happened to attack a hive.
		if( Target->GetType() == U_HIVE  || Target->GetType() == U_MINIONS)
		{
			Attacker->SetTarget( Attacker->GetBond() );
        }
	}

    Attacker->Unlock();
    Target->Unlock();
    
    return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////
// For attacking
int TFCMAIN::AttackBow(Unit *Attacker, Unit *Target, bool &blockedPath ,int iDefRangeAttack, BoostFormula *pBoust, int iArcherySkill)
{
   blockedPath = false;
   MultiLock( Attacker, Target );
   
   WorldPos wlAttPos = Attacker->GetWL();	
   WorldPos wlTarPos = Target->GetWL();
   
   WorldMap *wlWorld = GetWorld( wlAttPos.world );
   BOOL boAttack = TRUE;
   
   
   // If a collision is detected.
   WorldPos blockPos = { 0, 0, 0 };
   Unit *collideUnit = NULL;
   if( wlWorld->GetCollisionPos( wlAttPos, wlTarPos, &blockPos, &collideUnit ) )
   {
      // If the target unit isn't the target and the block pos isn't the target pos.
      if( collideUnit != Target && ( blockPos.X != wlTarPos.X || blockPos.Y != wlTarPos.Y ) )
      {
         if( Attacker->GetType() == U_NPC )
         {
            Attacker->SetTarget( NULL );
            Attacker->Do( wandering ,"AttackBow");
         }
         blockedPath = true;
         Attacker->Unlock();
         Target->Unlock();
         return 0;
      }
   }
   
   
   if( Target != Attacker && Target->GetType() != U_OBJECT && Target->GetType() != U_HIVE && Target->GetType() != U_MINIONS && boAttack )
   {
      
      if( wlWorld != NULL )
      {
         // If any of them is in a safe-haven.
         if( GAME_RULES::InSafeHaven( Attacker, Target ) )
         {
            static_cast< Character * >( Attacker )->StopAutoCombat();
            TFCPacket sending;
            sending << (RQ_SIZE)RQ_ServerMessage;
            sending << (short)30;
            sending << (short)3;
            CString csText = _STR( 22, Attacker->GetLang() );
            sending << csText;
            sending << (long) CL_BLUE_LIGHT;
            Attacker->SendPlayerMessage( sending );
            // Do not attack
            boAttack = FALSE;
         }
      }
      UINT uiRet = GAME_RULES::NMPVPCanAttack( Attacker, Target );
      if( uiRet > 0 )
      {
         if(uiRet == 1)
            Attacker->SendInfoMessage(_STR( 15037 , Attacker->GetLang() ),0x0570D5);
         else if(uiRet == 2)
            Attacker->SendInfoMessage(_STR( 15038 , Attacker->GetLang() ),0x0570D5);
         else if(uiRet == 3)
            Attacker->SendInfoMessage(_STR( 15039 , Attacker->GetLang() ),0x0570D5);
         else if(uiRet == 98)
            Attacker->SendInfoMessage(_STR( 15040 , Attacker->GetLang() ),0x0570D5);
         else if(uiRet == 99)
            Attacker->SendInfoMessage(_STR( 15345 , Attacker->GetLang() ),0x0570D5);
         else if(uiRet == 1000)
            Attacker->SendInfoMessage(_STR( 15511 , Attacker->GetLang() ),0x0570D5);
         else 
            Attacker->SendInfoMessage(_STR( 15041 , Attacker->GetLang() ),0x0570D5);
         boAttack = FALSE;
      }
      
      if( !Attacker->ViewFlag( __FLAG_STUN ) && boAttack   &&abs( wlAttPos.X - wlTarPos.X ) <= iDefRangeAttack &&abs( wlAttPos.Y - wlTarPos.Y ) <= iDefRangeAttack )
      {
         ATTACK_STRUCTURE blow;
         memset( &blow, 0, sizeof( ATTACK_STRUCTURE ) );
         
         blow.lDodgeSkill = Target->GetDODGE();
         blow.lDodgeSkill = blow.lDodgeSkill > 0 ? blow.lDodgeSkill : 1;
         
         // Calculate the range between the two players.
         int nXdiff = abs( Attacker->GetWL().X - Target->GetWL().X );
         int nYdiff = abs( Attacker->GetWL().Y - Target->GetWL().Y );
         int nRange = ::sqrt( double(nXdiff * nXdiff + nYdiff * nYdiff ));
         
         
         // Calculate damage done by the attacker.
         Attacker->attack( &blow, Target );
         
         blow.Strike += pBoust->GetBoost( Attacker, Target, 5, 0, nRange );
         blow.TrueStrike = blow.Strike;
         
         // Get the archery skill.
         DWORD archerySkill = iArcherySkill;
         
         archerySkill = archerySkill > 0 ? archerySkill : 1;
         
         blow.lAttackSkill = archerySkill;
         
         // Set the blow's precision.
         blow.Precision = GAME_RULES::GetBlowPrecision( archerySkill, blow.lDodgeSkill,Attacker->GetAGI(),Target->GetAGI());
         
         
         if(blow.Precision > 0 || Target->ViewFlag(__FLAG_STUN)) 
         {
            Attacker->DealExhaust( 500, 0, 1000 );
            
            double preACstrike = blow.Strike;
            
            if( Target->CanAttack() )
            {
               if( Target->GetType() == U_NPC )
               {
                  // Force unit into fight
                  Target->Do( fighting );
                  Target->SetTarget( Attacker );
               }
            }
            // If attack is previously hidden.
            // Hit more!!! Mouahahahahha
            if( Attacker->ViewFlag( __FLAG_HIDDEN ) )
               blow.Strike = blow.Strike * ( 149 + rnd.roll( dice( 1, 50 ) ) ) / 100;
            
            // Dispell any invisibility on the attack.
            Attacker->DispellInvisibility();
            
           
            // Tell attacker that the attack hit!
            Attacker->attack_hit( &blow, Target );
            
            
            // Tell target that it is being attacked.
            Target->attacked( &blow, Attacker );
            
            if(blow.Strike < 0)
               blow.Strike = 0;
            
            double postACstrike = blow.Strike;// Save POST-AC strike value
            
            // Proccess triggered skills and god flags and get final damage value.
            // Hit the target with the blow!
            int result = Target->hit( &blow, Attacker );
            
            // GameOp stuff
            bool bDamageSent = false;
            if( Attacker->GetType() == U_PC )
            {
               Players *lpPlayer = static_cast< Character *>( Attacker )->GetPlayer();
               if( lpPlayer != NULL )
               {
                  lpPlayer->m_DPSCounter +=(int)blow.Strike;                  
                  if( lpPlayer->GetGodFlags() & GOD_DEVELOPPER )
                  {
                     TFormat format;
                     Attacker->SendSystemMessage(format("Attack hits %s for %.2f damages (%.2f post-AC damages, %.2f final damage).",
                        (LPCTSTR)Target->GetName(_DEFAULT_LNG),
                        preACstrike,
                        postACstrike,
                        blow.Strike
                        )
                        );
                  }

                  if((int)blow.Strike >0 && theApp.dwSendDamageHealingSystem == 1)
                  {
                     DWORD color = CL_HEAL_DAMAGE_1;
                     CString strDamage;
                     strDamage.Format("-%d",(int)blow.Strike);
                     TFCPacket sending;
                     sending << (RQ_SIZE)RQ_DamageUnit;
                     sending << (long)Target->GetID();
                     sending << strDamage;
                     sending << (long)color;
                     lpPlayer->self->SendPlayerMessage( sending );
                  }
               }
            }
            if( Target->GetType() == U_PC)
            {
               Players *lpPlayer = static_cast< Character *>( Target )->GetPlayer();
               if( lpPlayer != NULL )
               {
                  if( lpPlayer->GetGodFlags() & GOD_DEVELOPPER )
                  {
                     TFormat format;
                     Target->SendSystemMessage(format("You were hit by %s for %.2f damages (%.2f post-AC damages, %.2f final damage).",
                        (LPCTSTR)Attacker->GetName(_DEFAULT_LNG),
                        preACstrike,
                        postACstrike,
                        blow.Strike
                        )
                        );
                  }

                  if((int)blow.Strike >0 && theApp.dwSendDamageHealingSystem == 1)
                  {
                     DWORD color = CL_HEAL_DAMAGE_2;
                     CString strDamage;
                     strDamage.Format("-%d",(int)blow.Strike);
                     TFCPacket sending;
                     sending << (RQ_SIZE)RQ_DamageUnit;
                     sending << (long)Target->GetID();
                     sending << strDamage;
                     sending << (long)color;
                     Target->SendPlayerMessage( sending );
                  }
               }
            }
            
           
            // Broadcast the attack.
            TFCPacket sending;
            sending << (RQ_SIZE)RQ_ArrowHit;
            sending << (long)Attacker->GetID();
            sending << (long)Target->GetID();
            sending << (char)( Target->GetHP() * 100 / Target->GetMaxHP() );
            
            WorldPos midPos = { 0, 0, Attacker->GetWL().world };  
            
            
            
            int xDiff = Attacker->GetWL().X - Target->GetWL().X;
            int yDiff = Attacker->GetWL().Y - Target->GetWL().Y;
            
            midPos.X = xDiff / 2 + Target->GetWL().X;
            midPos.Y = yDiff / 2 + Target->GetWL().Y;
            
            // Add the distance between the mid pos and the farest unit
            // to the default broadcasting range.
            int range;
            if( xDiff > yDiff )
               range = xDiff / 2 + _DEFAULT_RANGE;
            else
               range = yDiff / 2 + _DEFAULT_RANGE;
            
            Broadcast::BCast( midPos, range, sending );
            
            // If the unit got killed.
            if( result == -1 )
            {
               Broadcast::BCObjectChanged( Target->GetWL(), _DEFAULT_RANGE_CHANGE, 
                  Target->GetCorpse(),
                  Target->GetID(),1,
                  Attacker->GetInvisibleQuery()
                  );
            }
            
            
         }
         else
         {
            TFCPacket sending;
            sending << (RQ_SIZE)RQ_ArrowMiss;
            sending << (long)Attacker->GetID();
            sending << (short)Target->GetWL().X;
            sending << (short)Target->GetWL().Y;
            sending << (char)0;
            
            WorldPos midPos = { 0, 0, Attacker->GetWL().world };  
            
            
            
            int xDiff = Attacker->GetWL().X - Target->GetWL().X;
            int yDiff = Attacker->GetWL().Y - Target->GetWL().Y;
            
            midPos.X = xDiff / 2 + Target->GetWL().X;
            midPos.Y = yDiff / 2 + Target->GetWL().Y;
            
            // Add the distance between the mid pos and the farest unit
            // to the default broadcasting range.
            int range;
            if( xDiff > yDiff )
               range = xDiff / 2 + _DEFAULT_RANGE;
            else
               range = yDiff / 2 + _DEFAULT_RANGE;
            
            Broadcast::BCast( midPos, range, sending );
         }
         
         TRACE( "\r\nUnlocking units." );
         Attacker->Unlock();
         Target->Unlock();
         
         return 20; // dunno why actually.. this is 1 second exhaust			
      }
    }
    else
    {
       // Reset target if target happened to attack a hive.
       if( Target->GetType() == U_HIVE || Target->GetType() == U_MINIONS )
       {
          Attacker->SetTarget( Attacker->GetBond() );
       }
    }
    
    Attacker->Unlock();
    Target->Unlock();
    
    return 0;
}




/*TFCServerSocket remote;*/
//-------------------------------------------------------

//StaticObjects *Static_Objects_TABLE;

/***************************************/

/***************************************/

//////////////////////////////////////////////////////////////////////////////////////////
// Loads a dll list listed in the registry into the process's address space. If it
// fails to load a DLL, the function reports the error in the event manager.
// First parameter : The token used to load the list. Second parameter : Pointer
// to list which will contain the list of loaded DLLs
BOOL LoadDLLList (LPCTSTR lpszToken, TemplateList <HINSTANCE> *lptlDLLInstance,LPCTSTR lpszToken2,CStringArray &aNPCEXclusion)
/******************************************************************************/
{	
   CString DLLname;
   CString csString;
   RegKeyHandler regKey;


   BOOL boError = TRUE;
   BOOL boFound = TRUE;
   int i = 1;

   //Load Exclusion
   if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+EXTENSION_DLL_KEY ) )
   {
      // Loads the namespecs of the different DLLs associated to the objects
      do
      {
         csString.Format( "%s%u", lpszToken2, i );

         DLLname = regKey.GetProfileString( csString, _T("NO MORE EXCLUSIONSs") );
         if( DLLname != _T("NO MORE EXCLUSIONSs"))
         {
            aNPCEXclusion.Add(DLLname);
            i++;
         }
         else 
         {
            boFound = FALSE;
         }
      } while( boFound );
      regKey.Close();
   }
   
   boError = TRUE;
   boFound = TRUE;
   i = 1;
   if( regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+EXTENSION_DLL_KEY ) )
   {
      // Loads the namespecs of the different DLLs associated to the objects
      do
      {
         csString.Format( "%s%u", lpszToken, i );

         DLLname = regKey.GetProfileString( csString, _T("NO MORE DLLs") );
         if( DLLname != _T("NO MORE DLLs"))
         {
            HINSTANCE *lphInstance = new HINSTANCE;
            try
            {
               TRACE( "\r\n..%s..", (LPCTSTR)DLLname );

               _LOG_DEBUG
                  LOG_DEBUG_LVL1,
                  "Load DLL NAME %s",
                  (LPCTSTR)DLLname
                  LOG_

                char strDllName[512];
               sprintf_s(strDllName,512,"%s%s",TFCMAIN::GetHomeDir(),DLLname);

               (*lphInstance) = LoadLibrary(DLLname );

               /*
                DWORD dwLastError = GetLastError();
                if(dwLastError != 0)    // Don't want to see a "operation done successfully" error ;-)
                {
                   LPTSTR lpszTemp = 0;
                   DWORD dwRet = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      0, dwLastError, LANG_NEUTRAL, (LPTSTR)&lpszTemp, 0, 0);


                   if (lpszTemp)
                      LocalFree(HLOCAL(lpszTemp));
                }
                */
            }
            catch(...)
            {
               _LOG_DEBUG
                  LOG_DEBUG_LVL1,
                  "Crashed while loading DLL file %s.",
                  (LPCTSTR)DLLname
                  LOG_
                  exit( CRASH_LOADING_EXTENSION_DLL );
            }

            if( *lphInstance != NULL )
            {
               boError = FALSE;
               lptlDLLInstance->AddToTail( lphInstance );
            }
            else
            {
               DWORD dwErr = GetLastError();
               TRACE( "\r\nError=0x%x, %u.", dwErr, dwErr );
               _LOG_DEBUG
                  LOG_DEBUG_LVL1,
                  "Couldn't load DLL file %s specified in the registry.(err=0x%x(%u))",
                  DLLname,
                  dwErr,
                  dwErr
                  LOG_

                  TRACE( "\r\nUnable to load DLL file %s", (LPCTSTR)DLLname );
               if (lphInstance != NULL) 
               {
                  delete lphInstance;
                  lphInstance = NULL;
               }
            }
            i++;
         }
         else
         {
            boFound = FALSE;
         }
      } while( boFound );

      if( i == 0 )
      {
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "T4C Server does not have enough required DLLs to run."
            LOG_
            exit( FATAL_NO_REQUIRED_DLL1 );
      }

      regKey.Close();
   }

   

   return !boError;
}
/******************************************************************************/
// Creates the world. This is the primary initialisation of the server.
 TFC_MAIN::TFC_MAIN( void )
/******************************************************************************/
{	
   //************** validate SVR ID *********************
   char pszBuffer[MAX_PATH*2];
   int loop = GetModuleFileName( GetModuleHandle( NULL ), pszBuffer, _MAX_PATH * 2 );		
   do
   {
      loop--;
   } while( pszBuffer[ loop ] != '\\' && loop >= 0 );
   // End string after backslash.
   pszBuffer[ loop + 1 ] = 0;
   int iSvrID = 0;
   CString strIDF;
   strIDF.Format("%ssvr.ID",pszBuffer);
   FILE *pfID = NULL;
   fopen_s(&pfID,strIDF.GetBuffer(0),"rt");
   if(pfID)
   {
      char *pstrRead;
      char strLine[1024];
      pstrRead = fgets(strLine,1024,pfID);
      if(pstrRead)
         iSvrID = atoi(strLine);
      fclose(pfID);
   }
   if(iSvrID < 0 || iSvrID >9)
      iSvrID = 0;
   //************** validate SVR ID *********************

      CString strNMSURarPath;
      CString strNMSUBckPath;
      CString strNMSUNPCPath;
      CString strNMSUBinPath;

      RegKeyHandler regKeyNMSU;
      regKeyNMSU.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+PATCH_KEY );
      strNMSURarPath = regKeyNMSU.GetProfileString( "NMSUpdatePath_RAR", "" );                
      strNMSUBckPath = regKeyNMSU.GetProfileString( "NMSUpdatePath_BACKUP", "" );
      strNMSUNPCPath = regKeyNMSU.GetProfileString( "NMSUpdatePath_NPC", "" );

      regKeyNMSU.Close();
      regKeyNMSU.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+PATHS_KEY );
      strNMSUBinPath =	regKeyNMSU.GetProfileString( "BINARY_PATH", "" );
      regKeyNMSU.Close();

      CString strFileUpdate;
      CString strFileWDAPath;
      CString strFileUpdateNPC;
      CString strFileNPCPath;
      strFileUpdate    .Format("%s\\%s",strNMSURarPath,"FileToUpload.rar");
      strFileWDAPath   .Format("%s\\%s",strNMSUBinPath,"WDA\\");
      strFileUpdateNPC .Format("%s\\%s",strNMSURarPath,"NPCToUpload.rar");
      strFileNPCPath   .Format("%s\\",strNMSUNPCPath);

      _LOG_DEBUG
         LOG_CRIT_ERRORS,
         "***************************************************************"
         LOG_

      _LOG_DEBUG
         LOG_CRIT_ERRORS,
         "Check if WDA/Server Map / profession File Update avaiable..."
         LOG_

      printf( "\r\n***************************************************************");
      printf( "\r\nCheck if WDA/Server Map / profession File Update avaiable...");
      //Check si le fichier d'update est present...
      BOOL bNeedUpdateWDA = FALSE;
      BOOL bNeedUpdateNPC = FALSE;
      FILE *pfuWDA = NULL;
      fopen_s(&pfuWDA,strFileUpdate.GetBuffer(0),"rb");
      if(pfuWDA)
      {
         fclose(pfuWDA);
         bNeedUpdateWDA = TRUE;
      }
      FILE *pfuNPC = NULL;
      fopen_s(&pfuNPC,strFileUpdateNPC.GetBuffer(0),"rb");
      if(pfuNPC)
      {
         fclose(pfuNPC);
         bNeedUpdateNPC = TRUE;
      }

      if(bNeedUpdateWDA)
      {
         BOOL bBackupOKWDA = FALSE;
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "   -WDA Update Avaiable..."
            LOG_
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "   -WDA Create Backup : "
            LOG_

         printf( "\r\n   -WDA Update Avaiable...");
         printf( "\r\n   -WDA Create Backup : ");

         //Step 1 on backup les file existant sur le serveur...
         //  -Create a temporary .bat file and call it to backup WDA File on backup Folder...
         SYSTEMTIME sysTime; 
         GetLocalTime(&sysTime);


         CString strBackupCmd;
         CString strBackupFile;
         CString strBAtFileTmp;
         strBackupFile.Format("%s\\NMSU_Backup_%04d_%02d_%02d__%02dh%02d.rar",strNMSUBckPath,sysTime.wYear, sysTime.wMonth,sysTime.wDay,sysTime.wHour, sysTime.wMinute);
         strBackupCmd .Format("%srar.exe a -ep %s %s*.wda %s*.dat %s*.bin",strFileWDAPath,strBackupFile,strFileWDAPath,strFileWDAPath,strFileWDAPath);
         strBAtFileTmp.Format("%sNMSU_Backup.bat",strFileWDAPath);
         
         FILE *pfb = NULL;
         fopen_s(&pfb,strBAtFileTmp.GetBuffer(0),"wt");
         if(pfb)
         {
            fprintf(pfb,"%s",strBackupCmd.GetBuffer(0));
            fprintf(pfb,"\n");
            fclose(pfb);

            //Demarre le backup...
            SHELLEXECUTEINFO lpExecInfo;
            lpExecInfo.cbSize       = sizeof(SHELLEXECUTEINFO);
            lpExecInfo.lpFile       = strBAtFileTmp.GetBuffer(0);
            lpExecInfo.fMask        = SEE_MASK_DOENVSUBST|SEE_MASK_NOCLOSEPROCESS ;     
            lpExecInfo.hwnd         = NULL;  
            lpExecInfo.lpVerb       = NULL;
            lpExecInfo.lpParameters = NULL;
            lpExecInfo.lpDirectory  = NULL;   
            lpExecInfo.nShow        = SW_SHOWNORMAL ;  // show command prompt with normal window size 
            lpExecInfo.hInstApp = (HINSTANCE) SE_ERR_DDEFAIL ;   //WINSHELLAPI BOOL WINAPI result;
            ShellExecuteEx(&lpExecInfo);


            //wait until a file is finished printing
            if(lpExecInfo.hProcess !=NULL)
            {
               ::WaitForSingleObject(lpExecInfo.hProcess, INFINITE);
               ::CloseHandle(lpExecInfo.hProcess);
            }

            DeleteFile(strBAtFileTmp);


            bBackupOKWDA = TRUE;
         }


         //si backup OK on fait next step...
         if(bBackupOKWDA) 
         {
            _LOG_DEBUG
               LOG_CRIT_ERRORS,
               "        --> Success !!!"
               LOG_
            printf( "Success !!!");


            //Now on dois unpack le fichier de maj...
            //move le fichier de MAJ DANS le REPertoire des WDA...
            CString strUpdateFileReadyToUnpack;
            strUpdateFileReadyToUnpack.Format("%sFileToUnpack.rar",strFileWDAPath);
            MoveFileEx(strFileUpdate,strUpdateFileReadyToUnpack,MOVEFILE_REPLACE_EXISTING);

            BOOL bUnpackOK = FALSE;
            _LOG_DEBUG
               LOG_CRIT_ERRORS,
               "   -WDA Update Files  : "
               LOG_
            printf( "\r\n   -WDA Update Files : ");


            CString strUnpackCmd;
            CString strBAtUnpackTmp;
            strUnpackCmd .Format("%srar.exe e -y %s %s",strFileWDAPath,strUpdateFileReadyToUnpack,strFileWDAPath);
            strBAtUnpackTmp.Format("%sNMSU_Unpack.bat",strFileWDAPath);

            FILE *pfUpd = NULL;
            fopen_s(&pfUpd,strBAtUnpackTmp.GetBuffer(0),"wt");
            if(pfUpd)
            {
               fprintf(pfUpd,"%s",strUnpackCmd.GetBuffer(0));
               fprintf(pfUpd,"\n");
               fclose(pfUpd);

               //Demarre le backup...
               SHELLEXECUTEINFO lpExecInfo;
               lpExecInfo.cbSize       = sizeof(SHELLEXECUTEINFO);
               lpExecInfo.lpFile       = strBAtUnpackTmp.GetBuffer(0);
               lpExecInfo.fMask        = SEE_MASK_DOENVSUBST|SEE_MASK_NOCLOSEPROCESS ;     
               lpExecInfo.hwnd         = NULL;  
               lpExecInfo.lpVerb       = NULL;
               lpExecInfo.lpParameters = NULL;
               lpExecInfo.lpDirectory  = NULL;   
               lpExecInfo.nShow        = SW_SHOWNORMAL ;  // show command prompt with normal window size 
               lpExecInfo.hInstApp = (HINSTANCE) SE_ERR_DDEFAIL ;   //WINSHELLAPI BOOL WINAPI result;
               ShellExecuteEx(&lpExecInfo);


               //wait until a file is finished printing
               if(lpExecInfo.hProcess !=NULL)
               {
                  ::WaitForSingleObject(lpExecInfo.hProcess, INFINITE);
                  ::CloseHandle(lpExecInfo.hProcess);
               }

               DeleteFile(strBAtUnpackTmp);
               DeleteFile(strUpdateFileReadyToUnpack);
               bUnpackOK = TRUE;
            }
            if(bUnpackOK) 
            {
               _LOG_DEBUG
                  LOG_CRIT_ERRORS,
                  "        --> Success !!!  UPDATE Completed."
                  LOG_
                  printf( "Success !!!");
            }
            else
            {
               _LOG_DEBUG
                  LOG_CRIT_ERRORS,
                  "        --> Error... Update failed !!!"
                  LOG_
                  printf( "Error... UNPACK failed !!!");
            }
         }
         else
         {
            _LOG_DEBUG
               LOG_CRIT_ERRORS,
               "        --> Error... Update failed !!!"
               LOG_
            printf( "Error... Update failed !!!");
         }
      }
      else
      {
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "NO Update Found."
         LOG_
         printf( "\r\nNO Update Found.");
      }
      //*********** END UPDATE WDA

      _LOG_DEBUG
         LOG_CRIT_ERRORS,
         "Check if NPC DLL Update avaiable..."
         LOG_

      printf( "\r\n***************************************************************");
      printf( "\r\nCheck if NPC DLL Update avaiable...");
      //*********** STARt UPDATE NPC
      if(bNeedUpdateNPC)
      {
         BOOL bBackupOKNPC = FALSE;
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "   -NPC Update Avaiable..."
            LOG_
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "   -NPC Create Backup : "
            LOG_

         printf( "\r\n   -NPC Update Avaiable...");
         printf( "\r\n   -NPC Create Backup : ");

         //Step 1 on backup les file existant sur le serveur...
         //  -Create a temporary .bat file and call it to backup WDA File on backup Folder...
         SYSTEMTIME sysTime; 
         GetLocalTime(&sysTime);


         CString strBackupCmd;
         CString strBackupFile;
         CString strBAtFileTmp;
         strBackupFile.Format("%s\\NMSU_NPCDLL_Backup_%04d_%02d_%02d__%02dh%02d.rar",strNMSUBckPath,sysTime.wYear, sysTime.wMonth,sysTime.wDay,sysTime.wHour, sysTime.wMinute);
         strBackupCmd .Format("%srar.exe a -ep %s %s*.dll",strFileNPCPath,strBackupFile,strFileNPCPath);
         strBAtFileTmp.Format("%sNMSU_Backup.bat",strFileNPCPath);

         FILE *pfb = NULL;
         fopen_s(&pfb,strBAtFileTmp.GetBuffer(0),"wt");
         if(pfb)
         {
            fprintf(pfb,"%s",strBackupCmd.GetBuffer(0));
            fprintf(pfb,"\n");
            fclose(pfb);

            //Demarre le backup...
            SHELLEXECUTEINFO lpExecInfo;
            lpExecInfo.cbSize       = sizeof(SHELLEXECUTEINFO);
            lpExecInfo.lpFile       = strBAtFileTmp.GetBuffer(0);
            lpExecInfo.fMask        = SEE_MASK_DOENVSUBST|SEE_MASK_NOCLOSEPROCESS ;     
            lpExecInfo.hwnd         = NULL;  
            lpExecInfo.lpVerb       = NULL;
            lpExecInfo.lpParameters = NULL;
            lpExecInfo.lpDirectory  = NULL;   
            lpExecInfo.nShow        = SW_SHOWNORMAL ;  // show command prompt with normal window size 
            lpExecInfo.hInstApp = (HINSTANCE) SE_ERR_DDEFAIL ;   //WINSHELLAPI BOOL WINAPI result;
            ShellExecuteEx(&lpExecInfo);


            //wait until a file is finished printing
            if(lpExecInfo.hProcess !=NULL)
            {
               ::WaitForSingleObject(lpExecInfo.hProcess, INFINITE);
               ::CloseHandle(lpExecInfo.hProcess);
            }

            DeleteFile(strBAtFileTmp);


            bBackupOKNPC = TRUE;
         }


         //si backup OK on fait next step...
         if(bBackupOKNPC) 
         {
            _LOG_DEBUG
               LOG_CRIT_ERRORS,
               "        --> Success !!!"
               LOG_
               printf( "Success !!!");


            //Now on dois unpack le fichier de maj...
            //move le fichier de MAJ DANS le REPertoire des WDA...
            CString strUpdateFileReadyToUnpack;
            strUpdateFileReadyToUnpack.Format("%sNPCToUnpack.rar",strFileNPCPath);
            MoveFileEx(strFileUpdateNPC,strUpdateFileReadyToUnpack,MOVEFILE_REPLACE_EXISTING);

            BOOL bUnpackOK = FALSE;
            _LOG_DEBUG
               LOG_CRIT_ERRORS,
               "   -Update Files  : "
               LOG_
               printf( "\r\n   -Update Files : ");


            CString strUnpackCmd;
            CString strBAtUnpackTmp;
            strUnpackCmd .Format("%srar.exe e -y %s %s",strFileNPCPath,strUpdateFileReadyToUnpack,strFileNPCPath);
            strBAtUnpackTmp.Format("%sNMSU_Unpack.bat",strFileNPCPath);

            FILE *pfUpd = NULL;
            fopen_s(&pfUpd,strBAtUnpackTmp.GetBuffer(0),"wt");
            if(pfUpd)
            {
               fprintf(pfUpd,"%s",strUnpackCmd.GetBuffer(0));
               fprintf(pfUpd,"\n");
               fclose(pfUpd);

               //Demarre le backup...
               SHELLEXECUTEINFO lpExecInfo;
               lpExecInfo.cbSize       = sizeof(SHELLEXECUTEINFO);
               lpExecInfo.lpFile       = strBAtUnpackTmp.GetBuffer(0);
               lpExecInfo.fMask        = SEE_MASK_DOENVSUBST|SEE_MASK_NOCLOSEPROCESS ;     
               lpExecInfo.hwnd         = NULL;  
               lpExecInfo.lpVerb       = NULL;
               lpExecInfo.lpParameters = NULL;
               lpExecInfo.lpDirectory  = NULL;   
               lpExecInfo.nShow        = SW_SHOWNORMAL ;  // show command prompt with normal window size 
               lpExecInfo.hInstApp = (HINSTANCE) SE_ERR_DDEFAIL ;   //WINSHELLAPI BOOL WINAPI result;
               ShellExecuteEx(&lpExecInfo);


               //wait until a file is finished printing
               if(lpExecInfo.hProcess !=NULL)
               {
                  ::WaitForSingleObject(lpExecInfo.hProcess, INFINITE);
                  ::CloseHandle(lpExecInfo.hProcess);
               }

               DeleteFile(strBAtUnpackTmp);
               DeleteFile(strUpdateFileReadyToUnpack);
               bUnpackOK = TRUE;
            }
            if(bUnpackOK) 
            {
               _LOG_DEBUG
                  LOG_CRIT_ERRORS,
                  "        --> Success !!!  UPDATE Completed."
                  LOG_
                  printf( "Success !!!");
            }
            else
            {
               _LOG_DEBUG
                  LOG_CRIT_ERRORS,
                  "        --> Error... Update failed !!!"
                  LOG_
                  printf( "Error... UNPACK failed !!!");
            }
         }
         else
         {
            _LOG_DEBUG
               LOG_CRIT_ERRORS,
               "        --> Error... Update failed !!!"
               LOG_
               printf( "Error... Update failed !!!");
         }
      }
      else
      {
         _LOG_DEBUG
            LOG_CRIT_ERRORS,
            "NO Update Found."
            LOG_
            printf( "\r\nNO Update Found.");
      }

      //*********** END UPDATE NPC

      _LOG_DEBUG
         LOG_CRIT_ERRORS,
         "***************************************************************\r\n"
         LOG_

      printf( "\r\n***************************************************************\r\n");
      

      


   

   if(theApp.sGeneral.dwServerUseAllCPU == 0)
   {
      printf( "\r\nTry Server Process on ONE CPU");
      DWORD           *pProcessID = NULL; 
      DWORD           ProcessID; 
      HANDLE          hProcess; 
      BOOL            bStat;
      ProcessID = GetCurrentProcessId();
      hProcess = OpenProcess(PROCESS_SET_INFORMATION,false,ProcessID);
      if(hProcess)
      {
         DWORD dwAffinityID = 0x01;
         if(iSvrID == 0 || iSvrID == 3)
         {
            printf( "\r\nSet Affinity to CPU no 1");
            dwAffinityID = 0x01;
         }
         else if(iSvrID == 1 || iSvrID == 4)
         {
            printf( "\r\nSet Affinity to CPU no 2");
            dwAffinityID = 0x02;
         }
         else if(iSvrID == 2 || iSvrID == 5)
         {
            printf( "\r\nSet Affinity to CPU no 3");
            dwAffinityID = 0x04;
         }
         
         //bStat = SetPriorityClass(hProcess,HIGH_PRIORITY_CLASS);
         bStat = SetProcessAffinityMask(hProcess,dwAffinityID); 
         CloseHandle (hProcess);  
      }
      else
      {
         printf( "\r\nCant Get CPU process...");
      }
   }
   else
   {
      printf( "\r\nServer Process set on ALL CPU");
   }
   
	dwVersion = SERVER_CONNECTION_HI_VERSION;

	Skills::Create();
   


	Unit::InitializeMessagesProcs( );

	ObjectTimer::Create( );

	SpellMessageHandler::Create( );		

	// Initialize ODBC
	Players::InitializeODBC();

    //SetUnhandledExceptionFilter( DefaultExcpFilter );

	Character::InitializeODBC();    
	Character::InitXPchart();

   
   
	
   if( !LoadDLLList( "NPCDLL",&tlDllInstance,"NPCEXCLUSION",theApp.m_aNPCEXclusion ) )
	{
		_LOG_DEBUG
            LOG_CRIT_ERRORS,
            "\r\n     Cannot find any NPC DLL. Server is useless without NPCs."
            "\r\n     Run T4C Server Setup from the control panel and add NPC DLLs."
        LOG_
        exit( FATAL_NO_REQUIRED_DLL3 );
    }	

	RegisterObjects();
   SkillRegistration();

	// Now that all the DLLs have been loaded and the units registered, initialize the global variables
	ObjectListing();

	// Then call all the DLL's unit startup functions.
	TFCMAIN::CallUnitStartupFunctions();    

	world_number = 0;
	
	TRACE(_T("%u"), GetLastError());

	flush_time =  ( theApp.sGeneral.dwTimeBeforeWarning * 20 ) / 1000; // default time before declaring someone off hook, 5 secs
	max_chances = (unsigned char)theApp.sGeneral.wNbWarnings; // give 5 chances before flushing someone
    
	World = NULL;
	
	TFCTIME MaxTime;
	TFCTIME TimeLine;

	MaxTime.seconds = 60;
	MaxTime.minutes = 60;
	MaxTime.hours = 24;
	MaxTime.days = 7;
	MaxTime.weeks = 4;
	MaxTime.months = 12;
	
	TimeLine.seconds = 0;
	TimeLine.minutes = 0;
	TimeLine.hours   = 7;
	TimeLine.days    = 2;
	TimeLine.weeks   = 0;
	TimeLine.months  = 6;
	TimeLine.years   = 1997;

	TFCTime::Create(TimeLine, MaxTime);
 
    RegKeyHandler regKey;
    regKey.Open( HKEY_LOCAL_MACHINE, theApp.csT4CKEY+GEN_CFG_KEY );

    // Fetch version from registry, otherwise default to the executable's version.
    dwVersion = regKey.GetProfileInt( "Version", SERVER_CONNECTION_HI_VERSION );
    regKey.Close();

    CAutoConfig::AddRegString( theApp.csT4CKEY+GEN_CFG_KEY,   "SuperUser", "", HKEY_LOCAL_MACHINE );    
}
/******************************************************************************/
TFC_MAIN::~TFC_MAIN()
/******************************************************************************/
{

   
	if(World != NULL) 
	{
		delete [] World;
		World = NULL;
	}
   

	// Uninitialize the DLLs
	TFCServer->tlDllInstance.ToHead();
	while( TFCServer->tlDllInstance.QueryNext() )
	{
		FreeLibrary( (*TFCServer->tlDllInstance.Object() ) );
		TFCServer->tlDllInstance.DeleteAbsolute();
	}		
   
   

}
