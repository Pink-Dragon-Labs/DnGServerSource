//
// mlgood.hpp
//
// File that contains 'good' monster logic code.
//
// Author: Stephen Nichols
//

#ifndef _MLGOOD_HPP_
#define _MLGOOD_HPP_

#include "npc.hpp"

class GoodMonster : public NPC
{
public:
	GoodMonster();
	virtual ~GoodMonster();
	virtual int normalLogic ( void );
};

#endif
