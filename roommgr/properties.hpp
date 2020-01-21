/*
	PROPERTIES.HPP
	Property reference definitions for server/client property setting.

	Author: Stephen Nichols
*/

#ifndef _PROPERTIES_HPP_
#define _PROPERTIES_HPP_

// these definitions are used by wobjectbase.cpp to refer to properties
// on the client side.  they are used to set properties asynchronously on
// the client side.
//
enum {
	_PROP_BUILD_POINTS,
	_PROP_HEALTH,
	_PROP_MANA,
	_PROP_EXPERIENCE,
	_PROP_VALUE,
	_PROP_END = 30000
};

// these definitions tell the setProperty method what type each property is
enum {
	_PROP_STRING,
	_PROP_INT,
	_PROP_WORD,
	_PROP_BYTE
};

// this table has the property type information for all properties
extern int propertyTypeTable[];

#endif
