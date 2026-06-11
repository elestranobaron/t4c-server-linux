/******************************************************************************
Modify for vs2008 (06/05/2009)
/******************************************************************************/
#ifndef VERSION_H
#define VERSION_H
#include "CustomBuild.h"

#ifdef BUILD_NMS_CUSTOM_NPC
   #define STR_REVISION							"1.72 rev 1.07.0"
#else
   #define STR_REVISION							"1.77 rev 1.00.0"
#endif

class Version
{
public:
	static const std::string sBuildStamp();
};

#endif /* VERSION_H */
