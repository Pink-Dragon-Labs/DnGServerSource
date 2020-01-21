/*
	BLockable class
	author: Stephen Nichols
*/

#ifndef _BLOCK_HPP_
#define _BLOCK_HPP_

#include "wobjectbase.hpp"
#include "wobject.hpp"

class BLockable : public WorldObjectBase
{
public:
	int lockValue, unlockValue, skeletonLock, skeletonUnlock, autoLock;

	BLockable ( WorldObject *obj );
	virtual ~BLockable();

	// access members
	inline int isLocked ( void ) { return (self->physicalState & _STATE_LOCKED); };

	inline void isLocked ( int value ) {
		if ( value ) 
			self->physicalState |= _STATE_LOCKED;
		else
			self->physicalState &= ~_STATE_LOCKED;
	};

	void copy ( WorldObjectBase *base );

	virtual void buildPacket ( PackedData *packet, int override = 0 );
	virtual void writeSCIData ( FILE *file );

	// action handling methods
	virtual int handlesAction ( int action );
	virtual int perform ( int action, va_list *args );

	// handle being locked
	virtual int beLocked ( WorldObject *object, WorldObject *key = NULL );

	// handle being unlocked 
	virtual int beUnlocked ( WorldObject *object, WorldObject *key = NULL );
};

#endif
