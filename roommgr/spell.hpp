//
// spell
//
// This file contains the base class CSpell, which all spells derive from.
//
// author: Stephen Nichols
//

#ifndef _SPELL_HPP_
#define _SPELL_HPP_

// define external classes
class WorldObject;
class affect_t;
class PackedData;

class CSpell
{
protected:
	// this is the ID of this spell...
	int m_nID;

	// this is the mana cost of the spell...
	int m_nManaCost;

	// this is the speed of the spell...
	int m_nSpeed;

	// this is the skill that this spell uses...
	int m_nSkillUsed;

	// this is this spell's name
	char *m_pName;

	// this is the text to display while casting this spell...
	char *m_pVerbalText;

	// can this spell be used in combat?
	int m_bCombatAllowed;

	// can this spell be used out of combat?
	int m_bNonCombatAllowed;

	// is this spell considered aggressive?
	int m_bAggressive;

	// equal level success chance
	int m_nEqualChance;

	// max success chance
	int m_nMaxChance;

	// fixed level for calculating the success chance of non-targetted spells
	int m_nChanceLevel;

public:
	CSpell();
	virtual ~CSpell();

	// call to get the ID of this spell
	int GetID ( void );

	// call to get the mana cost of this spell
	int GetManaCost ( void );

	// call to get the spell speed
	int GetSpeed ( void );

	// call to get the verbal component
	char *GetVerbal ( void );

	// call to get the name of this spell
	char *GetName ( void );

	// call to see if this spell can be used in combat
	int CombatAllowed ( void );

	// call to see if this spell is combat only... 
	int CombatOnly ( void );

	// call to see if this spell is an aggressive one...
	int GetAggressive ( void );

	// return the chance to cast this spell using the given levels
	int CalcCastChance ( WorldObject *pCaster, int nTargetLevel );

	// rolls against the CalcCastChance to determine if the spell worked
	int CanCast ( WorldObject *pCaster, int nTargetLevel );

	// call to cast this spell...
	virtual affect_t *Cast ( WorldObject *pCaster, int nTargetID, int nTargetX, int nTargetY, char *pOutputString, PackedData *pPacket );
};

#endif
