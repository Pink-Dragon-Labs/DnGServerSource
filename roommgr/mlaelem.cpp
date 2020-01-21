//
// mlaelem.cpp
//
// Control logic for air elementals.
//
// Author: K. Sergent
//

#include "roommgr.hpp"
#include "globals.hpp"

AirElemental::AirElemental()
{
}

AirElemental::~AirElemental()
{
}

void AirElemental::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
//	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionTake ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "creates a sudden breeze." );
	emote->add ( "whirls with the force of the winds." );
	emote->add ( "moves around a howling vortex." );
}

int AirElemental::spellAttack ( void )
{
	// code for Air Elemental

	/* cast a random air related spell (grandmaster) */
	/* has elementalism, sorcery, mysticism, necromancy */

	int distance = getDistance ( combatTarget->combatX, combatTarget->combatY, character->combatX, character->combatY);

	if ( (distance < gTouchDistance) && isEnemy( combatTarget ) && random ( 0, 1 ) ) {

		switch ( random ( 0, 1 ) ) {

			case 0:
			return 0;

			case 1:
			// steal life
	   			setAction ( new CombatCastSpell ( character, _SPELL_STEAL_LIFE, combatTarget->servID, 0, 0 ) );
	   		break;
		}

	} else {

		switch ( random ( 0, 7 ) ) 
		{
			// just attack normal
			case 0:
			case 1:
			return 0;
			case 2:
			// defenselessness
	   			setAction ( new CombatCastSpell ( character, _SPELL_DEFENSELESSNESS, combatTarget->servID, 0, 0 ) );
	   		break;
			case 3:
			// sand storm
	   			setAction ( new CombatCastSpell ( character, _SPELL_SAND_STORM, combatTarget->servID, 0, 0 ) );
	   		break;
			case 4:
			// gust of wind
	   			setAction ( new CombatCastSpell ( character, _SPELL_GUST_OF_WIND, combatTarget->servID, 0, 0 ) );
	   		break;
			case 5:
			// lightning bolt
	   			setAction ( new CombatCastSpell ( character, _SPELL_LIGHTNING_BOLT, combatTarget->servID, 0, 0 ) );
	   		break;
			case 6:
			// electric fury
	   			setAction ( new CombatCastSpell ( character, _SPELL_ELECTRIC_FURY, character->servID, 0, 0 ) );
	   		break;
			case 7:
			// berserk
	   			setAction ( new CombatCastSpell ( character, _SPELL_MASS_BERSERK, character->servID, 0, 0 ) );
	   		break;
/*			case 5:
			// hurricane
	   			setAction ( new CombatCastSpell ( character, _SPELL_GUST_OF_WIND, combatTarget->servID, 0, 0 ) );
	   		break;
			case 6:
			// freezing wind
	   			setAction ( new CombatCastSpell ( character, _SPELL_GUST_OF_WIND, combatTarget->servID, 0, 0 ) );
	   		break;
*/
	   	}
	}
   	// spell cast, return.
   	return 1;
}


