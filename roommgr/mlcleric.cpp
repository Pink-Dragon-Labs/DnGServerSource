//
// mlcleric.cpp
//
// Control logic for cleric NPCs.
//
// Author: Janus Anderson
//
#include <algorithm>
#include "roommgr.hpp"

Cleric::Cleric()
{
}

Cleric::~Cleric()
{
}

void Cleric::init ( void )
{
	new MWActionChill ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionGroup ( 0, this );
	new MWActionHeal ( 0, this );
	new MWActionCure ( 0, this );

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "chants some solemn phrases." );
	emote->add ( "hums a peaceful hymn." );
	emote->add ( "mumbles a quick prayer to Enid." );
	emote->add ( "prays to Enid." );
	emote->add ( "smiles warmly." );
}

int Cleric::spellAttack ( void )
{

	/* heal myself or test to see if my party members need healing */
	int didHeal = partyHeal();
	if (didHeal) return 1;

	/* possibly summon a nymph */
	if (random (0, 2) == 1)
	{
		if ( getSummonedCount ( _FRIENDLY_SUMMONED_MONSTERS ) <  _MAX_SUMMONED_MONSTERS )
		{
			/* summon a nymph */
			setAction ( new CombatCastSpell ( character, _SPELL_SUMMON_NYMPH, character->servID, character->combatX, character->combatY ) );
			return 1;
		}
	}

	/* if character is out of range of melee then light dart */
	int dist = getDistance ( combatTarget->combatX, combatTarget->combatY, character->combatX, character->combatY );

	int rate = std::max(1, character->calcNumMoves());
//1 >? character->calcNumMoves();

	if ( dist > rate ) {
		/* light dart */
		setAction ( new CombatCastSpell ( character, _SPELL_LIGHT_DART, combatTarget->servID, 0, 0 ) );
		return 1;
	}

	/* just melee attack as normal */
	return 0;
}

