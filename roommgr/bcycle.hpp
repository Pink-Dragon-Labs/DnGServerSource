/*
	BCycle class
	author: Stephen Nichols
*/

#ifndef _BCYCLE_HPP_
#define _BCYCLE_HPP_

#include "wobjectbase.hpp"

enum { 
	_CYCLE_FORWARD,
	_CYCLE_REVERSE,
	_CYCLE_OSCILLATE,
	_CYCLE_RANDOM
};

class BCycle : public WorldObjectBase
{
public:
	int cycleType, cycleSpeed;

	BCycle ( WorldObject *obj );
	virtual ~BCycle();

	void copy ( WorldObjectBase *base );

	virtual void buildPacket ( PackedData *packet, int override = 0 );
	virtual void writeSCIData ( FILE *file );
};

#endif
