/*
	BDye class	
	author: Stephen Nichols
*/

#include "bdye.hpp"
#include "roommgr.hpp"

BDye::BDye ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BDYE;
	nHairDye = FALSE;
}

BDye::~BDye()
{
}

void BDye::copy ( WorldObjectBase *theBase )
{
	BDye* base = (BDye*) theBase;

	if ( base ) {
		nHairDye = base->nHairDye;
	}
}

void BDye::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t((aWhatObj addBase: BDye)\n" );
	fprintf ( file, "\t\t\tpHairDye: %d,\n", nHairDye );
	fprintf ( file, "\t\t)\n" );
}
