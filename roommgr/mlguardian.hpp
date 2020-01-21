//
// mlguardian.hpp
//
// File that contains faery logic code.
//
// Author: Stephen Nichols
//

#ifndef _MLGUARDIAN_HPP_
#define _MLGUARDIAN_HPP_

#include "mlmonstr.hpp"

class Guardian : public SmartMonster
{
public:
	Guardian();
	virtual ~Guardian();

    virtual void init ( void );
	virtual int isEnemy ( WorldObject *thisCharacter );
	virtual int spellAttack ( void );
};

#endif
