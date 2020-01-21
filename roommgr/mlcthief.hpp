//
// mlcthief.hpp
//
// File that contains castle thief logic code.
//
// Author: Zachary Kaiser
//

#ifndef _MLCTHIEF_HPP_
#define _MLCTHIEF_HPP_

#include "mlsmarty.hpp"

class RogueThief : public SmartMonster
{
	public:
		RogueThief();
		virtual ~RogueThief();
		virtual void init ( void );
		virtual int spellAttack ( void );
};

#endif
