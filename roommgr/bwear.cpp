/*
	BWearable class	
	author: Stephen Nichols
*/

#include "bwear.hpp"
#include "roommgr.hpp"

BWearable::BWearable ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BWEAR;
	areaWorn = 0;
	mask = _WEAR_MASK_EVERYONE;
	layer = 0;
	level = 0;	// added kds 10/6/98
	spellProcID = 0;
	forwardProcChance = 0;
    reverseProcChance = 0;
    reverseProcID = 0;
	owner = NULL;
}

BWearable::~BWearable()
{
}

void BWearable::copy ( WorldObjectBase *theBase )
{
	BWearable *base = (BWearable *)theBase;

	layer = base->layer;
	areaWorn = base->areaWorn;
	reverseProcID = base->reverseProcID;
	spellProcID = base->spellProcID;
	forwardProcChance = base->forwardProcChance;
	reverseProcChance = base->reverseProcChance;
	mask = base->mask;
}

void BWearable::buildPacket ( PackedData *packet, int override )
{
}

void BWearable::writeSCIData ( FILE *file )
{
   fprintf ( file, "\t\t((aWhatObj addBase: BWearable)\n" );
	fprintf ( file, "\t\t\tpLayer: %d,\n", layer );
	fprintf ( file, "\t\t\tpAreaWorn: %d,\n", areaWorn );
	fprintf ( file, "\t\t\tpMask: %d,\n", mask );
	fprintf ( file, "\t\t)\n" );
}

int BWearable::handlesAction ( int action )
{
	if ( !self->getBase ( _BWEAPON ) ) {
		switch ( action ) {
			case vBePutOn:
			case vBeTakenOff:
				return 1;
				break;
		}
	}

	return 0;
}

int BWearable::perform ( int action, va_list *args )
{
	int retVal = _WO_ACTION_ALLOWED;

	if ( self->getBase ( _BWEAPON ) )
		return retVal;

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

		// intercept take and drop requests and stop them if we are currently
		// being worn by an object
		//
		case vBeDroppedBy:
		case vBePutIn:
		case vBeTakenBy: {
			if ( owner )
				retVal = _ERR_MUST_REMOVE;
		}

		break;
	}

	return retVal;
}

int BWearable::bePutOn ( WorldObject *object )
{
	int retVal = _WO_ACTION_PROHIBITED;

	// skip out if self is not owned by object
	if ( self->getOwner() != object ) {
		goto end;
	}

	// skip out if we are already worn by a WorldObject
	if ( owner ) {
		goto end;
	}

	// if this object is to be worn as a ring, do some special logic here
	if ( areaWorn == _WEAR_RING_L || areaWorn == _WEAR_RING_R ) {
		WorldObject *leftRing = object->getWornOn ( _WEAR_RING_L );
		WorldObject *rightRing = object->getWornOn ( _WEAR_RING_R );

		if ( !leftRing )
			areaWorn = _WEAR_RING_L;

		else if ( !rightRing )
			areaWorn = _WEAR_RING_R;

		else {
			retVal = _ERR_NO_ROOM;
			goto end;		
		}
	}

	// skip out if object is already wearing something where self
	// would go
	//
	if ( object->getWornOn ( areaWorn ) ) {
		retVal = _ERR_NO_ROOM;
		goto end;		
	}

	// skip out if the object to be worn on is not compatible with my wear
	// mask
	//
	if ( (object->wearMask() & mask) != object->wearMask() ) {
		retVal = _ERR_CANT_WEAR;
		goto end;
	}

	// skip out if it is too powerful to wear
	if ( object->player && !object->player->isNPC && object->level < self->level ) {
		retVal = _ERR_TOO_EXPENSIVE;
		goto end;
	}

	// check to make sure that the current weapon is not conflicting with this 
	if ( areaWorn == _WEAR_SHIELD && object->curWeapon ) {
		BWeapon *weapon = (BWeapon *)object->curWeapon->getBase ( _BWEAPON );

		if ( weapon && weapon->hands == _WEAPON_TWO_HANDED ) {
			retVal = _ERR_NO_WEAPON;
			goto end;
		}
	}
	
	if ( areaWorn == _WEAR_SHIELD )
		object->curShield = self;

	owner = object;
	self->objectWornOn = object->servID;
	self->makeVisible ( 1 );

	if ( owner ) {
		owner->calcAC();
	}

	// set retVal to success
	retVal = _WO_ACTION_HANDLED;

end:

	return retVal;
}

int BWearable::beTakenOff ( WorldObject *object )
{
	int retVal = _WO_ACTION_PROHIBITED;

	if ( !object )
		object = owner;

	if ( !object )
		goto endNow;

	if ( self->hasAffect ( _AFF_CURSED ) )
		goto end;

	// skip out if self is not owned by object
	if ( !self->isOwnedBy ( object ) )
		goto end;

	// skip out if we are not being worn by object
	if ( owner != object )
		goto end;

	if ( areaWorn == _WEAR_SHIELD )
		owner->curShield = NULL;

	owner = NULL;
	self->objectWornOn = -1;
	self->makeVisible ( 0 );

	if ( object ) {
		object->calcAC();
	}

	// set the retVal to success
	retVal = _WO_ACTION_HANDLED;

end:

endNow:

	return retVal;
}
