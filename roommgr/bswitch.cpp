/*
	BSwitch class	
	author: Stephen Nichols
*/

#include "bswitch.hpp"
#include "roommgr.hpp"

BSwitch::BSwitch ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BSWITCH;
	isOpen ( 0 );
	enabled = TRUE;
}

BSwitch::~BSwitch()
{
}

void BSwitch::copy ( WorldObjectBase *theBase )
{
	BSwitch *base = (BSwitch *)theBase;
	isOpen ( base->isOpen() );
	enabled = base->enabled;
}

void BSwitch::buildPacket ( PackedData *packet, int override )
{
	packet->putByte ( isOpen()  );
}

void BSwitch::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t(aWhatObj addBase: BSwitch)\n" );
}

// return 1 if the passed action is handled, 0 if not
int BSwitch::handlesAction ( int action )
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
int BSwitch::perform ( int action, va_list *args )
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
int BSwitch::beOpened ( WorldObject *object )
{
	int retVal = _WO_ACTION_PROHIBITED;

	// skip out if I am already open
	if ( isOpen() ) {
		retVal = _ERR_REDUNDANT;
		goto end;
	}

	// skip out if not enabled
	if ( !enabled ) {
		retVal = _WO_ACTION_PROHIBITED;
		goto end;
	}

	isOpen ( 1 );

	// set the retVal to success
	retVal = _WO_ACTION_HANDLED;

end:

	return retVal;
}

// handle being closed by a WorldObject
int BSwitch::beClosed ( WorldObject *object )
{
	int retVal = _WO_ACTION_PROHIBITED;

	// skip out if I am already closed
	if ( !isOpen() ) {
		retVal = _ERR_REDUNDANT;
		goto end;
	}

	// skip out if not enabled
	if ( !enabled ) {
		retVal = _WO_ACTION_PROHIBITED;
		goto end;
	}

	isOpen ( 0 );

	// set the retVal to success
	retVal = _WO_ACTION_HANDLED;

end:

	return retVal;
}
