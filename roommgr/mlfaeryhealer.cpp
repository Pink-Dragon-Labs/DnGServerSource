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

void FaeryHealer::init ( void )
{
	// set up action list
	new MWActionHeal ( 0, this );
    new MWActionCure ( 0, this );
	new MWActionBless ( 0, this, _SPELL_EMPOWER, _AFF_EMPOWER, _AFF_TYPE_NORMAL );
	new MWActionBless ( 0, this, _SPELL_NIMBILITY, _AFF_POS_DEX_MOD, _AFF_TYPE_NORMAL );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "blows you a soft kiss." );
	emote->add ( "dances a happy jig in the air." );
	emote->add ( "daydreams of gentle breezes." );
	emote->add ( "giggles happily." );
	emote->add ( "smiles beatifically." );
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
