//
// mlstorm.cpp
//
// Control logic for storm bats.
//
// Author: Janus Anderson
//

#include "roommgr.hpp"

StormBat::StormBat()
{
}

StormBat::~StormBat()
{
}

int StormBat::spellAttack ( void )
{
	switch (random (0, 4))
	{
		case 0:
		case 1:
		case 2:
		{
			if ( combatTarget->hasAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_WEAKNESS ) )
			{
				// cast lightning bolt
				setAction ( new CombatCastSpell ( character, _SPELL_LIGHTNING_BOLT, combatTarget->servID, 0, 0 ) );
			}
			else
			{
				// curse 'em
				setAction ( new CombatCastSpell ( character, _SPELL_ELECTRIC_CURSE, combatTarget->servID, 0, 0 ) );
			}
			return 1;
		}
		break;

		default:
		break;
	}

	// just melee like normal.
	return 0;
}

