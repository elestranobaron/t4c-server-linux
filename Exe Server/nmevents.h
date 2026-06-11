#pragma once




#define PROF_APOTICAIRE  0
#define PROF_BIJOUTIER   1
#define PROF_COUTURIER   2
#define PROF_ARMURIER    3
#define PROF_FORGERON    4
#define PROF_EBENISTE    5




typedef struct _sSummonRequestM
{
   int           iSize;
   char         *pstrSN;
   USHORT        ushX;
   USHORT        ushY;
   USHORT        ushW;
   USHORT        ushQty;
}sSummonRequestM;

typedef struct _sSummonRequestI
{
   unsigned int  uiID;
   USHORT        ushX;
   USHORT        ushY;
   USHORT        ushW;
   USHORT        ushQty;
}sSummonRequestI;

typedef struct _sEvents
{
   BYTE    chEnable;               // si cette formule est active ou pas...
   USHORT  ushID;                  // ID Unique de Cet Event
   int     iNameSize;              // le nombre de byte du nom
   char    *pstrName;              // le nom
   int     iStartMsg;              // le nombre de byte Message start
   char    *pstrStartMsg;          // le Message start
   int     iStopMsg;               // le nombre de byte Message stop
   char    *pstrStopMsg;           // le Message stop
   int     iFlagID;                // le no du flag associer
   int     iFlagStopValue;         // la valeur du flag pour arreter levent
   int     iLevelMin;              // le level min
   int     iLevelMax;              // le level max
   USHORT  ushNbrSummonMonster;    // le Nbr de Monstre a summoner
   sSummonRequestM *pMonsterList;  // La liste des monstre / NPC a summoner
   USHORT  ushNbrSummonItem;       // le Nbr de Monstre a summoner
   sSummonRequestI *pItemList;     // La liste des monstre / NPC a summoner
}sEvents;







