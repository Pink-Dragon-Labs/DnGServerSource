//
// mldragon.hpp
//
// Control logic for Dragons.
//
// Author: Zachary Kaiser
//

#ifndef _MLDRAGON_HPP_
#define _MLDRAGON_HPP_

#include "mlaction.hpp"
#include "npc.hpp"

class Dragon: public NPC
{
public:
	Dragon();
	virtual ~Dragon();
	virtual void init ( void );
	virtual int normalLogic ( void );
	virtual int spellAttack ( void );
	virtual int combatLogic ( void );
};

#endif