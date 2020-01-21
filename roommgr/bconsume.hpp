/*
	BConsumable class
	author: Stephen Nichols
*/

#ifndef _BCONSUME_HPP_
#define _BCONSUME_HPP_

#include "wobjectbase.hpp"
#include "wobject.hpp"

class BConsumable : public WorldObjectBase
{
public:
	int state;

	BConsumable ( WorldObject *obj );
	virtual ~BConsumable();

	void copy ( WorldObjectBase *base );

	virtual void writeSCIData ( FILE *file );

	// action handling methods
	virtual int handlesAction ( int action );
	virtual int perform ( int action, va_list *args );

	// handle being consumed by a WorldObject
	int beConsumed ( WorldObject *object );
};

#endif
