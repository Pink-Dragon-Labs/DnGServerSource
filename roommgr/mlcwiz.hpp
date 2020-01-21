//
// mlcwiz.hpp
//
// Control logic for Castle Wizards.
//
// Author: Zachary Kaiser
//

#ifndef _MLCWIZ_HPP_
#define _MLCWIZ_HPP_

#include "mlsmarty.hpp"

class GoodCastleWizard: public SmartMonster
{
public:
	GoodCastleWizard();
	virtual ~GoodCastleWizard();

	virtual void init ( void );
	virtual int iWantToJump ( WorldObject *thisCharacter );
	virtual int spellAttack ( void );
};

class NeutralCastleWizard: public SmartMonster
{
public:
	NeutralCastleWizard();
	virtual ~NeutralCastleWizard();

	virtual void init ( void );
	virtual int iWantToJump ( WorldObject *thisCharacter );
	virtual int spellAttack ( void );
};

class EvilCastleWizard: public SmartMonster
{
public:
	EvilCastleWizard();
	virtual ~EvilCastleWizard();

	virtual void init ( void );
	virtual int iWantToJump ( WorldObject *thisCharacter );
	virtual int spellAttack ( void );
};

#endif
