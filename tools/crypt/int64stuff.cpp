#include "stdafx.h"
#include "int64stuff.h"
#include <cstdint>

//calcule piussance
std::uint64_t int64stuff::MyPow(std::uint64_t val, std::uint64_t pow)
{
	std::uint64_t answer = 1;
	if (pow != 0)
	{
		for (; pow > 0; pow--)
			answer = answer*val;
	}

	return answer;
}

//calcule la taille
int int64stuff::GetLenght(std::uint64_t value)
{
	int lenght = 0;
	do
	{
		lenght++;
		value = value/10;
	} while (value > 0);

	return lenght;
}