//
// mlbandit.hpp
//
// File that contains Bandit logic code.
//
// Author: Zachary Kaiser
// 
//

#ifndef _MLBANDIT_HPP_
#define _MLBANDIT_HPP_

#include "mlsmarty.hpp"

class Bandit : public SmartMonster
{
	public:
	Bandit();
	virtual ~Bandit();

	virtual void init ( void );	

	virtual int spellAttack ( void );
	
	virtual int combatLogic ( void );
};
	
#endif
