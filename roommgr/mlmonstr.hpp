//
// mlmonstr.hpp
//
// File that contains basic monster logic code.
//
// Author: Stephen Nichols
//

#ifndef _MLMONSTR_HPP_
#define _MLMONSTR_HPP_

#include "npc.hpp"

class Monster : public NPC
{
public:
	Monster();
	virtual ~Monster();
	virtual int normalLogic ( void );
};

#endif
