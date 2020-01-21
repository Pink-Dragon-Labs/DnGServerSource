/*
	BKey class	
	author: Stephen Nichols
*/

#include "bcycle.hpp"
#include "roommgr.hpp"

BKey::BKey ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BKEY;
	lockValue = 0;
	unlockValue = 0;
	skeletonLock = 0;
	skeletonUnlock = 0;
}

BKey::~BKey()
{
}

void BKey::copy ( WorldObjectBase *theBase )
{
	BKey *base = (BKey *)theBase;

	lockValue = base->lockValue;
	unlockValue = base->unlockValue;
	skeletonLock = base->skeletonLock;
	skeletonUnlock = base->skeletonUnlock;
}

void BKey::buildPacket ( PackedData *packet, int override )
{
	packet->putLong ( lockValue );
	packet->putLong ( unlockValue );
	packet->putByte ( skeletonLock );
	packet->putByte ( skeletonUnlock );
}

void BKey::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t((aWhatObj addBase: BKey)\n" );
	fprintf ( file, "\t\t\tpLockValue: %d,\n", lockValue );
	fprintf ( file, "\t\t\tpUnlockValue: %d,\n", unlockValue );
	fprintf ( file, "\t\t\tpSkeletonLock: %d,\n", skeletonLock );
	fprintf ( file, "\t\t\tpSkeletonUnlock: %d,\n", skeletonUnlock );
	fprintf ( file, "\t\t)\n" );
}
