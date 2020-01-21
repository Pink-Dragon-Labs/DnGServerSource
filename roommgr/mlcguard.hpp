//
// mlcguard.hpp
//
// File that contains castle guard logic code.
//
// Author: Zachary Kaiser
//

#ifndef _MLCGUARD_HPP_
#define _MLCGUARD_HPP_

#include "mlsmarty.hpp"

class CastleGuard : public SmartMonster
{
	public:
		CastleGuard();
		virtual ~CastleGuard();
		virtual void init ( void );
		virtual int spellAttack ( void );
};

#endif
