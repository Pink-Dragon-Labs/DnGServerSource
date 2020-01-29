//
// mlfaeryhealer.hpp
//
// File that contains Faery Healer logic code.
//
// Author: Zachary Kaiser
//

#ifndef _MLFAERYHEALER_HPP_
#define _MLFAERYHEALER_HPP_

#include "mlsmarty.hpp"

class FaeryHealer : public SmartMonster
{
public:
	FaeryHealer();
	virtual ~FaeryHealer();

	virtual void init ( void );
	virtual int spellAttack ( void );
};
#endif
