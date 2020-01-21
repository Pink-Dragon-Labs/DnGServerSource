//
// mleelem.cpp
//
// Control logic for earth elementals.
//
// Author: K. Sergent
//

#include "roommgr.hpp"
#include "globals.hpp"

EarthElemental::EarthElemental()
{
}

EarthElemental::~EarthElemental()
{
}

void EarthElemental::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
//	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionTake ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "shuffles about and raises a cloud of dust." );
	emote->add ( "absorbs some earthen material into its form." );
	emote->add ( "examines itself for moss and lichens." );
	emote->add ( "stares at you stonily." );
}

int EarthElemental::spellAttack ( void )
{
	// code for earth elemental

	/* cast a random earth related spell (grandmaster) */
	/* also has necromancy (grandmaster) */

	// check to make sure in range

	int distance = getDistance ( combatTarget->combatX, combatTarget->combatY, character->combatX, character->combatY);

	if ( distance < gTouchDistance && isEnemy( combatTarget ) ) {

		switch ( random ( 0, 1 ) ) {

			case 0:
			return 0;

			case 1:
			// steal life
	   			setAction ( new CombatCastSpell ( character, _SPELL_STEAL_LIFE, combatTarget->servID, 0, 0 ) );
   			break;
		}

	} else {

		switch ( random ( 0, 4 ) ) 
		{
			// just attack normal
			case 0:
			case 1:
			return 0;
	   		// earthquake
	   		case 2:
	   			setAction ( new CombatCastSpell ( character, _SPELL_EARTHQUAKE, combatTarget->servID, 0, 0 ) );
	   		break;
	   		case 3:
	   		// crushing boulder
	   			setAction ( new CombatCastSpell ( character, _SPELL_CRUSHING_BOULDER, combatTarget->servID, 0, 0 ) );
	   		break;
			case 4:
	   		// stoning
	   			setAction ( new CombatCastSpell ( character, _SPELL_STONING, combatTarget->servID, 0, 0 ) );
			break;
		}
   	}
   	// spell cast, return.
   	return 1;
}


