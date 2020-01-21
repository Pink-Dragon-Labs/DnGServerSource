//
// spelltbl
//
// This module contains the global spell table.
//
// author: Stephen Nichols
//

#ifndef _SPELLTBL_HPP_
#define _SPELLTBL_HPP_

// define external classes
class CSpell;

class CSpellTable
{
	// this is the array of available spells
	CSpell **m_ppSpells;

public:
	CSpellTable();
	virtual ~CSpellTable();

	// returns a pointer to the object that represents the given spell ID
	CSpell *GetSpell ( int nSpellID );

	// add a new spell to the table
	void AddSpell ( CSpell *pSpell );
};

// this is the global spell table variable
extern CSpellTable *g_pSpellTable;

#endif
