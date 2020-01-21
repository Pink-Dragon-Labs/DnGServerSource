//
// mlswashbuckler.hpp
//
// File that contains Swashbuckler logic code.
//
// Author: Zachary Kaiser
// 
//

#ifndef _MLSWASHBUCKLER_HPP_
#define _MLSWASHBUCKLER_HPP_

#include "mlsmarty.hpp"

class Swashbuckler : public SmartMonster
{
	public:
    Swashbuckler();
	virtual ~Swashbuckler();

	virtual void init ( void );
	virtual int spellAttack ( void );
};

#endif
