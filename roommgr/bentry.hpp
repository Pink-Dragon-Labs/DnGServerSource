/*
	BEntry class
	author: Stephen Nichols
*/

#ifndef _BENTRY_HPP_
#define _BENTRY_HPP_

#include "wobjectbase.hpp"

class BEntry : public WorldObjectBase
{
public:
	int room;
	int startingLoop, endingLoop;
	int startingX, startingY;
	int endingX, endingY;

	BEntry ( WorldObject *obj );
	virtual ~BEntry();

	void copy ( WorldObjectBase *base );

	virtual void buildPacket ( PackedData *packet, int override = 0 );
	virtual void writeSCIData ( FILE *file );
};

#endif
