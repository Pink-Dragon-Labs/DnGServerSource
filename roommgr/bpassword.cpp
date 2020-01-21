/*
	BPassword class
	author: Stephen Nichols
*/

#include "roommgr.hpp"

BPassword::BPassword ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	type = _BPASSWORD;
	owner = -1;
	sprintf ( sizeof ( password ), password, "password" );
}

BPassword::~BPassword()
{
}

void BPassword::copy ( WorldObjectBase *theBase )
{
	BPassword *base = (BPassword *)theBase;

	owner = base->owner;
	memcpy ( password, base->password, sizeof ( base->password ) );
}

void BPassword::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t(aWhatObj addBase: BPassword)\n" );
}
