//
// mlseraph.cpp
//
// Control logic for faeries.
//
// Author: Stephen Nichols
//

#include "roommgr.hpp"

Seraph::Seraph()
{
}

Seraph::~Seraph()
{
}

void Seraph::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionGroup ( 0, this );
	new MWActionHeal ( 0, this );
	new MWActionTake ( 0, this );
	new MWActionCure ( 0, this );
	new MWActionBless ( 0, this, _SPELL_EMPOWER, _AFF_EMPOWER, _AFF_TYPE_NORMAL );
	new MWActionBless ( 0, this, _SPELL_NIMBILITY, _AFF_POS_DEX_MOD, _AFF_TYPE_NORMAL );

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "floats in deep reflection." );
	emote->add ( "grooms her wings." );
	emote->add ( "looks upon you with interest." );
}

int Seraph::spellAttack ( void )
{

	/* heal myself or test to see if my party members need healing */
	int didHeal = partyHeal();
	if (didHeal) return 1;

	/* if I'm a grand-master thaumaturgist, summon another seraph if possible */
	if ( ( character->getSkill ( _SKILL_THAUMATURGY ) == 5 ) &&
		 ( !random ( 0, 3 ) ) )
	{
		/* summon another seraph */
		if ( getSummonedCount ( _FRIENDLY_SUMMONED_MONSTERS ) <  _MAX_SUMMONED_MONSTERS )
		{
			setAction ( new CombatCastSpell ( character, _SPELL_SUMMON_SERAPH, combatTarget->servID, combatTarget->combatX, combatTarget->combatY ) );
			return 1;
		}
	}

	/* try a banishment if there are enemy summoned creatures */
	if ( (!random ( 0, 1 )) &&
		 ( getSummonedCount ( _ENEMY_SUMMONED_MONSTERS ) ) )
	{
		/* cast banishment */
		setAction ( new CombatCastSpell ( character, _SPELL_BANISHMENT, character->servID, 0, 0 ) );
		return 1;
	}

	/* mabye cast a light dart / flame orb / ice orb */
	if ( character->getSkill ( _SKILL_ELEMENTALISM ) )
	{
		/* just do a random light dart / flame orb / lightning bolt / ice orb at my skill level */
		switch ( random ( 0, 3 ) )
		{
			case 0:
			setAction ( new CombatCastSpell ( character, _SPELL_LIGHT_DART, combatTarget->servID, 0, 0 ) );
			break;

			case 1:
			setAction ( new CombatCastSpell ( character, _SPELL_FLAME_ORB, combatTarget->servID, 0, 0 ) );
			break;

			case 2:
			setAction ( new CombatCastSpell ( character, _SPELL_ICE_ORB, combatTarget->servID, 0, 0 ) );
			break;

			case 3:
			setAction ( new CombatCastSpell ( character, _SPELL_LIGHTNING_BOLT, combatTarget->servID, 0, 0 ) );
			break;
		}
	}
	else
	{
		/* must be a warrior seraph - just attack */

		if (random (0, 1) )
		{
			// shoot a light dart
			setAction ( new CombatCastSpell ( character, _SPELL_LIGHT_DART, combatTarget->servID, 0, 0 ) );
			return 1;
		}
		// melee attack
		return 0;
	}

	return 1;
}

