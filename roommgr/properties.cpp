/*
	PROPERTIES.CPP
	Property reference definitions for server/client property setting.

	Author: Stephen Nichols
*/

#include "properties.hpp"

// this table has the property type information for all properties
int propertyTypeTable[] = {
	_PROP_WORD, 		// _PROP_BUILD_POINTS
	_PROP_WORD,			// _PROP_HEALTH
	_PROP_WORD,			// _PROP_MANA
	_PROP_INT,			// _PROP_EXPERIENCE
	_PROP_INT,			// _PROP_VALUE
};
