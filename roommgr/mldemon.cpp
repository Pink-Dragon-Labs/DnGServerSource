//
// demon.cpp
//
// Control logic for daemons.
//
// Author: Stephen Nichols
//

#include "roommgr.hpp"
#include "globals.hpp"

Daemon::Daemon()
{
}

Daemon::~Daemon()
{
}

void Daemon::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
   	new MWActionTake ( 0, this );

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "expunges a foul reek." );
	emote->add ( "hisses through his fangs." );
	emote->add ( "sharpens his claws." );
	emote->add ( "spits an acidic bile on the ground." );
	emote->add ( "stretches its muscular wings." );
}

int Daemon::spellAttack ( void )
{
	// daemons should all cast elemental + necromancer spells -

	int elementalSkill = character->getSkill ( _SKILL_ELEMENTALISM );
	int necroSkill = character->getSkill ( _SKILL_NECROMANCY );

	// do I want to cast a spell? (50% of the time)

	if ( ( random ( 0, 1 ) ) &&
	   ( elementalSkill + necroSkill ) )
	{
		int distance = getDistance ( combatTarget->combatX, combatTarget->combatY, character->combatX, character->combatY);

		// if in touch range, cast a touch spell.

		if ( (distance < gTouchDistance) && random ( 0, 1 ) )
		{
			switch ( random ( 0, 3 ) )
			{
				case 0:
				// cast spark
				if ( elementalSkill > 2 )
				{
					setAction ( new CombatCastSpell ( character, _SPELL_SPARK, combatTarget->servID, 0, 0 ) );
					return 1;
				}

				case 1:
				// cast steal life
				if ( necroSkill > 2 )
				{
					setAction ( new CombatCastSpell ( character, _SPELL_STEAL_LIFE, combatTarget->servID, 0, 0 ) );
					return 1;
				}

				case 2:
				// cast death touch
				if ( necroSkill > 3 )
				{
					setAction ( new CombatCastSpell ( character, _SPELL_DEATH_TOUCH, combatTarget->servID, 0, 0 ) );
					return 1;
				}

				case 3:
				// cast spark
				if ( elementalSkill > 2 )
				{
					setAction ( new CombatCastSpell ( character, _SPELL_SPARK, combatTarget->servID, 0, 0 ) );
					return 1;
				}
			}
		}
		else
		{
			// cast a distance spell
			switch ( random ( 0, 5 ) )
			{
				case 0:
				// cast a flame orb
				if ( elementalSkill )
				{
					setAction ( new CombatCastSpell ( character, _SPELL_FLAME_ORB, combatTarget->servID, 0, 0 ) );
					return 1;
				}

				case 1:
				// cast a poison bolt
				if ( necroSkill > 1 )
				{
					setAction ( new CombatCastSpell ( character, _SPELL_POISON_BOLT, combatTarget->servID, 0, 0 ) );
					return 1;
				}

				case 2:
				// summon some undead
				if ( necroSkill > 2 )
				{
					setAction ( new CombatCastSpell ( character, _SPELL_SUMMON_UNDEAD, combatTarget->servID, combatTarget->combatX, combatTarget->combatY ) );
					return 1;
				}

				case 3:
				//call a crushing boulder
				if ( elementalSkill > 3 )
				{
					setAction ( new CombatCastSpell ( character, _SPELL_CRUSHING_BOULDER, combatTarget->servID, 0, 0 ) );
					return 1;
				}

				case 4:
				// uh oh... summon another daemon
				if ( necroSkill > 4 )
				{
					setAction ( new CombatCastSpell ( character, _SPELL_SUMMON_DAEMON, combatTarget->servID, combatTarget->combatX, combatTarget->combatY ) );
					return 1;
				}

				case 5:
				// lightning bolt
				if ( elementalSkill > 3 )
				{
					setAction ( new CombatCastSpell ( character, _SPELL_LIGHTNING_BOLT, combatTarget->servID, 0, 0 ) );
					return 1;
				}
			}
		}
	}

	// normal melee attack
	return 0;
}
