/*
	BUse class
	author: Stephen Nichols
*/

#ifndef _BUSE_HPP_
#define _BUSE_HPP_

#include "wobjectbase.hpp"

class BUse : public WorldObjectBase
{
public:
	int uses, usesMax, verb, useCost, spell, theurgism;

	BUse ( WorldObject *obj );
	virtual ~BUse();

	void copy ( WorldObjectBase *base );
	void writeSCIData ( FILE *file );
};

#endif
