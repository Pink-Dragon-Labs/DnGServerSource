/*
	BWeapon class	
	author: Stephen Nichols
*/

#include "bweapon.hpp"
#include "roommgr.hpp"

Weapon::Weapon()
{
	damageType = 2;
	speed = 1;
	minDamage = 1;
	maxDamage = 4;
	spellProcID = 0;
	reverseProcID = 0;
	forwardProcChance = 0;
	reverseProcChance = 0;
	modifier = 0;
	bonus = 0;
	distance = 1;
	isMissile = 0;
	skillType = _SKILL_UNARMED;

	hands = _WEAPON_ONE_HANDED;
}

Weapon::~Weapon()
{
}

int Weapon::getSkill ( BCharacter *bchar )
{
	return _SKILL_LVL_FAMILIAR;
}

const int BWeapon::_DAMAGE_NORMAL;
const int BWeapon::_DAMAGE_FIRE;
const int BWeapon::_DAMAGE_COLD;
const int BWeapon::_DAMAGE_LIGHTNING;
const int BWeapon::_DAMAGE_ACID;
const int BWeapon::_DAMAGE_POISON;
const int BWeapon::_DAMAGE_STAMINA;
const int BWeapon::_DAMAGE_STEAL_STAMINA;
const int BWeapon::_DAMAGE_EXPERIENCE;
const int BWeapon::_DAMAGE_STEAL_EXPERIENCE;
const int BWeapon::_DAMAGE_STEAL_LIFE;
const int BWeapon::_DAMAGE_RUST;
const int BWeapon::_DAMAGE_ETHEREAL;
const int BWeapon::_DAMAGE_STUN;

BWeapon::BWeapon ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BWEAPON;
	owner = NULL;
	mask = _WEAR_MASK_EVERYONE;
}

BWeapon::~BWeapon()
{
}

void BWeapon::copy ( WorldObjectBase *theBase )
{
	BWeapon *base = (BWeapon *)theBase;
	mask = base->mask;
	spellProcID = base->spellProcID;
    forwardProcChance = base->forwardProcChance;
    reverseProcID = base->reverseProcID;
    reverseProcChance = base->reverseProcChance;

	*(Weapon *)this = *(Weapon *)base;
}


void BWeapon::buildPacket ( PackedData *packet, int override )
{
}

void BWeapon::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t((aWhatObj addBase: BWeapon)\n" );
	fprintf ( file, "\t\t\tpDamageType: %d,\n", damageType );
	fprintf ( file, "\t\t)\n" );
}

int BWeapon::handlesAction ( int action )
{
	switch ( action ) {
		case vBePutOn:
		case vBeTakenOff:
			return 1;
			break;
	}

	return 0;
}

int BWeapon::perform ( int action, va_list *args )
{
	int retVal = _WO_ACTION_ALLOWED;

	switch ( action ) {
		case vBePutOn: {
			WorldObject *object = va_arg ( *args, WorldObject * );

			if ( object )
				retVal = bePutOn ( object );
		}

		break;

		case vBeTakenOff: {
			WorldObject *object = va_arg ( *args, WorldObject * );

			if ( object )
				retVal = beTakenOff ( object );
		}

		break;

		// intercept ownership changers and stop them if we are being wielded
		// by an object
		//
		case vBeDroppedBy:
		case vBePutIn:
		case vBeTakenBy:
		case vBeGivenTo: {
			if ( owner )
				retVal = _ERR_MUST_REMOVE;
		}

		break;
	}

	return retVal;
}

int BWeapon::bePutOn ( WorldObject *object )
{
	int retVal = _WO_ACTION_PROHIBITED;

	// skip out if self is not owned by object
	if ( self->getOwner() != object )
		goto end;

	// skip out if self is already wielded by an object
	if ( owner ) {
		retVal = _ERR_REDUNDANT;
		goto end;
	}

	// skip out if object is already wielding an object
	if ( object->curWeapon ) {
		retVal = _ERR_NO_ROOM;
		goto end;
	}

	// skip out if we can't wield this
	if ( (object->wearMask() & mask) != object->wearMask() ) {
		retVal = _ERR_CANT_WEAR;
		goto end;
	}

	// skip out if this is a two-handed weapon and a shield is worn
	if ( object->curShield && hands == _WEAPON_TWO_HANDED ) {
		retVal = _ERR_NO_SHIELD;
		goto end;
	}

	// skip out if this is too high a level for me to use...
	if ( object->player && !object->player->isNPC && object->level < self->level ) {
		retVal = _ERR_TOO_EXPENSIVE;
		goto end;
	}

	owner = object;
	self->objectWieldedOn = object->servID;
	self->makeVisible ( 1 );

	object->curWeapon = self;

	if ( owner )
		owner->calcAC();

	// set retVal to success
	retVal = _WO_ACTION_HANDLED;

end:

	return retVal;
}

int BWeapon::beTakenOff ( WorldObject *object )
{
	int retVal = _WO_ACTION_PROHIBITED;

	if ( !object )
		object = owner;

	// skip out if self is not owned by object
	if ( !self->isOwnedBy ( object ) )
		goto end;

	// skip out if we are current wielded by object
	if ( owner != object || object->curWeapon != self ) {
		retVal = _ERR_REDUNDANT;
		goto end;
	}

	owner = NULL;
	self->objectWieldedOn = -1;
	self->makeVisible ( 0 );

	object->curWeapon = NULL;

	// set the retVal to success
	retVal = _WO_ACTION_HANDLED;

end:

	return retVal;
}
