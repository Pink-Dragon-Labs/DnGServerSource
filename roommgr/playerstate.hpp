//
// playerstate
//
// This module contains the CPlayerState class.
//
// author: Stephen Nichols
//

#ifndef _PLAYERSTATE_HPP_
#define _PLAYERSTATE_HPP_

// define external classes
class WorldObject;
class PackedData;

//
// CPlayerState: This class represents the special properties that a player
// can have attached to them.
//

class CPlayerState
{
public:
	enum {
		_CAST_RESISTANCE_SORCERY,
		_CAST_RESISTANCE_ELEMENTALISM,
		_CAST_RESISTANCE_MYSTICISM,
		_CAST_RESISTANCE_THAUMATURGY,
		_CAST_RESISTANCE_NECROMANCY,
		_CAST_RESISTANCE_MAX
	};

	// amount of mana to drain per round...
	float m_fManaDrain;

	// internal counter of how much mana we've drained (used to handle 
	// accumulated fractional drains)
	float m_fManaDrained;

	// percentage phased to melee attacks
	int m_nMeleePhase;

	// melee armor piercing...
	int m_nMeleeArmorPiercing;

	// evil mdm mod...
	int m_nEvilMDMMod;

	// evil mdm mod...
	int m_nGoodMDMMod;
	
	// mysticism immunity round count...
	int m_nMystImmunityCount;

	// table of spell casting resistances...
	int m_anCastResistance[_CAST_RESISTANCE_MAX];

	// table of spell target resistances...
	int m_anSpellResistance[_CAST_RESISTANCE_MAX];

	// table of spell damage modifiers...
	int m_anSDM[_CAST_RESISTANCE_MAX];

public:
	CPlayerState();
	virtual ~CPlayerState();

	// change SDM...
	void ChangeSDM ( int nType, int nValue );

	// get SDM...
	int GetSDM ( int nType );

	// change cast resistance...
	void ChangeCastResistance ( int nType, int nValue );

	// test cast resistance...
	int TestCastResistance ( int nType );

	// change spell resistance...
	void ChangeSpellResistance ( int nType, int nValue );

	// test spell resistance...
	int TestSpellResistance ( int nType );

	// change the mana drain property...
	void ChangeManaDrain ( float fDrain );

	// process the mana drain...
	void ProcessManaDrain ( WorldObject *pObject, PackedData *pPacket );

	// change melee phasing...
	void ChangeMeleePhasing ( int nValue );

	// roll against melee phasing with the given penetration...
	int TestMeleePhasing ( int nPenetration = 0 );

	// change evil mdm modifier...
	void ChangeEvilMDMMod ( int nValue );

	// apply the evil mdm modifier...
	int ApplyEvilMDMMod ( int nOriginalValue );

	// change good mdm modifier...
	void ChangeGoodMDMMod ( int nValue );

	// apply the good mdm modifier...
	int ApplyGoodMDMMod ( int nOriginalValue );

	// change the melee armor piercing...
	void ChangeMeleeArmorPierce ( int nValue );

	// apply the melee armor piercing...
	int ApplyMeleeArmorPierce ( int nValue );

	// change the myst immunity rounds...
	void ChangeMystImmunityCount ( int nValue );

	// get the myst immunity count...
	int GetMystImmunityCount ( void );
};

#endif
