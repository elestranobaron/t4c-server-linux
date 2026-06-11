// Professions.h: interface for the Professions class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROFESSIONS_H__D02F3B81_5542_11D1_BD7A_00E029058623__INCLUDED_)
#define AFX_PROFESSIONS_H__D02F3B81_5542_11D1_BD7A_00E029058623__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "VDList.h"
#include "IntlText.h"
#include "NMProfession.h"


class Professions;

// The class
class __declspec(dllexport) Professions
{
public:
	
	static void Create();  // Creates all internal structures
   static void Destroy(); // Destroys all internal structures
	
   static int GetNbrFormule();
   static int GetFormuleIDbyIndex(int iIdx);

	static BOOL    Professions::IsFormuleExist(USHORT ushID);        // returns TRUE if formule ID Exist
	static BYTE    IsFormuleLearnable(USHORT ushID, Unit *uLearner);
   static CString GetFormuleName(USHORT ushID);
   static USHORT  GetFormuleSkin(USHORT ushID);
   static USHORT  GetFormuleSkill(USHORT ushID);
   static sFormule * GetFormules(USHORT ushID);
   static BYTE GetFormulesProfession(USHORT ushID);

   static CString GetFormuleNameReqByID(USHORT ushID);
   static bool    GetFormuleNameReqIDISUnique(USHORT ushID);

private:
	static int nProfessionID;

	static CPtrArray c_aProfessions;
};

#endif // !defined(AFX_PROFESSIONS_H__D02F3B81_5542_11D1_BD7A_00E029058623__INCLUDED_)
