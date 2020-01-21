//
// mlstorm.hpp
//
// File that contains logic code for storm bats.
//
// Author: Stephen Nichols
//

#ifndef _MLSTORM_HPP_
#define _MLSTORM_HPP_

#include "mlsmarty.hpp"

class StormBat : public SmartMonster
{
public:
	StormBat();
	virtual ~StormBat();

	virtual int spellAttack ( void );
};

#endif
