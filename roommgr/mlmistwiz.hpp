//
// mlmistwiz.hpp
//
// Control logic for Mist Wizards.
//
// Author: Bryan Waters (with Scott Wochholz)
//

#ifndef _MLMISTWIZ_HPP_
#define _MLMISTWIZ_HPP_

#include "mlsmarty.hpp"

class GoodWizard: public SmartMonster
{
public:
	GoodWizard();
	virtual ~GoodWizard();

	virtual void init ( void );
	virtual int iWantToJump ( WorldObject *thisCharacter );
	virtual int spellAttack ( void );
};

class NeutralWizard: public SmartMonster
{
public:
	NeutralWizard();
	virtual ~NeutralWizard();

	virtual void init ( void );
	virtual int iWantToJump ( WorldObject *thisCharacter );
	virtual int spellAttack ( void );
};

class EvilWizard: public SmartMonster
{
public:
	EvilWizard();
	virtual ~EvilWizard();

	virtual void init ( void );
	virtual int iWantToJump ( WorldObject *thisCharacter );
	virtual int spellAttack ( void );
};

#endif
