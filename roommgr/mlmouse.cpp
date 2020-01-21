//
// mouse.cpp
//
// Control logic for meese.
//
// Author: Janus Anderson
//

#include "roommgr.hpp"

Mouse::Mouse()
{
}

Mouse::~Mouse()
{
}

void Mouse::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "emits a chittering noise." );
	emote->add ( "scratches at the ground." );
	emote->add ( "wiggles its whiskers and sniffs at the air." );
}
