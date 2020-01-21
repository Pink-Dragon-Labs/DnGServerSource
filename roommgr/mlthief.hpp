//
// mlthief.hpp
//
// File that contains thief logic code.
//
// Author: Janus Anderson
//

#ifndef _MLTHIEF_HPP_
#define _MLTHIEF_HPP_

#include "mlsmarty.hpp"

class Thief : public SmartMonster
{
	public:
		Thief();
		virtual ~Thief();
		virtual void init ( void );
		virtual int spellAttack ( void );
};

#endif
