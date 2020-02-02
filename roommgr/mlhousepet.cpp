//
// mlhousepet.cpp
//
// Control logic for house pets.
//
// Author: Zach Kaiser
//

#include "roommgr.hpp"

HousePet::HousePet()
{
}

HousePet::~HousePet()
{
}

void HousePet::init ( void )
{
	// set up action list
	// we only wnt house pets to wander the room they 'live' in
	new MWActionChill ( 0, this );
	new MWActionWander ( 0, this );
}


