//
// mltraveller.cpp
//
// Control logic for Travellers.
//
// Author: Zachary Kaiser
//

#include "roommgr.hpp"

Traveller::Traveller()
{
}

Traveller::~Traveller()
{
}

void Traveller::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRoomsBoss ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "says 'Have you seen any odd jewels?'" );
	emote->add ( "gazes into the distance, then quickly checks his pockets." );
	emote->add ( "juggles his coin purse." );
    emote->add ( "swats a fly." );
    emote->add ( "mumbles under his breath, then kicks the air." );
}

int Traveller::normalLogic ( void )
{
	int retVal = 10;

	if ( character->summoned || !room || character->health <= 0 )
		return -1;

	if ( groupLeader && groupLeader != this )
		return 10;

	return ( actionList.chooseAction() );
}
