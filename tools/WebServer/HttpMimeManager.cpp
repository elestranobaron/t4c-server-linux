/*
Module : HTTPMIMEMANAGER.CPP
Purpose: Implementation for the CHttpMimeManager class
Created: PJN / 22-04-1999
History: None                    

Copyright (c) 1999 by PJ Naughter.  
All rights reserved.

*/

//////////////// Includes ////////////////////////////////////////////

#include "stdafx.h"
#include "HttpMimeManager.h"



//////////////// Macros //////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//////////////// Implementation //////////////////////////////////////

CString CHttpMimeManager::GetMimeType(const CString& sExtension)
{
  //Prevent the string arrays from being manipulated
  //by multiple threads at the one time
  CSingleLock sl(&m_CS, TRUE);

  //Validate our parameters
  ASSERT(sExtension.GetLength());
  ASSERT(sExtension.GetAt(0) == _T('.'));

  //Look through our cache array before resorting to a registry lookup
  CString sType;
  BOOL bMatch = FALSE;
  for (int i=0; i<m_sExtensions.GetSize() && !bMatch; i++)
  {
    bMatch = sExtension.CompareNoCase(m_sExtensions.ElementAt(i)) == 0;
    if (bMatch)
      sType = m_sMimeTypes.GetAt(i);
  }

  //Not found in our cache, lookup in the registry
  if (!bMatch)
  {
    //Open the specified key
    HKEY hItem;
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, sExtension, 0, KEY_READ, &hItem) == ERROR_SUCCESS)
    { 
      //Query the key for the content type
      char         sPath[_MAX_PATH];
      DWORD dwSize = _MAX_PATH;
      DWORD dwType = REG_SZ;
      if (RegQueryValueEx(hItem, _T("Content Type"), NULL, &dwType, (LPBYTE) sPath, &dwSize) == ERROR_SUCCESS)
      {
        sType = sPath;
        bMatch = TRUE;

        //Add to the cache so that we do not have to do the registry lookup again
        m_sExtensions.Add(sExtension);
        m_sMimeTypes.Add(sType);
      }

      //Don't forget to close our key
      RegCloseKey(hItem);
    }
  }

  //If we could not find a match for it then just return it as "text/plain"
  if (!bMatch)
    sType = _T("text/plain");

  return sType;
}

void CHttpMimeManager::Empty()
{
  //Prevent the two arrays from being manipulated
  //by multiple threads at the one time
  CSingleLock sl(&m_CS, TRUE);

  //Set both array sizes back to 0
  m_sExtensions.SetSize(0);
  m_sMimeTypes.SetSize(0);
}

