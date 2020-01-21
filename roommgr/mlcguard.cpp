//
// mlcguard.cpp
//
// Control logic for castle guards.
//
// Author: Zachary Kaiser
//

#include "roommgr.hpp"

CastleGuard::CastleGuard()
{
}

CastleGuard::~CastleGuard()
{
}

void CastleGuard::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
	new MWActionWander ( 20, this );
	new MWActionTake ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "examines you carefully." );
	emote->add ( "looks about nervously." );
	emote->add ( "shifts about." );
    emote->add ( "squints in your direction." );
	emote->add ( "places hand on hilt." );
	emote->add ( "examines the landscape." );
}

int CastleGuard::spellAttack ( void )
{
	// melee only
	return 0;
}
