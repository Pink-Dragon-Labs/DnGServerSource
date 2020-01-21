/*
	BSit class
	author: Stephen Nichols
*/

#ifndef _BSIT_HPP_
#define _BSIT_HPP_

#include "wobjectbase.hpp"
#include "wobject.hpp"

class BSit : public WorldObjectBase
{
public:
	BSit ( WorldObject *obj );
	virtual ~BSit();

	void copy ( WorldObjectBase *base );

	virtual void writeSCIData ( FILE *file );

	// action handling methods
	virtual int handlesAction ( int action );
	virtual int perform ( int action, va_list *args );

	// handle being sat on by a WorldObject
	int beSatOn ( WorldObject *object );

	// handle being stood up on by a WorldObject
	int beStoodUpOn ( WorldObject *object );

	// object who is sitting on me
	WorldObject *owner;
};

#endif
