// WDAUtils.h: interface for the CWDAUtils class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WDAUTILS_H__3E00521F_3672_4C26_9EF5_63DC21DC5DF8__INCLUDED_)
#define AFX_WDAUTILS_H__3E00521F_3672_4C26_9EF5_63DC21DC5DF8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WDAStruct.h"

#define NBR_MAP_WORLD    10
#define MAP_X_SIZE    3072
#define MAP_Y_SIZE    3072

typedef struct _sNomPod
{
   char strPodName[100];
   int dwID;
}sNomPod;

class CWDAUtils  
{
public:
	CWDAUtils();
	virtual ~CWDAUtils();

   void SetWDAName(char*pstrName,int dwCrypt);
   void Initialize();
   void Free();
   void SaveUncomp(char*pstrName);

public:
   BOOL IsInit(){return m_bWDAInit;}
   int   GetMapWidth (int dwMapID){return m_sMapInfo[dwMapID].shLongeur;}
   int   GetMapHeight(int dwMapID){return m_sMapInfo[dwMapID].shHauteur;}
   BYTE *GetMapData  (int dwMapID){return m_sMapInfo[dwMapID].pMapData;}
   int  GetCollision(USHORT x, USHORT y, USHORT w);
   void SetCollision(USHORT  x, USHORT  y, USHORT  w, BYTE value);
   
   int  GetNbrMonsterHives(){return m_dwNbrHives;}
   void GetMonsterHiveInfo(int dwIndex, char *pstrName,int iNameSize, int *pdwSkinID);
   void GetMonsterHiveInfo2(int dwIndex,unsigned short &ushID,CString &strName);

   int  GetNbrMonster(){return m_dwNbrCreature;}
   void GetMonsterInfo2(int dwIndex,unsigned short &ushID,CString &strName);

   int  GetNbrItemHives(){return m_dwNbrItem;}
   void GetItemInfo(int dwIndex,char *pstrName,int iNameSize,int *pdwSkinID);
   void GetItemInfo2(int dwIndex,unsigned short &ushID,CString &strName);


   int  GetNbrSpellsHives(){return m_dwNbrSpell;}
   void GetSpellInfo2(int dwIndex,unsigned short &ushID,CString &strName);

   int  GetNbrItemHivesPod(){return m_dwNbrItemPod;}

   int GetNbrAreaLink(){return m_sLinkArea.GetSize();}
   void GetAreaLink(int dwIndex,int *dwSX,int *dwSY,int *dwSW,int *dwDX,int *dwDY,int *dwDW);
   void DeleteAreaLink(int dwSX,int dwSY,int dwSW,int dwDX,int dwDY,int dwDW);
   void AddAreaLink(int dwSX,int dwSY,int dwSW,int dwDX,int dwDY,int dwDW);

   int  GetNbrArealink(){return m_sLinkArea.GetCount();}
   void GetArealinkInfo(int dwIndex,USHORT &xS,USHORT &yS,USHORT &wS,USHORT &xD,USHORT &yD,USHORT &wD);


   BOOL GetAreaLinkDest  (USHORT  x, USHORT  y, USHORT  w,USHORT  *px, USHORT  *py, USHORT  *pw);
   BOOL GetAreaLinkSource(USHORT  x, USHORT  y, USHORT  w,USHORT  *px, USHORT  *py, USHORT  *pw);

   CArray <AppearanceMonster,AppearanceMonster> * GetMonsterAppList()   {return &m_sAppearanceMonster;}
   CArray <AppearanceItem   ,AppearanceItem   > * GetItemAppList()      {return &m_sAppearanceItem;}
   CArray <VisualEffect     ,VisualEffect     > * GetVisualEffectList() {return &m_sVisualEffect;}
   CArray <sLinkArea        ,sLinkArea        > * GetArealinkList()     {return &m_sLinkArea;}

   int GetNbrItems(){return m_dwNbrItem;}
   sItems * GetItemsList(){return m_psItemList;}


   int GetNbrQuestFlag(){return m_dwNbrQuestFlag;}
   void GetQuestFlag(int iIndex, unsigned short &ushID,CString &strName);



   BOOL PatchSpellDamage();

protected:
   CString m_strWDAName;
   int     m_dwWDACrypt;
   BOOL    m_bWDAInit;

   BYTE	 *m_pData;
   BYTE	 *m_pDataTmp;
   int 	 m_dwTaille;
   char   m_chLigne[2048];
   int     m_dwNbrMap;
   MapInfo m_sMapInfo[NBR_MAP_WORLD];
   int     m_dwMapOldPos;

   int    m_dwNbrSpell;
   sSpell *m_psSpellList;
   int    m_dwNbrHives;
   sHives *m_psHivesList;
   int    m_dwNbrItem;
   sItems *m_psItemList;
   int    m_dwNbrItemPod;
   CArray <sItemsPod,sItemsPod> m_psItemPodList2;

   int    m_dwNbrCreature;
   sCreature *m_psCreature;
   
   int       m_dwNbrLinkArea;
   CArray <sLinkArea,sLinkArea> m_sLinkArea;
   
   int       m_dwhighClan;
   int       m_dwNbrClan;
   int       m_dwNbrClanName;
   CArray <sClan,sClan> m_sClan;
   CArray <sClanName,sClanName> m_sClanName;

   int       m_dwNbrQuestFlag;
   CArray <sQuestFlag,sQuestFlag> m_sQuestFlag;

   char   m_chIsMiscWDAData;
   int    m_dwNbrAppearanceItem;
   CArray <AppearanceItem,AppearanceItem> m_sAppearanceItem;

   int    m_dwNbrAppearanceMonster;
   CArray <AppearanceMonster,AppearanceMonster> m_sAppearanceMonster;

   int    m_dwNbrItemLocation;
   CArray <ItemLocation,ItemLocation> m_sItemLocation;

   int    m_dwNbrSpellIcon;
   CArray <SpellIcon,SpellIcon> m_sSpellIcon;
   
   int    m_dwNbrVisualEffect;
   CArray <VisualEffect,VisualEffect> m_sVisualEffect;


   int m_dwendFileData;
   BYTE *m_pchAllEndFileData;

  

   BYTE   *m_pListMapPos;
   BYTE   *m_pListItemPos;
   BYTE   *m_pListCreaturePos;
   BYTE   *m_pListHivesPos;
   BYTE   *m_pListAreaLinkPos;
   BYTE   *m_pListClanPos;

   COLORREF m_crTableau[16];
   CString  m_strSurfaceType[16];

public:
   BOOL SaveWDA();
   void CreateUndo(int w);
   void RestoreUndo(int w);
protected:
   BOOL LoadWDA();
   

   // FONCTION UTILE POUR LE FICHIER WDa
   //Fonction qui lit les data, et incremente le ptr TMP
   unsigned int  get_dword();
   unsigned short get_word();
   BYTE get_byte();
   double get_double();
   int get_long();
   short get_short();
   char* get_string();

   void ReadListSpell();    // permet de modifier des spell existant... aucun delete ou noiuvelle creation
   void DeleteSpellList();  // delete la structure des spell 
   int  CalculeSpellSize(); // permet de retourner le nombre de byte de la strucure de SpellList

   void GetMapInfo();
   int  CalculeMapSize();  // permet de retourner le nombre de byte de la strucure des Map
   void DeleteMapInfo();
   
   void ReadListItems();    // lit la liste des item et item pod
   void DeleteItemList();   // delete la structure des item et des itempod...
   int  CalculeItemSize();  // permet de retourner le nombre de byte de la strucure de Item et ItemPod

   void ReadListCreature();     // lit la liste des creatures
   void DeleteCreatureList();   // delete la structure des item et des itempod...
   int  CalculeCreatureSize();  // retourne le total de la structure des creature
   
   void ReadListHives();    // permet de changer les hives...
   void DeleteHivesList();  // delete la structure des Hives
   int  CalculeHivesSize(); // permet de retourner le nombre de byte de la strucure de HiveList

   void ReadListLinkArea(); // La structure existe et permet d'ajouter et supprimer des point de teleport
   void DeleteLinkArea();
   int  CalculeAreaSize();

   void ReadListClan();     // Junk ... Aucune structure existe
   void DeleteClanList();
   int  CalculeClanSize();

   void ReadQuestFlag();
   void DeleteQuestList();
   int  CalculeQuestSize();

   void ReadAppearanceItem();
   void DeleteAppearanceItem();
   int  CalculeAppearanceItemSize();
   
   void ReadAppearanceMonster();
   void DeleteAppearanceMonster();
   int  CalculeAppearanceMonsterSize();

   void ReadItemLocation();
   void DeleteItemLocation();
   int  CalculeItemLocationSize();

   void ReadSpellIcon();
   void DeleteSpellIcon();
   int  CalculeSpellIconSize();

   void ReadVisualEffect();
   void DeleteVisualEffect();
   int  CalculeVisualEffectSize();

  

   int ColorToIndex(COLORREF crColor);

};

#endif // !defined(AFX_WDAUTILS_H__3E00521F_3672_4C26_9EF5_63DC21DC5DF8__INCLUDED_)
