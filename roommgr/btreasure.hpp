/*
	BTreasure class
	author: Stephen Nichols
*/

#ifndef _BTREASURE_HPP_
#define _BTREASURE_HPP_

#include "wobjectbase.hpp"
#include "wobject.hpp"

#define _MAX_TREASURE 	128

class BTreasure : public WorldObjectBase
{
public:
	BTreasure ( WorldObject *obj );
	virtual ~BTreasure();

	void copy ( WorldObjectBase *base );

	// add a new treasure to this table
	void addTreasure ( char *name, int low, int high );

	WorldObject *makeObj ( void );

	// table of class names
	char *table[_MAX_TREASURE];
	int minRange[_MAX_TREASURE], maxRange[_MAX_TREASURE];
	int tableSize;
};

#endif
