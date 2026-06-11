#pragma once

#include "NMProfession.h"

typedef struct _sQuestStepMsg
{
   DWORD dwLength; 
   char *pstrStep;
}sQuestStepMsg;

typedef struct _sQuestBook
{
   BYTE          chEnable;               // si cette QUEST est active ou pas...
   USHORT        ushUniqueID;            // le id unique de cette quest book
   DWORD         dwFlagID;               // le flag atacher a cette quest
   USHORT        ushQuestLevel;          // Niveau de la quete
   DWORD         dwNameLength;           // nombre de char de la quete
   char*         pStrName;               // nom de la quete
   DWORD         dwMsgLength;            // nombre de char presentation de la quete
   char*         pStrMsg;                // message presentation de la quete
   DWORD         dwNbrQuestStep;         // le nombre de step de la quest
   sQuestStepMsg *pStepMsg;              // la liste des message pour chaque step
}sQuestBook;






