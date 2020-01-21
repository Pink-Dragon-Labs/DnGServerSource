//
// mlmagis.hpp
//
// File that contains magistrate logic code.
//
// Author: Stephen Nichols
//

#ifndef _MLMAGIS_HPP_
#define _MLMAGIS_HPP_

#include "npc.hpp"

class Magistrate : public NPC
{
public:
	Magistrate();
	virtual ~Magistrate();

	virtual int normalLogic ( void );
};

#endif
