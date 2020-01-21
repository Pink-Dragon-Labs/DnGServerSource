//
// bountylist.cpp
//
// list of people who have bounties
//
// author: Stephen Nichols
//

#include "roommgr.hpp"

BountyList gBountyList;

Bounty::Bounty()
{
	name = NULL;
	criminal = 0;
}

Bounty::~Bounty()
{
	if ( name ) {
		free ( name );
		name = NULL;
	}
}

BountyList::BountyList()
{
}

BountyList::~BountyList()
{
}

// add (or update) an entry in the bounty list
void BountyList::add ( WorldObject *obj, CrimeData *crime )
{
}

// delete an entry from the bounty list
void BountyList::del ( WorldObject *obj )
{
}
