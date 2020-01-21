//
// projectilespell
//
// This file contains the CProjectileSpell class.
//
// author: Stephen Nichols
//

#ifndef _PROJECTILESPELL_HPP_
#define _PROJECTILESPELL_HPP_

#include "spell.hpp"

//
// CProjectileSpell: This class represents a spell that generates projectiles
// and hurls them toward a target.  These projectiles cause damage.
//

class CProjectileSpell : public CSpell
{
protected:
	// this is the minimum amount of damage done to the target
	int m_nMinDamage;

	// this is the maximum amount of damage done to the target
	int m_nMaxDamage;

	// this is the type of damage to do to the target
	int m_nDamageType;

	// this is the special effect to show for this spell
	int m_nSpecialEffect;

	// set to true if the special effect is a multi-target effect...
	int m_bMultiEffect;

	// set to true if the damage is to be spread across all opposition...
	int m_bSpreadDamage;

public:
	CProjectileSpell();
	virtual ~CProjectileSpell();

	// call to cast this spell
	virtual affect_t *Cast ( WorldObject *pCaster, int nTargetID, int nTargetX, int nTargetY, char *pOutputString, PackedData *pPacket );
};

#endif
