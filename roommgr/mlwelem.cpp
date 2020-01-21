//
// mlwelem.cpp
//
// Control logic for water elementals.
//
// Author: K. Sergent
//

#include "roommgr.hpp"
#include "globals.hpp"

WaterElemental::WaterElemental()
{
}

WaterElemental::~WaterElemental()
{
}

void WaterElemental::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
//	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionTake ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "looks skyward in anticipation of rain." );
	emote->add ( "pauses in quiet reflection." );
	emote->add ( "checks for leaks." );
}

int WaterElemental::spellAttack ( void )
{
	// code for Water Elemental

	/* cast a random water related spell (grandmaster) */
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
	   		// rust
	   		case 2:
	   			setAction ( new CombatCastSpell ( character, _SPELL_RUST, combatTarget->servID, 0, 0 ) );
	   		break;
	   		case 3:
	   		// mass rust
	   			setAction ( new CombatCastSpell ( character, _SPELL_MASS_RUST, combatTarget->servID, 0, 0 ) );
	   		break;
			case 4:
			// defenselessness
	   			setAction ( new CombatCastSpell ( character, _SPELL_DEFENSELESSNESS, combatTarget->servID, 0, 0 ) );
	   		break;
			case 5:
			// ice storm
	   			setAction ( new CombatCastSpell ( character, _SPELL_ICE_STORM, combatTarget->servID, 0, 0 ) );
	   		break;
			case 6:
			// freeze
	   			setAction ( new CombatCastSpell ( character, _SPELL_FREEZE, combatTarget->servID, 0, 0 ) );
	   		break;
			case 7:
			// cold snap
	   			setAction ( new CombatCastSpell ( character, _SPELL_COLD_SNAP, combatTarget->servID, 0, 0 ) );
	   		break;
	   	}
	}
   	// spell cast, return.
   	return 1;
}


