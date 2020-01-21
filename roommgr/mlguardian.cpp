//
// mlguardian.cpp
//
// Control logic for guardian NPCs.
//
// Author: Stephen Nichols
//
#include <algorithm>
#include "roommgr.hpp"

Guardian::Guardian() {}

Guardian::~Guardian() {}

void Guardian::init ( void ) {
	new MWActionChill ( 0, this );
	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionHeal ( 0, this );
	new MWActionCure ( 0, this );

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "squints in your direction." );
	emote->add ( "places hand on hilt." );
	emote->add ( "smiles and nods at you." );
	emote->add ( "examines the landscape." );
	emote->add ( "scribbles a note." );
}

int Guardian::isEnemy ( WorldObject *thisCharacter ) {
	if ( thisCharacter->player ) {
		CrimeData* crime = thisCharacter->player->getCrimeData();

		if ( crime->criminal ) {
			return TRUE;
		}
	}

	return FALSE;
}

int Guardian::spellAttack ( void ) {
	int retVal = 0;
	int rate = 0;

	int	distance = getDistance ( combatTarget->combatX, combatTarget->combatY, character->combatX, character->combatY );

	switch ( random ( 0, 5 ) ) {
		// melee attack
		case 0:
		return retVal;

		// combat teleport 
		case 1: {
			// check target distance and speed, if faster, teleport to it
		
			rate = std::max(1, character->calcNumMoves());
							//1 >? character->calcNumMoves();
		
			if ( ( distance > rate ) && ( distance > character->combatRange() ) ) {
				setAction ( new CombatCastSpell ( character, _SPELL_COMBAT_TELEPORT, character->servID, combatTarget->combatX, combatTarget->combatY ) );
				retVal = 1;
			}
		}
		break;
		
		// cast banishment 
		case 2:
			if ( getSummonedCount ( _ENEMY_SUMMONED_MONSTERS ) ) {
				setAction ( new CombatCastSpell ( character, _SPELL_BANISHMENT, character->servID, 0, 0 ) );
				retVal = 1;
			}
		break;
		
		case 3: {
			if ( combatTarget->player ) {
				CrimeData* crime = combatTarget->player->getCrimeData();

				if ( crime->criminal ) {
					setAction ( new CombatCastSpell ( character, _SPELL_ELPHAMES_JUSTICE, combatTarget->servID, 0, 0 ) );
					retVal = 1;
				}
			}
		}
		break;
		
		// wrath of the gods
		case 4:
			//MIKE-ALIGNMENT - changed to reflect alignment table
			//if ( combatTarget->alignment <= 175 )
			if ( combatTarget->alignment < 171 ) {
				setAction ( new CombatCastSpell ( character, _SPELL_WRATH_OF_THE_GODS, character->servID, 0, 0 ) );
				retVal = 1;
			}
		break;

		// heal party
		case 5:
			retVal = partyHeal();
		break;
	}
	// if retVal is 0, just attack normal - spell cast, return.

   	return retVal;
}

