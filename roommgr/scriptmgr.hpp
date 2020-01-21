//
// scriptmgr
//
// This module contains the classes required to manage the global script
// list.
//
// author: Stephen Nichols
//

#ifndef _SCRIPTMGR_HPP_
#define _SCRIPTMGR_HPP_

#include "../global/script.hpp"
#include "../global/list.hpp"

// define external classes
class CManagedScript;

//
// CScriptMgr: This is the global script manager.  It keeps a list of every
// script that is active and makes sure that they get their Doit called.
//

class CScriptMgr
{
protected:
	// this is the list of scripts...
	LinkedList *m_pScriptList;

public:
	CScriptMgr();
	virtual ~CScriptMgr();

	// call to process the scripts in the manager
	void Doit ( void );

	// call to add a script to the manager
	void AddScript ( CManagedScript *pScript );

	// call to delete a script from the manager
	void DelScript ( CManagedScript *pScript );
};

// this is the global pointer to the script manager
extern CScriptMgr *g_pScriptMgr;

//
// CManagedScript: This is a derivative of CScript that hooks into the global
// script manager.
//

class CManagedScript : public CScript
{
	friend class CScriptMgr;

protected:
	// pointer to the list element that represents this script in the manager
	LinkedElement *m_pListElement;

public:
	CManagedScript();
	virtual ~CManagedScript();
};

#endif
