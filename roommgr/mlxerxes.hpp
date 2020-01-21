//
// mlxerxes.hpp
//
// Control logic for Xerxes.
//
// Author: Zachary Kaiser
//

#ifndef _MLXERXES_HPP_
#define _MLXERXES_HPP_

#include "mlaction.hpp"
#include "npc.hpp"

class Xerxes: public NPC
{
public:
	Xerxes();
	virtual ~Xerxes();
	virtual void init ( void );
	virtual int normalLogic ( void );
	virtual int spellAttack ( void );
	virtual int combatLogic ( void );
};

#endif