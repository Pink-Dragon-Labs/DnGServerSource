//
// mlfaery.hpp
//
// File that contains faery logic code.
//
// Author: Stephen Nichols
//

#ifndef _MLFAERY_HPP_
#define _MLFAERY_HPP_

#include "mlsmarty.hpp"

extern int gEggTimer;
extern int gDoDropEggs;

class GoodFaery : public SmartMonster
{
public:
	GoodFaery();
	virtual ~GoodFaery();

	virtual void init ( void );
	virtual int spellAttack ( void );
	virtual int iWantToJump ( WorldObject *thisCharacter );
};

class EvilFaery : public SmartMonster
{
public:
	EvilFaery();
	virtual ~EvilFaery();

	virtual void init ( void );
	virtual int spellAttack ( void );
};

#endif
