//
// mlfaeryhealer.cpp
//
// Control logic for FaeryHealer.
//
// Author: Zachary Kaiser
//

#include "roommgr.hpp"

FaeryHealer::FaeryHealer()
{
}

FaeryHealer::~FaeryHealer()
{
}

int FaeryHealer::normalLogic ( void )
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


int FaeryHealer::spellAttack ( void )
{
	if ( character->health < (character->healthMax / 2) ) {
		setAction ( new CombatCastSpell ( character, _SPELL_GREATER_HEAL, character->servID, 0, 0 ) );
	} 

    else if ( character->hasAffect ( _AFF_DAMAGE_POISON, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT ) ) {
		setAction ( new CombatCastSpell ( character, _SPELL_CURE_POISON, character->servID, 0, 0 ) );
	} 

	return 1;
}
