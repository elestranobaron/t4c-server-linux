//--FILE DEFINITION--------------------------------------------------------
//
/*! \file
	$Logfile: /065591/Software/BSCCCoreService/BSCCCoreService/bsccLogger.cpp $

	$Author: User0559 $

	$Date: 8-25-08 4:20p $

	Library:

	$Revision: 2 $

	Address: INO, Sainte-Foy, Quebec, Canada.  (418) 657-7006

	Usage:

	Notes:

	Legal Notice: Copyright © 2008 INO, All Rights Reserved

	$History: bsccLogger.cpp $
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

//--FILE INCLUDES--------------------------------------------------------
#include "stdafx.h"
#include "windows.h"
#include "bsccLogger.h"
#include <cstdio>



//!--CONSTRUCTOR-----------------------------------------------------------
//
/*!
Notes
*/
//-------------------------------------------------------------------------
bsccLogger::bsccLogger(void)
{ 
   InitializeCriticalSection(&m_crLock);

   //get the directory of the application
   char strModName[MAX_PATH];
   char drive[_MAX_DRIVE];
   char dir[_MAX_DIR];
   char fname[_MAX_FNAME];
   char ext[_MAX_EXT];

   GetModuleFileName(NULL, strModName, MAX_PATH);
   _splitpath_s( strModName, drive,_MAX_DRIVE, dir,_MAX_DIR, fname,_MAX_FNAME, ext,_MAX_EXT );
   sprintf_s(m_strLogFile,MAX_PATH,"%s%s%s_Log.log",drive,dir,fname);

   //create a empty file if not exist...
   EnterCriticalSection(&m_crLock);
   FILE *pl = NULL;
   fopen_s(&pl,m_strLogFile,"a+t");
   if(pl)
   {
      //Write startup information...
      SYSTEMTIME sysTime; 
      GetLocalTime(&sysTime);
      fprintf(pl,"\r\n-+-----------------------------------------------+-\r\n");
      fprintf(pl,"Starting %s   (%04d/%02d/%02d  %02dh%02d,%02d)\r\n",fname,
                                                                      sysTime.wYear, sysTime.wMonth,sysTime.wDay,
                                                                      sysTime.wHour, sysTime.wMinute,sysTime.wSecond);
      fprintf(pl,"-+-----------------------------------------------+-\r\n\r\n");
   		
      fclose(pl);
   }

   m_iLogLevel       = LOG_L1;
   m_bDisplayConsole = false;
   m_bDisplayDebug   = false;
   m_bSendShare      = false;

   
   m_pSharedLog = new CNMSharedMem(sizeof(sLogShared) ,"T4CServer_Shavev001");
   m_pSharedLog->Lock();
   sLogShared *pSharedData = (sLogShared*)m_pSharedLog->GetData();
   memset(pSharedData,0x00,sizeof(sLogShared));
   pSharedData->iNbrLogMax = LOG_SHARED_MAX;
   m_pSharedLog->UnLock();


   LeaveCriticalSection(&m_crLock);
}

//!--DESTRUCTOR-------------------------------------------------------------
//
/*!
Notes
*/			
//--------------------------------------------------------------------------
bsccLogger::~bsccLogger(void)
{
   if(m_pSharedLog)
      delete m_pSharedLog;
   m_pSharedLog = NULL;

   DeleteCriticalSection(&m_crLock);
}

//--METHOD IMPLEMENTATION--------------------------------------------------
//
// Method Name: GetInstance()
//
/*!
Notes : in first use, we create unique instance of this logger...
        all other call will use this unique instance
*/			
//--------------------------------------------------------------------------
bsccLogger* bsccLogger::GetInstance( void )
{
   static bsccLogger gLogger;

   return &gLogger;
}

//--METHOD IMPLEMENTATION--------------------------------------------------
//
// Method Name: Add()
//
/*!
Notes : Add log entry
*/			
//--------------------------------------------------------------------------
void bsccLogger::Add(int iLevel,LPCTSTR pstrModuleName,LPCTSTR pstrMessage)
{
   if(iLevel > m_iLogLevel)
      return; //not log this level


   EnterCriticalSection(&m_crLock);

   char strTimeStamp[100];
   char strMsgComplete[2048];

   //Add log to file... 
   FILE *pl = NULL;
   fopen_s(&pl,m_strLogFile,"a+t");
   if(pl)
   {
      //Write startup information...
      SYSTEMTIME sysTime; 
      GetLocalTime(&sysTime);

      //replace all space with _ char... on the name...
      char strModuleNameT[100];
      sprintf_s(strModuleNameT,100,"%s",pstrModuleName);

      //remove space from module name...
      for(unsigned int  i=0;i<strlen(strModuleNameT);i++)
      {
         if(strModuleNameT[i] == ' ')
            strModuleNameT[i] = '_';
      }

      //if name grater than 30 char we add ... at the end of string...
      if(strlen(strModuleNameT) >26)
      {
         strModuleNameT[36] = '.';
         strModuleNameT[37] = '.';
         strModuleNameT[38] = '.';
         strModuleNameT[39] = 0x00;
      }


      sprintf_s(strTimeStamp,100,"[%04d/%02d/%02d:%02d:%02d:%02d]",sysTime.wYear, sysTime.wMonth,sysTime.wDay,
                                                                   sysTime.wHour, sysTime.wMinute,sysTime.wSecond);

      sprintf_s(strMsgComplete,2048,"%s [%d] %-30s --> %s\r\n",strTimeStamp,iLevel,strModuleNameT,pstrMessage);
      fprintf(pl,strMsgComplete);
      fclose(pl);

      if(m_bDisplayConsole)
         printf("[%d] %s\r\n",iLevel,pstrMessage);
      if(m_bDisplayDebug)
         OutputDebugString(strMsgComplete);

      

      if(m_bSendShare)
      {
         m_pSharedLog->Lock();
         sLogShared *pSharedData = (sLogShared*)m_pSharedLog->GetData();
         int iWritePos = 0;
         if(pSharedData->iNbrLogWrited >=LOG_SHARED_MAX)
         {
            iWritePos = pSharedData->iStartRead;
            pSharedData->iStartRead++;
            if(pSharedData->iStartRead >= LOG_SHARED_MAX)
               pSharedData->iStartRead = 0;
         }
         else
         {
            iWritePos = pSharedData->iNbrLogWrited;
            pSharedData->iNbrLogWrited++;
         }
         pSharedData->sLogData[iWritePos].iLevel = iLevel;
         sprintf_s(pSharedData->sLogData[iWritePos].strTime,100 ,"%s",strTimeStamp);
         sprintf_s(pSharedData->sLogData[iWritePos].strName,100 ,"%s",strModuleNameT);
         sprintf_s(pSharedData->sLogData[iWritePos].strMsg ,2048,"%s",pstrMessage);
         m_pSharedLog->UnLock();
      }
   }

   

   LeaveCriticalSection(&m_crLock);
}

//--METHOD IMPLEMENTATION--------------------------------------------------
//
// Method Name: AddFmt()
//
/*!
Notes : Add log entry and can use format message like sprintf
*/			
//--------------------------------------------------------------------------
void bsccLogger::AddFmt(int iLevel,LPCTSTR pstrModuleName,LPCTSTR pszFmt, ...)
{
   int iErr = 0;
   TCHAR pszBuffer[2048];
   va_list argptr;
   va_start( argptr, pszFmt );
   iErr = _vsnprintf_s( pszBuffer,2048, sizeof( pszBuffer )/sizeof( TCHAR ),pszFmt, argptr );
   va_end(argptr);
   if ( iErr != -1 ) 
   {
      Add(iLevel,pstrModuleName,pszBuffer);
   }
}