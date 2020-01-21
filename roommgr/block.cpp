/*
	BLockable class	
	author: Stephen Nichols
*/

#include "block.hpp"
#include "roommgr.hpp"

BLockable::BLockable ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BLOCK;
	isLocked ( 0 );
	autoLock = 0;
	skeletonLock = 0;
	skeletonUnlock = 0;
	lockValue = -1;
	unlockValue = -1;
}

BLockable::~BLockable()
{
}

void BLockable::copy ( WorldObjectBase *theBase )
{
	BLockable *base = (BLockable *)theBase;

	isLocked ( base->isLocked() );
	autoLock = base->autoLock;
	skeletonLock = base->skeletonLock;
	skeletonUnlock = base->skeletonUnlock;
	lockValue = base->lockValue;
	unlockValue = base->unlockValue;
}

void BLockable::buildPacket ( PackedData *packet, int override )
{
	packet->putByte ( isLocked() );
	packet->putByte ( autoLock );
	packet->putByte ( skeletonLock );
	packet->putByte ( skeletonUnlock );
	packet->putLong ( lockValue );
	packet->putLong ( unlockValue );
}

void BLockable::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t((aWhatObj addBase: BLockable)\n" );
	fprintf ( file, "\t\t\tpAutoLock: %d,\n", autoLock );
	fprintf ( file, "\t\t\tpLockValue: %d,\n", lockValue );
	fprintf ( file, "\t\t\tpUnlockValue: %d,\n", unlockValue );
	fprintf ( file, "\t\t\tpSkeletonLock: %d,\n", skeletonLock );
	fprintf ( file, "\t\t\tpSkeletonUnlock: %d,\n", skeletonUnlock );
	fprintf ( file, "\t\t\tpLocked: %d,\n", isLocked() );
	fprintf ( file, "\t\t)\n" );
}

// return 1 is the passed action is handled, 0 if not
int BLockable::handlesAction ( int action )
{
	switch ( action ) {
		case vBeLocked:
		case vBeUnlocked:
			return 1;
			break;
	}

	return 0;
}

// handle dispatching the actions
int BLockable::perform ( int action, va_list *args )
{
	int retVal = _WO_ACTION_ALLOWED;

	switch ( action ) {
		case vBeLocked: {
			WorldObject *object = va_arg ( *args, WorldObject * );
			WorldObject *key = va_arg ( *args, WorldObject * );
			retVal = beLocked ( object, key );
		}

		break;

		case vBeUnlocked: {
			WorldObject *object = va_arg ( *args, WorldObject * );
			WorldObject *key = va_arg ( *args, WorldObject * );
			retVal = beUnlocked ( object, key );
		}

		break;

		// intercept open requests and don't allow them if I am locked
		case vBeOpened: {
			if ( isLocked() )
				retVal = _ERR_MUST_UNLOCK;
		}

		break;
	}

	return retVal;
}

// handle being locked by a WorldObject
int BLockable::beLocked ( WorldObject *object, WorldObject *key )
{
	int retVal = _WO_ACTION_PROHIBITED;

	// skip out if I am already locked
	if ( isLocked() ) {
		retVal = _ERR_REDUNDANT;
		return retVal;
	}

	int doAction = 0;

	if ( lockValue == -1 || !object || !key ) {
		doAction = 1;
	} else {
		BKey *bkey = (BKey *)key->getBase ( _BKEY );

		if ( bkey && (bkey->lockValue == lockValue || (bkey->lockValue == -1 && skeletonLock <= bkey->skeletonLock)) )
			doAction = 1;
		else
			retVal = _ERR_WRONG_KEY;
	}

	if ( doAction ) {
		isLocked ( 1 );

		// set the retVal to success
		retVal = _WO_ACTION_HANDLED;
	}

	return retVal;
}

// handle being unlocked by a WorldObject
int BLockable::beUnlocked ( WorldObject *object, WorldObject *key )
{
	int retVal = _WO_ACTION_PROHIBITED;

	// skip out if I am already unlocked
	if ( !isLocked() ) {
		retVal = _ERR_REDUNDANT;
		return retVal;
	}

	int doAction = 0;

	if ( unlockValue == -1 || !object || !key ) {
		doAction = 1;
	} else {
		BKey *bkey = (BKey *)key->getBase ( _BKEY );			
	
		if ( bkey && (bkey->unlockValue == unlockValue || (bkey->unlockValue == -1 && skeletonUnlock <= bkey->skeletonUnlock)) )
			doAction = 1;
		else
			retVal = _ERR_WRONG_KEY;
	}

	if ( doAction ) {
		isLocked ( 0 );

		// set the retVal to success
		retVal = _WO_ACTION_HANDLED;
	}

	return retVal;
}
