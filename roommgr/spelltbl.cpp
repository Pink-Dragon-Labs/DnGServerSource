//
// spelltbl
//
// This module contains the global spell table.
//
// author: Stephen Nichols
//

#include <stdlib.h>
#include "spelltbl.hpp"
#include "spell.hpp"
#include "magic.hpp"
#include "fatal.hpp"

// include the spell circles...
#include "sorceryspells.hpp"

// this is the global spell table variable
CSpellTable *g_pSpellTable = NULL;

// this is the array of available spells
CSpell **m_ppSpells = NULL;

CSpellTable::CSpellTable()
{
	m_ppSpells = (CSpell **)malloc ( sizeof ( CSpell * ) * _SPELL_MAX );

	// add the spells to the table...
	AddSpell ( new CSpellHome );
	AddSpell ( new CSpellKillStar );
	AddSpell ( new CSpellMultiBlade );
}

CSpellTable::~CSpellTable()
{
}

// returns a pointer to the object that represents the given spell ID
CSpell *CSpellTable::GetSpell ( int nSpellID )
{
	if ( (nSpellID < 0) || (nSpellID >= _SPELL_MAX) ) {
		return NULL;
	}

	return m_ppSpells[nSpellID];
}

// add a new spell to the table
void CSpellTable::AddSpell ( CSpell *pSpell )
{
	if ( NULL == pSpell ) {
		return;
	}

	// validate the ID of this spell
	int nID = pSpell->GetID();

	if ( (nID < 0) || (nID >= _SPELL_MAX) ) {
		fatal ( "CSpellTable::AddSpell: Spell has invalid id of %d", nID );
	}

	// make sure a spell is not in the array with the same ID
	if ( NULL != GetSpell ( nID ) ) {
		fatal ( "CSpellTable::AddSpell: Duplicate spell ID %d", nID );
	}

	// put the spell in the table
	m_ppSpells[nID] = pSpell;
}
