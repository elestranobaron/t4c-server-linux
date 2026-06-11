// QuestBook.cpp: implementation of the Skills class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TFC Server.h"
#include "TFC_MAIN.h"
#include "QuestBook.h"
#include "IntlText.h"
#include "Unit.h"
#include "QUEST_key.h"
#include "DynObjManager.h"
#include "ReadDataFromMemory.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CPtrArray QuestBook::c_aQuestBook;


//////////////////////////////////////////////////////////////////////////////////////////
void QuestBook::Create( void )
{
   int  dwTaille;
   char strTmp[1024];
   CString strFileP;
   strFileP = TFCMAIN::GetHomeDir();

   strFileP += "WDA\\QuestBook.dat";


   FILE *pf = NULL;
   fopen_s(&pf,strFileP,"rb");
   if(!pf)
   {
      _LOG_DEBUG
         LOG_CRIT_ERRORS,
         "NO QuestBook file found %s",
         strFileP
         LOG_

         printf( "\r\nNO QuestBook file found %s.",strFileP);
      return;
   }


   fseek(pf,0,SEEK_END);
   dwTaille = ftell(pf);
   fseek(pf,0,SEEK_SET);

   BYTE	 *pData    = NULL;
   pData = new BYTE[dwTaille];
   // lit le contenue du fichier
   fread(pData,dwTaille,1,pf);
   fclose(pf);

   for (UINT i=0; i<dwTaille; i++)
      pData[i] ^= QUEST_key[i%3418];


   //Read the datafiles...

   CReadDataFromMemory rdM;
   rdM.SetMemoryPtr(pData,dwTaille);


   printf( "\n\n- Loading QuestBook File" );


   int uiNbrFormule = rdM.get_dword();
   if(uiNbrFormule >0)
   {
      for(int i=0;i<uiNbrFormule;i++)
      {
         sQuestBook *pQuestData = new sQuestBook;
         pQuestData->chEnable       = rdM.get_byte();
         pQuestData->ushUniqueID    = rdM.get_word();
         pQuestData->dwFlagID       = rdM.get_dword();
         pQuestData->ushQuestLevel  = rdM.get_word();

         sprintf_s(strTmp,1024,"%s",rdM.get_string());
         pQuestData->dwNameLength = strlen(strTmp);
         if(pQuestData->dwNameLength >0)
         {
            pQuestData->pStrName = new char[strlen(strTmp)+1];
            strcpy_s(pQuestData->pStrName,strlen(strTmp)+1,strTmp);
         }
         else
            pQuestData->pStrName = NULL;

         sprintf_s(strTmp,1024,"%s",rdM.get_string());
         pQuestData->dwMsgLength = strlen(strTmp);
         if(pQuestData->dwMsgLength >0)
         {
            pQuestData->pStrMsg = new char[strlen(strTmp)+1];
            strcpy_s(pQuestData->pStrMsg,strlen(strTmp)+1,strTmp);
         }
         else
            pQuestData->pStrMsg = NULL;

         pQuestData->dwNbrQuestStep = rdM.get_dword();
         if(pQuestData->dwNbrQuestStep >0)
         {
            pQuestData->pStepMsg = new sQuestStepMsg[pQuestData->dwNbrQuestStep];
            for(UINT n=0;n<pQuestData->dwNbrQuestStep;n++)
            {
               sprintf_s(strTmp,1024,"%s",rdM.get_string());
               pQuestData->pStepMsg[n].dwLength = strlen(strTmp);
               if(pQuestData->pStepMsg[n].dwLength >0)
               {
                  pQuestData->pStepMsg[n].pstrStep = new char[strlen(strTmp)+1];
                  strcpy_s(pQuestData->pStepMsg[n].pstrStep,strlen(strTmp)+1,strTmp);
               }
               else
                  pQuestData->pStepMsg[n].pstrStep = NULL;
            }
         }
         else
         {
            pQuestData->pStepMsg = NULL;
         }

         c_aQuestBook.SetAtGrow(i, pQuestData);
      }
   }

   printf( "\n  Found %d Quest(s) inside QuestBook",c_aQuestBook.GetSize() );

   if(pData)
      delete []pData;
   pData = NULL;
}


//////////////////////////////////////////////////////////////////////////////////////////
void QuestBook::Destroy( void )
{
   sQuestBook *lpQuest;
   int i;

   for(i = 0; i < c_aQuestBook.GetSize(); i++)
   {
      lpQuest = (sQuestBook *)c_aQuestBook.GetAt(i);
      if(lpQuest)
      {

         for(UINT n=0;n<lpQuest->dwNbrQuestStep;n++)
         {
            if(lpQuest->pStepMsg[n].pstrStep)
               delete []lpQuest->pStepMsg[n].pstrStep;
            lpQuest->pStepMsg[n].pstrStep = NULL;   
         }
         delete [] lpQuest->pStepMsg;
         lpQuest->pStepMsg = NULL;

         if(lpQuest->pStrName)
            delete []lpQuest->pStrName;
         lpQuest->pStrName = NULL;

         if(lpQuest->pStrMsg)
            delete []lpQuest->pStrMsg;
         lpQuest->pStrMsg = NULL;

         delete lpQuest;
         lpQuest = NULL;
      }
   }
}

int QuestBook::GetNbrQuest()
{
   return c_aQuestBook.GetSize();
}

int QuestBook::GetNbrQuestEnabled()
{
   sQuestBook *lpQuest;
   int iNbrQuest = 0;
   for(int i = 0; i < c_aQuestBook.GetSize(); i++)
   {
      lpQuest = (sQuestBook *)c_aQuestBook.GetAt(i);
      if(lpQuest && lpQuest->chEnable)
      {
         iNbrQuest++;
      }
   }
   return iNbrQuest;
}


sQuestBook * QuestBook::GetQuestByFlag(USHORT ushVal)
{
   sQuestBook *lpQuest;
   int i;

   for(i = 0; i < c_aQuestBook.GetSize(); i++)
   {
      lpQuest = (sQuestBook *)c_aQuestBook.GetAt(i);
      if(lpQuest)
      {
         if(lpQuest->dwFlagID == ushVal)
         {
            return lpQuest;
         }
      }
   }
   return NULL;
}

sQuestBook * QuestBook::GetQuestByID(USHORT ushVal)
{
   sQuestBook *lpQuest;
   int i;

   for(i = 0; i < c_aQuestBook.GetSize(); i++)
   {
      lpQuest = (sQuestBook *)c_aQuestBook.GetAt(i);
      if(lpQuest)
      {
         if(lpQuest->ushUniqueID == ushVal)
         {
            return lpQuest;
         }
      }
   }
   return NULL;
}

sQuestBook * QuestBook::GetQuestByIndex(USHORT ushVal)
{
   sQuestBook *lpQuest;
   
   if(ushVal >=0 && ushVal < c_aQuestBook.GetSize())
   {
      lpQuest = (sQuestBook *)c_aQuestBook.GetAt(ushVal);
      return  lpQuest;
   }

   return NULL;
}



/*
BOOL QuestBook::IsFormuleExist(USHORT ushID)
{
   sFormule *lpProfession;
   int i;
   
   for(i = 0; i < c_aProfessions.GetSize(); i++)
   {
      lpProfession = (sFormule *)c_aProfessions.GetAt(i);
      if(lpProfession)
      {
         if(lpProfession->ushID == ushID && lpProfession->chEnable)
            return TRUE;
      }
   }
   return FALSE;
}

// 0 = peu pas apprendre
// 1 = peu apprendre
// 2 = deja appris
BYTE QuestBook::IsFormuleLearnable(USHORT ushID, Unit *uLearner)
{
   sFormule *lpProfession;
   int i;
   
   for(i = 0; i < c_aProfessions.GetSize(); i++)
   {
      lpProfession = (sFormule *)c_aProfessions.GetAt(i);
      if(lpProfession)
      {
         if(lpProfession->ushID == ushID)
         {
            if(uLearner->ViewFlag(50+lpProfession->chProfession) >= lpProfession->ushSkillLevel)
            {
               //regarde si le user a pas deja appris cette formule
               Character *lpChar = static_cast< Character * >( uLearner );
               TemplateList< USER_PROFESSION_F > *lpProf = lpChar->GetProfession();
               lpProf->Lock();
               lpProf->ToHead();
               while( lpProf->QueryNext() )
               {
                  if( lpProf->Object()->ushID == ushID )
                  {
                     lpProf->Unlock();                       
                     return 2;
                  }
               }
               lpProf->Unlock();   
               return 1;
            }
            else
               return 0;
         }
      }
   }
   return 0;
}


CString QuestBook::GetFormuleName(USHORT ushID)
{
   CString strName;

   sFormule *lpProfession;
   int i;
   
   for(i = 0; i < c_aProfessions.GetSize(); i++)
   {
      lpProfession = (sFormule *)c_aProfessions.GetAt(i);
      if(lpProfession)
      {
         if(lpProfession->ushID == ushID)
         {
            //FILE* pf = fopen("C:\\!projets\\!MPP\\!Dev\\!!toto.txt","a+t");
            //fprintf(pf,"%d %d %s\n",lpObject->appearance,lpObject->itemStructureId,lpObject->name);
            //fclose(pf);

            ObjectStructure *lpObject = DynObjManager::GetRegisteredUnitID(lpProfession->ushItemResultID);
            if(lpObject)
               strName.Format("%s",IntlText::ParseString( lpObject->name, _DEFAULT_LNG ));
            else
               strName.Format("---");

            
            return strName;
         }
      }
   }

   strName.Format("---");
   return strName;
}

USHORT QuestBook::GetFormuleSkin(USHORT ushID)
{
   sFormule *lpProfession;
   int i;
   
   for(i = 0; i < c_aProfessions.GetSize(); i++)
   {
      lpProfession = (sFormule *)c_aProfessions.GetAt(i);
      if(lpProfession)
      {
         if(lpProfession->ushID == ushID)
         {
            USHORT ushSkin = 0;

            ObjectStructure *lpObject = DynObjManager::GetRegisteredUnitID(lpProfession->ushItemResultID);
            if(lpObject)
               ushSkin = lpObject->appearance;
            
            return ushSkin;
         }
      }
   }

   return 0;
}


USHORT QuestBook::GetFormuleSkill(USHORT ushID)
{
   sFormule *lpProfession;
   int i;
   
   for(i = 0; i < c_aProfessions.GetSize(); i++)
   {
      lpProfession = (sFormule *)c_aProfessions.GetAt(i);
      if(lpProfession)
      {
         if(lpProfession->ushID == ushID)
         {
            return lpProfession->ushSkillLevel;
         }
      }
   }
   return 0;
}

sFormule * QuestBook::GetFormules(USHORT ushID)
{
   sFormule *lpProfession;
   int i;
   
   for(i = 0; i < c_aProfessions.GetSize(); i++)
   {
      lpProfession = (sFormule *)c_aProfessions.GetAt(i);
      if(lpProfession)
      {
         if(lpProfession->ushID == ushID)
         {
            return lpProfession;
         }
      }
   }
   return NULL;
}

BYTE QuestBook::GetFormulesProfession(USHORT ushID)
{
   sFormule *lpProfession;
   int i;
   
   for(i = 0; i < c_aProfessions.GetSize(); i++)
   {
      lpProfession = (sFormule *)c_aProfessions.GetAt(i);
      if(lpProfession)
      {
         if(lpProfession->ushID == ushID)
         {
            return lpProfession->chProfession;
         }
      }
   }
   return 0;
}


int QuestBook::GetNbrFormule()
{
   return c_aProfessions.GetSize();
}

int QuestBook::GetFormuleIDbyIndex(int iIdx)
{
   if(iIdx <0 || iIdx >= GetNbrFormule())
      return 0;

   sFormule *lpProfession;
   lpProfession = (sFormule *)c_aProfessions.GetAt(iIdx);
   return lpProfession->ushID;
}


CString QuestBook::GetFormuleNameReqByID(USHORT ushID)
{
   CString strName;
   ObjectStructure *lpObject = DynObjManager::GetRegisteredUnitID(ushID);
   if(lpObject)
      strName.Format("%s",IntlText::ParseString( lpObject->name, _DEFAULT_LNG ));
   else
      strName.Format("---");
   
   
   return strName;
}

bool QuestBook::GetFormuleNameReqIDISUnique(USHORT ushID)
{
   ObjectStructure *lpObject = DynObjManager::GetRegisteredUnitID(ushID);
   if(lpObject)
     return lpObject->uniqueItem;
   
   return false;
}

*/


