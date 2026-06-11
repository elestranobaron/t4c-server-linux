#pragma once

class CReadDataFromMemory
{
public:
   CReadDataFromMemory(void);
   ~CReadDataFromMemory(void);

   void SetMemoryPtr(BYTE *pMemory, UINT uiLength);

   unsigned int   get_dword();
   unsigned short get_word();
   BYTE           get_byte();
   double         get_double();
   int            get_long();
   short          get_short();
   char*          get_string();


private:
   BYTE *m_pMemory;
   UINT  m_uiLength;
   UINT  m_uiCurRead;
   char  m_chString[4096];
};
