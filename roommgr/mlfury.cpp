//
// mlfury.cpp
//
// Control logic for furies.
//
// Author: Janus Anderson
//

#include "roommgr.hpp"

Fury::Fury()
{
	// type 0 = electric fury
	// type 1 = fire fury
	// type 2 = acid fury
	// type 3 = poison fury
	myType = random ( 0, 3 );
}

Fury::~Fury()
{
}

void Fury::init ( void )
{
	new MWActionChill ( 0, this );
	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "casts fiery stare at you." );
	emote->add ( "brushes her hair." );
	emote->add ( "emits a high-pitched screech." );
	emote->add ( "bellows a spooky wail." );
}

int Fury::spellAttack ( void )
{
	// if not shifting, shift...
	if ( !character->hasAffect ( _AFF_SHIFT ) ) {
		setAction ( new CombatCastSpell ( character, _SPELL_SHIFT, character->servID, 0, 0 ) );
		return 1;
	}

	switch ( myType ) {
		case 0: {
			// electric fury
			// cast electric curse, lightning bolt, or electric fury
			if ( random ( 0, 1 ) || combatTarget->hasAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_WEAKNESS ) ) {
				if ( random ( 0, 2 ) ) {
					// cast lightning bolt
					setAction ( new CombatCastSpell ( character, _SPELL_LIGHTNING_BOLT, combatTarget->servID, 0, 0 ) );
				} else {
					// cast electric fury
					setAction ( new CombatCastSpell ( character, _SPELL_ELECTRIC_FURY, character->servID, 0, 0 ) );
				}
			} else {
				// curse 'em
				setAction ( new CombatCastSpell ( character, _SPELL_ELECTRIC_CURSE, combatTarget->servID, 0, 0 ) );
			}

			return 1;
		}

		break;

		case 1: {
			// fire fury
			// cast fire curse, flame orb, or fireball
			if ( random ( 0, 1 ) || combatTarget->hasAffect ( _AFF_DAMAGE_FIRE, _AFF_TYPE_WEAKNESS ) ) {
				if ( random ( 0, 2 ) ) {
					// cast flame orb
					setAction ( new CombatCastSpell ( character, _SPELL_FLAME_ORB, combatTarget->servID, 0, 0 ) );
				} else {
					// cast fireball
					setAction ( new CombatCastSpell ( character, _SPELL_FIREBALL, character->servID, combatTarget->combatX, combatTarget->combatY ) );
				}
			} else {
				// curse 'em
				setAction ( new CombatCastSpell ( character, _SPELL_FIRE_CURSE, combatTarget->servID, 0, 0 ) );
			}

			return 1;
		}

		break;

		case 2: {
			// acid fury
			// cast acid curse, acid rain, or acid sphere.
			if ( random ( 0, 1 ) || combatTarget->hasAffect ( _AFF_DAMAGE_ACID, _AFF_TYPE_WEAKNESS ) ) {
				if ( random ( 0, 2 ) ) {
					// cast acid sphere
					setAction ( new CombatCastSpell ( character, _SPELL_ACID_SPHERE, combatTarget->servID, 0, 0 ) );
				} else {
					// cast acid rain
					setAction ( new CombatCastSpell ( character, _SPELL_ACID_RAIN, character->servID, combatTarget->combatX, combatTarget->combatY ) );
				}
			} else {
				// curse 'em
				setAction ( new CombatCastSpell ( character, _SPELL_ACID_CURSE, combatTarget->servID, 0, 0 ) );
			}

			return 1;
		}

		break;

		case 3: {
			// poison fury
			// cast poison curse or poison bolt
			if ( random ( 0, 1 ) || combatTarget->hasAffect ( _AFF_DAMAGE_POISON, _AFF_TYPE_WEAKNESS ) ) {
				// cast poison bolt
				setAction ( new CombatCastSpell ( character, _SPELL_POISON_BOLT, combatTarget->servID, 0, 0 ) );
			} else {
				// curse 'em
				setAction ( new CombatCastSpell ( character, _SPELL_POISON_CURSE, combatTarget->servID, 0, 0 ) );
			}

			return 1;
		}

		break;
	}

	// just melee
	return 0;
}

