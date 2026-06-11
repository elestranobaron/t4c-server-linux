// WDAUtils.cpp: implementation of the CWDAUtils class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WDAUtils.h"
#include "WDA_Key.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWDAUtils::CWDAUtils()
{
   m_bWDAInit    = FALSE;
   m_pData       = NULL;
   m_psSpellList = NULL;
   m_dwNbrSpell  = 0;

   m_psHivesList = NULL;
   m_dwNbrHives  = 0;

   m_psItemList  = NULL;
   m_dwNbrItem   = 0;

   m_dwNbrItemPod  = 0;

   m_psCreature    = NULL;
   m_dwNbrCreature = 0;

   m_dwNbrMap = 0;


   m_pListMapPos      = NULL;
   m_pListItemPos     = NULL;
   m_pListCreaturePos = NULL;
   m_pListHivesPos    = NULL;
   m_pListAreaLinkPos = NULL;
   m_pListClanPos     = NULL;
   m_pchAllEndFileData= NULL;

   m_crTableau[ 0] = RGB(000,000,000); // Place Qu'on peu marcher (Noir)
   m_crTableau[ 1] = RGB(255,000,000); // Zone nom marcgable -Obstacle (Rouge)
   m_crTableau[ 2] = RGB(000,255,000); // Chez pas c KOI ;/ (Vert)
   m_crTableau[ 3] = RGB(255,255,000); // Eau peu pas marcher, peu pas tirer (jaune)
   m_crTableau[ 4] = RGB(000,000,255); // Eau peu pas marcher peu tirer (bleue)
   m_crTableau[ 5] = RGB(255,000,255); // Plancher Batiment PVP (Maganta)
   m_crTableau[ 6] = RGB(000,255,255); // Surface Marchable NON PVP (Turkoise)
   m_crTableau[ 7] = RGB(255,255,255); // Plancher Batiment NON PVP (Blanc)
   m_crTableau[ 8] = RGB(128,128,128); //
   m_crTableau[ 9] = RGB(000,128,128); //
   m_crTableau[10] = RGB(128,000,128); //
   m_crTableau[11] = RGB(000,000,128); //
   m_crTableau[12] = RGB(128,128,000); //
   m_crTableau[13] = RGB(000,128,000); //
   m_crTableau[14] = RGB(128,000,000); //
   m_crTableau[15] = RGB(255,128,255); //
}

CWDAUtils::~CWDAUtils()
{
   Free();
}

void CWDAUtils::SetWDAName(char*pstrName,int dwCrypt)
{
   m_strWDAName.Format("%s",pstrName);
   m_dwWDACrypt = dwCrypt;
}

void CWDAUtils::Initialize()
{
   Free();
   m_bWDAInit = LoadWDA();
}

void CWDAUtils::SaveUncomp(char*pstrName)
{
   if(m_strWDAName == "")
      return ;
   
   // Oubre le fichier WDA
   FILE	*hfile = NULL;
   fopen_s(&hfile,m_strWDAName,"rb");
   if(hfile)
   {
      fseek(hfile,0,SEEK_END);
      m_dwTaille = ftell(hfile);
      fseek(hfile,0,SEEK_SET);
      // Alloue la memoire pour contenir le fichier
      if(m_pData)
      {
         delete []m_pData;
         m_pData = NULL;
      }
      m_pData = new BYTE[m_dwTaille];
      // lit le contenue du fichier
      fread(m_pData,m_dwTaille,1,hfile);
      fclose(hfile);

      if(m_dwWDACrypt == 1)
      {
         //decode le fichier WDA      
         int i;
         for (i=0; i<m_dwTaille; i++)
            m_pData[i] ^= WDA_key[i%3418];
      }

      FILE *pf2 = NULL;
      fopen_s(&pf2,pstrName,"wb");
      if(pf2)
      {
         fwrite(m_pData,m_dwTaille,1,pf2);
         fclose(pf2);
      }
      Free();
   }
}

void CWDAUtils::Free()
{
   if(m_pData)
   {
      delete []m_pData;
      m_pData = NULL;
   }
   DeleteMapInfo();
   DeleteSpellList();
   DeleteHivesList();
   DeleteCreatureList();
   DeleteItemList();
   DeleteLinkArea();
   DeleteClanList();
   DeleteQuestList();
   DeleteAppearanceItem();
   DeleteAppearanceMonster();
   DeleteItemLocation();
   DeleteSpellIcon();
   DeleteVisualEffect();

   if(m_pchAllEndFileData)
      delete []m_pchAllEndFileData;

   m_bWDAInit    = FALSE;
   m_pData       = NULL;
   m_psSpellList = NULL;
   m_dwNbrSpell  = 0;

   m_psHivesList = NULL;
   m_dwNbrHives  = 0;

   m_psItemList  = NULL;
   m_dwNbrItem   = 0;

   m_dwNbrItemPod  = 0;

   m_psCreature    = NULL;
   m_dwNbrCreature = 0;

   m_dwNbrMap = 0;


   m_pListMapPos      = NULL;
   m_pListItemPos     = NULL;
   m_pListCreaturePos = NULL;
   m_pListHivesPos    = NULL;
   m_pListAreaLinkPos = NULL;
   m_pListClanPos     = NULL;
   m_pchAllEndFileData= NULL;
}

BOOL CWDAUtils::LoadWDA()
{
   if(m_strWDAName == "")
      return FALSE;
   // Oubre le fichier WDA
   FILE	*hfile = NULL;
   fopen_s(&hfile,m_strWDAName,"rb");
   if(hfile)
   {
      fseek(hfile,0,SEEK_END);
      m_dwTaille = ftell(hfile);
      fseek(hfile,0,SEEK_SET);
      // Alloue la memoire pour contenir le fichier
      if(m_pData)
      {
         delete []m_pData;
         m_pData = NULL;
      }
      m_pData = new BYTE[m_dwTaille];
      // lit le contenue du fichier
      fread(m_pData,m_dwTaille,1,hfile);
      fclose(hfile);

      if(m_dwWDACrypt == 1)
      {
         //decode le fichier WDA      
         int i;
         for (i=0; i<m_dwTaille; i++)
            m_pData[i] ^= WDA_key[i%3418];
      }

      // recup la pos des MAP...
      ReadListSpell();
      m_dwMapOldPos = (m_pDataTmp - m_pData);
      GetMapInfo();
      ReadListItems();
      ReadListCreature();
      ReadListHives();
      ReadListLinkArea();
      ReadListClan();
      ReadQuestFlag();
      m_chIsMiscWDAData     = get_byte();
      if(m_chIsMiscWDAData ==1) //nous avons des data MISC
      {
         ReadAppearanceItem();
         ReadAppearanceMonster();
         ReadItemLocation();
         ReadSpellIcon();
         ReadVisualEffect();
      }
      else
      {
         m_chIsMiscWDAData = 0;
         m_dwNbrAppearanceItem    = 0;
         m_dwNbrAppearanceMonster = 0;
         m_dwNbrItemLocation      = 0;
         m_dwNbrSpellIcon         = 0;
         m_dwNbrVisualEffect      = 0;
      }

      m_dwendFileData = m_dwTaille - (m_pDataTmp - m_pData);
      if(m_pchAllEndFileData)
         delete []m_pchAllEndFileData;
      if(m_dwendFileData > 0)
      {
         m_pchAllEndFileData = new BYTE[m_dwendFileData];
         memcpy(m_pchAllEndFileData,m_pDataTmp,m_dwendFileData);
         m_pDataTmp+= m_dwendFileData;
      }
      
      int dwTotal = m_pDataTmp-m_pData;
   }
   return TRUE;
}

int CWDAUtils::GetCollision(USHORT x, USHORT y, USHORT w)
{
	if (x<0 || x>=MAP_X_SIZE || y<0 || y>=MAP_Y_SIZE || w < 0 || w >= NBR_MAP_WORLD) 
      return -1;

	if (x%2 == 0)
		return ((*(m_sMapInfo[w].pMapData + ((UINT) y*MAP_X_SIZE + x)/2)) >> 4);
	else
		return ((*(m_sMapInfo[w].pMapData + ((UINT) y*MAP_Y_SIZE + x)/2)) & 0x0f);
}

void CWDAUtils::SetCollision(USHORT  x, USHORT  y, USHORT  w, BYTE value)
{
	if (x<0 || x>=MAP_X_SIZE || y<0 || y>=MAP_Y_SIZE || w < 0 || w >= NBR_MAP_WORLD) 
      return;

	if (x%2 == 0)
		*(m_sMapInfo[w].pMapData + ((UINT) y*MAP_X_SIZE + x)/2) = (value << 4) + (*(m_sMapInfo[w].pMapData + ((UINT) y*MAP_X_SIZE + x)/2) & 0x0f);
	else
		*(m_sMapInfo[w].pMapData + ((UINT) y*MAP_X_SIZE + x)/2) = value + (*(m_sMapInfo[w].pMapData + ((UINT) y*MAP_X_SIZE + x)/2) & 0xf0);
}
 
void CWDAUtils::GetMonsterHiveInfo(int dwIndex, char *pstrName,int iNameSize, int *pdwSkinID)
{
   if(dwIndex >=0 && dwIndex < m_dwNbrHives)
   {
      sprintf_s(pstrName,iNameSize,"%s",m_psHivesList[dwIndex].pstrLarva);
      *pdwSkinID = m_psHivesList[dwIndex].dwSkinID;
   }
}

void CWDAUtils::GetMonsterHiveInfo2(int dwIndex,unsigned short &ushID,CString &strName)
{
   if(dwIndex >=0 && dwIndex < m_dwNbrHives)
   {
      strName.Format("%s",m_psHivesList[dwIndex].pstrLarva);
      ushID = m_psHivesList[dwIndex].dwSkinID;
   }
}

void CWDAUtils::GetMonsterInfo2(int dwIndex,unsigned short &ushID,CString &strName)
{
   if(dwIndex >=0 && dwIndex < m_dwNbrCreature)
   {
      strName.Format("%s",m_psCreature[dwIndex].pstrCreatureID);
      ushID = m_psCreature[dwIndex].dwBindedID;
   }
}

void CWDAUtils::GetItemInfo(int dwIndex,char *pstrName,int iNameSize,int *pdwSkinID)
{
   if(dwIndex >=0 && dwIndex < m_dwNbrItem)
   {
      sprintf_s(pstrName,iNameSize,"%s",m_psItemList[dwIndex].pstrItemName);
      *pdwSkinID = m_psItemList[dwIndex].dwAppearance;
   }
}

void CWDAUtils::GetItemInfo2(int dwIndex,unsigned short &ushID,CString &strName)
{
   if(dwIndex >=0 && dwIndex < m_dwNbrItem)
   {
      strName.Format("%s",m_psItemList[dwIndex].pstrItemName);
      ushID = m_psItemList[dwIndex].dwBindedID;
   }
}

void CWDAUtils::GetSpellInfo2(int dwIndex,unsigned short &ushID,CString &strName)
{
   if(dwIndex >=0 && dwIndex < m_dwNbrSpell)
   {
      strName.Format("%s",m_psSpellList[dwIndex].pstrNom);
      ushID = m_psSpellList[dwIndex].dwID;
   }
}

void CWDAUtils::GetArealinkInfo(int dwIndex,USHORT &xS,USHORT &yS,USHORT &wS,USHORT &xD,USHORT &yD,USHORT &wD)
{
   if(dwIndex >=0 && dwIndex < m_sLinkArea.GetCount())
   {
      xS = m_sLinkArea[dwIndex].dwSrcX;
      yS = m_sLinkArea[dwIndex].dwSrcY;
      wS = m_sLinkArea[dwIndex].dwSrcZ;
      xD = m_sLinkArea[dwIndex].dwDesX;
      yD = m_sLinkArea[dwIndex].dwDesY;
      wD = m_sLinkArea[dwIndex].dwDesZ;
   }
}

void CWDAUtils::GetAreaLink(int dwIndex,int *dwSX,int *dwSY,int *dwSW,int *dwDX,int *dwDY,int *dwDW)
{
   if(dwIndex >=0 && dwIndex < m_sLinkArea.GetSize())
   {
      *dwSX = m_sLinkArea[dwIndex].dwSrcX;
      *dwSY = m_sLinkArea[dwIndex].dwSrcY;
      *dwSW = m_sLinkArea[dwIndex].dwSrcZ;
      *dwDX = m_sLinkArea[dwIndex].dwDesX;
      *dwDY = m_sLinkArea[dwIndex].dwDesY;
      *dwDW = m_sLinkArea[dwIndex].dwDesZ;
   }
}

void CWDAUtils::DeleteAreaLink(int dwSX,int dwSY,int dwSW,int dwDX,int dwDY,int dwDW)
{
   for(int i=0; i<m_sLinkArea.GetSize(); i++)
	{
      if(m_sLinkArea[i].dwSrcX == dwSX &&
         m_sLinkArea[i].dwSrcY == dwSY &&
         m_sLinkArea[i].dwSrcZ == dwSW &&
         m_sLinkArea[i].dwDesX == dwDX &&
         m_sLinkArea[i].dwDesY == dwDY &&
         m_sLinkArea[i].dwDesZ == dwDW     )
      {
         m_sLinkArea.RemoveAt(i);
         return;
      }
   }
}

void CWDAUtils::AddAreaLink(int dwSX,int dwSY,int dwSW,int dwDX,int dwDY,int dwDW)
{
   sLinkArea sNewArea;

   sNewArea.dwSrcX = dwSX;
   sNewArea.dwSrcY = dwSY;
   sNewArea.dwSrcZ = dwSW;
   sNewArea.dwDesX = dwDX;
   sNewArea.dwDesY = dwDY;
   sNewArea.dwDesZ = dwDW;
   m_sLinkArea.Add(sNewArea);
}

BOOL CWDAUtils::GetAreaLinkDest  (USHORT  x, USHORT  y, USHORT  w,USHORT  *px, USHORT  *py, USHORT  *pw)
{
   for(int j=0; j<m_sLinkArea.GetSize(); j++)
	{
      if(m_sLinkArea[j].dwSrcX == x && m_sLinkArea[j].dwSrcY == y && m_sLinkArea[j].dwSrcZ == w)
      {
         *px = m_sLinkArea[j].dwDesX;
         *py = m_sLinkArea[j].dwDesY;
         *pw = m_sLinkArea[j].dwDesZ;
         return TRUE;
      }
   }
   return FALSE;
}


BOOL CWDAUtils::GetAreaLinkSource(USHORT  x, USHORT  y, USHORT  w,USHORT  *px, USHORT  *py, USHORT  *pw)
{
   for(int j=0; j<m_sLinkArea.GetSize(); j++)
	{
      if(m_sLinkArea[j].dwDesX == x && m_sLinkArea[j].dwDesY == y && m_sLinkArea[j].dwDesZ == w)
      {
         *px = m_sLinkArea[j].dwSrcX;
         *py = m_sLinkArea[j].dwSrcY;
         *pw = m_sLinkArea[j].dwSrcZ;
         return TRUE;
      }
   }
   return FALSE;
}


void CWDAUtils::ReadListSpell()
{
   if(!m_pData)
      return;

   // remets le pointeur au debut du Buffer
   m_pDataTmp = m_pData;
   //Skip entete
	get_word();	get_word();	get_byte();

   int dwNbrSpell = get_dword();

   DeleteSpellList();

   m_dwNbrSpell = dwNbrSpell;

   // recupere toute la liste des Spell....
   m_psSpellList = new sSpell[m_dwNbrSpell];
   for(int i=0; i<m_dwNbrSpell; i++)
	{
      char strTmp[2048];

      // recupere le ID du Spell
      m_psSpellList[i].dwID = get_dword();
      
      // Recupere le Nom du Spell
      sprintf_s(strTmp,2048,"%s",get_string());
      m_psSpellList[i].dwNom = strlen(strTmp);
      if(m_psSpellList[i].dwNom >0)
      {
         m_psSpellList[i].pstrNom = new char[strlen(strTmp)+1];
         strcpy_s(m_psSpellList[i].pstrNom,strlen(strTmp)+1,strTmp);
      }
      else
         m_psSpellList[i].pstrNom = NULL;
      // recupere le Mantal Exhaust
      sprintf_s(strTmp,2048,"%s",get_string());
      m_psSpellList[i].dwMentalExhaust = strlen(strTmp);
      if(m_psSpellList[i].dwMentalExhaust > 0)
      {
        m_psSpellList[i].pstrMentalExhaust = new char[strlen(strTmp)+1];
        strcpy_s(m_psSpellList[i].pstrMentalExhaust,strlen(strTmp)+1,strTmp);
      }
      else
         m_psSpellList[i].pstrMentalExhaust = NULL; 
      // recupere le Move Exhaust
      sprintf_s(strTmp,2048,"%s",get_string());
      m_psSpellList[i].dwMovementExhaust = strlen(strTmp);
      if(m_psSpellList[i].dwMovementExhaust > 0)
      {
         m_psSpellList[i].pstrMovementExhaust = new char[strlen(strTmp)+1];
         strcpy_s(m_psSpellList[i].pstrMovementExhaust,strlen(strTmp)+1,strTmp);
      }
      else
         m_psSpellList[i].pstrMovementExhaust = NULL;
      // recupere le Attack Exhaust
      sprintf_s(strTmp,2048,"%s",get_string());
      m_psSpellList[i].dwAttackExhaust = strlen(strTmp);
      if(m_psSpellList[i].dwAttackExhaust > 0)
      {
         m_psSpellList[i].pstrAttackExhaust = new char[strlen(strTmp)+1];
         strcpy_s(m_psSpellList[i].pstrAttackExhaust,strlen(strTmp)+1,strTmp);
      }
      else
         m_psSpellList[i].pstrAttackExhaust = NULL;
      // recupere le strDuration
      sprintf_s(strTmp,2048,"%s",get_string());
      m_psSpellList[i].dwDuration = strlen(strTmp);
      if(m_psSpellList[i].dwDuration > 0)
      {
         m_psSpellList[i].pstrDuration = new char[strlen(strTmp)+1];
         strcpy_s(m_psSpellList[i].pstrDuration,strlen(strTmp)+1,strTmp);
      }
      else
         m_psSpellList[i].pstrDuration = NULL;
      // recupere le strTimerFrequency
      sprintf_s(strTmp,2048,"%s",get_string());
      m_psSpellList[i].dwTimerFrequency = strlen(strTmp);
      if(m_psSpellList[i].dwTimerFrequency > 0)
      {
         m_psSpellList[i].pstrTimerFrequency = new char[strlen(strTmp)+1];
         strcpy_s(m_psSpellList[i].pstrTimerFrequency,strlen(strTmp)+1,strTmp);
      }
      else
         m_psSpellList[i].pstrTimerFrequency = NULL;

      // le type du sort Eau, terre, etc etc
      m_psSpellList[i].dwElement = get_dword();
      // recupere le ManaCost
      sprintf_s(strTmp,2048,"%s",get_string());
      m_psSpellList[i].dwManaCost = strlen(strTmp);
      if(m_psSpellList[i].dwManaCost > 0)
      {
         m_psSpellList[i].pstrManaCost = new char[strlen(strTmp)+1];
         strcpy_s(m_psSpellList[i].pstrManaCost,strlen(strTmp)+1,strTmp);
      }
      else
         m_psSpellList[i].pstrManaCost = NULL;

      // Plein de parametres... le nom les decrit bien...
      m_psSpellList[i].dwAreaRange = get_dword();
      m_psSpellList[i].dwVisualEffect = get_dword();
      m_psSpellList[i].dwRangeVisualEffect = get_dword();
      m_psSpellList[i].dwTargetType = get_dword();
      m_psSpellList[i].dwAttackType = get_dword();
      m_psSpellList[i].dwMinWis = get_dword();
      m_psSpellList[i].dwMinInt = get_dword();
      m_psSpellList[i].dwMinLevel = get_dword();
      m_psSpellList[i].dwMinStr     = get_dword();
      m_psSpellList[i].dwMinEnd     = get_dword();
      m_psSpellList[i].dwMinAgi     = get_dword();
      m_psSpellList[i].dwMinAttack  = get_dword();
      m_psSpellList[i].dwMinArchery = get_dword();
      m_psSpellList[i].dwMinDodge   = get_dword();
      m_psSpellList[i].dwMinCS      = get_dword();
      m_psSpellList[i].chLineOfSight = get_byte();
      m_psSpellList[i].chSkillExclu = get_byte();
      m_psSpellList[i].chPVPcheck = get_byte();
      m_psSpellList[i].chFlag = get_byte();
      m_psSpellList[i].dwIcon = get_dword();

      if(m_psSpellList[i].dwID == 10783)
      {
         int caca = 1;
         m_psSpellList[i].dwTargetType = 2;
      }

      // recupere le Success pourcent
      sprintf_s(strTmp,2048,"%s",get_string());
      m_psSpellList[i].dwSuccessPercentage = strlen(strTmp);
      if(m_psSpellList[i].dwSuccessPercentage > 0)
      {
         m_psSpellList[i].pstrSuccessPercentage = new char[strlen(strTmp)+1];
         strcpy_s(m_psSpellList[i].pstrSuccessPercentage,strlen(strTmp)+1,strTmp);
      }
      else
         m_psSpellList[i].pstrSuccessPercentage = NULL;
      // recupere la Description
      sprintf_s(strTmp,2048,"%s",get_string());
      m_psSpellList[i].dwDescText = strlen(strTmp);
      if(m_psSpellList[i].dwDescText > 0)
      {
         m_psSpellList[i].pstrDescText = new char[strlen(strTmp)+1];
         strcpy_s(m_psSpellList[i].pstrDescText,strlen(strTmp)+1,strTmp);
      }
      else
         m_psSpellList[i].pstrDescText = NULL;

      // recupere le nombre de spell effect
      int dwNbrSpellEffect = get_dword();
      // Alloue le nombre de SpellEffectNM...
      for(int nm=0;nm<dwNbrSpellEffect;nm++)
      {
         SpellEffectNM *pNewEffect = new SpellEffectNM;
         m_psSpellList[i].SpellE.Add(pNewEffect);
      }

      int dwTmp1 = dwNbrSpellEffect;
      dwTmp1--;
		for(;dwTmp1>=0; dwTmp1--)
		{
         // recupere l'effect ID
         m_psSpellList[i].SpellE[dwTmp1]->dwEffect = get_dword();
         // recupere le nombre de parametre pour l'effect
         m_psSpellList[i].SpellE[dwTmp1]->dwNbrEffectParam = get_dword();
         if(m_psSpellList[i].SpellE[dwTmp1]->dwNbrEffectParam > 0)
         {
            // Alloue l'espace pour les parametres
            m_psSpellList[i].SpellE[dwTmp1]->pSpellEffectP = new SpellEffectParam[m_psSpellList[i].SpellE[dwTmp1]->dwNbrEffectParam];
         }
         else
            m_psSpellList[i].SpellE[dwTmp1]->pSpellEffectP = NULL; 
         int dwTmp2 = m_psSpellList[i].SpellE[dwTmp1]->dwNbrEffectParam;
         dwTmp2--;
			for(;dwTmp2>=0; dwTmp2--)
			{
            // recupere le paramID (1-2-3-4)
            m_psSpellList[i].SpellE[dwTmp1]->pSpellEffectP[dwTmp2].dwParam1 = get_dword();
            // recupere la Description
            sprintf_s(strTmp,2048,"%s",get_string());
            m_psSpellList[i].SpellE[dwTmp1]->pSpellEffectP[dwTmp2].dwParam2 = strlen(strTmp);
            if(m_psSpellList[i].SpellE[dwTmp1]->pSpellEffectP[dwTmp2].dwParam2 > 0)
            {
               m_psSpellList[i].SpellE[dwTmp1]->pSpellEffectP[dwTmp2].pstrParam2 = new char[strlen(strTmp)+1];
               strcpy_s(m_psSpellList[i].SpellE[dwTmp1]->pSpellEffectP[dwTmp2].pstrParam2,strlen(strTmp)+1,strTmp);
            }
            else
               m_psSpellList[i].SpellE[dwTmp1]->pSpellEffectP[dwTmp2].pstrParam2 = NULL;

			}
		}

      // Recupere les spell Requirement
      m_psSpellList[i].dwNbrSpellReq = get_dword();
      if(m_psSpellList[i].dwNbrSpellReq > 0)
      {
         // Alloue l'espace pour les spells requirement
         m_psSpellList[i].pdwSpellReq = new int[m_psSpellList[i].dwNbrSpellReq];
      }
      else
         m_psSpellList[i].pdwSpellReq = NULL;

		dwTmp1 = m_psSpellList[i].dwNbrSpellReq;
      dwTmp1--;
		for(;dwTmp1>=0; dwTmp1--)
      {
         m_psSpellList[i].pdwSpellReq[dwTmp1] = get_dword();
      }
	}
}

void CWDAUtils::DeleteSpellList()
{
   if(m_psSpellList)
   {
      for(int i=0;i<m_dwNbrSpell;i++)
      {
         // Efface 
         if(m_psSpellList[i].pstrNom)
            delete m_psSpellList[i].pstrNom;
         if(m_psSpellList[i].pstrMentalExhaust)
            delete m_psSpellList[i].pstrMentalExhaust;
         if(m_psSpellList[i].pstrMovementExhaust)
            delete m_psSpellList[i].pstrMovementExhaust;
         if(m_psSpellList[i].pstrAttackExhaust)
            delete m_psSpellList[i].pstrAttackExhaust;
         if(m_psSpellList[i].pstrDuration)
            delete m_psSpellList[i].pstrDuration;
         if(m_psSpellList[i].pstrTimerFrequency)
	         delete m_psSpellList[i].pstrTimerFrequency;
         if(m_psSpellList[i].pstrManaCost)
	         delete m_psSpellList[i].pstrManaCost;
         if(m_psSpellList[i].pstrSuccessPercentage)
	         delete m_psSpellList[i].pstrSuccessPercentage;
         if(m_psSpellList[i].pstrDescText)
	         delete m_psSpellList[i].pstrDescText;
         if(m_psSpellList[i].pdwSpellReq)
            delete m_psSpellList[i].pdwSpellReq;

         SpellEffectNM *pNewEffect;
         while( m_psSpellList[i].SpellE.GetSize() > 0 ) 
         {
		      pNewEffect = m_psSpellList[i].SpellE[0];
            for(int k=0;k<pNewEffect->dwNbrEffectParam;k++)
            {
               if(pNewEffect->pSpellEffectP[k].pstrParam2)
                  delete pNewEffect->pSpellEffectP[k].pstrParam2;
            }
            if(pNewEffect->pSpellEffectP)
               delete pNewEffect->pSpellEffectP;

            m_psSpellList[i].SpellE.RemoveAt(0);
		      delete pNewEffect;
	      }
         m_psSpellList[i].SpellE.RemoveAll();
         m_psSpellList[i].SpellE.FreeExtra();
      }
      if(m_psSpellList)
         delete []m_psSpellList;

      m_psSpellList = NULL;
      m_dwNbrSpell = 0;
   }
}

int  CWDAUtils::CalculeSpellSize()
{
   int dwSize = 0;
   dwSize+=4; // le nombre de Spell
   if(m_psSpellList)
   {
      for(int i=0;i<m_dwNbrSpell;i++)
      {
         dwSize+=4; // le ID Du Spell
         dwSize+=m_psSpellList[i].dwNom; // le nom du spell
         dwSize+=m_psSpellList[i].dwMentalExhaust;   // le MantalExaust
         dwSize+=m_psSpellList[i].dwMovementExhaust; // le MoveExaust
         dwSize+=m_psSpellList[i].dwAttackExhaust;   // le AttackExaust
         dwSize+=m_psSpellList[i].dwDuration;        // duration
         dwSize+=m_psSpellList[i].dwTimerFrequency;  // Frequence du timer
         dwSize+=4; // Le type du sort (element) eau, terre, etc
         dwSize+=m_psSpellList[i].dwManaCost;        // cb de mana sa coute

         dwSize+=4; // le dwAreaRange
         dwSize+=4; // le dwVisualEffect
         dwSize+=4; // le dwRangeVisualEffect
         dwSize+=4; // le dwTargetType
         dwSize+=4; // le dwAttackType
         dwSize+=4; // le dwMinWis
         dwSize+=4; // le dwMinInt
         dwSize+=4; // le dwMinLevel
         dwSize+=4; // le dwMinStr
         dwSize+=4; // le dwMinEnd
         dwSize+=4; // le dwMinAgi
         dwSize+=4; // le dwMinAttack
         dwSize+=4; // le dwMinArchery
         dwSize+=4; // le dwMinDodge
         dwSize+=4; // le dwMinCS
         dwSize+=1; // le chLineOfSight
         dwSize+=1; // le chSkillExclu
         dwSize+=1; // le chPVPcheck
         dwSize+=1; // le chFlag
         dwSize+=4; // le dwIcon

         dwSize+=m_psSpellList[i].dwSuccessPercentage; //
         dwSize+=m_psSpellList[i].dwDescText; //

         dwSize+=4; // le Nombre de Spell Effect

         dwSize+= (9*4); // contient le INT qui contient cb chaque string ets longue

         for(int j=0;j<m_psSpellList[i].SpellE.GetSize();j++)
         {
            dwSize+=4; // l'effect ID
            dwSize+=4; // le nombre de pAram de l'effect
            for(int k=0;k<m_psSpellList[i].SpellE[j]->dwNbrEffectParam;k++)
            {
               dwSize+=4; // le param ID
               dwSize+=4; // le Length de la string
               dwSize+=m_psSpellList[i].SpellE[j]->pSpellEffectP[k].dwParam2; //
            }
         }
         dwSize+=4; // le nombre de Spell Requirement
		   for(int k=0;k<m_psSpellList[i].dwNbrSpellReq;k++)
         {
            dwSize+=4; // le ID DU Spell Req
         }
      }
   }

   return dwSize;
}

void CWDAUtils::GetMapInfo()
{
   m_pListMapPos = m_pDataTmp;
	m_dwNbrMap=get_dword();

   int iNew ;
	for(int i=0; i<m_dwNbrMap; i++)
	{
      iNew = i;

      /*
      //reordonne les maps....
      if(i == 0)
         iNew = 1;
      else if(i == 1)
         iNew = 2;
      else if(i == 2)
         iNew = 3;
      else if(i == 3)
         iNew = 0;
         */
      


      m_sMapInfo[iNew].shID = get_word();
      sprintf_s(m_sMapInfo[iNew].strMapName,50,"%s",get_string());
      
      TRACE("%s\n",m_sMapInfo[iNew].strMapName);

      m_sMapInfo[iNew].shLongeur = get_word();
      m_sMapInfo[iNew].shHauteur = get_word();

      m_sMapInfo[iNew].pMapData = m_pDataTmp;
      m_sMapInfo[iNew].pMapDataUndo = new BYTE[(m_sMapInfo[iNew].shLongeur*m_sMapInfo[iNew].shHauteur)/2];
      memcpy(m_sMapInfo[iNew].pMapDataUndo,m_sMapInfo[iNew].pMapData,((m_sMapInfo[iNew].shLongeur*m_sMapInfo[iNew].shHauteur)/2));
      m_pDataTmp+=(m_sMapInfo[iNew].shLongeur*m_sMapInfo[iNew].shHauteur)/2;
      m_sMapInfo[iNew].bMapUndoPossible = FALSE;
	}
}

void CWDAUtils::DeleteMapInfo()
{
   for(int i=0; i<m_dwNbrMap; i++)
	{
      if(m_sMapInfo[i].pMapDataUndo)
         delete []m_sMapInfo[i].pMapDataUndo;
      m_sMapInfo[i].pMapDataUndo = NULL;
	}
}

int CWDAUtils::CalculeMapSize()
{
   int dwSize = 4;
   for(int i=0; i<m_dwNbrMap; i++)
	{
      dwSize +=2;
      dwSize +=4;
      dwSize +=strlen(m_sMapInfo[i].strMapName);
      dwSize +=2;
      dwSize +=2;
      dwSize +=((m_sMapInfo[i].shLongeur*m_sMapInfo[i].shHauteur)/2);
	}
   return dwSize;
}

void CWDAUtils::ReadListItems()
{
   char strTmp[1000];

   if(!m_pData)
      return;

   // remets le pointeur au debut du Buffer
   m_pListItemPos = m_pDataTmp;

   int dwNbrItem = get_dword();
   DeleteItemList();
   m_dwNbrItem = dwNbrItem;

   // recupere toute la liste des Items....
   m_psItemList = new sItems[m_dwNbrItem];
	for(int i=0; i<m_dwNbrItem; i++)
	{
      // Recupere string
      sprintf_s(strTmp,1000,"%s",get_string());
      m_psItemList[i].dwItemName = strlen(strTmp);
      if(m_psItemList[i].dwItemName >0)
      {
         m_psItemList[i].pstrItemName = new char[strlen(strTmp)+1];
         strcpy_s(m_psItemList[i].pstrItemName,strlen(strTmp)+1,strTmp);
      }
      else
         m_psItemList[i].pstrItemName = NULL;

      //if(strcmp(m_psItemList[i].pstrItemName,"Sign 12") == 0)
      //   TRACE("**** [ %s ]\n",m_psItemList[i].pstrItemName);

      m_psItemList[i].dwBindedID = get_dword();
      m_psItemList[i].dwStructID = get_dword();
      // Recupere string
      sprintf_s(strTmp,1000,"%s",get_string());
      m_psItemList[i].dwName = strlen(strTmp);
      if(m_psItemList[i].dwName >0)
      {
         m_psItemList[i].pstrName = new char[strlen(strTmp)+1];
         strcpy_s(m_psItemList[i].pstrName,strlen(strTmp)+1,strTmp);
      }
      else
         m_psItemList[i].pstrName = NULL;
      m_psItemList[i].dwAppearance = get_dword();
      m_psItemList[i].dwSellType = get_dword();
      m_psItemList[i].dwEquipPos = get_dword();
      m_psItemList[i].dwBuyFlagID = get_dword();
      m_psItemList[i].dwSellPrice = get_dword();
      sprintf_s(strTmp,1000,"%s",get_string());
      m_psItemList[i].dwStrSellPrice = strlen(strTmp);
      if(m_psItemList[i].dwStrSellPrice >0)
      {
         m_psItemList[i].pstrSellPrice = new char[strlen(strTmp)+1];
         strcpy_s(m_psItemList[i].pstrSellPrice,strlen(strTmp)+1,strTmp);
      }
      else
         m_psItemList[i].pstrSellPrice = NULL;
      m_psItemList[i].dwSize = get_dword();
      m_psItemList[i].dArmorAC = get_double();
      m_psItemList[i].dParadePC = get_double();
      m_psItemList[i].dwArmorDodgeMalus = get_dword();
      m_psItemList[i].dwArmorMinEnd = get_dword();
      // Recupere string
      sprintf_s(strTmp,1000,"%s",get_string());
      m_psItemList[i].dwWeaponDmgRool = strlen(strTmp);
      if(m_psItemList[i].dwWeaponDmgRool >0)
      {
         m_psItemList[i].pstrWeaponDmgRool = new char[strlen(strTmp)+1];
         strcpy_s(m_psItemList[i].pstrWeaponDmgRool,strlen(strTmp)+1,strTmp);
      }
      else
         m_psItemList[i].pstrWeaponDmgRool = NULL;
      m_psItemList[i].dwWeaponAttack = get_dword();
      m_psItemList[i].dwWeaponStr = get_dword();
      m_psItemList[i].dwWeaponAgi = get_dword();
      // Recupere string
      sprintf_s(strTmp,1000,"%s",get_string());
      m_psItemList[i].dwLockKeyID = strlen(strTmp);
      if(m_psItemList[i].dwLockKeyID >0)
      {
         m_psItemList[i].pstrLockKeyID = new char[strlen(strTmp)+1];
         strcpy_s(m_psItemList[i].pstrLockKeyID,strlen(strTmp)+1,strTmp);
      }
      else
         m_psItemList[i].pstrLockKeyID = NULL;
      m_psItemList[i].dwLockDifficulty = get_dword();
      // Recupere string
      sprintf_s(strTmp,1000,"%s",get_string());
      m_psItemList[i].dwBookText = strlen(strTmp);
      if(m_psItemList[i].dwBookText >0)
      {
         m_psItemList[i].pstrBookText = new char[strlen(strTmp)+1];
         strcpy_s(m_psItemList[i].pstrBookText,strlen(strTmp)+1,strTmp);
      }
      else
         m_psItemList[i].pstrBookText = NULL;
      m_psItemList[i].dwContainerGold = get_dword();
      m_psItemList[i].dwContainerGlobalRespawn = get_dword();
      m_psItemList[i].dwContainerUserRespawn = get_dword();
      // Recupere string
      sprintf_s(strTmp,1000,"%s",get_string());
      m_psItemList[i].dwExhaust = strlen(strTmp);
      if(m_psItemList[i].dwExhaust >0)
      {
         m_psItemList[i].pstrExhaust = new char[strlen(strTmp)+1];
         strcpy_s(m_psItemList[i].pstrExhaust,strlen(strTmp)+1,strTmp);
      }
      else
         m_psItemList[i].pstrExhaust = NULL;
      m_psItemList[i].dwRadiance = get_dword();
      m_psItemList[i].dwCharges = get_long();
      m_psItemList[i].dwMinInt = get_dword();
      m_psItemList[i].dwMinWis = get_dword();
      m_psItemList[i].dwIntlID = get_dword();
      m_psItemList[i].dwDropFlags = get_dword();
      m_psItemList[i].chUnique = get_byte();
      // Recupere string
      sprintf_s(strTmp,1000,"%s",get_string());
      m_psItemList[i].dwGmItemLocation = strlen(strTmp);
      if(m_psItemList[i].dwGmItemLocation >0)
      {
         m_psItemList[i].pstrGmItemLocation = new char[strlen(strTmp)+1];
         strcpy_s(m_psItemList[i].pstrGmItemLocation,strlen(strTmp)+1,strTmp);
      }
      else
         m_psItemList[i].pstrGmItemLocation = NULL;
      m_psItemList[i].chCanSummon = get_byte();
      m_psItemList[i].chFlag1 = get_byte();
      m_psItemList[i].chFlag2 = get_byte();

      int dwNbrContainerList = get_dword();
      for(int nm=0;nm<dwNbrContainerList;nm++)
      {
         ItemContainerGrp *pNewContainList = new ItemContainerGrp;
         m_psItemList[i].aItemContainer.Add(pNewContainList);
      }

		int nb2=dwNbrContainerList;
      nb2--;
		for(;nb2>=0; nb2--)
		{
         int dwNbrContainerI = get_dword();
         for(int nm=0;nm<dwNbrContainerI;nm++)
         {
            ItemContaineri *pNewContainI = new ItemContaineri;
            m_psItemList[i].aItemContainer[nb2]->aItemItem.Add(pNewContainI);
         }

			int nb3=dwNbrContainerI;
         nb3--;
			for(;nb3>=0; nb3--)
			{
            // Recupere string
            sprintf_s(strTmp,1000,"%s",get_string());
            m_psItemList[i].aItemContainer[nb2]->aItemItem[nb3]->dwNbrItemName = strlen(strTmp);
            if(m_psItemList[i].aItemContainer[nb2]->aItemItem[nb3]->dwNbrItemName >0)
            {
               m_psItemList[i].aItemContainer[nb2]->aItemItem[nb3]->pstrItemName = new char[strlen(strTmp)+1];
               strcpy_s(m_psItemList[i].aItemContainer[nb2]->aItemItem[nb3]->pstrItemName,strlen(strTmp)+1,strTmp);
            }
            else
               m_psItemList[i].aItemContainer[nb2]->aItemItem[nb3]->pstrItemName = NULL;
			}
		}

      int dwNbrBoost = get_dword();
      for(int nm=0;nm<dwNbrBoost;nm++)
      {
         ItemBoost *pNewBoost = new ItemBoost;
         m_psItemList[i].aItemBoost.Add(pNewBoost);
      }
		nb2=dwNbrBoost;
      nb2--;
		for(;nb2>=0; nb2--)
		{
         m_psItemList[i].aItemBoost[nb2]->dwBoost = get_dword();
         m_psItemList[i].aItemBoost[nb2]->dwStat = get_dword();

         // Recupere string
         sprintf_s(strTmp,1000,"%s",get_string());
         m_psItemList[i].aItemBoost[nb2]->dwExhaust = strlen(strTmp);
         if(m_psItemList[i].aItemBoost[nb2]->dwExhaust >0)
         {
            m_psItemList[i].aItemBoost[nb2]->pstrExhaust = new char[strlen(strTmp)+1];
            strcpy_s(m_psItemList[i].aItemBoost[nb2]->pstrExhaust,strlen(strTmp)+1,strTmp);
         }
         else
            m_psItemList[i].aItemBoost[nb2]->pstrExhaust = NULL;

         m_psItemList[i].aItemBoost[nb2]->dwMinInt = get_dword();
         m_psItemList[i].aItemBoost[nb2]->dwMinWis = get_dword();
		}

      int dwNbrSpell = get_dword();
      for(int nm=0;nm<dwNbrSpell;nm++)
      {
         ItemSpell *pNewSpell = new ItemSpell;
         m_psItemList[i].aItemSpell.Add(pNewSpell);
      }
		nb2=dwNbrSpell;
      nb2--;
		for(;nb2>=0; nb2--)
		{
         m_psItemList[i].aItemSpell[nb2]->dwSpell = get_dword();
         m_psItemList[i].aItemSpell[nb2]->dwInstilled = get_dword();
         m_psItemList[i].aItemSpell[nb2]->dwLevel = get_dword();
		}
	}

   m_dwNbrItemPod = get_dword();
   // recupere toute la liste des Items....
	for(int i=0; i<m_dwNbrItemPod; i++)
	{
      sItemsPod sNewItemPod;
      // Recupere string
      sprintf_s(strTmp,1000,"%s",get_string());
      sNewItemPod.dwItemName = strlen(strTmp);
      if(sNewItemPod.dwItemName >0)
      {
         sNewItemPod.pstrItemName = new char[strlen(strTmp)+1];
         strcpy_s(sNewItemPod.pstrItemName,strlen(strTmp)+1,strTmp);
      }
      else
         sNewItemPod.pstrItemName = NULL;

      sNewItemPod.dwX = get_long();
      sNewItemPod.dwY = get_long();
      sNewItemPod.dwZ = get_long();
      int dwX = sNewItemPod.dwX;
      int dwY = sNewItemPod.dwY;
      int dwW = sNewItemPod.dwZ;

      // compare pour inserer entre 2 item... on tree en ordre croissant
      int bADD = FALSE;
      for(int j=0;j<m_psItemPodList2.GetSize();j++)
      {
         if(strcmp(sNewItemPod.pstrItemName,m_psItemPodList2[j].pstrItemName)<0)
         {
            m_psItemPodList2.InsertAt(j,sNewItemPod);  
            bADD = TRUE;
            j = m_psItemPodList2.GetSize()+10;
         }
      }
      if(!bADD)
         m_psItemPodList2.Add(sNewItemPod);
	}
}

void CWDAUtils::DeleteItemList()
{
   if(m_psItemList)
   {
      for(int i=0; i<m_dwNbrItem; i++)
	   {
         if(m_psItemList[i].pstrItemName)
            delete m_psItemList[i].pstrItemName;
         if(m_psItemList[i].pstrName)
            delete m_psItemList[i].pstrName;
         if(m_psItemList[i].pstrWeaponDmgRool)
            delete m_psItemList[i].pstrWeaponDmgRool;
         if(m_psItemList[i].pstrLockKeyID)
            delete m_psItemList[i].pstrLockKeyID;
         if(m_psItemList[i].pstrBookText)
            delete m_psItemList[i].pstrBookText;
         if(m_psItemList[i].pstrExhaust)
            delete m_psItemList[i].pstrExhaust;
         if(m_psItemList[i].pstrGmItemLocation)
            delete m_psItemList[i].pstrGmItemLocation;

         ItemContainerGrp *pNewContaie;
         while( m_psItemList[i].aItemContainer.GetSize() > 0 ) 
         {
		      pNewContaie = m_psItemList[i].aItemContainer[0];

            ItemContaineri *pNewConI;
            while( pNewContaie->aItemItem.GetSize() > 0 ) 
            {
		         pNewConI = pNewContaie->aItemItem[0];
               if(pNewConI->pstrItemName)
                  delete pNewConI->pstrItemName;
               pNewContaie->aItemItem.RemoveAt(0);
		         delete pNewConI;
	         }
            pNewContaie->aItemItem.RemoveAll();
            pNewContaie->aItemItem.FreeExtra();
            m_psItemList[i].aItemContainer.RemoveAt(0);
		      delete pNewContaie;
	      }
         m_psItemList[i].aItemContainer.RemoveAll();
         m_psItemList[i].aItemContainer.FreeExtra();


         ItemBoost *pNewB;
         while( m_psItemList[i].aItemBoost.GetSize() > 0 ) 
         {
		      pNewB = m_psItemList[i].aItemBoost[0];
            if(pNewB->pstrExhaust)
               delete pNewB->pstrExhaust;
            m_psItemList[i].aItemBoost.RemoveAt(0);
		      delete pNewB;
	      }
         m_psItemList[i].aItemBoost.RemoveAll();
         m_psItemList[i].aItemBoost.FreeExtra();

         ItemSpell *pNewS;
         while( m_psItemList[i].aItemSpell.GetSize() > 0 ) 
         {
		      pNewS = m_psItemList[i].aItemSpell[0];
            m_psItemList[i].aItemSpell.RemoveAt(0);
		      delete pNewS;
	      }
         m_psItemList[i].aItemSpell.RemoveAll();
         m_psItemList[i].aItemSpell.FreeExtra();
	   }

      if(m_psItemList)
         delete []m_psItemList;

      m_psItemList = NULL;
      m_dwNbrItem = 0;
   }

   while( m_psItemPodList2.GetSize() > 0 ) 
   {
      if(m_psItemPodList2[0].pstrItemName)
         delete m_psItemPodList2[0].pstrItemName;
      m_psItemPodList2.RemoveAt(0);
	}
   m_psItemPodList2.RemoveAll();
   m_psItemPodList2.FreeExtra();
}

int  CWDAUtils::CalculeItemSize()
{
   int dwSize = 0;
   dwSize+=4; // le nombre D'item
   if(m_psItemList)
   {
      for(int i=0;i<m_dwNbrItem;i++)
      {
         dwSize+=m_psItemList[i].dwItemName;
         dwSize+=4;//BindedID
         dwSize+=4;//StructID
         dwSize+=m_psItemList[i].dwName;
         dwSize+=4;//dwAppearance
         dwSize+=4;//dwSellType
         dwSize+=4;//dwEquipPos
         dwSize+=4;//dwBuyFlagID;
         dwSize+=4;//dwSellPrice
         dwSize+=4;//dwStrSellPrice
         dwSize+=m_psItemList[i].dwStrSellPrice;
         dwSize+=4;//dwSize
         dwSize+=8;//dArmorAC
         dwSize+=8;//dParadePC
         dwSize+=4;//dwArmorDodgeMalus
         dwSize+=4;//dwArmorMinEnd
         dwSize+=m_psItemList[i].dwWeaponDmgRool;
         dwSize+=4;//dwWeaponAttack
         dwSize+=4;//dwWeaponStr
         dwSize+=4;//dwWeaponAgi
         dwSize+=m_psItemList[i].dwLockKeyID;
         dwSize+=4;//dwLockDifficulty
         dwSize+=m_psItemList[i].dwBookText;
         dwSize+=4;//dwContainerGold
         dwSize+=4;//dwContainerGlobalRespawn
         dwSize+=4;//dwContainerUserRespawn
         dwSize+=m_psItemList[i].dwExhaust;

         dwSize+=4;//dwRadiance
         dwSize+=4;//dwCharges
         dwSize+=4;//dwMinInt
         dwSize+=4;//dwMinWis
         dwSize+=4;//dwIntlID
         dwSize+=4;//dwDropFlags
         dwSize+=1;//chUnique
         dwSize+=m_psItemList[i].dwGmItemLocation;
         dwSize+=1;//chCanSummon
         dwSize+=1;//chFlag1
         dwSize+=1;//chFlag2
         dwSize+=(7*4); // contient les bytes du nbr e char de chaque str

         dwSize+=4;//dwNbrContainner list
         for(int j=0;j<m_psItemList[i].aItemContainer.GetSize();j++)
         {
            dwSize+=4;//dwNbrContainner Item
            for(int k=0;k<m_psItemList[i].aItemContainer[j]->aItemItem.GetSize();k++)
            {
               dwSize+=4;//dwNbrContainner Item
               dwSize+=m_psItemList[i].aItemContainer[j]->aItemItem[k]->dwNbrItemName;
            }
         }

         dwSize+=4;//dwNbrBoost
         for(int j=0;j<m_psItemList[i].aItemBoost.GetSize();j++)
         {
            dwSize+=4;//dwBoost
            dwSize+=4;//dwStat
            dwSize+=4;//dwExhaust
            dwSize+=m_psItemList[i].aItemBoost[j]->dwExhaust;
            dwSize+=4;//dwMinInt
            dwSize+=4;//dwMinWis
         }

         dwSize+=4;//dwNbrSpell
         for(int j=0;j<m_psItemList[i].aItemSpell.GetSize();j++)
         {
            dwSize+=4;//dwSpell
            dwSize+=4;//dwInstilled
            dwSize+=4;//dwLevel
         }
      }
   }

   dwSize+=4; // le nombre D'item Pod

   for(int i=0;i<m_psItemPodList2.GetSize();i++)
   {
      dwSize+=4;//dwItemName
      dwSize+=m_psItemPodList2[i].dwItemName;
      dwSize+=4;//dwX
      dwSize+=4;//dwY
      dwSize+=4;//dwZ
   }

   return dwSize;
}

void CWDAUtils::ReadListCreature()
{
   char strTmp[512];

	m_pListCreaturePos = m_pDataTmp;
   int dwNbrCreature = get_dword();
   DeleteCreatureList();
   m_dwNbrCreature = dwNbrCreature;
   m_psCreature = new sCreature[m_dwNbrCreature];

	for(int i=0; i<m_dwNbrCreature; i++)
	{
      m_psCreature[i].dwBindedID = get_dword();  // BindedID
      // recupere le Creature ID
      sprintf_s(strTmp,512,"%s",get_string());
      m_psCreature[i].dwNbrCreatureID = strlen(strTmp);
      if(m_psCreature[i].dwNbrCreatureID > 0)
      {
         m_psCreature[i].pstrCreatureID = new char[strlen(strTmp)+1];
         strcpy_s(m_psCreature[i].pstrCreatureID,strlen(strTmp)+1,strTmp);
      }
      else
         m_psCreature[i].pstrCreatureID = NULL;
      // recupere le Creature Name
      sprintf_s(strTmp,512,"%s",get_string());
      m_psCreature[i].dwNbrName = strlen(strTmp);
      if(m_psCreature[i].dwNbrName > 0)
      {
         m_psCreature[i].pstrName = new char[strlen(strTmp)+1];
         strcpy_s(m_psCreature[i].pstrName,strlen(strTmp)+1,strTmp);
      }
      else
         m_psCreature[i].pstrName = NULL;

      m_psCreature[i].dwEnd = get_dword();  // END
      m_psCreature[i].dwAgi = get_dword();  // AGI
      m_psCreature[i].dwInt = get_dword();  // INT
      m_psCreature[i].dwVal1 = get_long();   // Value(?)
      m_psCreature[i].dwVal2 = get_long();   // Value(?)
      m_psCreature[i].dwVal3 = get_long();   // Value(?)
      m_psCreature[i].dwVal4 = get_long();   // Value(?)
      m_psCreature[i].dwAirResist = get_dword();  // AirResist
      m_psCreature[i].dwEarthResist = get_dword();  // EarthResist
      m_psCreature[i].dwWaterResist = get_dword();  // WaterResist
      m_psCreature[i].dwFireResist = get_dword();  // FireResist
      m_psCreature[i].dwDarkResist = get_dword();  // DarkResist
      m_psCreature[i].dwLightResist = get_dword();  // LightResist
      m_psCreature[i].dwAirPower = get_dword();  // AirPower
      m_psCreature[i].dwEarthPower = get_dword();  // EarthPower
      m_psCreature[i].dwWaterPower = get_dword();  // WaterPower
      m_psCreature[i].dwFirePower = get_dword();  // FirePower
      m_psCreature[i].dwDarkPower = get_dword();  // DarkPower
      m_psCreature[i].dwLightPower =get_dword();  // LightPower
      m_psCreature[i].dwLVL = get_dword();  // LVL
      m_psCreature[i].dwHP = get_dword();  // HP
      m_psCreature[i].dwDodgeSkill = get_dword();  // DodgeSkill
      m_psCreature[i].dAc = get_double(); // AC
      m_psCreature[i].dwAppearance    = get_dword();  // Appearance
      m_psCreature[i].dwDressBody     = get_dword();  // DressBody
      m_psCreature[i].dwDressFeet     = get_dword();  // DressFeet
      m_psCreature[i].dwDressGloves   = get_dword();  // DressGloves
      m_psCreature[i].dwDressHelm     = get_dword();  // DressHelm
      m_psCreature[i].dwDressLegs     = get_dword();  // DressLegs
      m_psCreature[i].dwDressWeapon   = get_dword();  // DressWeapon
      m_psCreature[i].dwDressShield   = get_dword();  // DressShield
      m_psCreature[i].dwDressCape     = get_dword();  // DressCape
      m_psCreature[i].dwAggressivness = get_long();   // Aggressivness
      m_psCreature[i].dwClanID        = get_dword();  // ClanID
      m_psCreature[i].dwSpeed         = get_dword();  // Speed
      m_psCreature[i].dXPperHit   = get_double(); // XPperHit
      m_psCreature[i].dXPperDeath = get_double(); // XPperDeath
      m_psCreature[i].dwMinGiveGold = get_dword();  // MinGiveGold
      m_psCreature[i].dwMaxGiveGold = get_dword();  // MaxGiveGold
      m_psCreature[i].chCanAttack   = get_byte();   // CanAttack
      m_psCreature[i].chChangeTargetAA   = get_byte();   // ChangeTargetAA
      m_psCreature[i].iFriendlyID        = get_dword(); //FriendlyID 

      


      int dwNbrAttack = get_dword();
      
      // Alloue le nombre de Creature Attack
      for(int nm=0;nm<dwNbrAttack;nm++)
      {
         sAttack *pNewAttack = new sAttack;
         m_psCreature[i].aAttack.Add(pNewAttack);
      }
      // lit les Creature Attack
      int dwTmp1 = dwNbrAttack;
      dwTmp1--;
		for(;dwTmp1>=0; dwTmp1--)
		{
         // recupere le Creature Name
         sprintf_s(strTmp,512,"%s",get_string());
         m_psCreature[i].aAttack[dwTmp1]->dwDmg = strlen(strTmp);
         if(m_psCreature[i].aAttack[dwTmp1]->dwDmg > 0)
         {
            m_psCreature[i].aAttack[dwTmp1]->pstrDmg = new char[strlen(strTmp)+1];
            strcpy_s(m_psCreature[i].aAttack[dwTmp1]->pstrDmg,strlen(strTmp)+1,strTmp);
         }
         else
            m_psCreature[i].aAttack[dwTmp1]->pstrDmg = NULL;
         m_psCreature[i].aAttack[dwTmp1]->dwAttackSkill      = get_dword();
         m_psCreature[i].aAttack[dwTmp1]->dwAttackPercentage = get_dword();
         m_psCreature[i].aAttack[dwTmp1]->dwAttackSpell      = get_dword();
         m_psCreature[i].aAttack[dwTmp1]->dwAttackMinRange   = get_dword();
		   m_psCreature[i].aAttack[dwTmp1]->dwAttackMaxRange   = get_dword();

         //NMNMNM testtesttest
         if(m_psCreature[i].aAttack[dwTmp1]->dwAttackMaxRange >0 && m_psCreature[i].aAttack[dwTmp1]->dwAttackSpell < 10000)
         {
            TRACE("RANGE ATTACK %s    %d\n",m_psCreature[i].pstrCreatureID,m_psCreature[i].aAttack[dwTmp1]->dwAttackSpell);
         }
		}

      int dwNbrDeadFlag = get_dword();
      // Alloue le nombre de Creature Attack
      for(int nm=0;nm<dwNbrDeadFlag;nm++)
      {
         sDeadFlagNM *pNewDead = new sDeadFlagNM;
         m_psCreature[i].aDeadFlag.Add(pNewDead);
      }
      // lit les Creature Attack
      dwTmp1 = dwNbrDeadFlag;
      dwTmp1--;
		for(;dwTmp1>=0; dwTmp1--)
		{
         m_psCreature[i].aDeadFlag[dwTmp1]->dwFlag      = get_dword();
         m_psCreature[i].aDeadFlag[dwTmp1]->dwFlagValue = get_dword();
         m_psCreature[i].aDeadFlag[dwTmp1]->chIncrement      = get_byte();
		}

      int dwNbrDrop = get_dword();
      // Alloue le nombre de Creature Attack
      for(int nm=0;nm<dwNbrDrop;nm++)
      {
         sDrop *pNewDrop = new sDrop;
         m_psCreature[i].aDrop.Add(pNewDrop);
      }
      // lit les Creature Attack
      dwTmp1 = dwNbrDrop;
      dwTmp1--;
		for(;dwTmp1>=0; dwTmp1--)
		{
         m_psCreature[i].aDrop[dwTmp1]->dwItemID      = get_dword();
         m_psCreature[i].aDrop[dwTmp1]->dDropPercentage = get_double();
		}
	}
}
void CWDAUtils::DeleteCreatureList()
{
   if(m_psCreature)
   {
      for(int i=0;i<m_dwNbrCreature;i++)
      {
         if(m_psCreature[i].pstrCreatureID)
            delete m_psCreature[i].pstrCreatureID;
         if(m_psCreature[i].pstrName)
            delete m_psCreature[i].pstrName;

         sAttack *pNewAttack;
         while( m_psCreature[i].aAttack.GetSize() > 0 ) 
         {
		      pNewAttack = m_psCreature[i].aAttack[0];
            if(pNewAttack->pstrDmg)
               delete pNewAttack->pstrDmg;
            m_psCreature[i].aAttack.RemoveAt(0);
		      delete pNewAttack;
	      }
         m_psCreature[i].aAttack.RemoveAll();
         m_psCreature[i].aAttack.FreeExtra();

         sDeadFlagNM *pNewDFlag;
         while( m_psCreature[i].aDeadFlag.GetSize() > 0 ) 
         {
		      pNewDFlag = m_psCreature[i].aDeadFlag[0];
            m_psCreature[i].aDeadFlag.RemoveAt(0);
		      delete pNewDFlag;
	      }
         m_psCreature[i].aDeadFlag.RemoveAll();
         m_psCreature[i].aDeadFlag.FreeExtra();

         sDrop *pNewDrop;
         while( m_psCreature[i].aDrop.GetSize() > 0 ) 
         {
		      pNewDrop = m_psCreature[i].aDrop[0];
            m_psCreature[i].aDrop.RemoveAt(0);
		      delete pNewDrop;
	      }
         m_psCreature[i].aDrop.RemoveAll();
         m_psCreature[i].aDrop.FreeExtra();
        
      }
      if(m_psCreature)
         delete []m_psCreature;
      m_psCreature = NULL;
      m_dwNbrCreature = 0;
   }
}

int  CWDAUtils::CalculeCreatureSize()
{
   int dwSize = 0;
   if(m_psCreature)
   {
      dwSize+=4;
      for(int i=0; i<m_dwNbrCreature; i++)
	   {
         dwSize+=4;
         dwSize+=m_psCreature[i].dwNbrCreatureID;
         dwSize+=m_psCreature[i].dwNbrName;
         dwSize+=4;// END
         dwSize+=4;// AGI
         dwSize+=4;// INT
         dwSize+=4;// Value(?)
         dwSize+=4;// Value(?)
         dwSize+=4;// Value(?)
         dwSize+=4;// Value(?)
         dwSize+=4;// AirResist
         dwSize+=4;// EarthResist
         dwSize+=4;// WaterResist
         dwSize+=4;// FireResist
         dwSize+=4;// DarkResist
         dwSize+=4;// LightResist
         dwSize+=4;// AirPower
         dwSize+=4;// EarthPower
         dwSize+=4;// WaterPower
         dwSize+=4;// FirePower
         dwSize+=4;// DarkPower
         dwSize+=4;// LightPower
         dwSize+=4;// LVL
         dwSize+=4;// HP
         dwSize+=4;// DodgeSkill
         dwSize+=8;// AC
         dwSize+=4;// Appearance
         dwSize+=4;// DressBody
         dwSize+=4;// DressFeet
         dwSize+=4;// DressGloves
         dwSize+=4;// DressHelm
         dwSize+=4;// DressLegs
         dwSize+=4;// DressWeapon
         dwSize+=4;// DressShield
         dwSize+=4;// DressCape
         dwSize+=4;// Aggressivness
         dwSize+=4;// ClanID
         dwSize+=4;// Speed
         dwSize+=8;// XPperHit
         dwSize+=8;// XPperDeath
         dwSize+=4;// MinGiveGold
         dwSize+=4;// MaxGiveGold
         dwSize+=1;// CanAttack
         dwSize+=1;// ChangeTargetAA
         dwSize+=4;// iFriendlyID
         

         dwSize+=(2*4);// Le nombre de string

         dwSize+=4;// Le nombre d'attack
		   for(int j=0;j<m_psCreature[i].aAttack.GetSize();j++)
		   {
            dwSize+=4;
            dwSize+=m_psCreature[i].aAttack[j]->dwDmg;
            dwSize+=4;
            dwSize+=4;
            dwSize+=4;
            dwSize+=4;
            dwSize+=4;
		   }

         dwSize+=4;// Le dead Flag
		   for(int j=0;j<m_psCreature[i].aDeadFlag.GetSize();j++)
		   {
            dwSize+=4;
            dwSize+=4;
            dwSize+=1;
		   }

         dwSize+=4;// Le dead Flag
		   for(int j=0;j<m_psCreature[i].aDrop.GetSize();j++)
		   {
            dwSize+=4;
            dwSize+=8;
		   }
	   }
   }
   return dwSize;
}


void CWDAUtils::ReadListHives()
{
   char strTmp[512];

	m_pListHivesPos = m_pDataTmp;
   m_dwNbrHives = get_dword();

   DeleteHivesList();

   m_psHivesList = new sHives[m_dwNbrHives];

	for(int i=0; i<m_dwNbrHives; i++)
	{
      m_psHivesList[i].dwMinEmergencyTime = get_dword();  // MinEmergeTime
      m_psHivesList[i].dwMaxEmergencyTime = get_dword();  // MaxEmergeTime
      m_psHivesList[i].dwMaxChildren      = get_dword();  // MaxChildren
      m_psHivesList[i].dwEmergencyRange   = get_dword();  // EmergenceRange
     
      // recupere le nom de Hives
      sprintf_s(strTmp,512,"%s",get_string());
      m_psHivesList[i].dwLarva = strlen(strTmp);
      if(m_psHivesList[i].dwLarva > 0)
      {
         m_psHivesList[i].pstrLarva = new char[strlen(strTmp)+1];
         strcpy_s(m_psHivesList[i].pstrLarva,strlen(strTmp)+1,strTmp);
      }
      else
         m_psHivesList[i].pstrLarva = NULL;

      m_psHivesList[i].dwNbrlarvaGrp = get_dword();
      
      // Alloue le nombre de LarvaGrp
      for(int nm=0;nm<m_psHivesList[i].dwNbrlarvaGrp;nm++)
      {
         HiveLarvaGrp *pNeLarvaGrp = new HiveLarvaGrp;
         m_psHivesList[i].aLarvaGrp.Add(pNeLarvaGrp);
      }
      // lit les groupe....
      int dwTmp1 = m_psHivesList[i].dwNbrlarvaGrp;
      dwTmp1--;
		for(;dwTmp1>=0; dwTmp1--)
		{
         sprintf_s(strTmp,512,"%s",get_string());
         m_psHivesList[i].aLarvaGrp[dwTmp1]->dwLarveGrp = strlen(strTmp);
         if(m_psHivesList[i].aLarvaGrp[dwTmp1]->dwLarveGrp > 0)
         {
            m_psHivesList[i].aLarvaGrp[dwTmp1]->pstrlarvaGrp = new char[strlen(strTmp)+1];
            strcpy_s(m_psHivesList[i].aLarvaGrp[dwTmp1]->pstrlarvaGrp,strlen(strTmp)+1,strTmp);
            // trouve le monstre et son SkinID...
            m_psHivesList[i].dwSkinID = 1;
            for(int n=0;n<m_dwNbrCreature;n++)
            {
               if(strcmp(m_psCreature[n].pstrCreatureID,strTmp)==0)
               {
                   m_psHivesList[i].dwSkinID = m_psCreature[n].dwAppearance;
                   n = m_dwNbrCreature;
               }
            }
         }
         else
            m_psHivesList[i].aLarvaGrp[dwTmp1]->pstrlarvaGrp = NULL;
		}

      // lit les nombre de pod...
      m_psHivesList[i].dwNbrPod = get_dword();
      // Alloue le nombre de pod
      for(int nm=0;nm<m_psHivesList[i].dwNbrPod;nm++)
      {
         HivePod *pNewPod = new HivePod;
         m_psHivesList[i].aHivePod.Add(pNewPod);
      }
      
      dwTmp1 = m_psHivesList[i].dwNbrPod;

		for(int nb3=0;nb3<dwTmp1; nb3++)
		{
         m_psHivesList[i].aHivePod[nb3]->dwX = get_long();
         m_psHivesList[i].aHivePod[nb3]->dwY = get_long();
         m_psHivesList[i].aHivePod[nb3]->dwZ = get_long();
         int dwX = m_psHivesList[i].aHivePod[nb3]->dwX;
         int dwY = m_psHivesList[i].aHivePod[nb3]->dwY;
         int dwW = m_psHivesList[i].aHivePod[nb3]->dwZ;
         int larva = m_psHivesList[i].dwSkinID;

		}
	}
}

void CWDAUtils::DeleteHivesList()
{
   if(m_psHivesList)
   {
      for(int i=0;i<m_dwNbrHives;i++)
      {
         // Efface 
         if(m_psHivesList[i].pstrLarva)
            delete m_psHivesList[i].pstrLarva;
         
         //delete les larva grp
         HiveLarvaGrp *pLarvaGrpTmp;
         while( m_psHivesList[i].aLarvaGrp.GetSize() > 0 ) 
         {
		      pLarvaGrpTmp = m_psHivesList[i].aLarvaGrp[0];
            
            if(pLarvaGrpTmp->pstrlarvaGrp)
               delete pLarvaGrpTmp->pstrlarvaGrp;
            m_psHivesList[i].aLarvaGrp.RemoveAt(0);
		      delete pLarvaGrpTmp;
	      }
         m_psHivesList[i].aLarvaGrp.RemoveAll();
         m_psHivesList[i].aLarvaGrp.FreeExtra();

         //delete les Hive Pod
         HivePod *pPodTmp;
         while( m_psHivesList[i].aHivePod.GetSize() > 0 ) 
         {
		      pPodTmp = m_psHivesList[i].aHivePod[0];
            m_psHivesList[i].aHivePod.RemoveAt(0);
		      delete pPodTmp;
	      }
         m_psHivesList[i].aHivePod.RemoveAll();
         m_psHivesList[i].aHivePod.FreeExtra();
      }
      if(m_psHivesList)
         delete []m_psHivesList;

      m_psHivesList = NULL;
      m_dwNbrHives = 0;


   }
}

int  CWDAUtils::CalculeHivesSize()
{
   int dwSize = 0;
   dwSize+=4; // le nombre de Spell
   if(m_psHivesList)
   {
      for(int i=0;i<m_dwNbrHives;i++)
      {
         dwSize+=4; // MinEmergeTime
         dwSize+=4; // MaxEmergeTime
         dwSize+=4; // MaxChildren
         dwSize+=4; // EmergenceRange
         dwSize+=4; // Le nbr de Byte des larva
         dwSize+=m_psHivesList[i].dwLarva; // le nom du Hives
         dwSize+=4; // Le nbr de LarvaGRP
         for(int j=0;j<m_psHivesList[i].aLarvaGrp.GetSize();j++)
         {
            dwSize+=4; // Le nbr de Byte des larvaGrpname
            dwSize+=m_psHivesList[i].aLarvaGrp[j]->dwLarveGrp; // le nom du larvagrp
         }
         dwSize+=4; // Le nbr de Pod
         for(int j=0;j<m_psHivesList[i].aHivePod.GetSize();j++)
         {
            dwSize+=4; // le pod X
            dwSize+=4; // le pod Y
            dwSize+=4; // le pod Z
         }
      }
   }

   return dwSize;
}


void CWDAUtils::ReadListLinkArea()
{
   DeleteLinkArea();

   m_pListAreaLinkPos = m_pDataTmp;
	m_dwNbrLinkArea = get_dword();

   int dwRemoved = 0;

	for(int i=0; i<m_dwNbrLinkArea; i++)
	{
      sLinkArea sNewArea;
      sNewArea.dwSrcX = get_long();
      sNewArea.dwSrcY = get_long();
      sNewArea.dwSrcZ = get_long();
      sNewArea.dwDesX = get_long();
      sNewArea.dwDesY = get_long();
      sNewArea.dwDesZ = get_long();
      // cherche si il en existe deja 1 si oui on ajoute pas...

      BOOL bAdd = TRUE;
      for(int j=0; j<m_sLinkArea.GetSize(); j++)
	   {
         if(m_sLinkArea[j].dwSrcX == sNewArea.dwSrcX &&
            m_sLinkArea[j].dwSrcY == sNewArea.dwSrcY &&
            m_sLinkArea[j].dwSrcZ == sNewArea.dwSrcZ &&
            m_sLinkArea[j].dwDesX == sNewArea.dwDesX &&
            m_sLinkArea[j].dwDesY == sNewArea.dwDesY &&
            m_sLinkArea[j].dwDesZ == sNewArea.dwDesZ     )
         {
            bAdd = FALSE;
            dwRemoved++;
            j = m_sLinkArea.GetSize()+1;
         }

      }
      if(bAdd)
      {
         m_sLinkArea.Add(sNewArea);
      }
	}
}

void CWDAUtils::DeleteLinkArea()
{
   while( m_sLinkArea.GetSize() > 0 ) 
   {
      m_sLinkArea.RemoveAt(0);
	}
   m_sLinkArea.RemoveAll();
   m_sLinkArea.FreeExtra();
}

int  CWDAUtils::CalculeAreaSize()
{
   int dwSize = 0;
   dwSize +=4;
   dwSize += 24*m_sLinkArea.GetSize();
   return dwSize;
}

void CWDAUtils::ReadListClan()
{
   char strTmp[512];
   DeleteClanList();
   m_pListClanPos = m_pDataTmp;
	m_dwhighClan = get_dword(); // Highest clan ID
   m_dwNbrClan  = get_dword(); // le nombre de clan

	for(int i=0; i<m_dwNbrClan; i++)
	{
      sClan sNewClan;
      sNewClan.shClan1 = get_word();
      sNewClan.shClan2 = get_word();
      sNewClan.shNiveaux = get_short();
      m_sClan.Add(sNewClan);
	}

   m_dwNbrClanName = get_dword();
   for(int i=0;i<m_dwNbrClanName;i++)
   {
      sClanName sNewClanName;
      sNewClanName.shClanIndex = get_short();
       // recupere le nom de clan
      sprintf_s(strTmp,512,"%s",get_string());
      sNewClanName.dwName = strlen(strTmp);
      if(sNewClanName.dwName > 0)
      {
         sNewClanName.pstrName = new char[strlen(strTmp)+1];
         strcpy_s(sNewClanName.pstrName,strlen(strTmp)+1,strTmp);
      }
      else
         sNewClanName.pstrName = NULL;
      m_sClanName.Add(sNewClanName);
   }
}

void CWDAUtils::DeleteClanList()
{
   while( m_sClan.GetSize() > 0 ) 
   {
      m_sClan.RemoveAt(0);
	}
   m_sClan.RemoveAll();
   m_sClan.FreeExtra();

   while( m_sClanName.GetSize() > 0 ) 
   {
      if(m_sClanName[0].pstrName)
         delete m_sClanName[0].pstrName;
      m_sClanName.RemoveAt(0);
	}
   m_sClanName.RemoveAll();
   m_sClanName.FreeExtra();
}

int  CWDAUtils::CalculeClanSize()
{
   int dwSize = 0;
   dwSize +=4; // le hightest clan
   dwSize +=4; // le nbr de clan
   dwSize += 6*m_sClan.GetSize();

   dwSize +=4;
   for(int i=0;i<m_dwNbrClanName;i++)
   {
      dwSize +=2;
      dwSize +=4;
      dwSize +=m_sClanName[i].dwName;
   }

   return dwSize;
}


void CWDAUtils::ReadQuestFlag()
{
   DeleteQuestList();
   char strTmp[512];
   m_dwNbrQuestFlag = get_dword();
   for(int i=0;i<m_dwNbrQuestFlag;i++)
   {
      sQuestFlag sNewQuestFlag;
      sNewQuestFlag.dwFlagValue = get_dword();
       // recupere le nom de clan
      sprintf_s(strTmp,512,"%s",get_string());
      sNewQuestFlag.dwName = strlen(strTmp);
      if(sNewQuestFlag.dwName > 0)
      {
         sNewQuestFlag.pstrName = new char[strlen(strTmp)+1];
         strcpy_s(sNewQuestFlag.pstrName,strlen(strTmp)+1,strTmp);
      }
      else
         sNewQuestFlag.pstrName = NULL;
      m_sQuestFlag.Add(sNewQuestFlag);
   }
}

void CWDAUtils::DeleteQuestList()
{
   while( m_sQuestFlag.GetSize() > 0 ) 
   {
      if(m_sQuestFlag[0].pstrName)
         delete []m_sQuestFlag[0].pstrName;
      m_sQuestFlag.RemoveAt(0);
	}
   m_sQuestFlag.RemoveAll();
   m_sQuestFlag.FreeExtra();

}

int  CWDAUtils::CalculeQuestSize()
{
   int dwSize = 4;

   for(int i=0;i<m_dwNbrQuestFlag;i++)
   {
      dwSize += 4;
      dwSize += 4;
      dwSize += m_sQuestFlag[i].dwName;
   }
   return dwSize;
}

void CWDAUtils::ReadAppearanceItem()
{
   DeleteAppearanceItem();
   char strTmp[512];
   m_dwNbrAppearanceItem = get_dword();
   for(int i=0;i<m_dwNbrAppearanceItem;i++)
   {
      AppearanceItem sAppItem;
      sAppItem.dwID = get_dword();
       // recupere le nom de clan
      sprintf_s(strTmp,512,"%s",get_string());
      //TRACE("[%04d] %s\n",sAppItem.dwID,strTmp);
      sAppItem.dwNbrName = strlen(strTmp);
      if(sAppItem.dwNbrName > 0)
      {
         sAppItem.pstrName = new char[strlen(strTmp)+1];
         strcpy_s(sAppItem.pstrName,strlen(strTmp)+1,strTmp);
      }
      else
         sAppItem.pstrName = NULL;
      m_sAppearanceItem.Add(sAppItem);
   }
}

void CWDAUtils::DeleteAppearanceItem()
{
   while( m_sAppearanceItem.GetSize() > 0 ) 
   {
      if(m_sAppearanceItem[0].pstrName)
         delete []m_sAppearanceItem[0].pstrName;
      m_sAppearanceItem.RemoveAt(0);
	}
   m_sAppearanceItem.RemoveAll();
   m_sAppearanceItem.FreeExtra();
}

int  CWDAUtils::CalculeAppearanceItemSize()
{
   int dwSize = 4;

   if(m_dwNbrAppearanceItem != m_sAppearanceItem.GetSize())
   {
       m_dwNbrAppearanceItem = m_sAppearanceItem.GetSize();
   }
   for(int i=0;i<m_dwNbrAppearanceItem;i++)
   {
      dwSize += 4;
      dwSize += 4;
      dwSize += m_sAppearanceItem[i].dwNbrName;
   }
   return dwSize;
}

void CWDAUtils::ReadAppearanceMonster()
{
   DeleteAppearanceMonster();
   char strTmp[512];
   m_dwNbrAppearanceMonster = get_dword();
   for(int i=0;i<m_dwNbrAppearanceMonster;i++)
   {
      AppearanceMonster sAppMonster;
      sAppMonster.dwID  = get_dword();
      sAppMonster.dwIDC = get_dword();
       // recupere le nom de clan
      sprintf_s(strTmp,512,"%s",get_string());
      //TRACE("[%04d] %s\n",sAppMonster.dwID,strTmp);
      sAppMonster.dwNbrName = strlen(strTmp);
      if(sAppMonster.dwNbrName > 0)
      {
         sAppMonster.pstrName = new char[strlen(strTmp)+1];
         strcpy_s(sAppMonster.pstrName,strlen(strTmp)+1,strTmp);
      }
      else
         sAppMonster.pstrName = NULL;
      m_sAppearanceMonster.Add(sAppMonster);
   }

}

void CWDAUtils::DeleteAppearanceMonster()
{
   while( m_sAppearanceMonster.GetSize() > 0 ) 
   {
      if(m_sAppearanceMonster[0].pstrName)
         delete []m_sAppearanceMonster[0].pstrName;
      m_sAppearanceMonster.RemoveAt(0);
	}
   m_sAppearanceMonster.RemoveAll();
   m_sAppearanceMonster.FreeExtra();
}

int  CWDAUtils::CalculeAppearanceMonsterSize()
{
   if(m_dwNbrAppearanceMonster != m_sAppearanceMonster.GetSize())
   {
       m_dwNbrAppearanceMonster = m_sAppearanceMonster.GetSize();
   }
   int dwSize    = 4;
   for(int i=0;i<m_dwNbrAppearanceMonster;i++)
   {
      dwSize += 4;
      dwSize += 4;
      dwSize += 4;
      dwSize += m_sAppearanceMonster[i].dwNbrName;
   }
   return dwSize;
}


void CWDAUtils::ReadItemLocation()
{
   DeleteItemLocation();
   char strTmp[512];
   m_dwNbrItemLocation = get_dword();
   for(int i=0;i<m_dwNbrItemLocation;i++)
   {
      ItemLocation sItemLocation;
       // recupere le nom de clan
      sprintf_s(strTmp,512,"%s",get_string());
      sItemLocation.dwNbrName = strlen(strTmp);
      if(sItemLocation.dwNbrName > 0)
      {
         sItemLocation.pstrName = new char[strlen(strTmp)+1];
         strcpy_s(sItemLocation.pstrName,strlen(strTmp)+1,strTmp);
      }
      else
         sItemLocation.pstrName = NULL;
      m_sItemLocation.Add(sItemLocation);
   }
}

void CWDAUtils::DeleteItemLocation()
{
   while( m_sItemLocation.GetSize() > 0 ) 
   {
      if(m_sItemLocation[0].pstrName)
         delete []m_sItemLocation[0].pstrName;
      m_sItemLocation.RemoveAt(0);
	}
   m_sItemLocation.RemoveAll();
   m_sItemLocation.FreeExtra();
}

int  CWDAUtils::CalculeItemLocationSize()
{
   if(m_dwNbrItemLocation != m_sItemLocation.GetSize())
   {
       m_dwNbrItemLocation = m_sItemLocation.GetSize();
   }
   int dwSize    = 4;
   for(int i=0;i<m_dwNbrItemLocation;i++)
   {
      dwSize += 4;
      dwSize += m_sItemLocation[i].dwNbrName;
   }
   return dwSize;
}


void CWDAUtils::ReadSpellIcon()
{
   DeleteSpellIcon();
   char strTmp[512];
   m_dwNbrSpellIcon = get_dword();
   for(int i=0;i<m_dwNbrSpellIcon;i++)
   {
      SpellIcon sStruct;
      sStruct.dwID  = get_dword();
       // recupere le nom de clan
      sprintf_s(strTmp,512,"%s",get_string());
      //TRACE("[%04d] %s\n",sStruct.dwID,strTmp);
      sStruct.dwNbrName = strlen(strTmp);
      if(sStruct.dwNbrName > 0)
      {
         sStruct.pstrName = new char[strlen(strTmp)+1];
         strcpy_s(sStruct.pstrName,strlen(strTmp)+1,strTmp);
      }
      else
         sStruct.pstrName = NULL;
      m_sSpellIcon.Add(sStruct);
   }
}

void CWDAUtils::DeleteSpellIcon()
{
   while( m_sSpellIcon.GetSize() > 0 ) 
   {
      if(m_sSpellIcon[0].pstrName)
         delete []m_sSpellIcon[0].pstrName;
      m_sSpellIcon.RemoveAt(0);
	}
   m_sSpellIcon.RemoveAll();
   m_sSpellIcon.FreeExtra();
}

int  CWDAUtils::CalculeSpellIconSize()
{
   if(m_dwNbrSpellIcon != m_sSpellIcon.GetSize())
   {
       m_dwNbrSpellIcon = m_sSpellIcon.GetSize();
   }
   int dwSize    = 4;
   for(int i=0;i<m_dwNbrSpellIcon;i++)
   {
      dwSize += 4;
      dwSize += 4;
      dwSize += m_sSpellIcon[i].dwNbrName;
   }
   return dwSize;
}

void CWDAUtils::ReadVisualEffect()
{
   DeleteVisualEffect();
   char strTmp[512];
   m_dwNbrVisualEffect = get_dword();
   for(int i=0;i<m_dwNbrVisualEffect;i++)
   {
      VisualEffect sStruct;
      sStruct.dwID  = get_dword();
       // recupere le nom de clan
      sprintf_s(strTmp,512,"%s",get_string());
      //TRACE("[%04d] %s\n",sStruct.dwID,strTmp);
      sStruct.dwNbrName = strlen(strTmp);
      if(sStruct.dwNbrName > 0)
      {
         sStruct.pstrName = new char[strlen(strTmp)+1];
         strcpy_s(sStruct.pstrName,strlen(strTmp)+1,strTmp);
      }
      else
         sStruct.pstrName = NULL;
      m_sVisualEffect.Add(sStruct);
   }
}
void CWDAUtils::DeleteVisualEffect()
{
   while( m_sVisualEffect.GetSize() > 0 ) 
   {
      if(m_sVisualEffect[0].pstrName)
         delete []m_sVisualEffect[0].pstrName;
      m_sVisualEffect.RemoveAt(0);
	}
   m_sVisualEffect.RemoveAll();
   m_sVisualEffect.FreeExtra();
}

int  CWDAUtils::CalculeVisualEffectSize()
{
   if(m_dwNbrVisualEffect != m_sVisualEffect.GetSize())
   {
       m_dwNbrVisualEffect = m_sVisualEffect.GetSize();
   }
   int dwSize    = 4;
   for(int i=0;i<m_dwNbrVisualEffect;i++)
   {
      dwSize += 4;
      dwSize += 4;
      dwSize += m_sVisualEffect[i].dwNbrName;
   }
   return dwSize;
}




unsigned int CWDAUtils::get_dword()
{
	unsigned int val;

	val=*(unsigned int *)m_pDataTmp;	m_pDataTmp+=4;
	return val;
}

unsigned short CWDAUtils::get_word()
{
	unsigned short val;

	val=*(unsigned short *)m_pDataTmp;	m_pDataTmp+=2;
	return val;
}

BYTE CWDAUtils::get_byte()
{
	unsigned char val;

	val=*m_pDataTmp;	m_pDataTmp+=1;
	return val;
}

double CWDAUtils::get_double()
{
	double val;

	val=*(double *)m_pDataTmp;	m_pDataTmp+=8;
	return val;
}

int CWDAUtils::get_long()
{
	int val;

	val=*(int *)m_pDataTmp;	m_pDataTmp+=4;
	return val;
}

short CWDAUtils::get_short()
{
	short val;

	val=*(short *)m_pDataTmp;	m_pDataTmp+=2;
	return val;
}

char* CWDAUtils::get_string()
{
	int lg,i;

	lg=*(int *)m_pDataTmp;	m_pDataTmp+=4;
	for(i=0; i<lg; i++)
		m_chLigne[i]=*m_pDataTmp++;

	m_chLigne[i]=0;

	return (char *)m_chLigne;
}

BOOL CWDAUtils::SaveWDA() 
{
   CWaitCursor WaitCursor;
   //Force un update de la map en cours

   int i=0;
   int j=0;
   int k=0;
   int l=0;

   int dwNewtaille = 5; // file header
   dwNewtaille    += CalculeSpellSize();    //taille des spell
   dwNewtaille    += CalculeMapSize();      //taille des Map
   dwNewtaille    += CalculeItemSize();     //taille des Items
   dwNewtaille    += CalculeCreatureSize(); //taille des Creature
   dwNewtaille    += CalculeHivesSize();    //taille des Hives
   dwNewtaille    += CalculeAreaSize();     //taille des Link Area
   dwNewtaille    += CalculeClanSize();     //taille des Clan 
   dwNewtaille    += CalculeQuestSize();    //taille des Flag Quest

   dwNewtaille    +=1; //is misc data
   if(m_chIsMiscWDAData == 1)
   {
      dwNewtaille    += CalculeAppearanceItemSize(); //taille des Appearances Items
      dwNewtaille    += CalculeAppearanceMonsterSize(); //taille des Appearances Monster
      dwNewtaille    += CalculeItemLocationSize();
      dwNewtaille    += CalculeSpellIconSize();
      dwNewtaille    += CalculeVisualEffectSize();
   }
   
   //dwNewtaille    += CalculeTypeItemSize(); //taille du type des items
   
   //dwNewtaille    += m_dwendFileData; //now on lite 100% des file comme faut donc plus supposer avoir de reste
   
   
   //dwNewtaille    += CalculeTypeMonstreSize(); //taille du type des items

  
   BYTE *pNewFile = new BYTE[dwNewtaille];
   // copie les 5 bytes de header...
   for(i=0;i<5;i++)
      pNewFile[i] = m_pData[i];

   // maintenant on ecris les fameux Spell...
   ////////////////////////////////////////////////////////////////////////////////////
   int dwWritePos = 5;
   uIntToChar    uiNbrSpell;
   uDoubleToChar udDoubleVal;
   uiNbrSpell.dwVal = m_dwNbrSpell;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

   for(i=0; i<m_dwNbrSpell; i++)
	{
      // recupere le ID du Spell
      uiNbrSpell.dwVal = m_psSpellList[i].dwID;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le Nom du Spell
      uiNbrSpell.dwVal = m_psSpellList[i].dwNom;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psSpellList[i].pstrNom[j];
         dwWritePos++;
      }

      // recupere le Mantal Exhaust
      uiNbrSpell.dwVal = m_psSpellList[i].dwMentalExhaust;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psSpellList[i].pstrMentalExhaust[j];
         dwWritePos++;
      }

      // recupere le Move Exhaust
      uiNbrSpell.dwVal = m_psSpellList[i].dwMovementExhaust;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psSpellList[i].pstrMovementExhaust[j];
         dwWritePos++;
      }

      // recupere le Attack Exhaust
      uiNbrSpell.dwVal = m_psSpellList[i].dwAttackExhaust;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psSpellList[i].pstrAttackExhaust[j];
         dwWritePos++;
      }

      // recupere le strDuration
      uiNbrSpell.dwVal = m_psSpellList[i].dwDuration;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psSpellList[i].pstrDuration[j];
         dwWritePos++;
      }

      // recupere le strTimerFrequency
      uiNbrSpell.dwVal = m_psSpellList[i].dwTimerFrequency;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psSpellList[i].pstrTimerFrequency[j];
         dwWritePos++;
      }

      // le type du sort Eau, terre, etc etc
      uiNbrSpell.dwVal = m_psSpellList[i].dwElement;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le dwManaCost
      uiNbrSpell.dwVal = m_psSpellList[i].dwManaCost;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psSpellList[i].pstrManaCost[j];
         dwWritePos++;
      }
     
      // recupere le dwAreaRange
      uiNbrSpell.dwVal = m_psSpellList[i].dwAreaRange;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le dwVisualEffect
      uiNbrSpell.dwVal = m_psSpellList[i].dwVisualEffect;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le dwRangeVisualEffect
      uiNbrSpell.dwVal = m_psSpellList[i].dwRangeVisualEffect;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le dwTargetType
      uiNbrSpell.dwVal = m_psSpellList[i].dwTargetType;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le dwAttackType
      uiNbrSpell.dwVal = m_psSpellList[i].dwAttackType;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le dwMinWis
      uiNbrSpell.dwVal = m_psSpellList[i].dwMinWis;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le dwMinInt
      uiNbrSpell.dwVal = m_psSpellList[i].dwMinInt;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le dwMinLevel
      uiNbrSpell.dwVal = m_psSpellList[i].dwMinLevel;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le dwMinStr
      uiNbrSpell.dwVal = m_psSpellList[i].dwMinStr;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le dwMinEnd
      uiNbrSpell.dwVal = m_psSpellList[i].dwMinEnd;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le dwMinAgi
      uiNbrSpell.dwVal = m_psSpellList[i].dwMinAgi;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le dwMinAttack
      uiNbrSpell.dwVal = m_psSpellList[i].dwMinAttack;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le dwMinArchery
      uiNbrSpell.dwVal = m_psSpellList[i].dwMinArchery;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le dwMinDodge
      uiNbrSpell.dwVal = m_psSpellList[i].dwMinDodge;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le dwMinCs
      uiNbrSpell.dwVal = m_psSpellList[i].dwMinCS;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      pNewFile[dwWritePos] = m_psSpellList[i].chLineOfSight;dwWritePos++;
      pNewFile[dwWritePos] = m_psSpellList[i].chSkillExclu;dwWritePos++;
      pNewFile[dwWritePos] = m_psSpellList[i].chPVPcheck;dwWritePos++;
      pNewFile[dwWritePos] = m_psSpellList[i].chFlag;dwWritePos++;

      // recupere le dwIcon
      uiNbrSpell.dwVal = m_psSpellList[i].dwIcon;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      // recupere le dwSuccessPercentage
      uiNbrSpell.dwVal = m_psSpellList[i].dwSuccessPercentage;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psSpellList[i].pstrSuccessPercentage[j];
         dwWritePos++;
      }

      // recupere le dwDescText
      uiNbrSpell.dwVal = m_psSpellList[i].dwDescText;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psSpellList[i].pstrDescText[j];
         dwWritePos++;
      }

      // recupere le dwNbrEffect
      uiNbrSpell.dwVal = m_psSpellList[i].SpellE.GetSize();
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      int dwTmp1 = m_psSpellList[i].SpellE.GetSize();
      dwTmp1--;

      // Boucle pour toutes les effets...
		for(;dwTmp1>=0; dwTmp1--)
		{
         // recupere le dwNbrEffect
         uiNbrSpell.dwVal = m_psSpellList[i].SpellE[dwTmp1]->dwEffect;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

         // recupere le dwNbrEffect
         uiNbrSpell.dwVal = m_psSpellList[i].SpellE[dwTmp1]->dwNbrEffectParam;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

         int dwTmp2 = m_psSpellList[i].SpellE[dwTmp1]->dwNbrEffectParam;
         dwTmp2--;
         // Boucle pour toute les parametres de l<effect...
			for(;dwTmp2>=0; dwTmp2--)
			{
            // recupere le dwNbrEffect
            uiNbrSpell.dwVal = m_psSpellList[i].SpellE[dwTmp1]->pSpellEffectP[dwTmp2].dwParam1;
            pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
            pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
            pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
            pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

            // recupere le Param Value
            uiNbrSpell.dwVal = m_psSpellList[i].SpellE[dwTmp1]->pSpellEffectP[dwTmp2].dwParam2;
            pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
            pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
            pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
            pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

            for(j=0;j<uiNbrSpell.dwVal;j++)
            {
               pNewFile[dwWritePos] = m_psSpellList[i].SpellE[dwTmp1]->pSpellEffectP[dwTmp2].pstrParam2[j];
               dwWritePos++;
            }
			}
		}

      // Recupere les spell Requirement
      uiNbrSpell.dwVal = m_psSpellList[i].dwNbrSpellReq;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

		dwTmp1 = m_psSpellList[i].dwNbrSpellReq;
      dwTmp1--;
		for(;dwTmp1>=0; dwTmp1--)
      {
         // Recupere les spell Requirement
         uiNbrSpell.dwVal = m_psSpellList[i].pdwSpellReq[dwTmp1];
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      }
	}

   uShortToChar ushVal;
   // la on ecris les map
   BYTE *pchTmpDes = pNewFile+dwWritePos;
   BYTE *pchTmpSrc = m_pData+m_dwMapOldPos;
   memcpy(pchTmpDes,pchTmpSrc,CalculeMapSize());
   dwWritePos += CalculeMapSize();
  
   /*
   //ecrit les map manuellement...

   //Write the number of maps...
   uiNbrSpell.dwVal = m_dwNbrMap;
   

   pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

    //ecrit les info de la map 2 manuellement...
   for(i=0;i<m_dwNbrMap;i++)
   {
      //IOD de la map
      ushVal.shVal = m_sMapInfo[i].shID;
      pNewFile[dwWritePos] = ushVal.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = ushVal.chVal[1];dwWritePos++;

      //nbr Char map name
      uiNbrSpell.dwVal = strlen(m_sMapInfo[i].strMapName);
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      //le nom de la map
      for(int j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_sMapInfo[i].strMapName[j];
         dwWritePos++;
      }

      //Longueur
      ushVal.shVal = m_sMapInfo[i].shLongeur;
      pNewFile[dwWritePos] = ushVal.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = ushVal.chVal[1];dwWritePos++;

      //Hauteur
      ushVal.shVal = m_sMapInfo[i].shHauteur;
      pNewFile[dwWritePos] = ushVal.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = ushVal.chVal[1];dwWritePos++;

      pchTmpDes = pNewFile+dwWritePos;
      memcpy(pchTmpDes,m_sMapInfo[i].pMapData,((m_sMapInfo[i].shLongeur*m_sMapInfo[i].shHauteur)/2));
      dwWritePos+=(m_sMapInfo[i].shLongeur*m_sMapInfo[i].shHauteur)/2;
   }
   */

   
   // Maintenant les items
   ///////////////////////////////////////////////////////////////////////////////////////
   uiNbrSpell.dwVal = m_dwNbrItem;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
	for(i=0; i<m_dwNbrItem; i++)
	{
      uiNbrSpell.dwVal = m_psItemList[i].dwItemName;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psItemList[i].pstrItemName[j];
         dwWritePos++;
      }
      uiNbrSpell.dwVal = m_psItemList[i].dwBindedID;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemList[i].dwStructID;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      
      uiNbrSpell.dwVal = m_psItemList[i].dwName;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psItemList[i].pstrName[j];
         dwWritePos++;
      }

      uiNbrSpell.dwVal = m_psItemList[i].dwAppearance;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemList[i].dwSellType;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemList[i].dwEquipPos;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemList[i].dwBuyFlagID;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemList[i].dwSellPrice;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      uiNbrSpell.dwVal = m_psItemList[i].dwStrSellPrice;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psItemList[i].pstrSellPrice[j];
         dwWritePos++;
      }



      uiNbrSpell.dwVal = m_psItemList[i].dwSize;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      udDoubleVal.dVal = m_psItemList[i].dArmorAC;
      pNewFile[dwWritePos] = udDoubleVal.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[3];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[4];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[5];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[6];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[7];dwWritePos++;

      udDoubleVal.dVal = m_psItemList[i].dParadePC;
      pNewFile[dwWritePos] = udDoubleVal.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[3];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[4];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[5];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[6];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[7];dwWritePos++;

      uiNbrSpell.dwVal = m_psItemList[i].dwArmorDodgeMalus;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemList[i].dwArmorMinEnd;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      uiNbrSpell.dwVal = m_psItemList[i].dwWeaponDmgRool;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psItemList[i].pstrWeaponDmgRool[j];
         dwWritePos++;
      }
      
      uiNbrSpell.dwVal = m_psItemList[i].dwWeaponAttack;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemList[i].dwWeaponStr;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemList[i].dwWeaponAgi;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      uiNbrSpell.dwVal = m_psItemList[i].dwLockKeyID;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psItemList[i].pstrLockKeyID[j];
         dwWritePos++;
      }

      uiNbrSpell.dwVal = m_psItemList[i].dwLockDifficulty;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      uiNbrSpell.dwVal = m_psItemList[i].dwBookText;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psItemList[i].pstrBookText[j];
         dwWritePos++;
      }

      uiNbrSpell.dwVal = m_psItemList[i].dwContainerGold;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemList[i].dwContainerGlobalRespawn;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemList[i].dwContainerUserRespawn;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      uiNbrSpell.dwVal = m_psItemList[i].dwExhaust;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psItemList[i].pstrExhaust[j];
         dwWritePos++;
      }
      uiNbrSpell.dwVal = m_psItemList[i].dwRadiance;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemList[i].dwCharges;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemList[i].dwMinInt;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemList[i].dwMinWis;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemList[i].dwIntlID;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemList[i].dwDropFlags;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      pNewFile[dwWritePos] = m_psItemList[i].chUnique ;dwWritePos++;

      uiNbrSpell.dwVal = m_psItemList[i].dwGmItemLocation;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psItemList[i].pstrGmItemLocation[j];
         dwWritePos++;
      }

      pNewFile[dwWritePos] = m_psItemList[i].chCanSummon ;dwWritePos++;
      pNewFile[dwWritePos] = m_psItemList[i].chFlag1 ;dwWritePos++;
      pNewFile[dwWritePos] = m_psItemList[i].chFlag2 ;dwWritePos++;


      // recupere le dwNbrEffect
      uiNbrSpell.dwVal = m_psItemList[i].aItemContainer.GetSize();
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      int dwTmp1 = m_psItemList[i].aItemContainer.GetSize();
      dwTmp1--;

      // Boucle pour toutes les effets...
		for(;dwTmp1>=0; dwTmp1--)
		{
         // recupere le dwNbrEffect
         uiNbrSpell.dwVal = m_psItemList[i].aItemContainer[dwTmp1]->aItemItem.GetSize();
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

         int dwTmp2 = m_psItemList[i].aItemContainer[dwTmp1]->aItemItem.GetSize();
         dwTmp2--;

         // Boucle pour toutes les effets...
		   for(;dwTmp2>=0; dwTmp2--)
			{
            uiNbrSpell.dwVal = m_psItemList[i].aItemContainer[dwTmp1]->aItemItem[dwTmp2]->dwNbrItemName;
            pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
            pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
            pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
            pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
            for(j=0;j<uiNbrSpell.dwVal;j++)
            {
               pNewFile[dwWritePos] = m_psItemList[i].aItemContainer[dwTmp1]->aItemItem[dwTmp2]->pstrItemName[j];
               dwWritePos++;
            }
			}
		}

      // recupere le dwNbrEffect
      uiNbrSpell.dwVal = m_psItemList[i].aItemBoost.GetSize();
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      dwTmp1 = m_psItemList[i].aItemBoost.GetSize();
      dwTmp1--;

      // Boucle pour toutes les effets...
		for(;dwTmp1>=0; dwTmp1--)
		{
         uiNbrSpell.dwVal = m_psItemList[i].aItemBoost[dwTmp1]->dwBoost;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         uiNbrSpell.dwVal = m_psItemList[i].aItemBoost[dwTmp1]->dwStat;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

         uiNbrSpell.dwVal = m_psItemList[i].aItemBoost[dwTmp1]->dwExhaust;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         for(j=0;j<uiNbrSpell.dwVal;j++)
         {
            pNewFile[dwWritePos] = m_psItemList[i].aItemBoost[dwTmp1]->pstrExhaust[j];
            dwWritePos++;
         }

         uiNbrSpell.dwVal = m_psItemList[i].aItemBoost[dwTmp1]->dwMinInt;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         uiNbrSpell.dwVal = m_psItemList[i].aItemBoost[dwTmp1]->dwMinWis;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      
      }

      // recupere le dwNbrEffect
      uiNbrSpell.dwVal = m_psItemList[i].aItemSpell.GetSize();
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      dwTmp1 = m_psItemList[i].aItemSpell.GetSize();
      dwTmp1--;

      // Boucle pour toutes les effets...
		for(;dwTmp1>=0; dwTmp1--)
		{
         uiNbrSpell.dwVal = m_psItemList[i].aItemSpell[dwTmp1]->dwSpell;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         uiNbrSpell.dwVal = m_psItemList[i].aItemSpell[dwTmp1]->dwInstilled;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         uiNbrSpell.dwVal = m_psItemList[i].aItemSpell[dwTmp1]->dwLevel;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      }
	}

   // Maintenant les items POD POD
   ///////////////////////////////////////////////////////////////////////////////////////
   uiNbrSpell.dwVal = m_psItemPodList2.GetSize();
   pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
	for(i=0; i<m_psItemPodList2.GetSize(); i++)
	{
      uiNbrSpell.dwVal = m_psItemPodList2[i].dwItemName;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psItemPodList2[i].pstrItemName[j];
         dwWritePos++;
      }
      uiNbrSpell.dwVal = m_psItemPodList2[i].dwX;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemPodList2[i].dwY;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psItemPodList2[i].dwZ;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

   }


   // Maintenant les Creatures
   ///////////////////////////////////////////////////////////////////////////////////////
   uiNbrSpell.dwVal = m_dwNbrCreature;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
	for(i=0; i<m_dwNbrCreature; i++)
	{
      uiNbrSpell.dwVal = m_psCreature[i].dwBindedID;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      uiNbrSpell.dwVal = m_psCreature[i].dwNbrCreatureID;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psCreature[i].pstrCreatureID[j];
         dwWritePos++;
      }

      uiNbrSpell.dwVal = m_psCreature[i].dwNbrName;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_psCreature[i].pstrName[j];
         dwWritePos++;
      }

      uiNbrSpell.dwVal = m_psCreature[i].dwEnd;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwAgi;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwInt;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwVal1;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwVal2;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwVal3;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwVal4;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      uiNbrSpell.dwVal = m_psCreature[i].dwAirResist;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwEarthResist;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwWaterResist;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwFireResist;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwDarkResist;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwLightResist;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwAirPower;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwEarthPower;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwWaterPower;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwFirePower;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwDarkPower;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwLightPower;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwLVL;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwHP;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwDodgeSkill;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      udDoubleVal.dVal = m_psCreature[i].dAc;
      pNewFile[dwWritePos] = udDoubleVal.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[3];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[4];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[5];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[6];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[7];dwWritePos++;

      uiNbrSpell.dwVal = m_psCreature[i].dwAppearance;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwDressBody;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwDressFeet;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwDressGloves;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwDressHelm;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwDressLegs;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwDressWeapon;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwDressShield;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwDressCape;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwAggressivness;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwClanID;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwSpeed;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      udDoubleVal.dVal = m_psCreature[i].dXPperHit;
      pNewFile[dwWritePos] = udDoubleVal.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[3];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[4];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[5];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[6];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[7];dwWritePos++;
      udDoubleVal.dVal = m_psCreature[i].dXPperDeath;
      pNewFile[dwWritePos] = udDoubleVal.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[3];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[4];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[5];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[6];dwWritePos++;
      pNewFile[dwWritePos] = udDoubleVal.chVal[7];dwWritePos++;

      uiNbrSpell.dwVal = m_psCreature[i].dwMinGiveGold;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      uiNbrSpell.dwVal = m_psCreature[i].dwMaxGiveGold;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      pNewFile[dwWritePos] = m_psCreature[i].chCanAttack;dwWritePos++;
      pNewFile[dwWritePos] = m_psCreature[i].chChangeTargetAA;dwWritePos++;

      uiNbrSpell.dwVal = m_psCreature[i].iFriendlyID;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;


      

      

      // recupere le dwNbrEffect
      uiNbrSpell.dwVal = m_psCreature[i].aAttack.GetSize();
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      int dwTmp1 = m_psCreature[i].aAttack.GetSize();
      dwTmp1--;

      // Boucle pour toutes les effets...
		for(;dwTmp1>=0; dwTmp1--)
		{
         uiNbrSpell.dwVal = m_psCreature[i].aAttack[dwTmp1]->dwDmg;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         for(j=0;j<uiNbrSpell.dwVal;j++)
         {
            pNewFile[dwWritePos] = m_psCreature[i].aAttack[dwTmp1]->pstrDmg[j];
            dwWritePos++;
         }
         
         uiNbrSpell.dwVal = m_psCreature[i].aAttack[dwTmp1]->dwAttackSkill;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         uiNbrSpell.dwVal = m_psCreature[i].aAttack[dwTmp1]->dwAttackPercentage;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         uiNbrSpell.dwVal = m_psCreature[i].aAttack[dwTmp1]->dwAttackSpell;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         uiNbrSpell.dwVal = m_psCreature[i].aAttack[dwTmp1]->dwAttackMinRange;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         uiNbrSpell.dwVal = m_psCreature[i].aAttack[dwTmp1]->dwAttackMaxRange;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      }

      // recupere le dwNbrEffect
      uiNbrSpell.dwVal = m_psCreature[i].aDeadFlag.GetSize();
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      dwTmp1 = m_psCreature[i].aDeadFlag.GetSize();
      dwTmp1--;

      // Boucle pour toutes les effets...
		for(;dwTmp1>=0; dwTmp1--)
		{
         uiNbrSpell.dwVal = m_psCreature[i].aDeadFlag[dwTmp1]->dwFlag;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         uiNbrSpell.dwVal = m_psCreature[i].aDeadFlag[dwTmp1]->dwFlagValue;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

         pNewFile[dwWritePos] = m_psCreature[i].aDeadFlag[dwTmp1]->chIncrement;dwWritePos++;
      }

      // recupere le dwNbrEffect
      uiNbrSpell.dwVal = m_psCreature[i].aDrop.GetSize();
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      dwTmp1 = m_psCreature[i].aDrop.GetSize();
      dwTmp1--;

      // Boucle pour toutes les effets...
		for(;dwTmp1>=0; dwTmp1--)
		{
         uiNbrSpell.dwVal = m_psCreature[i].aDrop[dwTmp1]->dwItemID;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

         udDoubleVal.dVal = m_psCreature[i].aDrop[dwTmp1]->dDropPercentage;
         pNewFile[dwWritePos] = udDoubleVal.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = udDoubleVal.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = udDoubleVal.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = udDoubleVal.chVal[3];dwWritePos++;
         pNewFile[dwWritePos] = udDoubleVal.chVal[4];dwWritePos++;
         pNewFile[dwWritePos] = udDoubleVal.chVal[5];dwWritePos++;
         pNewFile[dwWritePos] = udDoubleVal.chVal[6];dwWritePos++;
         pNewFile[dwWritePos] = udDoubleVal.chVal[7];dwWritePos++;
         
      }
	}

   ////////////////////////////////////////////////////////////////////////////////////
   // mainternant on dois ecrire les Hives...
   //NMNMNM

   i = dwWritePos;

   uIntToChar uiNbrHive;
   uiNbrHive.dwVal = m_dwNbrHives;
   pNewFile[i] = uiNbrHive.chVal[0];i++;
   pNewFile[i] = uiNbrHive.chVal[1];i++;
   pNewFile[i] = uiNbrHive.chVal[2];i++;
   pNewFile[i] = uiNbrHive.chVal[3];i++;

   for(j=0; j<m_dwNbrHives; j++)
	{
      // recupere dwMinEmergencyTime
      uiNbrHive.dwVal = m_psHivesList[j].dwMinEmergencyTime;
      pNewFile[i] = uiNbrHive.chVal[0];i++;
      pNewFile[i] = uiNbrHive.chVal[1];i++;
      pNewFile[i] = uiNbrHive.chVal[2];i++;
      pNewFile[i] = uiNbrHive.chVal[3];i++;

      // recupere dwMaxEmergencyTime
      uiNbrHive.dwVal = m_psHivesList[j].dwMaxEmergencyTime;
      pNewFile[i] = uiNbrHive.chVal[0];i++;
      pNewFile[i] = uiNbrHive.chVal[1];i++;
      pNewFile[i] = uiNbrHive.chVal[2];i++;
      pNewFile[i] = uiNbrHive.chVal[3];i++;

      // recupere dwMaxChildren
      uiNbrHive.dwVal = m_psHivesList[j].dwMaxChildren;
      pNewFile[i] = uiNbrHive.chVal[0];i++;
      pNewFile[i] = uiNbrHive.chVal[1];i++;
      pNewFile[i] = uiNbrHive.chVal[2];i++;
      pNewFile[i] = uiNbrHive.chVal[3];i++;

      // recupere dwEmergencyRange
      uiNbrHive.dwVal = m_psHivesList[j].dwEmergencyRange;
      pNewFile[i] = uiNbrHive.chVal[0];i++;
      pNewFile[i] = uiNbrHive.chVal[1];i++;
      pNewFile[i] = uiNbrHive.chVal[2];i++;
      pNewFile[i] = uiNbrHive.chVal[3];i++;

      // recupere dwLarva
      uiNbrHive.dwVal = m_psHivesList[j].dwLarva;
      pNewFile[i] = uiNbrHive.chVal[0];i++;
      pNewFile[i] = uiNbrHive.chVal[1];i++;
      pNewFile[i] = uiNbrHive.chVal[2];i++;
      pNewFile[i] = uiNbrHive.chVal[3];i++;

      for(k=0;k<uiNbrHive.dwVal;k++)
      {
         pNewFile[i] = m_psHivesList[j].pstrLarva[k];
         i++;
      }

      // recupere dwNbrlarvaGrp
      uiNbrHive.dwVal = m_psHivesList[j].dwNbrlarvaGrp;
      pNewFile[i] = uiNbrHive.chVal[0];i++;
      pNewFile[i] = uiNbrHive.chVal[1];i++;
      pNewFile[i] = uiNbrHive.chVal[2];i++;
      pNewFile[i] = uiNbrHive.chVal[3];i++;
      
      //for( k=0;k<m_psHivesList[j].aLarvaGrp.GetSize();k++)
      for( k=m_psHivesList[j].aLarvaGrp.GetSize()-1;k>=0;k--)
      {
         // recupere dwLarveGrp
         uiNbrHive.dwVal = m_psHivesList[j].aLarvaGrp[k]->dwLarveGrp;
         pNewFile[i] = uiNbrHive.chVal[0];i++;
         pNewFile[i] = uiNbrHive.chVal[1];i++;
         pNewFile[i] = uiNbrHive.chVal[2];i++;
         pNewFile[i] = uiNbrHive.chVal[3];i++;

         for(int l=0;l<uiNbrHive.dwVal;l++)
         {
            pNewFile[i] = m_psHivesList[j].aLarvaGrp[k]->pstrlarvaGrp[l];
            i++;
         }
      }

      // recupere dwNbrPod
      uiNbrHive.dwVal = m_psHivesList[j].dwNbrPod;
      pNewFile[i] = uiNbrHive.chVal[0];i++;
      pNewFile[i] = uiNbrHive.chVal[1];i++;
      pNewFile[i] = uiNbrHive.chVal[2];i++;
      pNewFile[i] = uiNbrHive.chVal[3];i++;
      for(k=0;k<m_psHivesList[j].aHivePod.GetSize();k++)
      {
         // recupere dwX
         uiNbrHive.dwVal = m_psHivesList[j].aHivePod[k]->dwX;
         pNewFile[i] = uiNbrHive.chVal[0];i++;
         pNewFile[i] = uiNbrHive.chVal[1];i++;
         pNewFile[i] = uiNbrHive.chVal[2];i++;
         pNewFile[i] = uiNbrHive.chVal[3];i++;

         // recupere dwY
         uiNbrHive.dwVal = m_psHivesList[j].aHivePod[k]->dwY;
         pNewFile[i] = uiNbrHive.chVal[0];i++;
         pNewFile[i] = uiNbrHive.chVal[1];i++;
         pNewFile[i] = uiNbrHive.chVal[2];i++;
         pNewFile[i] = uiNbrHive.chVal[3];i++;

         // recupere dwZ
         uiNbrHive.dwVal = m_psHivesList[j].aHivePod[k]->dwZ;
         pNewFile[i] = uiNbrHive.chVal[0];i++;
         pNewFile[i] = uiNbrHive.chVal[1];i++;
         pNewFile[i] = uiNbrHive.chVal[2];i++;
         pNewFile[i] = uiNbrHive.chVal[3];i++;
      }
   }
   ////////////////////////////////////////////////////////////////////////////////////
   // mainternant on dois ecrire les AreaLink...
   uIntToChar uiNbrLink;
   uiNbrLink.dwVal = m_sLinkArea.GetSize();
   pNewFile[i] = uiNbrLink.chVal[0];i++;
   pNewFile[i] = uiNbrLink.chVal[1];i++;
   pNewFile[i] = uiNbrLink.chVal[2];i++;
   pNewFile[i] = uiNbrLink.chVal[3];i++;

   for(int dwPosLink = 0;dwPosLink<m_sLinkArea.GetSize();dwPosLink++)
   {
      // Source X
      uiNbrLink.dwVal = m_sLinkArea[dwPosLink].dwSrcX;
      pNewFile[i] = uiNbrLink.chVal[0];i++;
      pNewFile[i] = uiNbrLink.chVal[1];i++;
      pNewFile[i] = uiNbrLink.chVal[2];i++;
      pNewFile[i] = uiNbrLink.chVal[3];i++;
      // Source Y
      uiNbrLink.dwVal = m_sLinkArea[dwPosLink].dwSrcY;
      pNewFile[i] = uiNbrLink.chVal[0];i++;
      pNewFile[i] = uiNbrLink.chVal[1];i++;
      pNewFile[i] = uiNbrLink.chVal[2];i++;
      pNewFile[i] = uiNbrLink.chVal[3];i++;
      // Source Z
      uiNbrLink.dwVal = m_sLinkArea[dwPosLink].dwSrcZ;
      pNewFile[i] = uiNbrLink.chVal[0];i++;
      pNewFile[i] = uiNbrLink.chVal[1];i++;
      pNewFile[i] = uiNbrLink.chVal[2];i++;
      pNewFile[i] = uiNbrLink.chVal[3];i++;
      // Dest X
      uiNbrLink.dwVal = m_sLinkArea[dwPosLink].dwDesX;
      pNewFile[i] = uiNbrLink.chVal[0];i++;
      pNewFile[i] = uiNbrLink.chVal[1];i++;
      pNewFile[i] = uiNbrLink.chVal[2];i++;
      pNewFile[i] = uiNbrLink.chVal[3];i++;
      // Dest Y
      uiNbrLink.dwVal = m_sLinkArea[dwPosLink].dwDesY;
      pNewFile[i] = uiNbrLink.chVal[0];i++;
      pNewFile[i] = uiNbrLink.chVal[1];i++;
      pNewFile[i] = uiNbrLink.chVal[2];i++;
      pNewFile[i] = uiNbrLink.chVal[3];i++;
      // Dest Z
      uiNbrLink.dwVal = m_sLinkArea[dwPosLink].dwDesZ;
      pNewFile[i] = uiNbrLink.chVal[0];i++;
      pNewFile[i] = uiNbrLink.chVal[1];i++;
      pNewFile[i] = uiNbrLink.chVal[2];i++;
      pNewFile[i] = uiNbrLink.chVal[3];i++;
   }
   dwWritePos = i;


   uiNbrSpell.dwVal = m_dwhighClan;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
   uiNbrSpell.dwVal = m_dwNbrClan;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

	for(i=0; i<m_dwNbrClan; i++)
	{
      ushVal.shVal = m_sClan[i].shClan1;
      pNewFile[dwWritePos] = ushVal.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = ushVal.chVal[1];dwWritePos++;

      ushVal.shVal = m_sClan[i].shClan2;
      pNewFile[dwWritePos] = ushVal.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = ushVal.chVal[1];dwWritePos++;

      ushVal.shVal = m_sClan[i].shNiveaux;
      pNewFile[dwWritePos] = ushVal.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = ushVal.chVal[1];dwWritePos++;
   }

   uiNbrSpell.dwVal = m_dwNbrClanName;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
	for(i=0; i<m_dwNbrClanName; i++)
	{
      ushVal.shVal = m_sClanName[i].shClanIndex;
      pNewFile[dwWritePos] = ushVal.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = ushVal.chVal[1];dwWritePos++;

      // recupere dwLarveGrp
      uiNbrHive.dwVal = m_sClanName[i].dwName;
      pNewFile[dwWritePos] = uiNbrHive.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrHive.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrHive.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrHive.chVal[3];dwWritePos++;

      for(l=0;l<uiNbrHive.dwVal;l++)
      {
         pNewFile[dwWritePos] = m_sClanName[i].pstrName[l];
         dwWritePos++;
      }
   }

   uiNbrSpell.dwVal = m_dwNbrQuestFlag;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
   pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
	for(i=0; i<m_dwNbrQuestFlag; i++)
	{
      uiNbrSpell.dwVal = m_sQuestFlag[i].dwFlagValue;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

      uiNbrSpell.dwVal = m_sQuestFlag[i].dwName;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      for(j=0;j<uiNbrSpell.dwVal;j++)
      {
         pNewFile[dwWritePos] = m_sQuestFlag[i].pstrName[j];
         dwWritePos++;
      }
   }


   // Maintenant les item Type...
   pNewFile[dwWritePos] = m_chIsMiscWDAData;dwWritePos++;

   if(m_chIsMiscWDAData == 1)
   {

      uiNbrSpell.dwVal = m_dwNbrAppearanceItem;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      
      for(i=0;i<m_dwNbrAppearanceItem;i++)
      {
         
         uiNbrSpell.dwVal = m_sAppearanceItem[i].dwID;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         
         uiNbrSpell.dwVal = m_sAppearanceItem[i].dwNbrName;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         for(j=0;j<uiNbrSpell.dwVal;j++)
         {
            pNewFile[dwWritePos] = m_sAppearanceItem[i].pstrName[j];
            dwWritePos++;
         }
      }

      // Maintenant les item Monster...
      uiNbrSpell.dwVal = m_dwNbrAppearanceMonster;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      
      for(i=0;i<m_dwNbrAppearanceMonster;i++)
      {
         
         uiNbrSpell.dwVal = m_sAppearanceMonster[i].dwID;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

         uiNbrSpell.dwVal = m_sAppearanceMonster[i].dwIDC;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         
         uiNbrSpell.dwVal = m_sAppearanceMonster[i].dwNbrName;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         for(j=0;j<uiNbrSpell.dwVal;j++)
         {
            pNewFile[dwWritePos] = m_sAppearanceMonster[i].pstrName[j];
            dwWritePos++;
         }
      }

      // Maintenant les item Monster...
      uiNbrSpell.dwVal = m_dwNbrItemLocation;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      
      for(i=0;i<m_dwNbrItemLocation;i++)
      {
         uiNbrSpell.dwVal = m_sItemLocation[i].dwNbrName;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         for(j=0;j<uiNbrSpell.dwVal;j++)
         {
            pNewFile[dwWritePos] = m_sItemLocation[i].pstrName[j];
            dwWritePos++;
         }
      }

      // Maintenant les Spell Icon...
      uiNbrSpell.dwVal = m_dwNbrSpellIcon;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      
      for(i=0;i<m_dwNbrSpellIcon;i++)
      {
         uiNbrSpell.dwVal = m_sSpellIcon[i].dwID;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

         uiNbrSpell.dwVal = m_sSpellIcon[i].dwNbrName;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         for(j=0;j<uiNbrSpell.dwVal;j++)
         {
            pNewFile[dwWritePos] = m_sSpellIcon[i].pstrName[j];
            dwWritePos++;
         }
      }

      // Maintenant les Spell Icon...
      uiNbrSpell.dwVal = m_dwNbrVisualEffect;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
      pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
      
      for(i=0;i<m_dwNbrVisualEffect;i++)
      {
         uiNbrSpell.dwVal = m_sVisualEffect[i].dwID;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;

         uiNbrSpell.dwVal = m_sVisualEffect[i].dwNbrName;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[0];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[1];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[2];dwWritePos++;
         pNewFile[dwWritePos] = uiNbrSpell.chVal[3];dwWritePos++;
         for(j=0;j<uiNbrSpell.dwVal;j++)
         {
            pNewFile[dwWritePos] = m_sVisualEffect[i].pstrName[j];
            dwWritePos++;
         }
      }
   }


   

   // Maintenant le fin du fichier les bytes restant...
   /////////////////////////////////////////////////////////////////////////////////
   
   /*
   BYTE *pchTmpDesE = pNewFile+dwWritePos;
   if(m_dwendFileData > 0)
   {
      memcpy(pchTmpDesE,m_pchAllEndFileData,m_dwendFileData);
      dwWritePos += m_dwendFileData;
   }
   */
   //Plus de reste...
   


   if(m_dwWDACrypt == 1)
   {
      //Reencode le fichier WDA      
      for ( i=0; i<dwNewtaille; i++)
         pNewFile[i] ^= WDA_key[i%3418];
   }
   // Oubre le Nouveau fichier WDA
   FILE	*hfile = NULL;
   fopen_s(&hfile,m_strWDAName,"wb");
   if(hfile)
   {
      fwrite(pNewFile,dwNewtaille,1,hfile);
      fclose(hfile);
   }
   delete []pNewFile;
   return TRUE;
}

void CWDAUtils::CreateUndo(int w)
{
   memcpy(m_sMapInfo[w].pMapDataUndo,m_sMapInfo[w].pMapData,(m_sMapInfo[w].shLongeur*m_sMapInfo[w].shHauteur)/2);
   m_sMapInfo[w].bMapUndoPossible = TRUE;
}

void CWDAUtils::RestoreUndo(int w)
{
   if(m_sMapInfo[w].bMapUndoPossible)
   {
      memcpy(m_sMapInfo[w].pMapData,m_sMapInfo[w].pMapDataUndo,(m_sMapInfo[w].shLongeur*m_sMapInfo[w].shHauteur)/2);
      m_sMapInfo[w].bMapUndoPossible = FALSE;
   }
}



BOOL CWDAUtils::PatchSpellDamage()
{
   //Scann All Spell
   for(int i=0; i<m_dwNbrSpell; i++)
	{
      //Scann All Effect
      for(int e=0;e<m_psSpellList[i].SpellE.GetSize();e++)
      {
         if(m_psSpellList[i].SpellE[e]->dwEffect == 1) //Heal Modifier
         {
         
            int dwTmp2 = m_psSpellList[i].SpellE[e]->dwNbrEffectParam;
            

            //verifie si ya un area damage....
            if(m_psSpellList[i].SpellE[e]->pSpellEffectP[1].pstrParam2 && m_psSpellList[i].SpellE[e]->pSpellEffectP[2].pstrParam2)
            {
               
               if( _stricmp(m_psSpellList[i].SpellE[e]->pSpellEffectP[1].pstrParam2,m_psSpellList[i].SpellE[e]->pSpellEffectP[2].pstrParam2) == 0 ||
                   strstr (m_psSpellList[i].SpellE[e]->pSpellEffectP[2].pstrParam2,m_psSpellList[i].SpellE[e]->pSpellEffectP[1].pstrParam2) == NULL )
               {
                  TRACE("Spell Affecter [%s]\n",m_psSpellList[i].pstrNom);   
                  // on dois ajouter la string 1 a la string 2...

                  TRACE("Old [%s]\n",m_psSpellList[i].SpellE[e]->pSpellEffectP[2].pstrParam2);   
                  char strNewString[2048];
                  sprintf_s(strNewString,2048,"%s + %s",m_psSpellList[i].SpellE[e]->pSpellEffectP[2].pstrParam2,m_psSpellList[i].SpellE[e]->pSpellEffectP[1].pstrParam2);
                  if(m_psSpellList[i].SpellE[e]->pSpellEffectP[2].pstrParam2)
                     delete []m_psSpellList[i].SpellE[e]->pSpellEffectP[2].pstrParam2;

                  m_psSpellList[i].SpellE[e]->pSpellEffectP[2].dwParam2 = strlen(strNewString);
                  if(m_psSpellList[i].SpellE[e]->pSpellEffectP[2].dwParam2 > 0)
                  {
                     m_psSpellList[i].SpellE[e]->pSpellEffectP[2].pstrParam2 = new char[strlen(strNewString)+1];
                     strcpy_s(m_psSpellList[i].SpellE[e]->pSpellEffectP[2].pstrParam2,strlen(strNewString)+1,strNewString);
                  }
                  else
                     m_psSpellList[i].SpellE[e]->pSpellEffectP[2].pstrParam2 = NULL;
                  
                  TRACE("New [%s]\n\n",m_psSpellList[i].SpellE[e]->pSpellEffectP[2].pstrParam2);   



               }
               if(strstr (m_psSpellList[i].SpellE[e]->pSpellEffectP[2].pstrParam2,m_psSpellList[i].SpellE[e]->pSpellEffectP[1].pstrParam2) == NULL )
               {
                  return FALSE;
               }
            }
         }
      }
   }
   
   return TRUE;
}



void CWDAUtils::GetQuestFlag(int iIndex, unsigned short &ushID,CString &strName)
{
   if(iIndex >=0 && iIndex < m_dwNbrQuestFlag)
   {
      ushID = m_sQuestFlag[iIndex].dwFlagValue;
      strName.Format("%s",m_sQuestFlag[iIndex].pstrName);
   }
}