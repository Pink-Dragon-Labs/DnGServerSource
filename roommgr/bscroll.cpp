/*
	BScroll class
	author: Stephen Nichols
*/

#include "roommgr.hpp"

BScroll::BScroll ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BSCROLL;

	spell = -1;
	skill = -1;
	bpCost = 1;
}

BScroll::~BScroll()
{
}

void BScroll::copy ( WorldObjectBase *theBase )
{
	BScroll *base = (BScroll *)theBase;
	spell = base->spell;
	skill = base->skill;
	bpCost = base->bpCost;
}

void BScroll::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t(aWhatObj addBase: BScroll)\n" );
}

// action handling methods
int BScroll::handlesAction ( int action )
{
	return action == vBeMemorized;
}

int BScroll::perform ( int action, va_list *args )
{
	int retVal = _WO_ACTION_ALLOWED;

	switch ( action ) {
		case vBeMemorized: {
			WorldObject *object = va_arg ( *args, WorldObject * );
			retVal = beMemorized ( object );
		}

		break;
	}

	return retVal;
}

// handle being memorized by a player 
int BScroll::beMemorized ( WorldObject *object )
{
	BCharacter *bchar = (BCharacter *)object->getBase ( _BCHARACTER );

	// skip out if there is no character
	if ( !bchar )
		return _ERR_BAD_SERVID;


	if ( object->player && !object->player->isNPC && object->level < self->level ) {
		return _ERR_TOO_EXPENSIVE;
	}

	// if we are trying to learn a spell, do this
	if ( spell > -1 ) {
		// if we already know the spell, skip out
		if ( bchar->knowsSpell ( spell ) )
			return _ERR_REDUNDANT;

		spell_info *theSpell = &gSpellTable[spell];

		// if the character does not have the skill, tell him so
		if ( bchar->getSkill ( theSpell->skillType ) < theSpell->skillLevel ) 
			return _ERR_SKILL_FAILURE;

		// learn the spell
		bchar->learnSpell ( spell );
	}

	// if we are trying to learn a skill, do this
	else if ( skill > -1 ) {
		// if we already have the level of skill, skip out
		if ( bchar->getSkill ( skill ) >= self->level ) 
			return _ERR_REDUNDANT;

		// if we are trying to memorize something way too powerful for us, check here
		if ( bchar->getSkill ( skill ) < (self->level - 1) )
			return _ERR_SKILL_FAILURE;

		int theBPCost = calcBPCost ( object );

		// if we don't have the proper number of build points, bail out
		if ( bchar->buildPoints < theBPCost )
			return _ERR_TOO_EXPENSIVE;
	
		// learn the skill now
		bchar->setSkill ( skill, self->level );
		bchar->buildPoints -= theBPCost;
	}

	return _WO_ACTION_HANDLED;
}

int BScroll::calcBPCost ( WorldObject *obj )
{
	double attribute = 0, numAttribs = 0;

	if ( self->strength ) {
		attribute += obj->strength;
		numAttribs++;
	}

	if ( self->dexterity ) {
		attribute += obj->dexterity;
		numAttribs++;
	}

	if ( self->intelligence ) {
		attribute += obj->intelligence;
		numAttribs++;
	}

	if ( self->endurance ) {
		attribute += obj->endurance;
		numAttribs++;
	}

	if ( numAttribs )
		attribute /= numAttribs;

	int theCost = bpCost;

	if ( attribute > 14 ) {
		double percent = 0.03125 * (attribute - 14.0);
		int reduction = (int)floor ( percent * theCost );
		theCost -= reduction;
	} 
	
	else if ( attribute < 14 ) {
		double percent = 0.083 * (14.0 - attribute);
		int increase = (int)floor ( percent * theCost );
		theCost += increase;
	}

	theCost = (theCost * 5) / 4;

	if ( theCost < 1 )
		theCost = 1;

	return theCost;
}
