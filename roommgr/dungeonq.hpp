//
// dungeonq
//
// This file contains the dungeon line management class.
//
// author: Stephen Nichols
//

#ifndef _DUNGEONQ_HPP_
#define _DUNGEONQ_HPP_

#include "list.hpp"

//
// DungeonQueue: This object contains a list of RMPlayers that are waiting to
// enter the associated dungeon.
// 
class DungeonQueue : public LinkedList
{
public:
	DungeonQueue();
	virtual ~DungeonQueue();
};

#endif
