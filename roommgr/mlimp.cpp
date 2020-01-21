//
// imp.cpp
//
// Control logic for imps.
//
// Author: Janus Anderson
//
#include <algorithm>
#include "roommgr.hpp"

Imp::Imp()
{
}

Imp::~Imp()
{
}

void Imp::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionTake ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "appears to relax." );
	emote->add ( "crouches." );
	emote->add ( "flexes his muscles." ); 
	emote->add ( "gauges your abilities." );
	emote->add ( "glares at you." );
	emote->add ( "looks at you intently." );
	emote->add ( "stomps the ground in displeasure." );
}

int Imp::spellAttack ( void )
{
	// imps should all cast elemental + necromancer spells -

	int thaumaturgySkill = character->getSkill ( _SKILL_THAUMATURGY );
	int sorcerySkill = character->getSkill (_SKILL_SORCERY );
	int elementalSkill = character->getSkill ( _SKILL_ELEMENTALISM );
	int necroSkill = character->getSkill ( _SKILL_NECROMANCY );

	if ( thaumaturgySkill )
	{
		/* heal myself or test to see if my party members need healing */
		int didHeal = partyHeal();
		if (didHeal) return 1;
	}

	// do I wanna cast a spell?
	if ( random ( 0, 1 ) )
	{
		if ( elementalSkill )
		{
			switch ( random ( 0, 3 ) )
			{
				case 0:
				// cast lightning bolt
				if ( elementalSkill > 3 )
				{
					setAction ( new CombatCastSpell ( character, _SPELL_LIGHTNING_BOLT, combatTarget->servID, 0, 0 ) );
					return 1;
				}

				case 1:
				// cast ice orb
				setAction ( new CombatCastSpell ( character, _SPELL_ICE_ORB, combatTarget->servID, 0, 0 ) );
				return 1;

				case 2:
				// cast flame orb
				setAction ( new CombatCastSpell ( character, _SPELL_FLAME_ORB, combatTarget->servID, 0, 0 ) );
				return 1;
			}
		}

		if ( necroSkill )
		{
			switch ( random ( 0, 3 ) )
			{
				case 0:
				// cast night friends
				if ( necroSkill > 2 )
				{
					setAction ( new CombatCastSpell ( character, _SPELL_NIGHT_FRIENDS, combatTarget->servID, 0, 0 ) );
					return 1;
				}

				case 1:
				// cast poison bolt
				setAction ( new CombatCastSpell ( character, _SPELL_POISON_BOLT, combatTarget->servID, 0, 0 ) );
				return 1;

				case 2:
				// cast acid sphere
				setAction ( new CombatCastSpell ( character, _SPELL_ACID_SPHERE, combatTarget->servID, 0, 0 ) );
				return 1;
			}
		}

		if ( thaumaturgySkill )
		{
			// cast a light dart
			setAction ( new CombatCastSpell ( character, _SPELL_LIGHT_DART, combatTarget->servID, 0, 0 ) );
			return 1;
		}
	}

	// if I'm a sorcerer combat teleport if out of range
	if ( sorcerySkill )
	{
		int dist = getDistance ( combatTarget->combatX, combatTarget->combatY, character->combatX, character->combatY );

		int rate = std::max(1, character->calcNumMoves());
//1 >? character->calcNumMoves();

		if ( dist > rate ) {
			/* combat teleport */
			setAction ( new CombatCastSpell ( character, _SPELL_COMBAT_TELEPORT, character->servID, combatTarget->combatX, combatTarget->combatY ) );
			return 1;
		}
	}

	// normal melee attack
	return 0;
}
