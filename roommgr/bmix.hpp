/*
	BMix class
	author: Stephen Nichols
*/

#ifndef _BMIX_HPP_
#define _BMIX_HPP_

#include "wobjectbase.hpp"

class BMix : public WorldObjectBase
{
public:
	int skill, skillLevel;

	BMix ( WorldObject *obj );
	virtual ~BMix();

	void copy ( WorldObjectBase *base );
	void writeSCIData ( FILE *file );
};

#endif
