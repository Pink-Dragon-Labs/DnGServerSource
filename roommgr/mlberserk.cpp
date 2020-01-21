//
// mlberserk.cpp
//
// Control logic for Berserkers.
//
// Author: Janus Anderson
// modified from mlhunter by Scott Wochholz
//

#include "roommgr.hpp"

Berserker::Berserker()
{
}

Berserker::~Berserker()
{
}

void Berserker::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionTake ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	// Berserker descript says they are berserked due to wearing too-tight shoes...
	emote->add ( "fidgets and grimaces." );
	emote->add ( "lifts a foot and shakes it." );
	emote->add ( "shifts his eyes wildly as he tugs at his boots." );
	emote->add ( "bares his teeth and snarls." );
	emote->add ( "looks for a cobbler." );
}

int Berserker::isFriend ( WorldObject *thisCharacter )
{
	return FALSE;
}

