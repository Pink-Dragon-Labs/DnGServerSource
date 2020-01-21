/*
	BGate class
	author: Stephen Nichols
*/

#ifndef _BGATE_HPP_
#define _BGATE_HPP_

#include "wobjectbase.hpp"
#include "wobject.hpp"

class BGate : public WorldObjectBase
{
public:
	BGate ( WorldObject *obj );
	virtual ~BGate();

	void copy ( WorldObjectBase *base );
	virtual void writeSCIData ( FILE *file );
};

#endif
