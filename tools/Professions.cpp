// Skills.cpp: implementation of the Skills class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TFC Server.h"
#include "TFC_MAIN.h"
#include "Professions.h"
#include "IntlText.h"
#include "Unit.h"
#include "PROFESSION_key.h"
#include "DynObjManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif




unsigned int  get_dword();
unsigned short get_word();
BYTE get_byte();
double get_double();
int get_long();
short get_short();
char* get_string();

CPtrArray Professions::c_aProfessions;

BYTE *g_pDataTmp = NULL;
char g_chLigne[2048];





BOOL Professions::IsFormuleExist(USHORT ushID)
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
BYTE Professions::IsFormuleLearnable(USHORT ushID, Unit *uLearner)
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


CString Professions::GetFormuleName(USHORT ushID)
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

USHORT Professions::GetFormuleSkin(USHORT ushID)
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


USHORT Professions::GetFormuleSkill(USHORT ushID)
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

sFormule * Professions::GetFormules(USHORT ushID)
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

BYTE Professions::GetFormulesProfession(USHORT ushID)
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


int Professions::GetNbrFormule()
{
   return c_aProfessions.GetSize();
}

int Professions::GetFormuleIDbyIndex(int iIdx)
{
   if(iIdx <0 || iIdx >= GetNbrFormule())
      return 0;

   sFormule *lpProfession;
   lpProfession = (sFormule *)c_aProfessions.GetAt(iIdx);
   return lpProfession->ushID;
}


CString Professions::GetFormuleNameReqByID(USHORT ushID)
{
   CString strName;
   ObjectStructure *lpObject = DynObjManager::GetRegisteredUnitID(ushID);
   if(lpObject)
      strName.Format("%s",IntlText::ParseString( lpObject->name, _DEFAULT_LNG ));
   else
      strName.Format("---");
   
   
   return strName;
}

bool Professions::GetFormuleNameReqIDISUnique(USHORT ushID)
{
   ObjectStructure *lpObject = DynObjManager::GetRegisteredUnitID(ushID);
   if(lpObject)
     return lpObject->uniqueItem;
   
   return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
void Professions::Create( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Creates the class
// 
//////////////////////////////////////////////////////////////////////////////////////////
{
   char strTmp[512];

   CString strFileP;
   strFileP = TFCMAIN::GetHomeDir();

   strFileP += "WDA\\NMS_Profession.dat";


   FILE *pf = NULL;
   fopen_s(&pf,strFileP,"rb");
   if(!pf)
   {
      _LOG_DEBUG
         LOG_CRIT_ERRORS,
            "NO profession file found %s",
         strFileP
         LOG_
         
         printf( "\r\nNO profession file found %s.",strFileP);
      return;
   }


   fseek(pf,0,SEEK_END);
   int dwTaille = ftell(pf);
   fseek(pf,0,SEEK_SET);
   // Alloue la memoire pour contenir le fichier

   BYTE *pData = NULL;

   pData = new BYTE[dwTaille];
   // lit le contenue du fichier
   fread(pData,dwTaille,1,pf);
   fclose(pf);

   for (UINT i=0; i<dwTaille; i++)
      pData[i] ^= PROFESSION_key[i%3418];


   //Read the datafiles...
   printf( "\n\n- Loading Profession File" );

   g_pDataTmp = pData;
   int uiNbrFormule = get_dword();
   if(uiNbrFormule >0)
   {
      for(int i=0;i<uiNbrFormule;i++)
      {
         sFormule *pFormuleData = new sFormule;
         pFormuleData->chProfession    = get_byte();
         pFormuleData->ushSkillLevel   = get_word();
         pFormuleData->ushItemResultID = get_word();
         pFormuleData->ushItemResultQty= get_word();
         pFormuleData->ushNbrRequestID = get_word();
         if(pFormuleData->ushNbrRequestID >0)
         {
            pFormuleData->pRequestItemList = new sRequestItem[pFormuleData->ushNbrRequestID];
            for(int n=0;n<pFormuleData->ushNbrRequestID;n++)
            {
               pFormuleData->pRequestItemList[n].ushRequestID = get_word();
               pFormuleData->pRequestItemList[n].ushQty       = get_word();
            }
         }
         else
         {
            pFormuleData->pRequestItemList = NULL;
         }

         sprintf_s(strTmp,512,"%s",get_string());
         pFormuleData->iFormuleCreateSize = strlen(strTmp);
         if(pFormuleData->iFormuleCreateSize >0)
         {
            pFormuleData->pstrFormuleCreate = new char[strlen(strTmp)+1];
            strcpy_s(pFormuleData->pstrFormuleCreate,strlen(strTmp)+1,strTmp);
         }
         else
            pFormuleData->pstrFormuleCreate = NULL;

         sprintf_s(strTmp,512,"%s",get_string());
         pFormuleData->iFormuleCompGagner = strlen(strTmp);
         if(pFormuleData->iFormuleCompGagner >0)
         {
            pFormuleData->pstrFormuleCompGagner = new char[strlen(strTmp)+1];
            strcpy_s(pFormuleData->pstrFormuleCompGagner,strlen(strTmp)+1,strTmp);
         }
         else
            pFormuleData->pstrFormuleCompGagner = NULL;

         pFormuleData->ushID    = get_word();
         pFormuleData->chEnable = get_byte();

         c_aProfessions.SetAtGrow(i, pFormuleData);
      }
   }

   printf( "\n  Found %d profession formule(s)",c_aProfessions.GetSize() );
   
   if(pData)
      delete []pData;
   pData = NULL;
}


//////////////////////////////////////////////////////////////////////////////////////////
void Professions::Destroy( void )
//////////////////////////////////////////////////////////////////////////////////////////
// Destroys skills
// 
//////////////////////////////////////////////////////////////////////////////////////////
{
	sFormule *lpProfession;
	int i;
	
   for(i = 0; i < c_aProfessions.GetSize(); i++)
   {
      lpProfession = (sFormule *)c_aProfessions.GetAt(i);
      if(lpProfession)
      {
         if(lpProfession->pRequestItemList)
            delete []lpProfession->pRequestItemList;
         lpProfession->pRequestItemList = NULL;
         
         if(lpProfession->pstrFormuleCreate)
            delete lpProfession->pstrFormuleCreate;
         lpProfession->pstrFormuleCreate = NULL;
         
         if(lpProfession->pstrFormuleCompGagner)
            delete lpProfession->pstrFormuleCompGagner;
         lpProfession->pstrFormuleCompGagner = NULL;
         
         delete lpProfession;
         lpProfession = NULL;
      }
   }
}



unsigned int get_dword()
{
   unsigned int val;
   
   val=*(unsigned int *)g_pDataTmp;	g_pDataTmp+=4;
   return val;
}

unsigned short get_word()
{
   unsigned short val;
   
   val=*(unsigned short *)g_pDataTmp;	g_pDataTmp+=2;
   return val;
}

BYTE get_byte()
{
   unsigned char val;
   
   val=*g_pDataTmp;	g_pDataTmp+=1;
   return val;
}

double get_double()
{
   double val;
   
   val=*(double *)g_pDataTmp;	g_pDataTmp+=8;
   return val;
}

int get_long()
{
   int val;
   
   val=*(int *)g_pDataTmp;	g_pDataTmp+=4;
   return val;
}

short get_short()
{
   short val;
   
   val=*(short *)g_pDataTmp;	g_pDataTmp+=2;
   return val;
}

char* get_string()
{
   int lg,i;
   
   lg=*(int *)g_pDataTmp;	g_pDataTmp+=4;
   for(i=0; i<lg; i++)
      g_chLigne[i]=*g_pDataTmp++;
   
   g_chLigne[i]=0;
   
   return (char *)g_chLigne;
}
