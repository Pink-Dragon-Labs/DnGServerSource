//
// mlogreMage.cpp
//
// Control logic for ogre mages.
//
// Author: Janus Anderson
//

#include "roommgr.hpp"

OgreMage::OgreMage()
{
}

OgreMage::~OgreMage()
{
}

void OgreMage::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionTake ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "attempts to cast some spell but fails miserably." );
	emote->add ( "belches." );
	emote->add ( "fidgets with his mana." );
	emote->add ( "forgets what he was doing." );
	emote->add ( "grumbles." );
	emote->add ( "mumbles something unintelligible." );
	emote->add ( "scratches at a flea bite." );
}

int OgreMage::spellAttack ( void )
{
	if (random (0, 3) == 1)
	{
		/* try to cast a multiblade */
		if ( random ( 0, 1 ) )
		{
			/* success */
			setAction ( new CombatCastSpell ( character, _SPELL_MULTI_BLADE, combatTarget->servID, 0, 0 ) );
		}
		else
		{
			/* failed */
			setAction ( new CombatSay ( character, "%s motions in the air and mumbles '%s', but nothing happens!", character->getName(), gSpellTable[_SPELL_MULTI_BLADE].verbal ) );
		}
		return 1;
	}

	// attempt to cast killstar
	if (random (0, 3) == 1)
	{
		/* try to cast a killstar*/
		if ( random ( 0, 2 ) )
		{
			/* success */
			setAction ( new CombatCastSpell ( character, _SPELL_KILL_STAR, combatTarget->servID, 0, 0 ) );
		}
		else
		{
			/* failed */
			/* did I critical fail? */
			if ( random ( 0, 2 ) )
			{
				/* nope, just failed */
				setAction ( new CombatSay ( character, "%s motions in the air and mumbles '%s', but nothing happens!", character->getName(), gSpellTable[_SPELL_KILL_STAR].verbal ) );
			}
			else
			{
				/* hit myself with the killstar */
				CombatCastSpell *tAction;
				CombatSay *tAction2;
				tAction = new CombatCastSpell ( character, _SPELL_KILL_STAR, character->servID, 0, 0 );
				tAction2 = new CombatSay ( character, "%s looks dumbfounded as it's spell has somehow backfired!.", character->getName() );
				tAction->setNext ( tAction2 );
				setAction ( tAction );
			}
		}
		return 1;
	}

	// just attack as normal
	return 0;
}

