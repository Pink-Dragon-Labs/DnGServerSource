//
// mlcleric.hpp
//
// File that contains cleric logic code.
//
// Author: Janus Anderson
//

#ifndef _MLCLERIC_HPP_
#define _MLCLERIC_HPP_

#include "mlmonstr.hpp"

class Cleric : public SmartMonster
{
public:
	Cleric();
	virtual ~Cleric();

	virtual void init ( void );
	virtual int spellAttack ( void );
};

#endif
