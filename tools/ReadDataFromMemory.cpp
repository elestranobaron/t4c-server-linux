#include "StdAfx.h"
#include "ReadDataFromMemory.h"

CReadDataFromMemory::CReadDataFromMemory(void)
{
   m_pMemory   = NULL;
   m_uiLength  = 0;
   m_uiCurRead = 0;
}

CReadDataFromMemory::~CReadDataFromMemory(void)
{
}

void CReadDataFromMemory::SetMemoryPtr(BYTE *pMemory, UINT uiLength)
{
   m_pMemory   = pMemory;
   m_uiLength  = uiLength;
   m_uiCurRead = 0;
}

unsigned int CReadDataFromMemory::get_dword()
{
   unsigned int val;

   val=*(unsigned int *)m_pMemory;	m_pMemory+=4;
   return val;
}

unsigned short CReadDataFromMemory::get_word()
{
   unsigned short val;

   val=*(unsigned short *)m_pMemory;	m_pMemory+=2;
   return val;
}

BYTE CReadDataFromMemory::get_byte()
{
   unsigned char val;

   val=*m_pMemory;	m_pMemory+=1;
   return val;
}

double CReadDataFromMemory::get_double()
{
   double val;

   val=*(double *)m_pMemory;	m_pMemory+=8;
   return val;
}

int CReadDataFromMemory::get_long()
{
   int val;

   val=*(int *)m_pMemory;	m_pMemory+=4;
   return val;
}

short CReadDataFromMemory::get_short()
{
   short val;

   val=*(short *)m_pMemory;	m_pMemory+=2;
   return val;
}

char* CReadDataFromMemory::get_string()
{
   int lg,i;

   lg=*(int *)m_pMemory;	m_pMemory+=4;
   for(i=0; i<lg; i++)
      m_chString[i]=*m_pMemory++;

   m_chString[i]=0;

   return (char *)m_chString;
}

