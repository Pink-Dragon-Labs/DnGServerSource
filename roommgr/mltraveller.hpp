//
// mltraveller.hpp
//
// File that contains logic code for Travellers
//
// Author: Zachary Kaiser
//

#ifndef _MLTRAVELLER_HPP_
#define _MLTRAVELLER_HPP_

#include "mlaction.hpp"
#include "npc.hpp"

class Traveller : public NPC
{
public:
	Traveller();
	virtual ~Traveller();
	virtual int normalLogic ( void );
	virtual void init ( void );
};

#endif