//
// mlfury.hpp
//
// File that contains logic code for furies.
//
// Author: Janus Anderson
//

#ifndef _MLFURY_HPP_
#define _MLFURY_HPP_

#include "mlsmarty.hpp"

class Fury : public SmartMonster
{
public:
	Fury();
	virtual ~Fury();
    	virtual void init ( void );
	virtual int spellAttack ( void );

protected:
	int myType;

};

#endif
