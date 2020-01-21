/*
	BConsumable class	
	author: Stephen Nichols
*/

#include <algorithm>
#include "bconsume.hpp"
#include "roommgr.hpp"

using namespace std;
BConsumable::BConsumable ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BCONSUME;
	state = _STATE_LIQUID;
}

BConsumable::~BConsumable()
{
}

void BConsumable::copy ( WorldObjectBase *theBase )
{
	BConsumable *base = (BConsumable *)theBase;
	state = base->state;
}

void BConsumable::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t((aWhatObj addBase: BConsume)\n" );
	fprintf ( file, "\t\t\tpState: %s, \n", (state == _STATE_LIQUID)? "_STATE_LIQUID" : "_STATE_SOLID" );
	fprintf ( file, "\t\t)\n" );
}

// return 1 if the passed action is handled, 0 if not
int BConsumable::handlesAction ( int action )
{
	switch ( action ) {
		case vBeConsumed:
			return 1;
			break;
	}

	return 0;
} 

// handle dispatching the actions
int BConsumable::perform ( int action, va_list *args )
{
	int retVal = _WO_ACTION_ALLOWED;

	switch ( action ) {
		case vBeConsumed: {
			WorldObject *object = va_arg ( *args, WorldObject * );
			retVal = beConsumed ( object );
		}

		break;
	}

	return retVal;
}

// handle being consumed by a WorldObject
int BConsumable::beConsumed ( WorldObject *object )
{
	int retVal = _WO_ACTION_PROHIBITED;

	// skip out if I am not owned by the object trying to consume me
	if ( !self->isOwnedBy ( object ) ) {
		goto end;
	}

	if ( state == _STATE_LIQUID ) {
		if ( object->thirst < 0 ) {
			object->thirst = 0;
			roomMgr->sendPlayerText ( object->player, "You are no longer so parched.\n" );
		}

		object->thirst += 14;

		if ( object->hunger >= _MAX_THIRST ) {
			roomMgr->sendPlayerText ( object->player, "You are now full of drink.\n" );
		}
	} else {
		if ( object->hunger < 0 ) {
			object->hunger = 0;
			roomMgr->sendPlayerText ( object->player, "You are no longer so terribly hungry.\n" );
		}

		object->hunger += 21;

		if ( object->hunger >= _MAX_HUNGER ) {
			roomMgr->sendPlayerText ( object->player, "You are now full of food.\n" );
		}
	}

	object->thirst = std::min(object->thirst,_MAX_THIRST);
	object->hunger = std::min(object->hunger,_MAX_HUNGER);

	// set the retVal to success
	retVal = _WO_ACTION_HANDLED;

end:

	return retVal;
}

