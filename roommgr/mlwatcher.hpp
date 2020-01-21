//
// mlwatcher.hpp
//
// This file contains the Watcher AI code.
//
// Author: Stephen Nichols
//

#ifndef _MLWATCHER_HPP_
#define _MLWATCHER_HPP_

#include "npc.hpp"

class Watcher : public NPC
{
protected:
	// this is the servID of our watch target
	int m_nTargetServID;

	// this is the time that this watcher started watching its current target
	int m_nTargetAcquireTime;

	// this is the number of seconds that the current target is to be watched
	int m_nTargetWatchTime;

	// this is the current state of this watcher
	int m_nWatchState;

	//
	// Call this to choose another random player as a watch target.
	//
	WorldObject *chooseTarget ( void );

public:
	Watcher();
	virtual ~Watcher();

	//
	// This is called by the system to allow this NPC to make out of combat
	// choices.
	//
	virtual int normalLogic ( void );

	//
	// This is called by the system to allow this NPC to make choices about 
	// combat actions.
	//
	virtual int combatLogic ( void );
};

#endif
