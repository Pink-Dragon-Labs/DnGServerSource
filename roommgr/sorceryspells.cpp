//
// sorceryspells
//
// This file contains the Sorcery spells.
//
// author: Stephen Nichols
//

#include "sorceryspells.hpp"
#include "roommgr.hpp"
#include "callbacks.hpp"

//
// CSpellHome: This spell sends the caster to his house.
//

CSpellHome::CSpellHome()
{
	m_nID = _SPELL_HOME;
	m_nManaCost = 1;
}

CSpellHome::~CSpellHome()
{
}

// cast the home spell
affect_t *CSpellHome::Cast ( WorldObject *pCaster, int nTargetServID, int nTargetX, int nTargetY, char *pOutputString, PackedData *pPacket )
{
	// if no caster, skip out 
	if ( !pCaster ) {
		return NULL;
	}

	// check for invalid casting situation..
	RMPlayer *pPlayer = pCaster->player;

	if ( !pPlayer || pPlayer->isNPC || pPlayer->isTeleporting || pPlayer->teleportRoomNum != -1 ) {
		return NULL;
	}

	teleportHouse ( pCaster, pCaster->getName() );

	return NULL;
}

// 
// CSpellKillStar: Attacks target with spiked projectiles.
//

CSpellKillStar::CSpellKillStar()
{
	m_nID = _SPELL_KILL_STAR;
	m_nSkillUsed = _SKILL_SORCERY;
	m_nSpeed = _SPELL_FAST;
	m_nManaCost = 1;
	m_pName = "Killstar";
	m_pVerbalText = "Projen bladir!";
	m_nEqualChance = 75;
	m_nMaxChance = 100;
	m_nSpecialEffect = _SE_MULTI_BLADE;
	m_nMinDamage = 5;
	m_nMaxDamage = 10;
	m_bMultiEffect = 1;
	m_bSpreadDamage = 0;
}

CSpellKillStar::~CSpellKillStar()
{
}

// 
// CSpellMultiBlade: Attacks target's group with spiked projectiles.
//

CSpellMultiBlade::CSpellMultiBlade()
{
	m_nID = _SPELL_MULTI_BLADE;
	m_nSkillUsed = _SKILL_SORCERY;
	m_nSpeed = _SPELL_FAST;
	m_nManaCost = 6;
	m_pName = "Multi-blade";
	m_pVerbalText = "Projen bladiros!";
	m_nEqualChance = 75;
	m_nMaxChance = 100;
	m_nSpecialEffect = _SE_MULTI_BLADE;
	m_nMinDamage = 5;
	m_nMaxDamage = 10;
	m_bMultiEffect = 1;
	m_bSpreadDamage = 1;
}

CSpellMultiBlade::~CSpellMultiBlade()
{
}
