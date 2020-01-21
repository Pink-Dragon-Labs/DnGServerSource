//
// mlGodsewer.hpp
//
// Control logic for Sewer Gods.
//
// Author: Zachary Kaiser
//

#ifndef _MLGODSEWER_HPP_
#define _MLGODSEWER_HPP_

#include "mlsmarty.hpp"

class Godsewer: public SmartMonster
{
public:
	Godsewer();
	virtual ~Godsewer();

	virtual void init ( void );
	virtual int spellAttack ( void );
	virtual int combatLogic ( void );
};

#endif