//
// playerstate
//
// This module contains the CPlayerState class.
//
// author: Stephen Nichols
//
#include <algorithm>
#include "playerstate.hpp"
#include "roommgr.hpp"

//
// CPlayerState: This class represents the special properties that a player
// can have attached to them.
//

CPlayerState::CPlayerState()
{
	m_fManaDrain = 0;
	m_fManaDrained = 0;
	m_nMeleePhase = 0;
	m_nMeleeArmorPiercing = 0;
	m_nEvilMDMMod = 0;
	m_nGoodMDMMod = 0;
	m_nMystImmunityCount = 0;

	for ( int i=0; i<_CAST_RESISTANCE_MAX; i++ ) {
		m_anCastResistance[i] = 0;
		m_anSpellResistance[i] = 0;
		m_anSDM[i] = 0;
	}
}

CPlayerState::~CPlayerState()
{
}

// change SDM ...
void CPlayerState::ChangeSDM ( int nType, int nValue )
{
	if ( nType >= 0 && nType < _CAST_RESISTANCE_MAX ) {
		m_anSDM[nType] += nValue;
	}
}

// get SDM ...
int CPlayerState::GetSDM ( int nType )
{
	if ( nType >= 0 && nType < _CAST_RESISTANCE_MAX ) {
		return m_anSDM[nType];
	}

	return 0;
}

// change spell resistance...
void CPlayerState::ChangeSpellResistance ( int nType, int nValue )
{
	if ( nType >= 0 && nType < _CAST_RESISTANCE_MAX ) {
		m_anSpellResistance[nType] += nValue;
	}
}

// test spell resistance...
int CPlayerState::TestSpellResistance ( int nType )
{
	if ( nType >= 0 && nType < _CAST_RESISTANCE_MAX ) {
		int nValue = std::max(0, m_anSpellResistance[nType]);
//0 >? m_anSpellResistance[nType];
		int nRoll = random ( 1, 100 );

		if ( nRoll <= nValue )
			return 1;
	}

	return 0;
}

// change cast resistance...
void CPlayerState::ChangeCastResistance ( int nType, int nValue )
{
	if ( nType >= 0 && nType < _CAST_RESISTANCE_MAX ) {
		m_anCastResistance[nType] += nValue;
	}
}

// test cast resistance...
int CPlayerState::TestCastResistance ( int nType )
{
	if ( nType >= 0 && nType < _CAST_RESISTANCE_MAX ) {
		int nValue = std::max(0, m_anCastResistance[nType]);
//0 >? m_anCastResistance[nType];
		int nRoll = random ( 1, 100 );

		if ( nRoll <= nValue )
			return 1;
	}

	return 0;
}

// change the mana drain property...
void CPlayerState::ChangeManaDrain ( float fDrain )
{
	m_fManaDrain += fDrain;
}

// process the mana drain...
void CPlayerState::ProcessManaDrain ( WorldObject *pObject, PackedData *pPacket )
{
	// apply the mana drain...
	m_fManaDrained += m_fManaDrain;

	int nWholeManaDrained = (int)m_fManaDrained;

	if ( nWholeManaDrained ) {
		if ( pObject->manaValue >= nWholeManaDrained ) {	
			pObject->changeMana ( -nWholeManaDrained, pPacket );
		} else {
			char output[1024] = "";
			pObject->takeDamage ( _AFF_DAMAGE_NORMAL, NULL, nWholeManaDrained * 10, output, pPacket, 1, 0, 1 ); 
			putMovieText ( pObject, pPacket, "%s", output );
		}

		m_fManaDrained -= nWholeManaDrained;
	}
}

// change melee phasing...
void CPlayerState::ChangeMeleePhasing ( int nValue )
{
	m_nMeleePhase += nValue;
}

// roll against melee phasing with the given penetration...
int CPlayerState::TestMeleePhasing ( int nPenetration )
{
	int nValue = std::max(0,  m_nMeleePhase - nPenetration);
//0 >? m_nMeleePhase - nPenetration;
	int nRoll = random ( 1, 100 );

	if ( nRoll <= nValue )
		return 1;

	return 0;
}

// change evil mdm modifier...
void CPlayerState::ChangeEvilMDMMod ( int nValue )
{
	m_nEvilMDMMod += nValue;
}

// apply the evil mdm modifier...
int CPlayerState::ApplyEvilMDMMod ( int nOriginalValue )
{
	return nOriginalValue + ((nOriginalValue * m_nEvilMDMMod) / 100);
}

// change good mdm modifier...
void CPlayerState::ChangeGoodMDMMod ( int nValue )
{
	m_nGoodMDMMod += nValue;
}

// apply the good mdm modifier...
int CPlayerState::ApplyGoodMDMMod ( int nOriginalValue )
{
	return nOriginalValue + ((nOriginalValue * m_nGoodMDMMod) / 100);
}

// change the melee armor piercing...
void CPlayerState::ChangeMeleeArmorPierce ( int nValue )
{
	m_nMeleeArmorPiercing += nValue;
}

// apply the melee armor pierce...
int CPlayerState::ApplyMeleeArmorPierce ( int nValue )
{
	nValue -= m_nMeleeArmorPiercing;

	if ( nValue < 0 )
		nValue = 0;

	return nValue;
}

// change the myst immunity count...
void CPlayerState::ChangeMystImmunityCount ( int nValue )
{
	m_nMystImmunityCount += nValue;

	if ( m_nMystImmunityCount < 0 )
		m_nMystImmunityCount = 0;
}

// get the myst immunity count...
int CPlayerState::GetMystImmunityCount ( void )
{
	return m_nMystImmunityCount;
}
