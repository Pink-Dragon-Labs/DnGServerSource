//
// mlbmystic.hpp
//
// File that contains Bandit Mystics logic code.
//
// Author: Zachary Kaiser
// 
//

#ifndef _MLBMYSTIC_HPP_
#define _MLBMYSTIC_HPP_

#include "mlsmarty.hpp"

class Bmystic : public SmartMonster
{
	public:
	Bmystic();
	virtual ~Bmystic();

	virtual void init ( void );
	virtual int combatLogic ( void );
};

#endif
