//
// ambushgroup
//
// This file contains the CAmbushGroup class.
//
// author: Stephen Nichols
//

#ifndef _AMBUSHGROUP_HPP_
#define _AMBUSHGROUP_HPP_

#include "../global/listobj.hpp"

// define external classes
class WorldObject;

//
// CAmbushGroup: This class represents a group of monsters that can be
// spawned to attack a player.
//
class CAmbushGroup : public ListObject
{
	// list of monster classes
	LinkedList m_MonsterList;

	// chance of appearing
	int m_nChance;

	// current level of the group
	int m_nLevel;

public:
	CAmbushGroup();
	virtual ~CAmbushGroup();

	// call to set the chance that this group will appear
	void SetChance ( int nChance );

	// call to get the current chance value
	int GetChance ( void );

	// call to get the current level value
	int GetLevel ( void );

	// call to add a new monster to this grouping
	void AddMonster ( char *pName );

	// call to spawn this group of monsters to attack the given player
	void SpawnAndAttack ( WorldObject *pCharacter );
};

#endif
