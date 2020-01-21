/*
	BSwitch class
	author: Stephen Nichols
*/

#ifndef _BSWITCH_HPP_
#define _BSWITCH_HPP_

#include "wobjectbase.hpp"
#include "wobject.hpp"

class BSwitch : public WorldObjectBase
{
public:
	BSwitch ( WorldObject *obj );
	virtual ~BSwitch();

	// access members
	inline int isOpen ( void ) { return !(self->physicalState & _STATE_CLOSED); };

	inline void isOpen ( int value ) { 
		if ( value )
			self->physicalState &= ~_STATE_CLOSED;
		else
			self->physicalState |= _STATE_CLOSED;
	};

	void copy ( WorldObjectBase *base );

	virtual void buildPacket ( PackedData *packet, int override = 0 );
	virtual void writeSCIData ( FILE *file );

	// action handling methods
	virtual int handlesAction ( int action );
	virtual int perform ( int action, va_list *args );

	// handle being opened by a WorldObject
	int beOpened ( WorldObject *object );

	// handle being closed by a WorldObject
	int beClosed ( WorldObject *object );

	// basic properties
	int enabled;
};

#endif
