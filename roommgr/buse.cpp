/*
	BUse class
	author: Stephen Nichols
*/

#include "buse.hpp"
#include "roommgr.hpp"

BUse::BUse ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	uses = usesMax = 1;
	verb = 0;
	useCost = 25;
	spell = -1;
	type = _BUSE;
	theurgism = 0;
}

BUse::~BUse()
{
}

void BUse::copy ( WorldObjectBase *theBase )
{
	BUse *base = (BUse *)theBase;
	uses = base->uses;
	usesMax = base->usesMax;
	spell = base->spell;
	theurgism = base->theurgism;
}

void BUse::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t((aWhatObj addBase: BUse)\n" );
	fprintf ( file, "\t\t\tpVerb: %d,\n", verb );
	fprintf ( file, "\t\t\tpSpell: %d,\n", spell );
	fprintf ( file, "\t\t\tpTheurgism: %d,\n", theurgism );
	fprintf ( file, "\t\t)\n" );
}
