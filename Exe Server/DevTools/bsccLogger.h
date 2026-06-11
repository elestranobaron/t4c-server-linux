//--FILE DEFINITION--------------------------------------------------------
//
/*! \file
	$Logfile: /065591/Software/BSCCCoreService/BSCCCoreService/bsccLogger.h $

	$Author: User0559 $

	$Date: 8-25-08 4:20p $

	Library:

	$Revision: 2 $

	Address: INO, Sainte-Foy, Quebec, Canada.  (418) 657-7006

	Usage:

	Notes:

	Legal Notice: Copyright © 2008 INO, All Rights Reserved

	$History: bsccLogger.h $
 * 
 * *****************  Version 2  *****************
 * User: User0559     Date: 8-25-08    Time: 4:20p
 * Updated in $/065591/Software/BSCCCoreService/BSCCCoreService
 * 
 * *****************  Version 1  *****************
 * User: User0559     Date: 6-26-08    Time: 12:27p
 * Created in $/065591/Software/BSCCCoreService/BSCCCoreService
*/
//-------------------------------------------------------------------------
#pragma once

//--FILE INCLUDES--------------------------------------------------------
#include "NMSharedMem.h"

//--FILE DEFINITIONS--------------------------------------------------------
#define LOG_L1          0x01
#define LOG_L2          0x02
#define LOG_LD          0x09

#define LOG_SHARED_MAX  0x64 //100 max entry...

//--CLASS DEFINITION-------------------------------------------------------
//
//  Class name: bsccLogger
//
/*! Notes:        
*/
//-------------------------------------------------------------------------
class bsccLogger
{
public:
   //--CONSTRUCTOR-------------------------------------
   bsccLogger(void);
   //--DESTRUCTOR--------------------------------------
   ~bsccLogger(void);

   //--PUBLIC METHODS-------------------------------
   inline void SetLevel(int iLevel){m_iLogLevel = iLevel;}
   inline void DisplayOnConsole(bool bDisplay){m_bDisplayConsole = bDisplay;}
   inline void DisplayOnDebug(bool bDisplay){m_bDisplayDebug = bDisplay;}
   inline void SendToShare(bool bSend){m_bSendShare = bSend;}
   static bsccLogger* GetInstance( void );
   void Add(int iLevel,LPCTSTR pstrModuleName,LPCTSTR pstrMessage);
   void AddFmt(int iLevel,LPCTSTR pstrModuleName,LPCTSTR pszFmt, ...);



protected:
   //--PROTECTED METHODS----------------------------

private:
   //--PRIVATE METHODS------------------------------



public:
   //--PUBLIC ATTRIBUTES----------------------------

protected:
   //--PROTECTED ATTRIBUTES-------------------------
   char m_strLogFile[MAX_PATH];

   typedef struct _sLogSharedData
   {
      int iLevel;          
      char strTime[25];
      char strName[30];
      char strMsg[2048];
   }sLogSharedData;

   typedef struct _sLogShared
   {
      int iNbrLogMax;          //must be LOG_SHARED_MAX
      int iNbrLogWrited;
      int iStartRead;
      sLogSharedData sLogData[LOG_SHARED_MAX];
   }sLogShared;

private:
   //--PRIVATE ATTRIBUTES---------------------------
   CRITICAL_SECTION      m_crLock;
   int                   m_iLogLevel;
   bool                  m_bDisplayConsole;
   bool                  m_bDisplayDebug;
   bool                  m_bSendShare;

   CNMSharedMem         *m_pSharedLog;


};
