//
// friendmgr
//
// This module manages the global friend list system.
//
// author: Stephen Nichols
//

#ifndef _FRIENDMGR_HPP_
#define _FRIENDMGR_HPP_

#include "../global/list.hpp"
#include "../global/tree.hpp"

// define external classes
class RMPlayer;

//
// CFriend: This class represents a single entry in the CFriendMgr
//
class CFriend 
{
protected:
	//
	// Pointer to the rmplayer that represents this friend (only set if player
	// is logged in).
	//
	RMPlayer *m_pPlayer;

	//
	// Pointer to the list of logged in players that want to watch our 
	// player.
	//
	LinkedList *m_pWatcherList;

	//
	// Pointer to the list of LinkedElement objects that represent m_pPlayer's
	// m_pWatcherList containment.  Each m_pWatcherList that m_pPlayer is 
	// added to adds its LinkedElement to this list.  This allows for rapid
	// erasure of m_pPlayer from all m_pWatcherLists.
	//
	LinkedList *m_pElementList;

public:
	CFriend();
	virtual ~CFriend();

	// call to add a player to the watcher list
	bool AddWatcher ( RMPlayer *pPlayer );

	// call to delete a player from the watcher list
	bool DelWatcher ( RMPlayer *pPlayer );

	// call to see if they are being watched by this player
	bool beingWatchedBy( RMPlayer* pPlayer );

	// call to add a new LinkedElement to our element list
	bool AddElement ( LinkedElement *pElement );

	// call to delete a LinkedElement from our element list
	bool DelElement ( LinkedElement *pElement );

	// handle a player logging in...
	bool HandleLogin ( RMPlayer *pPlayer );

	// handle a player logging out...
	bool HandleLogout ( RMPlayer *pPlayer );

	// send the busy indicator to my watchers
	void SendBusy ( void );

	// send the unbusy indicator to my watchers
	void SendUnbusy ( void );

	// return TRUE if we are at (or exceeding) our element capacity
	bool IsAtCapacity ( void );
};

//
// CFriendMgr: This class represents the global set of friend relationships.
//
class CFriendMgr
{
protected:
	//
	// This is the binary tree of all known CFriend objects.  Each CFriend
	// object is keyed off of the friend's character name.
	//
	BinaryTree m_FriendTree;

public:
	CFriendMgr();
	virtual ~CFriendMgr();

	// call to return the CFriend object for the given name...
	CFriend *FindFriend ( char *pPlayerName );
};

// global pointer to the friend manager
extern CFriendMgr *g_pFriendMgr;

#endif
