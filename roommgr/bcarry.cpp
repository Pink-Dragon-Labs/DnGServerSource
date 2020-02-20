/*
	BCarryable class	
	author: Stephen Nichols
*/

#include "bcarry.hpp"
#include "roommgr.hpp"

BCarryable::BCarryable ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BCARRY;
	weight = 0;
	bulk = 0;
	show = 1;

	sLastOwner = NULL;

	setOwner ( NULL );
}

BCarryable::~BCarryable()
{
	if ( sLastOwner ) {
		delete sLastOwner;

		sLastOwner = NULL;
	}
}

void BCarryable::setOwner ( WorldObject *object )
{
	if ( _owner ) {
		if ( sLastOwner ) {
			delete sLastOwner;
		}

		sLastOwner = strdup( _owner->getName() );
	}

	_owner = object;
	self->ownerID = object? object->servID : -1;
}

void BCarryable::copy ( WorldObjectBase *theBase )
{
	BCarryable *base = (BCarryable *)theBase;
	weight = base->weight;
	bulk = base->bulk;
	show = base->show;
}

void BCarryable::buildPacket ( PackedData *packet, int override )
{
	packet->putByte ( show );
}

void BCarryable::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t((aWhatObj addBase: BCarryable)\n" );
	fprintf ( file, "\t\t\tpBulk: %d,\n", bulk );
	fprintf ( file, "\t\t\tpWeight: %d,\n", weight );
	fprintf ( file, "\t\t)\n" );
}

// return 1 if the passed action is handled, 0 if not
int BCarryable::handlesAction ( int action )
{
	switch ( action ) {
		case vBeTakenBy:
		case vBeDroppedBy:
		case vBePutIn:
		case vBeGivenTo:
			return 1;
			break;
	}

	return 0;
} 

// handle dispatching the actions
int BCarryable::perform ( int action, va_list *args )
{
	int retVal = _WO_ACTION_ALLOWED;

	switch ( action ) {
		case vBeTakenBy: {
			WorldObject *object = va_arg ( *args, WorldObject * );
			retVal = beTakenBy ( object );
		}

		break;

		case vBeDroppedBy: {
			WorldObject *object = va_arg ( *args, WorldObject * );
			retVal = beDroppedBy ( object );
		}

		break;

		case vBeGivenTo: {
			WorldObject *object = va_arg ( *args, WorldObject * );
			retVal = beGivenTo ( object );
		}

		break;

		case vBePutIn: {
			WorldObject *object = va_arg ( *args, WorldObject * );
			retVal = bePutIn ( object );
		}

		break;
	}

	return retVal;
}

// handle being taken by a WorldObject
int BCarryable::beTakenBy ( WorldObject *object )
{
	int retVal = _WO_ACTION_PROHIBITED;

	if ( !object || object->owns ( self, 1 ) || self->owns ( object ) )
		return retVal;

	// if self is owned by another object, ask that object to give
	// self to the object that would like to take it
	//
	if ( owner() ) {
		retVal = owner()->give ( self, object );
		return retVal;
	}

	// if self belongs to a room, let's go ahead and be taken 
	// by object
	//
	if ( self->room ) {
		RMRoom *room = self->room;
		BContainer *bContain = (BContainer *)object->getBase ( _BCONTAIN ); 

		int theWeight = weight;
		int theBulk = bulk;

		BContainer *bcontain = (BContainer *)self->getBase ( _BCONTAIN );

		if ( bcontain ) {
			theWeight = bcontain->weight;
			theBulk = bcontain->bulk;
		}

		if ( bContain->tooHeavy ( theWeight ) ) {
			retVal = _ERR_TOO_HEAVY;
			return retVal;
		}

		if ( bContain->tooBulky ( theBulk ) ) {
			retVal = _ERR_TOO_BULKY;
			return retVal;
		}

		// delete self from it's combatGroup
		if ( self->combatGroup )
			self->combatGroup->deleteObject ( self );

		// delete self from the room that it belongs to	
		room->delObject ( self, 0 );

		// set my owner to be the object
		setOwner ( object );

		// add self to the contents list of the object
		bContain->contents.add ( self );
		bContain->changeWeight ( theWeight );
		bContain->changeBulk ( theBulk );

		// save character if not npc
		if(!owner()->player->isNPC)
		{
			logInfo ( _LOG_ALWAYS, "%s taken by character. Writing character %s", object->getName(), owner()->getName() );
			owner()->writeCharacterData();
		}

		// set the retVal to success
		retVal = _WO_ACTION_HANDLED; 
	}

	return retVal;
}

// handle being dropped by another WorldObject
int BCarryable::beDroppedBy ( WorldObject *object, int override )
{
	int retVal = _WO_ACTION_PROHIBITED;

	if ( !object || !object->isWorldObject() ) 
		return retVal;

//	if ( !object )
//		return retVal;

	// skip out if object does not belong to a room
	RMRoom *room = object->getRoom();

	if ( !override ) {
		// skip out if self is not owned by object
		if ( !self->isOwnedBy ( object, 1 ) )
			return retVal;

		// skip out if we are currently being worn 
		if ( self->objectWornOn != -1 || self->objectWieldedOn != -1 ) {
			retVal = _ERR_MUST_REMOVE;
			return retVal;
		}
	}

	if ( owner()->combatGroup ) 
		owner()->combatGroup->addObject ( self );

	if ( !override )
		self->makeVisible ( 1 );

	int theWeight = weight, theBulk = bulk;

	BContainer *bcontain = (BContainer *)self->getBase ( _BCONTAIN );

	if ( bcontain ) {
		theWeight = bcontain->weight;
		theBulk = bcontain->bulk;
	}

	// delete self from the contents list of it's owner
	BContainer *bContain = (BContainer *)owner()->getBase ( _BCONTAIN );

	if ( bContain ) {
		bContain->contents.del ( self );
		bContain->changeWeight ( -theWeight );
		bContain->changeBulk ( -theBulk );
	}
	
	// set my owner to NULL
	setOwner ( NULL );

	// add self to the room
	if ( room ) {
		room->addObject ( self, 0 ); 
	}

	// set self's position
	self->x = object->x;
	self->y = object->y;

	if ( !override ) {
		// if I'm a container, let my contents be known
		bContain = (BContainer *)self->getBase ( _BCONTAIN );

		if ( bContain ) 
			bContain->makeVisible ( 1 );
	}

	// set the retVal to success
	retVal = _WO_ACTION_HANDLED; 
	
	return retVal;
}

// handle being given to another WorldObject
int BCarryable::beGivenTo ( WorldObject *object, int override )
{
	int retVal = _WO_ACTION_PROHIBITED;

	if ( !object || !object->isWorldObject() ) 
		return retVal;

	// skip out if we giving to ourself
	if ( object == self )
		return retVal;

	// skip out if we are not owned by anyone
	if ( !override && !owner() )
		return retVal;

	// skip out if we are already directly owned by object
	if ( self->isOwnedBy ( object, 1 ) )
		return retVal;

	// skip out if we own the object we are being given to
	if ( self->owns ( object ) )
		return retVal;

	// skip out if who we are being given to is a closed openable
	BOpenable *bopen = (BOpenable *)object->getBase ( _BOPEN );

	if ( !override && bopen && !bopen->isOpen() ) {
		retVal = _ERR_MUST_OPEN;
		return retVal;
	}

	BContainer *bContain = (BContainer *)object->getBase ( _BCONTAIN );
	BContainer *bcontain = (BContainer *)self->getBase ( _BCONTAIN );

	if ( !bContain )
		return retVal;

	int theWeight = weight, theBulk = bulk;

	if ( bcontain ) {
		theWeight = bcontain->weight;
		theBulk = bcontain->bulk;
	}

	if ( (override < 1) && bContain->tooHeavy ( theWeight ) ) {
		retVal = _ERR_TOO_HEAVY;
		return retVal;
	}

	if ( (override < 1) && bContain->tooBulky ( theBulk ) ) {
		retVal = _ERR_TOO_BULKY;
		return retVal;
	}

	if ( self->objectWornOn != -1 ) {
		retVal = _ERR_MUST_REMOVE;
		return retVal;
	}

	self->makeVisible ( 1 );

	// mark the current building as changed
	self->markBuildingAsChanged();

	RMRoom *room = self->room;

	if ( room )
		room->delObject ( self, 0 );

	// delete self from it's combatGroup
	if ( self->combatGroup )
		self->combatGroup->deleteObject ( self );

	WorldObject *oldOwner = owner();

	// set my owner
	setOwner ( object );

	// add self to object
	bContain->contents.add ( self );
	bContain->changeWeight ( theWeight );
	bContain->changeBulk ( theBulk );

	WorldObject *baseOwner = object->getBaseOwner();
	BCharacter *character = (BCharacter *)baseOwner->getBase ( _BCHARACTER );
	if( reinterpret_cast< int>( character ) == 0x21 ) { logInfo( _LOG_ALWAYS, "%s:%d - BCharacter value corrupted", __FILE__, __LINE__ ); }

	if ( character ) 
		self->makeVisible ( 0 );

	// remove self from it's owner
	if ( oldOwner ) {
		bContain = (BContainer *)oldOwner->getBase ( _BCONTAIN );
		bContain->contents.del ( self );
		bContain->changeWeight ( -theWeight );
		bContain->changeBulk ( -theBulk );
	}

	// mark the current building as changed
	self->markBuildingAsChanged();

	// set retVal to success
	retVal = _WO_ACTION_HANDLED;
	
	return retVal;
}

// handle being put into another WorldObject
int BCarryable::bePutIn ( WorldObject *object, int override )
{
	return beGivenTo ( object, override );
}
