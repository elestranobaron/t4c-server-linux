////////////////////////////////////////////////////////////////////////////////
//
//   Notes:      Sharing Memory between Process.
//	
//               Init a Dimension.
//               Lock
//               GetData (Get a pointer to data)
//               Read or Write
//               UnLock
//               
//               If SharedMemory from one Process to other
//               One is responsable to CreateSharedMemory()
//               The other to OpenSharedMemory()
//               The common between those two is the unique ShareName
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Windows.h>
#include <string>


static char cstrSMName[] = "SharedMemory";


////////////////////////////////////////////////////////////////////////////////
//
// class CNMSharedMem
//
////////////////////////////////////////////////////////////////////////////////
class CNMSharedMem
{
public:
   CNMSharedMem()
   {
      m_szSharedName = cstrSMName;
      InitData();
   }
   CNMSharedMem(const char* _szSharedName)
   {
      m_szSharedName = _szSharedName;
      InitData();
   }
   CNMSharedMem(int _iSize)
   {
      m_szSharedName = cstrSMName;
      InitData();
      CreateSharedMemory(_iSize);
   }
   CNMSharedMem(int _iSize, const char* _szSharedName)
   {
      InitData();
      CreateSharedMemory(_iSize, _szSharedName);
   }

   ~CNMSharedMem()
   {
      if( m_bInit )
		{
         ::UnmapViewOfFile(m_pwData);
         ::CloseHandle(m_hSharedMemoryFile);
		}
   }

public:
   bool  OpenSharedMemory(const char* _szSharedName)
   {
      m_szSharedName = _szSharedName;
      m_hSharedMemoryFile = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, 
		                        FALSE, _szSharedName);
      
      if( m_hSharedMemoryFile )
      {
         m_pwData = ::MapViewOfFile(m_hSharedMemoryFile, 
		                  FILE_MAP_ALL_ACCESS, //FILE_MAP_WRITE | FILE_MAP_READ, 
		                  0,
		                  0,
		                  0);
         if( m_pwData )
         {
            m_hMutex = ::CreateMutex(NULL, FALSE, m_szMutexName.c_str());
            m_bInit=true;
            return true;
         }
      }

      return false;
   }

   bool  CreateSharedMemory(int _iSize, const char* _szSharedName)
   {
      if( m_bInit )
         return false;
      m_szSharedName = _szSharedName;
      InitData();
      return CreateSharedMemory(_iSize);
   }

   bool  CreateSharedMemory(int _iSize)
   {
      if( m_bInit )
         return false;

      m_hMutex = ::CreateMutex(NULL, FALSE, m_szMutexName.c_str());
      m_ulNumberOfBytesToMap = _iSize;

      m_hSharedMemoryFile = ::CreateFileMapping(INVALID_HANDLE_VALUE/*(HANDLE)0xFFFFFFFF*/,
										NULL,
										PAGE_READWRITE,
										0                       /*dwMaximumSizeHigh*/,
										m_ulNumberOfBytesToMap  /*dwMaximumSizeLow*/,
										m_szSharedName.c_str());
      if(m_hSharedMemoryFile == NULL)
		{
			m_bAlreadyExist = false;
			m_bInit = false;
			return false;
		}
		else
		{
         if( ::GetLastError() == ERROR_ALREADY_EXISTS )
          m_bAlreadyExist = true;
		}

      m_pwData = ::MapViewOfFile(m_hSharedMemoryFile,
								FILE_MAP_WRITE,
								0  /*dwFileOffsetHigh*/,
								0  /*dwFileOffsetLow*/,
								m_ulNumberOfBytesToMap);
		if(m_pwData == NULL)
		{
			m_bInit = false;
         ::CloseHandle(m_hSharedMemoryFile);
         return false;
		}
		else
			m_bInit = true;
      return true;
   }

   // Helpers
   void* GetData()
   {
      if( m_bInit )
         return m_pwData;
      else
         return NULL;
   }
   bool  AlreadyExist() { return m_bAlreadyExist; }
   bool  Lock(DWORD _dwMilliSec = INFINITE)
   {
      if( ::WaitForSingleObject(m_hMutex, _dwMilliSec) == WAIT_OBJECT_0)
         return true;
      return false;
   }
   bool  UnLock()
   {
      if( ::ReleaseMutex(m_hMutex) )
         return true;
      else
         return false;
   }
private:
   HANDLE	   m_hSharedMemoryFile;
   HANDLE      m_hMutex;
   bool	      m_bInit;
	bool        m_bAlreadyExist;
   std::string m_szSharedName;
   std::string m_szMutexName;
   void*	      m_pwData;
   unsigned long  m_ulNumberOfBytesToMap;

   void InitData()
   {
      m_bInit = m_bAlreadyExist = false;
      m_szMutexName = m_szSharedName;
      m_szMutexName += "Mutex";
      m_pwData = NULL;
   }
   

};

