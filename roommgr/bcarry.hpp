/*
	BCarryable class
	author: Stephen Nichols
*/

#ifndef _BCARRY_HPP_
#define _BCARRY_HPP_

#include "wobjectbase.hpp"

class BCarryable : public WorldObjectBase
{
	WorldObject *_owner;

public:
	int weight, bulk, show;

	BCarryable ( WorldObject *obj );
	virtual ~BCarryable();

	// owner manipulations
	void setOwner ( WorldObject *object );
	inline WorldObject *owner ( void ) { return _owner; };

	void copy ( WorldObjectBase *base );

	virtual void buildPacket ( PackedData *packet, int override = 0 );
	virtual void writeSCIData ( FILE *file );

	// action handling methods
	virtual int handlesAction ( int action );
	virtual int perform ( int action, va_list *args );

	// handle being taken by a WorldObject
	int beTakenBy ( WorldObject *object );
	
	// handle being dropped by a WorldObject
	int beDroppedBy ( WorldObject *object, int override = 0 );

	// handle being given to a WorldObject
	int beGivenTo ( WorldObject *object, int override = 0 );

	// handle being put in a WorldObject by a WorldObject
	int bePutIn ( WorldObject *container, int override = 0 );

	char*	sLastOwner;
};

#endif
