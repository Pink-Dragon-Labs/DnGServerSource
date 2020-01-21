//
// faery.hpp
//
// File that contains faery logic code. 
//
// Author: Stephen Nichols
//

#ifndef _FAERY_HPP_
#define _FAERY_HPP_

#include "npc.hpp"

class Faery : public GoodMonster
{
public:
	Faery();
	virtual ~Faery();

	virtual int normalLogic ( void );
	virtual int spellAttack ( void );
};

#endif
