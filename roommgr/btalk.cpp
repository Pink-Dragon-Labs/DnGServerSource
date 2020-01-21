/*
	BTalk class
	author: Stephen Nichols
*/

#include "roommgr.hpp"

BTalk::BTalk ( WorldObject *obj )
{
	type = _BTALK;
	file = NULL;
	talkTreeID = 0;
}

BTalk::~BTalk()
{
	setFile ( NULL );
}

void BTalk::copy ( WorldObjectBase *theBase )
{
	BTalk *base = (BTalk *)theBase;
	setFile ( base->file );
	talkTreeID = base->talkTreeID;
}

void BTalk::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t(aWhatObj addBase: BTalk)\n" );
}

void BTalk::setFile ( char *str )
{
	if ( file ) {
		free ( file );
		file = NULL;
	}

	if ( str )
		file = strdup ( str );
}
