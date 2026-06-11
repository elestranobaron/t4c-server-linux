#ifndef KEYGESTIONNAR_H
#define KEYGESTIONNAR_H

/*
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
*/

#include "int64stuff.h"
#include <cstdint>

class Key
{
public:
	std::uint64_t key;		//cle: e ou d
	std::uint64_t keymod;	//cle n	
	
	//constructeur
	Key();
	Key(std::uint64_t, std::uint64_t);
	Key& operator=(const Key &oldkey);


	//fonctions
	//static Key loadkey( const CString&);	
	//static bool savekey(  const CString&, const Key&);
	int getprecision();
	void init();
};

#endif