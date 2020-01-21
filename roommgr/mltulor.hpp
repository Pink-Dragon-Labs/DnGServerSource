//
// mltulor.hpp
//
// File that contains logic code for tulor.
//
// Author: Stephen Nichols
//

#ifndef _MLTULOR_HPP_
#define _MLTULOR_HPP_

#include "mlsmarty.hpp"

class Tulor : public SmartMonster
{
public:
	Tulor();
	virtual ~Tulor();

	virtual int spellAttack ( void );

	virtual int combatLogic ( void );
};

#endif
