//
// script
//
// This module contains the CScript finite state machine.
//
// author: Stephen Nichols
//

#include "script.hpp"
#include "tools.hpp"

//
// CScript: This is a simple finite state machine.
//

CScript::CScript()
{
	m_nState = -1;
	m_nSeconds = 0;
	m_nCycles = 0;
	m_pClient = NULL;
	m_nLastDoitTime = getsecondsfast();
}

CScript::~CScript()
{
}

// call to set the client of this script...
void CScript::SetClient ( void *pClient )
{
	m_pClient = pClient;
}

// call to get the client of this script...
void *CScript::GetClient ( void )
{
	return m_pClient;
}

// call to change the state of this script to another state
void CScript::ChangeState ( int nNewState )
{
}

// call to allow this script to do something...
void CScript::Doit ( void )
{
	int nCurTime = getsecondsfast();

	// check for seconds being set...
	if ( m_nSeconds > 0 ) {
		int nSecondsElapsed = m_nLastDoitTime - nCurTime;

		if ( nSecondsElapsed > 0 ) {
			m_nSeconds -= nSecondsElapsed;

			if ( m_nSeconds <= 0 ) {
				m_nSeconds = 0;
				ChangeState ( m_nState + 1 );
			}

			m_nLastDoitTime = nCurTime;
		}

		return;
	}

	// check for cycles being set...
	if ( m_nCycles > 0 ) {
		m_nCycles--;

		if ( 0 == m_nCycles ) {
			ChangeState ( m_nState + 1 );	
		}
	}

	m_nLastDoitTime = nCurTime;
}
