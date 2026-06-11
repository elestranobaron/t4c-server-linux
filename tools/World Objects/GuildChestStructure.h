/******************************************************************************
Rewrite for vs2008 by Nightmare (28/06/2009)
/******************************************************************************/
#if !defined(AFX_GUILDCHESTSTRUCTURE_H__71302979_E8E6_4222_A0AE_505FF3D47467__INCLUDED_)
#define AFX_GUILDCHESTSTRUCTURE_H__71302979_E8E6_4222_A0AE_505FF3D47467__INCLUDED_

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000

#include "../ObjectStructure.h"
/******************************************************************************/
class GuildChestStructure : public ObjectStructure  
/******************************************************************************/
{
public:
   GuildChestStructure();
   virtual ~GuildChestStructure();
   
   void OnInitialise( UNIT_FUNC_PROTOTYPE );
   void OnUse( UNIT_FUNC_PROTOTYPE );
   
   static ObjectStructure *CreateObject( void );
  
};

#endif // !defined(AFX_GUILDCHESTSTRUCTURE_H__71302979_E8E6_4222_A0AE_505FF3D47467__INCLUDED_)
