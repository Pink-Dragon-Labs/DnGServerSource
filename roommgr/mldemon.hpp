//
// mldemon.hpp
//
// File that contains demon logic code.
//
// Author: Janus Anderson
//

#ifndef _MLDAEMON_HPP_
#define _MLDAEMON_HPP_

#include "mlsmarty.hpp"

class Daemon : public SmartMonster
{
public:
	Daemon();
	virtual ~Daemon();
	virtual void init ( void );

	virtual int spellAttack ( void );
};

#endif
