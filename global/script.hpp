//
// script
//
// This module contains the CScript finite state machine.
//
// author: Stephen Nichols
//

#ifndef _SCRIPT_HPP_
#define _SCRIPT_HPP_

#include "listobj.hpp"

//
// CScript: This is a simple finite state machine.
//

class CScript : public ListObject
{
protected:
	// this is the current state of this script...
	int m_nState;

	// this is the number of seconds to wait before advancing to the next state
	int m_nSeconds;

	// this is the number of cycles to wait before advancing to the next state
	int m_nCycles;

	// this is the pointer to this script's client...
	void *m_pClient;

	// this is the last time (in seconds) that this script Doit was called
	int m_nLastDoitTime;

public:
	CScript();
	virtual ~CScript();

	// call to set the client of this script...
	void SetClient ( void *pClient );

	// call to get the client of this script...
	void *GetClient ( void );

	// call to change the state of this script to another state
	virtual void ChangeState ( int nNewState );

	// call to allow this script to do something...
	virtual void Doit ( void );
};

#endif
