//
// ipcpollmgr
//
// This module contains the IPCPollMgr class.
//
// author: Stephen Nichols
//

#ifndef _IPCPOLLMGR_HPP_
#define _IPCPOLLMGR_HPP_

#include "list.hpp"

// declare the following classes to avoid compile error
class IPCClient;

// define the size of the global poll array
#define _IPCPOLL_MAX_HANDLE	5000

class IPCPollMgr
{
public:
	IPCPollMgr();
	virtual ~IPCPollMgr();

	// this member is the maximum registered handle
	int maxHandle;

	// this member is the poll array
	pollfd pollArray[_IPCPOLL_MAX_HANDLE];

	// this is the client list
	IPCClient *clients[_IPCPOLL_MAX_HANDLE];

	// this member is called to poll all registered file handles
	void doit ( void );

	// this member is called to add a client to the poll manager
	void addClient ( IPCClient *client );

	// this member is called to delete a client from the poll manager
	void delClient ( IPCClient *client );
};

// this is the global poll manager
extern IPCPollMgr gIPCPollMgr;

#endif
