//
// mllightwiz.hpp
//
// File that contains lightwiz logic code.
//
// Author: Janus Anderson
//

#ifndef _MLLIGHTWIZ_HPP_
#define _MLLIGHTWIZ_HPP_

#include "mlmonstr.hpp"

class LightWiz : public SmartMonster
{
public:
	LightWiz();
	virtual ~LightWiz();

	virtual void init ( void );
	virtual int spellAttack ( void );
};

#endif
