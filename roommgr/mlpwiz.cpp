//
// mlpwiz.cpp
//
// Control logic for wizard NPCs.
//
// Author: Kerry Sergent
//

#include "roommgr.hpp"

PowerWiz::PowerWiz()
{
}

PowerWiz::~PowerWiz()
{
}

void PowerWiz::init ( void )
{
	new MWActionChill ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionGroup ( 0, this );

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "peers slyly towards you." );
	emote->add ( "listens intently." );
	emote->add ( "studies the land carefully." );
}

int PowerWiz::spellAttack ( void )
{
	/* heal myself or test to see if my party members need healing */
	int didHeal = partyHeal();
	int didSpell = 1;

	if ( didHeal ) 
		return didSpell;

	if ( random ( 0, 1 ) )
	{
		// cast a mysticism spell

		switch ( random ( 1, 5 ) )
		{
			case 1:
				if (character->opposition->size() > 1)
				{
					// mass berserk
					setAction ( new CombatCastSpell ( character, _SPELL_MASS_BERSERK, character->servID, 0, 0 ) );
					break;
				}
				else didSpell = 0;
			case 2:
			// fear
				setAction ( new CombatCastSpell ( character, _SPELL_FEAR, character->servID, 0, 0 ) );
			break;

			case 3:
			// confuse target
				setAction ( new CombatCastSpell ( character, _SPELL_CONFUSION, combatTarget->servID, 0, 0 ) );
			break;

			case 4:
			// warp mind on target
				setAction ( new CombatCastSpell ( character, _SPELL_WARP_MIND, combatTarget->servID, 0, 0 ) );
			break;

			case 5:
			// psychic orb
				setAction ( new CombatCastSpell ( character, _SPELL_PSYCHIC_ORB, combatTarget->servID, 0, 0 ) );
			break;
		}
		return didSpell;

	} else {
		
		// cast a thaum spell

		switch ( random ( 1, 2 ) )
		{
			case 1:
			// wrath of god if more than 1 opponent
				if (character->opposition->size() > 1)
				{
					setAction ( new CombatCastSpell ( character, _SPELL_WRATH_OF_THE_GODS, character->servID, 0, 0 ) );
					break;
				}
				else didSpell = 0;
			case 2:
			// summon an elemental
				setAction ( new CombatCastSpell ( character, _SPELL_SUMMON_ELEMENTAL, character->servID, character->combatX, character->combatY ) );
			break;
		}	
		return didSpell;
	}

	/* just melee attack as normal */
	return 0;
}

