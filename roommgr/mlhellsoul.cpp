//
// mlhellsoul.cpp
//
// This file contains the HellSoul AI code.
//
// Author: Stephen Nichols
//

#include "roommgr.hpp"

HellSoul::HellSoul()
{
}

HellSoul::~HellSoul()
{
}

int HellSoul::combatLogic ( void )
{
	setAction ( new CombatFlee ( character ) );
	return 1;
}

int HellSoul::normalLogic ( void )
{
	return -1;
}
