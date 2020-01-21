//
// mlsmarty.hpp
//
// File that contains 'smart' monster logic code.
//
// Author: Stephen Nichols
//

#ifndef _MLSMARTY_HPP_
#define _MLSMARTY_HPP_

#include "mlaction.hpp"
#include "npc.hpp"

class SmartMonster : public NPC
{
	public:
	SmartMonster();
	virtual ~SmartMonster();
	virtual int normalLogic ( void );
	virtual void init ( void );
};

#endif
