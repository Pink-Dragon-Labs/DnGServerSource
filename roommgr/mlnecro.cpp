//
// mlnecro.cpp
//
// Control logic for undead necromancer and entombed one.
//
// Author: Janus Anderson
//

#include "roommgr.hpp"

Necro::Necro()
{
}

Necro::~Necro()
{
}

void Necro::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionTake ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "brushes maggots from his face." );
	emote->add ( "emits a foul, rotten odor." );
	emote->add ( "sighs an eerie moan." );
	emote->add ( "scratches an itch and dislodges a chunk of decayed flesh." );
	emote->add ( "stares blankly through you." );
}

int Necro::spellAttack ( void )
{
	// necromancer code
	if (random (0, 3) == 1)
	{
		/* cast acid bolt 25% first */
		setAction ( new CombatCastSpell ( character, _SPELL_ACID_SPHERE, combatTarget->servID, 0, 0 ) );
		return 1;
	}

	// summon zombie
	if ( character->getSkill ( _SKILL_NECROMANCY ) > 2 )
	{
		if (random (0, 2) == 1)
		{
			if ( getSummonedCount ( _FRIENDLY_SUMMONED_MONSTERS ) <  _MAX_SUMMONED_MONSTERS )
			{
				/* summon a zombie */
				setAction ( new CombatCastSpell ( character, _SPELL_SUMMON_ZOMBIE, combatTarget->servID, combatTarget->combatX, combatTarget->combatY ) );
				return 1;
			}
		}
	}

	// just attack as normal
	return 0;
}

