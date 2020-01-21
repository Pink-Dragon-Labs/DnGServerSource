//
// mlsmarty.cpp
//
// Control logic for a basic monster.
//
// Author: Stephen Nichols
//

#include "mlaction.hpp"
#include "roommgr.hpp"

SmartMonster::SmartMonster()
{
}

SmartMonster::~SmartMonster()
{
}

void SmartMonster::init( void )
{
	new MWActionChill ( 0, this );
	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionGroup ( 0, this );
	new MWActionTake ( 0, this );
}

int SmartMonster::normalLogic ( void )
{
	int retVal = 10;

	if ( character->summoned || !room || character->health <= 0 )
		return -1;

	if ( groupLeader && groupLeader != this )
		return 10;

	return ( actionList.chooseAction() );
}
