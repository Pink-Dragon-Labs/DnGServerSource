//
// mlgonan.hpp
//
// Control logic for Gonan. (Raid Boss)
//
// Author: Zachary Kaiser
//

#ifndef _MLGONAN_HPP_
#define _MLGONAN_HPP_

#include "mlaction.hpp"
#include "npc.hpp"

class Gonan: public NPC
{
public:
	Gonan();
	virtual ~Gonan();
	virtual void init ( void );
	virtual int normalLogic ( void );
	virtual int spellAttack ( void );
	virtual int combatLogic ( void );
};

#endif