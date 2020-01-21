//
// mlcthief.cpp
//
// Control logic for castle thieves.
//
// Author: Zachary Kaiser
//

#include "roommgr.hpp"

RogueThief::RogueThief()
{
}

RogueThief::~RogueThief()
{
}

void RogueThief::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
	new MWActionWander ( 20, this );
	new MWActionSteal ( 0, this );
	new MWActionTake ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "examines you carefully." );
	emote->add ( "eats a stolen apple." );
	emote->add ( "judges your wealth." );
	emote->add ( "looks about nervously." );
	emote->add ( "looks at you askance." );
	emote->add ( "looks for a place to hide." );
	emote->add ( "shifts about." );
	emote->add ( "tries to gauge the size of your purse." );
}

int RogueThief::spellAttack ( void )
{
	// check our health and randomly flee from combat.
	if ( character->health < ( character->healthMax / 3 ) &&
		 !random ( 0, 3 ) )
	{
		setAction ( new CombatFlee ( character ) );
		return 1;
	}

	// has no spell ability - melee only.
	return 0;
}
