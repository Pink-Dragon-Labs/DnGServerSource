//
// mlpaladin.hpp
//
// File that contains faery logic code.
//
// Author: Stephen Nichols
//

#ifndef _MLPALADIN_HPP_
#define _MLPALADIN_HPP_

#include "mlmonstr.hpp"

class Paladin : public SmartMonster
{
public:
	Paladin();
	virtual ~Paladin();

    virtual void init ( void );
	virtual int spellAttack ( void );
};

#endif
