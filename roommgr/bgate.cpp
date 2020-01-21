#include "roommgr.hpp"

BGate::BGate ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BGATE;
}

BGate::~BGate()
{
}

void BGate::copy ( WorldObjectBase *theBase )
{
}

void BGate::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t(aWhatObj addBase: BGatekeeper)\n" );
}
