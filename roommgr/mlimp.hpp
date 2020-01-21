//
// mlimp.hpp
//
// File that contains logic code for imps
//
// Author: Janus Anderson
//

#ifndef _MLIMP_HPP_
#define _MLIMP_HPP_

#include "mlsmarty.hpp"

class Imp : public SmartMonster
{
public:
	Imp();
	virtual ~Imp();

	virtual void init ( void );
	virtual int spellAttack ( void );
};

#endif
