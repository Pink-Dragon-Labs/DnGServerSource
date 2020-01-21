/*
	EFFECTS.CPP
	This module contains all of the WorldEffect classes that are throughout the
	server code.

	Author: Stephen Nichols
*/

#include "effects.hpp"

//
// WEInvisibility: This effect, when not dormant, makes the client invisible
// to all players that can not see invisibility.
//
WEInvisibility::WEInvisibility()
{
	type = _EFF_INVISIBLE;
}

WEInvisibility::~WEInvisibility()
{
}

//
// WESeeInvisibility: This effect, when not dormant, makes the client see invisible
//
WESeeInvisibility::WESeeInvisibility()
{
	type = _EFF_SEE_INVISIBLE;
}

WESeeInvisibility::~WESeeInvisibility()
{
}
