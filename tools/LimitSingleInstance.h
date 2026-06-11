#ifndef LimitSingleInstance_H
#define LimitSingleInstance_H

#include <windows.h> 

//This code is from Q243953 in case you lose the article and wonder
//where this code came from.
class CLimitSingleInstance
{
protected:
   DWORD  m_dwLastError;
   HANDLE m_hMutex;

public:
   CLimitSingleInstance()
   {
      //Make sure that you use a name that is unique for this application otherwise
      //two apps may think they are the same if they are using same name for
      //3rd parm to CreateMutex

      //read server ID to get good instance...

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



      TCHAR strMutexNameInt[_MAX_PATH];
      switch(iSvrID)
      {
         case 0  : sprintf_s(strMutexNameInt,_MAX_PATH,TEXT("Global\\{2194ABA1-ACBC-4e6b-6666-D191BB16F9E6}")); break;
         case 1  : sprintf_s(strMutexNameInt,_MAX_PATH,TEXT("Global\\{2194ABA1-BFFA-4e6b-8C26-D191BB16F9E6}")); break;
         case 2  : sprintf_s(strMutexNameInt,_MAX_PATH,TEXT("Global\\{ddcfe948-52ce-4656-bf71-102ed22d5d97}")); break;
         case 3  : sprintf_s(strMutexNameInt,_MAX_PATH,TEXT("Global\\{bdc7f74f-b66e-41ea-9d99-b86868107ed3}")); break;
         case 4  : sprintf_s(strMutexNameInt,_MAX_PATH,TEXT("Global\\{7aadd610-8d99-4370-a3d6-90887932a368}")); break;
         case 5  : sprintf_s(strMutexNameInt,_MAX_PATH,TEXT("Global\\{7e1b3170-2be2-4781-afd7-9b756a3e614a}")); break;
         case 6  : sprintf_s(strMutexNameInt,_MAX_PATH,TEXT("Global\\{61401ddc-f60c-4494-bb21-b2c09efcd1c1}")); break;
         case 7  : sprintf_s(strMutexNameInt,_MAX_PATH,TEXT("Global\\{7bb87e79-65a1-495f-9189-38416206fbae}")); break;
         case 8  : sprintf_s(strMutexNameInt,_MAX_PATH,TEXT("Global\\{1e61057d-1fcc-4ce4-b446-06996d5a9361}")); break;
         case 9  : sprintf_s(strMutexNameInt,_MAX_PATH,TEXT("Global\\{416d64cf-ae49-4382-8c2b-ad8828721be3}")); break;
         default : sprintf_s(strMutexNameInt,_MAX_PATH,TEXT("Global\\{2194ABA1-ACBC-4e6b-6666-D191BB16F9E6}")); break; 
      }

      m_hMutex = CreateMutex(NULL, FALSE, strMutexNameInt); //do early
      m_dwLastError = GetLastError(); //save for use later...
   }

   ~CLimitSingleInstance() 
   {
      if (m_hMutex)  //Do not forget to close handles.
      {
         CloseHandle(m_hMutex); //Do as late as possible.
         m_hMutex = NULL; //Good habit to be in.
      }
   }

   BOOL IsAnotherInstanceRunning() 
   {
      return (ERROR_ALREADY_EXISTS == m_dwLastError);
   }
};
#endif
