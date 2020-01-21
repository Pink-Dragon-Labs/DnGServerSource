/*
	BNPC class	
	author: Stephen Nichols
*/

#include "bnpc.hpp"
#include "roommgr.hpp"

BNPC::BNPC ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BNPC;
	code = NULL;
	isExternal = 0;
}

BNPC::~BNPC()
{
	if ( code ) {
		free ( code );
		code = NULL;
	}
}

void BNPC::copy ( WorldObjectBase *theBase )
{
	BNPC *base = (BNPC *)theBase;
	setCode ( base->code );
	isExternal = base->isExternal;
}

void BNPC::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t(aWhatObj addBase: BNPC)\n" );
}

void BNPC::setCode ( char *txt )
{
	if ( code ) {
		free ( code );
		code = NULL;
	}

	if ( txt )
		code = strdup ( txt );
}
