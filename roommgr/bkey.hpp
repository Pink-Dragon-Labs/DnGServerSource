/*
	BKey class
	author: Stephen Nichols
*/

#ifndef _BKEY_HPP_
#define _BKEY_HPP_

#include "wobjectbase.hpp"

class BKey : public WorldObjectBase
{
public:
	int lockValue, unlockValue, skeletonLock, skeletonUnlock;

	BKey ( WorldObject *obj );
	virtual ~BKey();

	void copy ( WorldObjectBase *base );

	virtual void buildPacket ( PackedData *packet, int override = 0 );
	virtual void writeSCIData ( FILE *file );
};

#endif
