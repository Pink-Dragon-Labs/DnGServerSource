//
// mlguardian.hpp
//
// File that contains minotaur guard logic code.
// They wander their zone and 
//
// Author: Michael Nicolella
//

#ifndef _MLMINOGUARD_HPP_
#define _MLMINOGUARD_HPP_

#include "mlsmarty.hpp"

class MinotaurGuard : public SmartMonster
{
public:
	MinotaurGuard();
	virtual ~MinotaurGuard();

    virtual void init ( void );
	virtual int normalLogic( void );
	virtual int spellAttack ( void );
};

#endif
