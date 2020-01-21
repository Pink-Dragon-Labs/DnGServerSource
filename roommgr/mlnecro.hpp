//
// mlnecro.hpp
//
// File that contains undead necromancer / entombed one logic code.
//
// Author: Janus Anderson
//

#ifndef _MLNECRO_HPP_
#define _MLNECRO_HPP_

#include "mlsmarty.hpp"

class Necro : public SmartMonster
{
public:
	Necro();
	virtual ~Necro();
	virtual void init ( void );
	virtual int spellAttack ( void );
};

#endif
