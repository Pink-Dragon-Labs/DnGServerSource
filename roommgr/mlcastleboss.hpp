//
// mlcastleboss.hpp
//
// Control logic for Castle Bosses.
//
// Author: Zachary Kaiser
//

#ifndef _MLCASTLEBOSS_HPP_
#define _MLCASTLEBOSS_HPP_

#include "mlaction.hpp"
#include "npc.hpp"

class CastleBoss: public NPC
{
public:
	CastleBoss();
	virtual ~CastleBoss();
	virtual void init ( void );
	virtual int normalLogic ( void );
	virtual int spellAttack ( void );
	virtual int combatLogic ( void );
};

#endif