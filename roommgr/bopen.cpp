/*
	BOpenable class	
	author: Stephen Nichols
*/

#include "bopen.hpp"
#include "roommgr.hpp"

BOpenable::BOpenable ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BOPEN;
	isOpen ( 0 );
}

BOpenable::~BOpenable()
{
}

void BOpenable::copy ( WorldObjectBase *theBase )
{
	BOpenable *base = (BOpenable *)theBase;
	isOpen ( base->isOpen() );
}

void BOpenable::buildPacket ( PackedData *packet, int override )
{
	packet->putByte ( isOpen()  );
}

void BOpenable::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t(aWhatObj addBase: BOpenable)\n" );
}

// return 1 if the passed action is handled, 0 if not
int BOpenable::handlesAction ( int action )
{
	switch ( action ) {
		case vBeOpened:
		case vBeClosed:
			return 1;
			break;
	}

	return 0;
} 

// handle dispatching the actions
int BOpenable::perform ( int action, va_list *args )
{
	int retVal = _WO_ACTION_ALLOWED;

	switch ( action ) {
		case vBeOpened: {
			WorldObject *object = va_arg ( *args, WorldObject * );
			retVal = beOpened ( object );
		}

		break;

		case vBeClosed: {
			WorldObject *object = va_arg ( *args, WorldObject * );
			retVal = beClosed ( object );
		}

		break;
	}

	return retVal;
}

// handle being opened by a WorldObject
int BOpenable::beOpened ( WorldObject *object )
{
	int retVal = _WO_ACTION_PROHIBITED;

	// skip out if I am already open
	if ( isOpen() ) {
		retVal = _ERR_REDUNDANT;
		return retVal;
	}

	isOpen ( 1 );

	BContainer *bcontain = (BContainer *)self->getBase ( _BCONTAIN );

	if ( bcontain ) 
		bcontain->makeVisible ( 1 );

	// set the retVal to success
	retVal = _WO_ACTION_HANDLED;

	return retVal;
}

// handle being closed by a WorldObject
int BOpenable::beClosed ( WorldObject *object )
{
	int retVal = _WO_ACTION_PROHIBITED;

	// skip out if I am already closed
	if ( !isOpen() ) {
		retVal = _ERR_REDUNDANT;
		return retVal;
	}

	isOpen ( 0 );

	// handle locking self if self is autoLocking
	BLockable *base = (BLockable *)self->getBase ( _BLOCK );

	if ( base && base->autoLock ) 
		base->beLocked ( NULL );

	BContainer *bcontain = (BContainer *)self->getBase ( _BCONTAIN );

	if ( bcontain ) 
		bcontain->makeVisible ( 0 );

	// set the retVal to success
	retVal = _WO_ACTION_HANDLED;

	return retVal;
}
