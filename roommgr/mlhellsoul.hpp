//
// mlhellsoul.hpp
//
// This file contains the HellSoul AI code.
//
// Author: Stephen Nichols
//

#ifndef _MLHELLSOUL_HPP_
#define _MLHELLSOUL_HPP_

#include "npc.hpp"

class HellSoul : public NPC
{
public:
	HellSoul();
	virtual ~HellSoul();

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
