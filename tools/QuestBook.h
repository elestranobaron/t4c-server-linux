// QuestBook.h: interface for the QuestBook class.

#pragma once

#include "VDList.h"
#include "IntlText.h"
#include "NMProfession.h"
#include "QuestBookInc.h"


class QuestBook;

// The class
class __declspec(dllexport) QuestBook
{
public:
	
	static void Create();  // Creates all internal structures
   static void Destroy(); // Destroys all internal structures
   static int GetNbrQuest();
   static int GetNbrQuestEnabled();

   static sQuestBook * GetQuestByIndex(USHORT ushVal);
   static sQuestBook * GetQuestByID(USHORT ushVal);
   static sQuestBook * GetQuestByFlag(USHORT ushVal);



   /*
   static int GetFormuleIDbyIndex(int iIdx);

	static BOOL    QuestBook::IsFormuleExist(USHORT ushID);        // returns TRUE if formule ID Exist
	static BYTE    IsFormuleLearnable(USHORT ushID, Unit *uLearner);
   static CString GetFormuleName(USHORT ushID);
   static USHORT  GetFormuleSkin(USHORT ushID);
   static USHORT  GetFormuleSkill(USHORT ushID);
   static sFormule * GetFormules(USHORT ushID);
   static BYTE GetFormulesProfession(USHORT ushID);

   static CString GetFormuleNameReqByID(USHORT ushID);
   static bool    GetFormuleNameReqIDISUnique(USHORT ushID);
   */

private:
	static int nProfessionID;

	static CPtrArray c_aQuestBook;
};
