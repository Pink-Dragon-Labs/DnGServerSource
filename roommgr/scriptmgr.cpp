//
// scriptmgr
//
// This module contains the classes required to manage the global script
// list.
//
// author: Stephen Nichols
//

#include "scriptmgr.hpp"
#include "../global/logmgr.hpp"

// this is the global pointer to the script manager
CScriptMgr *g_pScriptMgr = NULL;

//
// CScriptMgr: This is the global script manager.  It keeps a list of every
// script that is active and makes sure that they get their Doit called.
//

CScriptMgr::CScriptMgr()
{
	m_pScriptList = new LinkedList;
}

CScriptMgr::~CScriptMgr()
{
	delete m_pScriptList;
	m_pScriptList = NULL;
}

// call to process the scripts in the manager
void CScriptMgr::Doit ( void )
{
	// step through the managed scripts and let each of them do something...
	LinkedElement *pElement = m_pScriptList->head();

	while ( pElement ) {
		CManagedScript *pScript = (CManagedScript *)pElement->ptr();
		pElement = pElement->next();

		logDisplay ( "processing script 0x%p", pScript );

		// let the script do something
		pScript->Doit();
	}
}

// call to add a script to the manager
void CScriptMgr::AddScript ( CManagedScript *pScript )
{
	// don't add the script if it's already in the list
	if ( pScript->m_pListElement ) 
		return;

	// add the script to the list...
	pScript->m_pListElement = m_pScriptList->add ( pScript );
}

// call to delete a script from the manager
void CScriptMgr::DelScript ( CManagedScript *pScript )
{
	// don't delete the script unless it's in the list...
	if ( NULL == pScript->m_pListElement )
		return;

	// delete the script from the list...
	m_pScriptList->delElement ( pScript->m_pListElement );
	pScript->m_pListElement = NULL;
}

//
// CManagedScript: This is a derivative of CScript that hooks into the global
// script manager.
//

CManagedScript::CManagedScript()
{
	m_pListElement = NULL;
	g_pScriptMgr->AddScript ( this );
}

CManagedScript::~CManagedScript()
{
	g_pScriptMgr->DelScript ( this );
}
