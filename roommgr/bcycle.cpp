/*
	BCycle class	
	author: Stephen Nichols
*/

#include "bcycle.hpp"
#include "roommgr.hpp"

BCycle::BCycle ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BCYCLE;
	cycleSpeed = 6;
	cycleType = _CYCLE_FORWARD;
}

BCycle::~BCycle()
{
}

void BCycle::copy ( WorldObjectBase *theBase )
{
	BCycle *base = (BCycle *)theBase;

	cycleSpeed = base->cycleSpeed;
	cycleType = base->cycleType;
}

void BCycle::buildPacket ( PackedData *packet, int override )
{
	packet->putByte ( cycleType );
	packet->putByte ( cycleSpeed );
}

void BCycle::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t((aWhatObj addBase: BCycle)\n" );
	fprintf ( file, "\t\t\tpCycleSpeed: %d,\n", cycleSpeed );
	fprintf ( file, "\t\t\tpCycleType: %d,\n", cycleType );
	fprintf ( file, "\t\t\tdoit:,\n" );
	fprintf ( file, "\t\t)\n" );
}
