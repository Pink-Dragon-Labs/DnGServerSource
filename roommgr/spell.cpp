//
// spell
//
// This file contains the base class CSpell, which all spells derive from.
//
// author: Stephen Nichols
//

#include "spell.hpp"
#include "roommgr.hpp"

CSpell::CSpell()
{
	m_nID = -1;
	m_nManaCost = 10;
	m_nSpeed = 1;
	m_nEqualChance = 100;
	m_nMaxChance = 100;
	m_nChanceLevel = 1;
	m_nSkillUsed = _SKILL_SORCERY;

	m_pName = "Generic Spell";
	m_pVerbalText = "Spell";

	m_bCombatAllowed = 0;
	m_bNonCombatAllowed = 0;
	m_bAggressive = 0;
}

CSpell::~CSpell()
{
}

// call to get the spell ID
int CSpell::GetID ( void )
{
	return m_nID;
}

// call to get the mana cost of this spell
int CSpell::GetManaCost ( void )
{
	return m_nManaCost;
}

// call to get the spell speed
int CSpell::GetSpeed ( void )
{
	return m_nSpeed;
}

// call to get the verbal component
char *CSpell::GetVerbal ( void )
{
	return m_pVerbalText;
}

// call to get the name of this spell
char *CSpell::GetName ( void )
{
	return m_pName;
}

// call to see if this spell can be used in combat
int CSpell::CombatAllowed ( void )
{
	return m_bCombatAllowed;
}

// call to see if this spell is combat only... 
int CSpell::CombatOnly ( void )
{
	return (m_bCombatAllowed && !m_bNonCombatAllowed);	
}

// call to see if this spell is an aggressive one...
int CSpell::GetAggressive ( void )
{
	return m_bAggressive;
}

// return the chance to cast this spell based on the given caster and level
int CSpell::CalcCastChance ( WorldObject *pCaster, int nTargetLevel )
{
	return 100;
}

// rolls against the CalcCastChange to determine if the spell worked
int CSpell::CanCast ( WorldObject *pCaster, int nTargetLevel )
{
	int nChance = CalcCastChance ( pCaster, nTargetLevel );
	int nRoll = random ( 1, 100 );

	return nRoll <= nChance;
}

// call to cast this spell...
affect_t *CSpell::Cast ( WorldObject *pCaster, int nTargetID, int nTargetX, int nTargetY, char *pOutputString, PackedData *pPacket )
{
	strcat ( pOutputString, pCaster->getName() );
	strcat ( pOutputString, " casts " );
	strcat ( pOutputString, m_pName );
	strcat ( pOutputString, "on object. " );
	return NULL;
}
