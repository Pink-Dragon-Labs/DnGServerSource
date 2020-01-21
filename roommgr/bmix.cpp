/*
	BMix class
	author: Stephen Nichols
*/

#include "bmix.hpp"
#include "roommgr.hpp"

BMix::BMix ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	skillLevel = _SKILL_LVL_FAMILIAR;
	type = _BMIX;
}

BMix::~BMix()
{
}

void BMix::copy ( WorldObjectBase *theBase )
{
	BMix *base = (BMix *)theBase;
	skillLevel = base->skill;
}

void BMix::writeSCIData ( FILE *file )
{
	fprintf ( file, "\t\t((aWhatObj addBase: BMix)\n" );
	fprintf ( file, "\t\t\tpSkill: %d,\n", self->skill );
	fprintf ( file, "\t\t\tpSkillLevel: %d\n", skillLevel );
	fprintf ( file, "\t\t)\n" );
}
