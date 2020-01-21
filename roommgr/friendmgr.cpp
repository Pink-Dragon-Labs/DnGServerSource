//
// friendmgr
//
// This module manages the global friend list system.
//
// author: Stephen Nichols
//

// SNTODO: make sure unused CFriend objects are disposed...

#include "roommgr.hpp"
#include "friendmgr.hpp"

// global pointer to friend manager
CFriendMgr *g_pFriendMgr = NULL;

//
// CFriend: This class represents a single entry in the CFriendMgr
//

CFriend::CFriend()
{
	m_pPlayer = NULL;
	m_pWatcherList = NULL;
	m_pElementList = NULL;
}

CFriend::~CFriend()
{
	// release the watcher list
	if ( m_pWatcherList ) {
		m_pWatcherList->release();
		delete m_pWatcherList;
		m_pWatcherList = NULL;
	}

	// release the element list
	if ( m_pElementList ) {
		m_pElementList->release();
		delete m_pElementList;
		m_pElementList = NULL;
	}

	m_pPlayer = NULL;
}

// call to add a LinkedElement to the element list
bool CFriend::AddElement ( LinkedElement *pElement )
{
	if ( NULL == pElement ) {
		logDisplay ( "UNEXPECTED BEHAVIOR: %s(%d)", __FILE__, __LINE__ );
		return FALSE;
	}

	// create the element list, if needed
	if ( NULL == m_pElementList ) {
		m_pElementList = new LinkedList;
	}

	// add the element to the list
	m_pElementList->add ( (ListObject *)pElement );

	return TRUE;
}

// call to delete a LinkedElement from the element list
bool CFriend::DelElement ( LinkedElement *pElement )
{
	if ( (NULL == pElement) || (NULL == m_pElementList) ) {
		logDisplay ( "UNEXPECTED BEHAVIOR: %s(%d)", __FILE__, __LINE__ );
		return FALSE;
	}

	// get the element of pElement
	LinkedElement *pTargetElement = m_pElementList->elementOf ( (ListObject *)pElement );

	if ( NULL == pTargetElement ) {
		logDisplay ( "UNEXPECTED BEHAVIOR: %s(%d)", __FILE__, __LINE__ );
		return FALSE;
	}

	// delete it from the list...
	m_pElementList->delElement ( pTargetElement );

	return TRUE;
}

// call to add a player to the watcher list
bool CFriend::AddWatcher ( RMPlayer *pPlayer )
{
	if ( NULL == pPlayer ) {
		logDisplay ( "UNEXPECTED BEHAVIOR: %s(%d)", __FILE__, __LINE__ );
		return FALSE;
	}

	// don't add if pPlayer is the same as m_pPlayer
	if ( m_pPlayer == pPlayer ) {
		logDisplay ( "UNEXPECTED BEHAVIOR: %s(%d)", __FILE__, __LINE__ );
		return FALSE;
	}

	// get pPlayer CFriend object...
	CFriend *pFriend = pPlayer->GetFriendEntry();

	// don't add if pPlayer has no friend object...
	if ( NULL == pFriend ) {
		logDisplay ( "UNEXPECTED BEHAVIOR: %s(%d)", __FILE__, __LINE__ );
		return FALSE;
	}

	// don't add if pFriend is at capacity...
	if ( pFriend->IsAtCapacity() ) {
		logDisplay ( "UNEXPECTED BEHAVIOR: %s(%d)", __FILE__, __LINE__ );
		return FALSE;
	}

	// create the watcher list if needed...
	if ( NULL == m_pWatcherList ) {
		m_pWatcherList = new LinkedList;
	}

	// check for multiple additions...
	if ( m_pWatcherList->contains ( pPlayer ) ) {
		return FALSE;
	}

	// add the player to the list...
	LinkedElement *pElement = m_pWatcherList->add ( pPlayer );

	// add the pElement object to pPlayer's friend object...
	pFriend->AddElement ( pElement );

	// if our player is logged in, tell pPlayer
	if ( m_pPlayer ) {

		// if our player is Implementor only another imp can friendlist them.
		if ( m_pPlayer->checkAccess( _ACCESS_IMPLEMENTOR ) && !pPlayer->checkAccess( _ACCESS_IMPLEMENTOR ) ) {
			return TRUE;
		}

		// if our player is guide or better
		if ( m_pPlayer->checkAccess( _ACCESS_GUIDE ) && !pPlayer->checkAccess( _ACCESS_GUIDE ) ) {
			return TRUE;
		}

		WorldObject *obj = m_pPlayer->player;

		if ( obj->physicalState & _STATE_BUSY ) {
			roomMgr->sendPlayerText ( pPlayer, "-fb%s", m_pPlayer->getName() );
		} else {
			roomMgr->sendPlayerText ( pPlayer, "-f+%s", m_pPlayer->getName() );
		}
	}

	return TRUE;
}

// call to see if they are being watched by this player
bool CFriend::beingWatchedBy( RMPlayer* pPlayer ) {
        // check for multiple additions...
        if ( m_pWatcherList && m_pWatcherList->contains ( pPlayer ) ) {
                return TRUE;
        }

	return FALSE;
}

// call to delete a player from the watcher list
bool CFriend::DelWatcher ( RMPlayer *pPlayer )
{
	if ( (NULL == pPlayer) || (NULL == m_pWatcherList) ) {
		logDisplay ( "UNEXPECTED BEHAVIOR: %s(%d)", __FILE__, __LINE__ );
		return FALSE;
	}

	// get pPlayer CFriend object...
	CFriend *pFriend = pPlayer->GetFriendEntry();

	// don't delete if pPlayer has no friend object...
	if ( NULL == pFriend ) {
		logDisplay ( "UNEXPECTED BEHAVIOR: %s(%d)", __FILE__, __LINE__ );
		return FALSE;
	}

	// get the element of pPlayer in our watcher list
	LinkedElement *pElement = m_pWatcherList->elementOf ( pPlayer );

	if ( NULL == pElement ) {
		return FALSE;
	}

	// delete the element from pPlayer's element list
	pFriend->DelElement ( pElement );

	// delete pPlayer from the watcher list...
	m_pWatcherList->delElement ( pElement );

	return TRUE;
}

// handle our player logging in...
bool CFriend::HandleLogin ( RMPlayer *pPlayer ) {
	// handle the player already being logged in...
	if ( m_pPlayer ) {
		logDisplay ( "UNEXPECTED BEHAVIOR: %s(%d)", __FILE__, __LINE__ );
		return FALSE;
	}

	char *pPlayerName = pPlayer->getName();

	// hook the player to this object
	m_pPlayer = pPlayer;

	// step through the watchers and let them know the player is here...
	if ( m_pWatcherList ) {
		LinkedElement *pElement = m_pWatcherList->head();

		if ( m_pPlayer->checkAccess( _ACCESS_IMPLEMENTOR ) ) {
			while ( pElement ) {
				RMPlayer *pWatcher = (RMPlayer *)pElement->ptr();
				pElement = pElement->next();

				if ( pWatcher->checkAccess( _ACCESS_IMPLEMENTOR ) ) {
					roomMgr->sendPlayerText ( pWatcher, "-f+%s", pPlayerName );
				}
			}
		} else if ( m_pPlayer->checkAccess( _ACCESS_GUIDE ) ) {
			while ( pElement ) {
				RMPlayer *pWatcher = (RMPlayer *)pElement->ptr();
				pElement = pElement->next();

				if ( pWatcher->checkAccess( _ACCESS_GUIDE ) ) {
					roomMgr->sendPlayerText ( pWatcher, "-f+%s", pPlayerName );
				}
			}
		} else {
			while ( pElement ) {
				RMPlayer *pWatcher = (RMPlayer *)pElement->ptr();
				pElement = pElement->next();

				roomMgr->sendPlayerText ( pWatcher, "-f+%s", pPlayerName );
			}
		}
	}

	return TRUE;
}

// handle our player logging out...
bool CFriend::HandleLogout ( RMPlayer *pPlayer )
{
	// handle the passed player not matching our player...
	if ( m_pPlayer != pPlayer ) {
		logDisplay ( "UNEXPECTED BEHAVIOR: %s(%d)", __FILE__, __LINE__ );
		return FALSE;
	}

	// get the player name
	char *pPlayerName = pPlayer->getName();

	// unhook the player from this object
	m_pPlayer = NULL;

	// step through the watcher and let them know the player is leaving...
	if ( m_pWatcherList ) {
		LinkedElement *pElement = m_pWatcherList->head();

		if ( pPlayer->checkAccess( _ACCESS_IMPLEMENTOR ) ) {
			while ( pElement ) {
				RMPlayer *pWatcher = (RMPlayer *)pElement->ptr();
				pElement = pElement->next();

				if ( pWatcher->checkAccess( _ACCESS_IMPLEMENTOR ) )
					roomMgr->sendPlayerText ( pWatcher, "-f-%s", pPlayerName );
			}
		} else if ( pPlayer->checkAccess( _ACCESS_GUIDE ) ) {
			while ( pElement ) {
				RMPlayer *pWatcher = (RMPlayer *)pElement->ptr();
				pElement = pElement->next();

				if ( pWatcher->checkAccess( _ACCESS_GUIDE ) )
					roomMgr->sendPlayerText ( pWatcher, "-f-%s", pPlayerName );
			}
		} else {
			while ( pElement ) {
				RMPlayer *pWatcher = (RMPlayer *)pElement->ptr();
				pElement = pElement->next();

				roomMgr->sendPlayerText ( pWatcher, "-f-%s", pPlayerName );
			}
		}
	}

	// step through our element list and unhook them all...
	if ( m_pElementList ) {
		LinkedElement *pElement = m_pElementList->head();

		while ( pElement ) {
			LinkedElement *pTargetElement = (LinkedElement *)pElement->ptr();
			pElement = pElement->next();

			pTargetElement->DeleteFromList();
		}

		// release the element list
		m_pElementList->release();
		delete m_pElementList;
		m_pElementList = NULL;
	}

	return TRUE;
}

// send the busy indicator to my watchers
void CFriend::SendBusy ( void )
{
	if ( m_pWatcherList ) {
		LinkedElement *pElement = m_pWatcherList->head();
		char *pPlayerName = m_pPlayer->getName();

		if ( m_pPlayer->checkAccess( _ACCESS_IMPLEMENTOR ) ) {
			while ( pElement ) {
				RMPlayer *pWatcher = (RMPlayer *)pElement->ptr();
				pElement = pElement->next();

				if ( pWatcher->checkAccess( _ACCESS_IMPLEMENTOR ) )
					roomMgr->sendPlayerText ( pWatcher, "-fb%s", pPlayerName );
			}
		} else if ( m_pPlayer->checkAccess( _ACCESS_GUIDE ) ) {
			while ( pElement ) {
				RMPlayer *pWatcher = (RMPlayer *)pElement->ptr();
				pElement = pElement->next();

				if ( pWatcher->checkAccess( _ACCESS_GUIDE ) )
					roomMgr->sendPlayerText ( pWatcher, "-fb%s", pPlayerName );
			}
		} else {
			while ( pElement ) {
				RMPlayer *pWatcher = (RMPlayer *)pElement->ptr();
				pElement = pElement->next();

				roomMgr->sendPlayerText ( pWatcher, "-fb%s", pPlayerName );
			}
		}
	}
}

// send the unbusy indicator to my watchers
void CFriend::SendUnbusy ( void )
{
	if ( m_pWatcherList ) {
		LinkedElement *pElement = m_pWatcherList->head();
		char *pPlayerName = m_pPlayer->getName();

		if ( m_pPlayer->checkAccess( _ACCESS_IMPLEMENTOR ) ) {
			while ( pElement ) {
				RMPlayer *pWatcher = (RMPlayer *)pElement->ptr();
				pElement = pElement->next();

				if ( pWatcher->checkAccess( _ACCESS_IMPLEMENTOR ) )
					roomMgr->sendPlayerText ( pWatcher, "-fu%s", pPlayerName );
			}
		} else if ( m_pPlayer->checkAccess( _ACCESS_GUIDE ) ) {
			while ( pElement ) {
				RMPlayer *pWatcher = (RMPlayer *)pElement->ptr();
				pElement = pElement->next();

				if ( pWatcher->checkAccess( _ACCESS_GUIDE ) )
					roomMgr->sendPlayerText ( pWatcher, "-fu%s", pPlayerName );
			}
		} else {
			while ( pElement ) {
				RMPlayer *pWatcher = (RMPlayer *)pElement->ptr();
				pElement = pElement->next();

				roomMgr->sendPlayerText ( pWatcher, "-fu%s", pPlayerName );
			}
		}
	}
}

// return TRUE if we are at (or exceeding) element capacity...
bool CFriend::IsAtCapacity ( void )
{
	static int nMaxCapacity = 200;

	if ( m_pElementList ) {
		if ( m_pElementList->size() >= nMaxCapacity ) {
			logDisplay ( "UNEXPECTED BEHAVIOR: %s(%d)", __FILE__, __LINE__ );
			return TRUE;
		}
	}

	return FALSE;
}

//
// CFriendMgr: This class represents the global set of friend relationships.
//

CFriendMgr::CFriendMgr()
{
	m_FriendTree.SetCaseSensitive ( FALSE );
}

CFriendMgr::~CFriendMgr()
{
}

// call to find the friend object for a particular player...
CFriend *CFriendMgr::FindFriend ( char *pPlayerName )
{
	if ( NULL == pPlayerName ) {
		logDisplay ( "UNEXPECTED BEHAVIOR: %s(%d)", __FILE__, __LINE__ );
		return NULL;
	}

	// this is the friend object to return...
	CFriend *pFriend = NULL;

	// search the tree for the given name
	TreeNode *pNode = m_FriendTree.find ( pPlayerName );

	// create a new friend object if none exists for the given name...
	if ( NULL == pNode ) {
		pFriend = new CFriend; 
		m_FriendTree.add ( pPlayerName, pFriend );
	} else {
		pFriend = (CFriend *)pNode->data;
	}

	return pFriend;
}
