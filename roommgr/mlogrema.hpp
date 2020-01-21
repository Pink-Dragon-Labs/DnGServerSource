//
// mlogremage.hpp
//
// File that contains ogre mage logic code.
//
// Author: Janus Anderson
//

#ifndef _MLOGREMAGE_HPP_
#define _MLOGREMAGE_HPP_

#include "mlsmarty.hpp"

class OgreMage : public SmartMonster
{
public:
	OgreMage();
	virtual ~OgreMage();
	virtual void init ( void );

	virtual int spellAttack ( void );
};

#endif
