//
// mlseraph.hpp
//
// File that contains seraph logic code.
//
// Author: Stephen Nichols
//

#ifndef _MLSERAPH_HPP_
#define _MLSERAPH_HPP_

#include "mlsmarty.hpp"

class Seraph : public SmartMonster
{
public:
	Seraph();
	virtual ~Seraph();
	virtual void init ( void );
	virtual int spellAttack ( void );
};

#endif
