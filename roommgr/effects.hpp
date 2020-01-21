/*
	EFFECTS.HPP
	This module contains all of the WorldEffect classes that are throughout the
	server code.

	Author: Stephen Nichols
*/

#ifndef _EFFECT_HPP_
#define _EFFECT_HPP_

#include "weffect.hpp"

//
// WEInvisibility: This effect, when not dormant, makes the client invisible
// to all players that can not see invisibility.
//
class WEInvisibility : public WorldEffect
{
public:
	WEInvisibility();
	WEInvisibility ( WEDescriptor *info ) : WorldEffect ( info ) {};
	virtual ~WEInvisibility();
};

//
// WESeeInvisibility: This effect, when not dormant, makes the client see invisible
//
class WESeeInvisibility : public WorldEffect
{
public:
	WESeeInvisibility();
	WESeeInvisibility ( WEDescriptor *info ) : WorldEffect ( info ) {};
	virtual ~WESeeInvisibility();
};

#endif
