//
// mltroll.hpp
//
// File that contains troll logic code.
//
// Author: Janus Anderson
//

#ifndef _MLTROLL_HPP_
#define _MLTROLL_HPP_

#include "mlsmarty.hpp"

class TrollSpellcaster : public SmartMonster
{
public:
	TrollSpellcaster();
	virtual ~TrollSpellcaster();
	virtual void init ( void );
	virtual int spellAttack ( void );
};

#endif
