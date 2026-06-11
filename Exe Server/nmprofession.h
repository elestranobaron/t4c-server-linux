#pragma once




#define PROF_APOTICAIRE  0
#define PROF_BIJOUTIER   1
#define PROF_COUTURIER   2
#define PROF_ARMURIER    3
#define PROF_FORGERON    4
#define PROF_EBENISTE    5




typedef struct _sRequestItem
{
   USHORT  ushRequestID;
   USHORT  ushQty;
}sRequestItem;

typedef struct _sFormule
{

   BYTE    chProfession;           // la profession
   USHORT  ushSkillLevel;          // le skill level pour cet items
   USHORT  ushItemResultID;        // le ID de l'item fabriquer...
   USHORT  ushItemResultQty;       // la quantite d<item creer
   USHORT  ushNbrRequestID;        // le nombre d'item necessaire
   sRequestItem *pRequestItemList; // la liste de chaque items avec leur quantite
   int     iFormuleCreateSize;     // le nombre de character de la formule de creation 
   char    *pstrFormuleCreate;     // la formule de % de chance de creation resultat devrait etre 0 a 100%
   int     iFormuleCompGagner;     // le nombre de character de la formule de competance gagner 
   char    *pstrFormuleCompGagner; // la formule de competence gagner (de 0 a X nbr point gagner)
   USHORT  ushID;                  // ID Unique de cette formule
   BYTE    chEnable;               // si cette formule est active ou pas...
}sFormule;


typedef union _IntToChar
{
   BYTE chVal[4];
   int  dwVal;
}IntToChar;

typedef union _ShortToChar
{
   BYTE chVal[2];
   short  shVal;
}ShortToChar;

typedef union _UShortToChar
{
   BYTE chVal[2];
   unsigned short  ushVal;
}UShortToChar;





