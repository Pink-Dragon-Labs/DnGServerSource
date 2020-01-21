//
// zonescripts
//
// This module contains various CZoneScripts.
//
// author: Stephen Nichols
//

#ifndef _ZONESCRIPTS_HPP_
#define _ZONESCRIPTS_HPP_

#include "zone.hpp"

//
// CEnidDngZoneScript: This script handles special behavior for the 
// enid dungeon zone.
//

class CEnidDngZoneScript : public CZoneScript
{
public:
	CEnidDngZoneScript();
	virtual ~CEnidDngZoneScript();

	// handle changing states...
	virtual void ChangeState ( int nNewState );
};

#endif
