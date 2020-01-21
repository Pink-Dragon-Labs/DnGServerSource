/*
	BEntry class	
	author: Stephen Nichols
*/

#include "bentry.hpp"
#include "roommgr.hpp"

BEntry::BEntry ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BENTRY;
	room = 0;
	startingX = startingY = 0;
	endingX = endingY = 0;
	startingLoop = endingLoop = 0;
}

BEntry::~BEntry()
{
}

void BEntry::copy ( WorldObjectBase *theBase )
{
	BEntry *base = (BEntry *)theBase;
	room = base->room;
	startingX = base->startingX;
	startingY = base->startingY;
	endingX = base->endingX;
	endingY = base->endingY;
	startingLoop = base->startingLoop;
	endingLoop = base->endingLoop;
}

void BEntry::buildPacket ( PackedData *packet, int override )
{
	packet->putLong ( room );
}

void BEntry::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t(aWhatObj addBase: BEntry)\n" );
}
