//
// mllwiz.cpp
//
// Control logic for lightwiz NPCs.
//
// Author: Janus Anderson
//

#include "roommgr.hpp"

LightWiz::LightWiz()
{
}

LightWiz::~LightWiz()
{
}

void LightWiz::init ( void )
{
	new MWActionChill ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionGroup ( 0, this );

	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "reaches into her pouch for mana crystals." );
	emote->add ( "hums an ancient melody." );
	emote->add ( "looks at her broken fingernail." );
	emote->add ( "mumbles a few arcane syllables." );
	emote->add ( "ruffles through her spellbook." );
}

int LightWiz::spellAttack ( void )
{

	/* heal myself or test to see if my party members need healing */
	int didHeal = partyHeal();
	int didSpell = 1;

	if (didHeal) 
		return didSpell;

	if ( random ( 0, 1 ) )
	{
		switch ( random ( 1, 5 ) )
		{
			// cast a mysticism spell
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
	}
	else
	{
		// cast a thaum spell
		switch ( random ( 1, 4 ) )
		{
			case 1:
			// invulnerability myself if not already
			if ( !character->hasAffect (_AFF_INVULNERABLE))
			{
				setAction ( new CombatCastSpell ( character, _SPELL_INVULNERABILITY, character->servID, 0, 0 ) );
				break;
			}
			else didSpell = 0;

			case 2:
			// wrath of god if more than 1 opponent
			if (character->opposition->size() > 1)
			{
				setAction ( new CombatCastSpell ( character, _SPELL_WRATH_OF_THE_GODS, character->servID, 0, 0 ) );
				break;
			} 
			else didSpell = 0;

			case 3:
			// summon a faery queen
				setAction ( new CombatCastSpell ( character, _SPELL_SUMMON_FAERY_QUEEN, character->servID, character->combatX, character->combatY ) );
			break;

			case 4:
			// light dart
				setAction ( new CombatCastSpell ( character, _SPELL_LIGHT_DART, combatTarget->servID, 0, 0 ) );
			break;
		}
		return didSpell;
	}

	/* just melee attack as normal */
	return 0;
}

