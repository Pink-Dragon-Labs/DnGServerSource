//
// mlmarvin.hpp
//
// File that contains Marvin logic code.
//
// Author: Stephen Nichols
//

#ifndef _MLMARVIN_HPP_
#define _MLMARVIN_HPP_

#include "mlgood.hpp"
#include "mlmonstr.hpp"

class Marvin : public GoodMonster
{
public:
	Marvin();
	virtual ~Marvin();

	virtual int normalLogic ( void );
	virtual int spellAttack ( void );
};

#endif
