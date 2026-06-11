#include "stdafx.h"
#include "Skills.h"
#include "..\SkillListing.h"

#include "StunBlow.h"
#include "PowerfullBlow.h"
#include "CriticalStrike.h"
#include "FirstAid.h"
#include "Parry.h"
#include "Meditate.h"
#include "PickLock.h"
#include "DisarmTrap.h"
#include "ForgetPain.h"
#include "EvadeView.h"
#include "TwoWeapons.h"
#include "ArmorPenetration.h"
#include "Peek.h"
#include "Rob.h"
#include "plunder.h"
#include "resurect.h"
#include "FastHealing.h"
#include "GenericSkill.h"
#include "Hide.h"
#include "Sneak.h"
#include "Search.h"
#include "PowerConjuring.h"
#include "Immobilization.h"
#include "PrimalScream.h"
#include "Rundumschlag.h"

#define REGISTER(object, name, descId, hook, ID)\
	Skills::Register(name, descId, ID, object.Func, &object.s_saAttrib, hook, object.lpOnAddPnts);

#define DESTROY( obj ) obj.Destroy();

static StunBlow			c_StunBlow;
static PowerfullBlow  	c_PowerfullBlow;
static CriticalStrike	c_CriticalStrike;
static FirstAid			c_FirstAid;
static Parry		   	c_Parry;
static Meditate			c_Meditate;
static CPickLock	   	c_PickLock;
static DisarmTrap	   	c_DisarmTrap;
static EvadeView	   	c_EvadeView;
static ForgetPain	   	c_ForgetPain;
static TwoWeapons	   	c_TwoWeapons;
static ArmorPenetration c_ArmorPenetration;
static Peek             c_Peek;
static Rob              c_Rob;
static FastHealing      c_FastHealing;
static PowerConjuring	c_PowerConjuring;
static PrimalScream     c_PrimalScream;
static Immobilization	c_Immobilization;
static Plunder	      	c_Plaunder;
static Resurect	    	c_Resurect;

static GenericSkill     c_Alchemy;
static GenericSkill     c_ArcaneScripting;
static GenericSkill     c_Enchanting;
static GenericSkill     c_Smithing;
static GenericSkill     c_Tinkering;
static GenericSkill     c_Archery;

static Hide c_Hide;
static Sneak c_Sneak;
static Search c_Search;

static Rundumschlag	  c_Rundumschlag;

extern Random rnd;
void SkillRegistration(){                
   
	REGISTER( c_StunBlow,			_T("[393]Stun blow"),			 7902, HOOK_ATTACK,						      __SKILL_STUN_BLOW )

   // AJOUT (modif Moen_OK)
	REGISTER( c_CriticalStrike,	_T("[395]Coup critique"),		15353, HOOK_SPELL_ATTACK | HOOK_ATTACK,   __SKILL_CRITICAL_STRIKE ) 

	REGISTER( c_FirstAid,			_T("[396]First aid"),			 7891, HOOK_USE,							      __SKILL_FIRST_AID )
	REGISTER( c_Parry,				_T("[398]Parry"),				    7894, HOOK_HIT,							      __SKILL_PARRY )
	REGISTER( c_Meditate,			_T("[399]Meditate"),			    7893, HOOK_USE | HOOK_REGEN,				   __SKILL_MEDITATE )
	REGISTER( c_PickLock,			_T("[402]Pick lock"),			 7896, HOOK_USE_TARGET_UNIT,			 	   __SKILL_PICKLOCK )
	REGISTER( c_ArmorPenetration,	_T("[409]Armor penetration"),	 7888, HOOK_ATTACK,					     	   __SKILL_ARMOR_PENETRATION )
	REGISTER( c_PowerfullBlow,		_T("[394]Powerful blow"),		 7897, HOOK_ATTACK,						      __SKILL_POWERFULL_BLOW )
	REGISTER( c_Peek,				   _T("[410]Peek"),				    7895, HOOK_USE | HOOK_USE_TARGET_UNIT,   __SKILL_PEEK )
	REGISTER( c_Rob,			   	_T("[411]Rob"),					 7899, HOOK_USE | HOOK_USE_TARGET_UNIT,   __SKILL_ROB )
	REGISTER( c_FastHealing,		_T("[412]Rapid healing"),		 7898, HOOK_REGEN,						      __SKILL_RAPID_HEALING )
	REGISTER( c_Hide,				   _T("[7251]Hide"),				    7892, HOOK_USE_TARGET_UNIT,				   __SKILL_HIDE )
	REGISTER( c_Sneak,				_T("[7252]Sneak"),				 7901, HOOK_MOVE,						         __SKILL_SNEAK )
	REGISTER( c_Search,				_T("[7253]Search"),				 7900, HOOK_USE,							      __SKILL_SEARCH )
   
   // AJOUT (modif Moen_OK)
	REGISTER( c_PowerConjuring,   _T("[15350]PowerConjuring"),	15354, HOOK_SPELL_ATTACK,					   __SKILL_POWERCONJURING )
   REGISTER( c_PrimalScream,     _T("[15352]PrimalScream"),  	15356, HOOK_USE,		         			   __SKILL_PRIMAL_SCREAM )
	REGISTER( c_Immobilization,	_T("[15351]Immobilization"),	15355, HOOK_USE | HOOK_USE_TARGET_UNIT,   __SKILL_IMMOBILIZATION ) 
   //fin des ajouts

	REGISTER( c_Archery,			   _T("[7691]Archery"),			    7887, HOOK_ATTACK,						      __SKILL_ARCHERY )
	REGISTER( c_TwoWeapons,			_T("[12959]Dual Weapons"),		12960, HOOK_ATTACK,						      __SKILL_TWOWEAPONS )
	REGISTER( c_Plaunder,   	  	_T("[15006]Plunder"),			15007, HOOK_USE | HOOK_USE_TARGET_UNIT,	__SKILL_PLUNDER )
	REGISTER( c_Resurect,   	  	_T("[15012]Resurect"),			15013, HOOK_USE | HOOK_USE_TARGET_UNIT,	__SKILL_RESURECT )
	REGISTER( c_Rundumschlag,   "[40000]Rundumschlag",		40001, HOOK_USE | HOOK_ATTACK,	__SKILL_RUNDUMSCHLAG )
	
}

