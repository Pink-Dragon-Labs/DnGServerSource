//
// sorceryspells
//
// This file contains the Sorcery spells.
//
// author: Stephen Nichols
//

#ifndef _SORCERYSPELLS_HPP_
#define _SORCERYSPELLS_HPP_

#include "projectilespell.hpp"

//
// CSpellHome: This spell sends the caster to his house.
//

class CSpellHome : public CSpell
{
public:
	CSpellHome();
	virtual ~CSpellHome();

	// cast the home spell
	virtual affect_t *Cast ( WorldObject *pCaster, int nTargetServID, int nTargetX, int nTargetY, char *pOutputString, PackedData *pPacket );
};

//
// CSpellKillStar: Attacks target with spiked projectiles.
//

class CSpellKillStar : public CProjectileSpell
{
public:
	CSpellKillStar();
	virtual ~CSpellKillStar();
};

//
// CSpellMultiBlade: Attacks target's group with spiked projectiles.
//

class CSpellMultiBlade : public CProjectileSpell
{
public:
	CSpellMultiBlade();
	virtual ~CSpellMultiBlade();
};

#endif
