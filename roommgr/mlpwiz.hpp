//
// mlpwiz.hpp
//
// File that contains powerwiz logic code.
//
// Author: Kerry Sergent
//

#ifndef _MLPOWERWIZ_HPP_
#define _MLPOWERWIZ_HPP_

#include "mlmonstr.hpp"

class PowerWiz : public SmartMonster
{
public:
	PowerWiz();
	virtual ~PowerWiz();

	virtual void init ( void );
	virtual int spellAttack ( void );
};

#endif
