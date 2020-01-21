/*
	BTreasure class
	author: Stephen Nichols
*/

#include "roommgr.hpp"

BTreasure::BTreasure ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BTREASURE;
	memset ( table, 0, sizeof ( table ) );
	tableSize = 0;
}

BTreasure::~BTreasure()
{
	for ( int i=0; i<tableSize; i++ ) 
		free ( table[i] );
}

void BTreasure::copy ( WorldObjectBase *theBase )
{
	BTreasure *base = (BTreasure *)theBase;

	for ( int i=0; i<base->tableSize; i++ ) 
		addTreasure ( base->table[i], base->minRange[i], base->maxRange[i] );
}

void BTreasure::addTreasure ( char *name, int low, int high ) 
{
	if ( tableSize == _MAX_TREASURE )
		return;

	table[tableSize] = strdup ( name );
	minRange[tableSize] = low;
	maxRange[tableSize] = high;
	tableSize++;
}

WorldObject *BTreasure::makeObj ( void ) 
{
	int index = random ( 0, tableSize - 1 );
	char *name = table[index];

	WorldObject *super = roomMgr->findClass ( name );
	WorldObject *object = NULL;

	if ( super ) {
		// translate class name based on treasure table entry
		BTreasure *btreasure = (BTreasure*) super->getBase ( _BTREASURE );

		if ( btreasure ) {
			object = btreasure->makeObj();
		} else {
			object = new WorldObject ( super );

			if ( !strcmp ( name, "MoneyBag" ) ) {
				object->physicalState |= _STATE_MONEY;
				object->value = random ( minRange[index], maxRange[index] );
			}

			else if ( !strcmp ( name, "ManaBag" ) ) {
				object->physicalState |= _STATE_MONEY;
				object->value = random ( minRange[index], maxRange[index] );
			}
		}
	}

	return object;
}
