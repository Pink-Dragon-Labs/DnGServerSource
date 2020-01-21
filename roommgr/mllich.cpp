//
// lich.cpp
//
// Control logic for daemons.
//
// Author: Stephen Nichols
//

#include "roommgr.hpp"
#include "globals.hpp"

Lich::Lich()
{
}

Lich::~Lich()
{
}

int Lich::spellAttack ( void )
{

	int distance = getDistance ( combatTarget->combatX, combatTarget->combatY, character->combatX, character->combatY);

	if ( (distance < gTouchDistance) && random ( 0, 1 ) )
	{
		switch ( random ( 0, 6 ) )
		{
			case 0:
			// cast death wish
			setAction ( new CombatCastSpell ( character, _SPELL_DEATH_WISH, combatTarget->servID, 0, 0 ) );
			return 1;

			case 1:
			// cast steal life
			setAction ( new CombatCastSpell ( character, _SPELL_STEAL_LIFE, combatTarget->servID, 0, 0 ) );
			return 1;

			case 2:
			// cast death touch
			setAction ( new CombatCastSpell ( character, _SPELL_DEATH_TOUCH, combatTarget->servID, 0, 0 ) );
			return 1;

			case 3:
			case 4:
			// teleport to other side of combat field

			if ( character->combatX < ( _COMBAT_GRID_WIDTH / 2 ) )
			{
				// teleport to right side
				setAction ( new CombatCastSpell ( character, _SPELL_COMBAT_TELEPORT, character->servID, _COMBAT_GRID_WIDTH - 1, combatTarget->combatY ) );
				return 1;
			}
			else
			{
				// teleport to left side side
				setAction ( new CombatCastSpell ( character, _SPELL_COMBAT_TELEPORT, character->servID, 0, combatTarget->combatY ) );
				return 1;
			}

			case 5:
			case 6:
			// melee attack - return 0
			return 0;
		}
	}

	else
	{
		// cast a distance or mass spell
		switch ( random ( 0, 3 ) )
		{
			case 0:
			// cast mass drain, muhuhaha.
			setAction ( new CombatCastSpell ( character, _SPELL_MASS_DRAIN, character->servID, 0, 0 ) );
			return 1;

			case 1:
			// cast a poison bolt
			setAction ( new CombatCastSpell ( character, _SPELL_POISON_BOLT, combatTarget->servID, 0, 0 ) );
			return 1;

			case 2:
			// summon some undead
			setAction ( new CombatCastSpell ( character, _SPELL_SUMMON_UNDEAD, combatTarget->servID, combatTarget->combatX, combatTarget->combatY ) );
			return 1;

			case 3:
			// cast acid sphere
			setAction ( new CombatCastSpell ( character, _SPELL_ACID_SPHERE, combatTarget->servID, 0, 0 ) );
			return 1;
		}
	}

	// normal melee attack
	return 0;
}
