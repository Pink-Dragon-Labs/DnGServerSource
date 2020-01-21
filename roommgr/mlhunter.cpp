//
// mlhunter.cpp
//
// Control logic for bounty hunters.
//
// Author: Janus Anderson
//

#include "roommgr.hpp"

Hunter::Hunter()
{
}

Hunter::~Hunter()
{
}

void Hunter::init ( void )
{
	// set up action list
	new MWActionChill ( 0, this );
	new MWActionJump ( 0, this );
	new MWActionWander ( 0, this );
	new MWActionChangeRooms ( 0, this );
	new MWActionTake ( 0, this );
	MWActionRandomEmote* emote = new MWActionRandomEmote ( 0, this );
	emote->add ( "examines you closely." );
	emote->add ( "pages through a logbook." );
	emote->add ( "scratches an itch." );
	emote->add ( "stares at you trying to match the description." );
	emote->add ( "whistles a nameless tune." );
}

int Hunter::isFriend ( WorldObject *thisCharacter )
{
	return !isEnemy ( thisCharacter );
}

int Hunter::isEnemy ( WorldObject *thisCharacter ) {
	if ( thisCharacter->player ) {
		CrimeData* crime = thisCharacter->player->getCrimeData();

		if ( crime->bountyOnHead ) {
			return TRUE;
		}
	}

	return FALSE;
}

int Hunter::spellAttack ( void )
{
	// get our magical skills
	int thaumaturgySkill = character->getSkill (_SKILL_THAUMATURGY);
	int necromancySkill = character->getSkill (_SKILL_NECROMANCY);
	int elementalSkill = character->getSkill (_SKILL_ELEMENTALISM);
	int mysticismSkill = character->getSkill (_SKILL_MYSTICISM);
	int sorcerySkill = character->getSkill (_SKILL_SORCERY);

	// check our health and randomly flee from combat or heal.
	if ( thaumaturgySkill )
	{
		// just heal
		if ( partyHeal() ) return 1;
	}
	else if ( character->health < ( character->healthMax / 3 ) &&
		 !random ( 0, 3 ) )
	{
		// run away!
		fleeMode = TRUE;
		setAction ( new CombatFlee ( character ) );
		return 1;
	}

	// has no spell ability just yet -
	// melee only.

	return 0;
}
