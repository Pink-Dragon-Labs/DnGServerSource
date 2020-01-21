/*
	BSit class
	author: Stephen Nichols
*/

#include "wobjectbase.hpp"
#include "wobject.hpp"

BSit::BSit ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BSIT;
	owner = NULL;
}

BSit::~BSit()
{
}

void BSit::copy ( WorldObjectBase *theBase )
{
	BSit *base = (BSit *)theBase;
	owner = base->owner;
}

void BSit::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t(aWhatObj addBase: BSit)\n" );
}

// action handling methods
int BSit::handlesAction ( int action )
{
	switch ( action ) {
		case vBeSatOn: 
		case vBeStoodUpOn:
			return 1;
	}

	return 0;
}

int BSit::perform ( int action, va_list *args )
{
	int retVal = _WO_ACTION_ALLOWED;

	switch ( action ) {
		case vBeSatOn: {
			WorldObject *object = va_arg ( *args, WorldObject * );
			retVal = beSatOn ( object );
		}

		break;

		case vBeStoodUpOn: {
			WorldObject *object = va_arg ( *args, WorldObject * );
			retVal = beStoodUpOn ( object );
		}

		break;
	}

	return retVal;
}

// handle being sat on by a WorldObject
int BSit::beSatOn ( WorldObject *object )
{
	int retVal = _WO_ACTION_PROHIBITED;

	// skip out if someone is already sitting on me
	if ( owner ) {
		retVal = _ERR_REDUNDANT;
		goto end;
	}

	owner = object;
	object->sittingOn = self;

	retVal = _WO_ACTION_HANDLED;

end:

	return retVal;
}

// handle being stood up on by a WorldObject
int BSit::beStoodUpOn ( WorldObject *object )
{
	int retVal = _WO_ACTION_PROHIBITED;

	// skip if the passed object is not who I know is sitting on me
	if ( object != owner ) {
		object->sittingOn = NULL;
		retVal = _ERR_BAD_SERVID;
		goto end;
	}

	owner = NULL;
	object->sittingOn = NULL;

	retVal = _WO_ACTION_HANDLED;

end:

	return retVal;
}
