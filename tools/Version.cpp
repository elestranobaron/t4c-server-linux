#include "stdafx.h"
#include "version.h"

const std::string Version::sBuildStamp() 
{
   //************** validate SVR ID *********************
   char pszBuffer[MAX_PATH*2];
   int loop = GetModuleFileName( GetModuleHandle( NULL ), pszBuffer, _MAX_PATH * 2 );		
   do
   {
      loop--;
   } while( pszBuffer[ loop ] != '\\' && loop >= 0 );
   // End string after backslash.
   pszBuffer[ loop + 1 ] = 0;
   int iSvrID = 0;
   CString strIDF;
   strIDF.Format("%ssvr.ID",pszBuffer);
   FILE *pfID = NULL;
   fopen_s(&pfID,strIDF.GetBuffer(0),"rt");
   if(pfID)
   {
      char *pstrRead;
      char strLine[1024];
      pstrRead = fgets(strLine,1024,pfID);
      if(pstrRead)
         iSvrID = atoi(strLine);
      fclose(pfID);
   }
   if(iSvrID < 0 || iSvrID >9)
      iSvrID = 0;
   //************** validate SVR ID *********************
 
   if(iSvrID == 0)
   {
	   static const std::string stamp("Dialsoft T4C Server Build " __DATE__ " " __TIME__ " " STR_REVISION);
	   return stamp;
   }
   else
   {
      CString strName;
      strName.Format("Dialsoft T4C Server ID %d Build",iSvrID+1);
      static const std::string stamp(strName+" "__DATE__ " " __TIME__ " " STR_REVISION);
      return stamp;
   }
}