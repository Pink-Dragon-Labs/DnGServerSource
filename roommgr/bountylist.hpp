//
// bountylist.hpp
//
// list of people who have bounties
//
// author: Stephen Nichols
//

#ifndef _BOUNTYLIST_HPP_
#define _BOUNTYLIST_HPP_

#include "../global/system.hpp"

class WorldObject;
class CrimeData;

class Bounty : public ListObject
{
public:
	Bounty();
	virtual ~Bounty();

	// properties
	char *name;
	int criminal;
};

class BountyList : public ListObject
{
public:
	BountyList();
	virtual ~BountyList();

	// add (or update) an entry in the bounty list
	void add ( WorldObject *obj, CrimeData *crime );

	// delete an entry from the bounty list
	void del ( WorldObject *obj );

	BinaryTree bountyTree;
	LinkedList bountyList;
};

extern BountyList gBountyList;

#endif
