//
// mlfelem.cpp
//
// Control logic for fire elementals.
//
// Author: K. Sergent
//

#include "roommgr.hpp"
#include "globals.hpp"

FireElemental::FireElemental()
{
}

FireElemental::~FireElemental()
{
}

void FireElemental::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
//	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionTake ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "scorches the earth." );
	emote->add ( "crackles with flame." );
	emote->add ( "glares at you." );
}

int FireElemental::spellAttack ( void )
{
	// code for Fire Elemental

	/* cast a random fire related spell (grandmaster) */
	/* has elementalism, sorcery, mysticism, necromancy */

	int distance = getDistance ( combatTarget->combatX, combatTarget->combatY, character->combatX, character->combatY);

	if ( (distance < gTouchDistance) && isEnemy( combatTarget ) && random ( 0, 1 ) ) {

		switch ( random ( 0, 1 ) ) {

			case 0:
			return 0;

			case 1:
			// steal life
   				setAction ( new CombatCastSpell ( character, _SPELL_DEATH_TOUCH, combatTarget->servID, 0, 0 ) );
	   		break;
		}

	} else {

		switch ( random ( 0, 7 ) ) 
		{
			// just attack normal
			case 0:
			case 1:
			case 3:
				return 0;
	   		// flame orb
	   		case 2:
	   			setAction ( new CombatCastSpell ( character, _SPELL_FLAME_ORB, combatTarget->servID, 0, 0 ) );
	   		break;
			case 4:
			// defenselessness
	   			setAction ( new CombatCastSpell ( character, _SPELL_DEFENSELESSNESS, combatTarget->servID, 0, 0 ) );
	   		break;
			case 5:
			// incinerate
	   			setAction ( new CombatCastSpell ( character, _SPELL_INCINERATE, combatTarget->servID, 0, 0 ) );
	   		break;
			case 6:
			// immolation
	   			setAction ( new CombatCastSpell ( character, _SPELL_IMMOLATION, character->servID, 0, 0 ) );
	   		break;
			case 7:
			// fireball
	   			setAction ( new CombatCastSpell ( character, _SPELL_FIREBALL, combatTarget->servID, combatTarget->combatX, combatTarget->combatY ) );
	   		break;
	   	}
	}
   	// spell cast, return.
   	return 1;
}


