//
// mlgonanpet.hpp
//
// Control logic for Volkor's Pets
//
// Author: Zachary Kaiser
//

#ifndef _MLGONANPET_HPP_
#define _MLGONANPET_HPP_

#include "mlaction.hpp"
#include "npc.hpp"

class Gonanpet: public NPC
{
public:
	Gonanpet();
	virtual ~Gonanpet();
	virtual void init ( void );
	virtual int normalLogic ( void );
	virtual int spellAttack ( void );
	virtual int combatLogic ( void );
};

#endif