//
// mlswashbuckler.cpp
//
// Control logic for Swashbucklers.
//
// Author: Zachary Kaiser
//

#include "roommgr.hpp"

Swashbuckler::Swashbuckler()
{
}

Swashbuckler::~Swashbuckler()
{
}

void Swashbuckler::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionGroup ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionTake ( 0, this );
	new MWActionSteal ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "eyes your coinpurse." );
	emote->add ( "fiddles with his sword." );
	emote->add ( "mumbles an ancient swear." );
	emote->add ( "sizes you up." );
    emote->add ( "laughs at you." );
    emote->add ( "spits towards you." );
    emote->add ( "swears under his breath at you." );
}

int Swashbuckler::spellAttack ( void ) {

	/* ----------- Cast Banishment if there are enemy summoned creatures -----------*/
	if ( (!random ( 0, 1 )) && ( getSummonedCount ( _ENEMY_SUMMONED_MONSTERS ) ) ) {
		setAction ( new CombatCastSpell ( character, _SPELL_BANISHMENT, character->servID, 0, 0 ) );
		return 1;
	}

	/* ----------- If we're not Invisible, become so -----------*/
	if ( !character->hasAffect ( _AFF_IMPROVED_INVISIBILITY ) ) {
		setAction ( new CombatCastSpell ( character, _SPELL_IMPROVED_INVISIBILITY, character->servID, 0, 0 ) );
		return 1;	
	}

	/* ----------- Pick an Attack -----------*/
	switch ( random ( 0, 23 ) ) {
		case 0:
		return 0;
		case 1:
		return 0;
		case 2:
			setAction ( new CombatCastSpell ( character, _SPELL_IRON_CHAINS, combatTarget->servID, 0, 0 ) );
			break;
		case 3:
			setAction ( new CombatCastSpell ( character, _SPELL_DEFENSELESSNESS, combatTarget->servID, 0, 0 ) );
			break;
		case 4:
		return 0;
		case 5:
		return 0;
		case 6:
			setAction ( new CombatCastSpell ( character, _SPELL_CURSE_OF_CLUMSINESS, combatTarget->servID, 0, 0 ) );
			break;
		case 7:
		return 0;
		case 8:
		return 0;
		case 9:
		return 0;
		case 10:
		return 0;
		case 11:
		return 0;
		case 12:
		return 0;
		case 13:
		return 0;
		case 14:
		return 0;
		case 15:
		return 0;
		case 16:
		return 0;
		case 17:
		return 0;
		case 18:
		return 0;
		case 19:
		return 0;
		case 20:
		return 0;
		case 21:
		return 0;
		case 22:
		return 0;
		case 23:
			setAction ( new CombatCastSpell ( character, _SPELL_ENFEEBLE, combatTarget->servID, 0, 0 ) );
			break;
	}				// ~~~~~ End switch

	return 1;
}					// ~~~~~ End Swashbuckler::spellAttack