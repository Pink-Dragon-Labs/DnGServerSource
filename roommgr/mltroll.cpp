//
// mltroll.cpp
//
// Control logic for trolls.
//
// Author: Janus Anderson
//

#include "roommgr.hpp"

TrollSpellcaster::TrollSpellcaster()
{
}

TrollSpellcaster::~TrollSpellcaster()
{
}

void TrollSpellcaster::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionTake ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "begins an ancient chant but forgets the words." );
	emote->add ( "checks its equipment over." );
	emote->add ( "leafs through a spellbook, pretending it can read." );
	emote->add ( "fumbles in its pouch for mana crystals." );
}

int TrollSpellcaster::spellAttack ( void )
{
	// code for elementalist troll
	if ( character->getSkill ( _SKILL_ELEMENTALISM ) )
	{
		/* cast a random elementalist spell (assumes expert ) */
		switch ( random ( 0, ( character->getSkill ( _SKILL_ELEMENTALISM ) ) ) )
		{
			case 0:
			case 1:
			// just attack normall
			return 0;

			case 2:
			// ice orb
			setAction ( new CombatCastSpell ( character, _SPELL_ICE_ORB, combatTarget->servID, 0, 0 ) );
			break;

			case 3:
			// flame orb
			setAction ( new CombatCastSpell ( character, _SPELL_FLAME_ORB, combatTarget->servID, 0, 0 ) );
			break;

			case 4:
			// lightning bolt
			setAction ( new CombatCastSpell ( character, _SPELL_LIGHTNING_BOLT, combatTarget->servID, 0, 0 ) );
			break;

			case 5:
			// earthquake
			setAction ( new CombatCastSpell ( character, _SPELL_EARTHQUAKE, combatTarget->servID, 0, 0 ) );
			break;
		}
		// spell cast, return.
		return 1;
	}
	else if ( character->getSkill ( _SKILL_SORCERY ) )
	{
		/* cast a random sorcery spell or attack */
			switch ( random ( 0, ( character->getSkill ( _SKILL_SORCERY ) ) ) )
		{
			case 0:
			case 1:
			// normal attack
			return 0;
			break;

			case 2:
				setAction ( new CombatCastSpell ( character, _SPELL_MULTI_BLADE, combatTarget->servID, 0, 0 ) );
			break;

			case 3:
				setAction ( new CombatCastSpell ( character, _SPELL_DISPEL_MAGIC, combatTarget->servID, 0, 0 ) );
			break;

			case 4:
				setAction ( new CombatCastSpell ( character, _SPELL_RUST, combatTarget->servID, 0, 0 ) );
			break;

			case 5:
				setAction ( new CombatCastSpell ( character, _SPELL_IMPROVED_INVISIBILITY, character->servID, 0, 0 ) );
			break;
		}
		// spell cast, return.
		return 1;
	}

	// just normal attack
	return 0;
}


