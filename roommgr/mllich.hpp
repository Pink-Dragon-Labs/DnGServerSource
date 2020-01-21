//
// mllich.hpp
//
// File that contains lich logic code.
//
// Author: Janus Anderson
//

#ifndef _MLLICH_HPP_
#define _MLLICH_HPP_

#include "mlsmarty.hpp"

class Lich : public SmartMonster
{
public:
	Lich();
	virtual ~Lich();

	virtual int spellAttack ( void );
};

#endif
