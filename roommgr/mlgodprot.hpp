//
// mlGodprot.hpp
//
// Control logic for Sewer God Protectors.
//
// Author: Zachary Kaiser
//

#ifndef _MLGODPROT_HPP_
#define _MLGODPROT_HPP_

#include "mlsmarty.hpp"

class Godprot: public SmartMonster
{
public:
	Godprot();
	virtual ~Godprot();

	virtual void init ( void );
	virtual int spellAttack ( void );
	virtual int combatLogic ( void );
};

#endif