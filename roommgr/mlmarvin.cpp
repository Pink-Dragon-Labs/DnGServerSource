//
// mlmarvin.cpp
//
// Control logic for Marvin.
//
// Author: Stephen Nichols
//

#include "roommgr.hpp"

Marvin::Marvin()
{
}

Marvin::~Marvin()
{
}

int Marvin::normalLogic ( void )
{
	if ( character->summoned || !room || character->health <= 0 )
		return -1;

	if ( character->health < character->healthMax ) {
		castOOCSpell ( _SPELL_GREATER_HEAL, character );
	}

	else if ( character->hasAffect ( _AFF_DAMAGE_POISON, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT ) ) {
		castOOCSpell ( _SPELL_CURE_POISON, character );
	}

	return 5;
}


int Marvin::spellAttack ( void )
{
	if ( character->health < (character->healthMax / 2) ) {
		setAction ( new CombatCastSpell ( character, _SPELL_GREATER_HEAL, character->servID, 0, 0 ) );
	} 
	
	else {   
		if ( combatTarget->hasAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_RESISTANCE ) ) {
			setAction ( new CombatCastSpell ( character, _SPELL_EARTHQUAKE, character->servID, 0, 0 ) );
		} else {
			setAction ( new CombatCastSpell ( character, (random ( 0, 1 )? _SPELL_EARTHQUAKE : _SPELL_LIGHTNING_BOLT), combatTarget->servID, 0, 0 ) );
		}
	}

	return 1;
}
