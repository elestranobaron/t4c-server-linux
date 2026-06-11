/******************************************************************************
Modify for vs2008 (30/04/2009)
/******************************************************************************/
#include "stdafx.h"
#include "EncRandom.h"

/******************************************************************************/
EncRandom::~EncRandom()
/******************************************************************************/
{
}
/******************************************************************************/
EncRandom::EncRandom(
unsigned int NewMinValue,
unsigned int NewMaxValue,
unsigned int InitSeed,
unsigned int seedNumber) 
/******************************************************************************/
{
	MinValue = NewMinValue;
	MaxValue = NewMaxValue + 1;
	Seed = InitSeed;
	SeedID = seedNumber;

	CreateTable();
}
/******************************************************************************/
// Change the Seed Value.
void EncRandom::SetSeed(unsigned int NewSeed) 
/******************************************************************************/
{
	Seed = NewSeed;

	CreateTable();
}
/******************************************************************************/
// Change the Minimum Value;
void EncRandom::SetMinValue(unsigned int NewMinValue) 
/******************************************************************************/
{
	MinValue = NewMinValue;

	CreateTable();
}
/******************************************************************************/
// Change the Maximum Value;
void EncRandom::SetMaxValue(unsigned int NewMaxValue) 
/******************************************************************************/
{
	MaxValue = NewMaxValue + 1;

	CreateTable();
}
/******************************************************************************/
// Change the Minimum and Maximum Value;
void EncRandom::SetMinMaxValue(unsigned int NewMinValue, unsigned int NewMaxValue)
/******************************************************************************/
{
	MinValue = NewMinValue;
	MaxValue = NewMaxValue + 1;

	CreateTable();
}
/******************************************************************************/
// Return the EncRandom Number.
EncRandom::operator int (void) 
/******************************************************************************/
{
	return EncRandomize();
}
/******************************************************************************/
// Generate a EncRandom Number
inline unsigned int EncRandom::EncRandomize(void) 
/******************************************************************************/
{
	switch (SeedID) 
	{
		case 0:
			Seed = Seed * 8203597 + 5 * 3;
			break;
		case 1:
			Seed = Seed * 7563921 + 1;
			break;
	}

	if (MaxValue==MinValue) 
	{
		return MinValue;
	}
   
   return (Seed % (MaxValue-MinValue)) + MinValue;
}
/******************************************************************************/
// Return the EncRandom Number using a New Seed.
unsigned int EncRandom::operator () (unsigned int NewSeed) 
/******************************************************************************/
{
	Seed = NewSeed;

	return EncRandomize();
}
/******************************************************************************/
// Return the EncRandom Number using a New Minimun and Maximum Value.
unsigned int EncRandom::operator () (unsigned int NewMinValue, unsigned int NewMaxValue) 
/******************************************************************************/
{
	MinValue = NewMinValue;
	MaxValue = NewMaxValue + 1;

	CreateTable();
	return EncRandomize();
}
/******************************************************************************/
// Return the EncRandom Number using a New Seed, New Minimun and Maximum Value.
unsigned int EncRandom::operator () (
unsigned int NewSeed,
unsigned int NewMinValue,
unsigned int NewMaxValue)
/******************************************************************************/
{
	Seed = NewSeed;
	MinValue = NewMinValue;
	MaxValue = NewMaxValue + 1;

	CreateTable();
	return EncRandomize();
}