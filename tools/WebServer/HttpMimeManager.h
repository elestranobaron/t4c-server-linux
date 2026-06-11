/*
Module : HTTPMIMEMANAGER.H
Purpose: Defines the interface for the CHttpMimeManager class
Created: PJN / 22-04-1999
History: None

Copyright (c) 1999 by PJ Naughter.  
All rights reserved.

*/


/////////////////////////////// Defines ///////////////////////////////////////

#ifndef __HTTPMIMEMANAGER_H__
#define __HTTPMIMEMANAGER_H__



/////////////////////////////// Classes ///////////////////////////////////////

//Class which encapsultates retreival of MIME types
//given a filename extension
class CHttpMimeManager
{
public:
//Methods
  CString GetMimeType(const CString& sExtension);
  void    Empty();

protected:
  CStringArray     m_sExtensions;
  CStringArray     m_sMimeTypes;
  CCriticalSection m_CS;
};

#endif //__HTTPMIMEMANAGER_H__

