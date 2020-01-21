//
// ipcpollmgr
//
// This module contains the IPCPollMgr class.
//
// author: Stephen Nichols
//

#include "system.hpp"

// this is the global poll manager
IPCPollMgr gIPCPollMgr;

IPCPollMgr::IPCPollMgr()
{
	for ( int i=0; i<_IPCPOLL_MAX_HANDLE; i++ ) {
		pollArray[i].fd = -1;
		pollArray[i].events = POLLIN;
		pollArray[i].revents = 0;
		clients[i] = NULL;
	}

	maxHandle = 0;
}

IPCPollMgr::~IPCPollMgr()
{
}

// this member is called to poll all registered file handles
void IPCPollMgr::doit ( void )
{
	// poll the clients
	int count = poll ( pollArray, maxHandle + 1, 5 );

	if ( count < 1 )
		return;

	// step through all of the clients and handle any states that have occured
	for ( int i=0; i<=maxHandle; i++ ) {
		IPCClient *client = clients[i];

		if ( client ) {
			IPCServer *server = client->server;
			client->revents = pollArray[i].revents;

			// check for invalid connection
			if ( pollArray[i].revents & (POLLHUP | POLLERR | POLLNVAL) || client->isDead ) {
				if ( server ) {
					server->_deadClientList.add ( client );
				}
			}

			// check for incoming data
			else if ( pollArray[i].revents & POLLIN ) {
				if ( server ) {
					server->_incomingClientList.add ( client );
				}
			}
		}
	}
}

// this member is called to add a client to the poll manager
void IPCPollMgr::addClient ( IPCClient *client )
{
	int handle = client->handle();

	if ( handle < 0 || handle >= _IPCPOLL_MAX_HANDLE )
		fatal ( "IPCPollMgr::addClient with handle of %d!", handle );

	// add the client
	clients[handle] = client;
	pollArray[handle].fd = handle;

	// update max handle
	if ( handle > maxHandle )
		maxHandle = handle;
}

// this member is called to delete a client from the poll manager
void IPCPollMgr::delClient ( IPCClient *client )
{
	int handle = client->handle();

	if ( handle < 0 || handle >= _IPCPOLL_MAX_HANDLE )
		fatal ( "IPCPollMgr::delClient with handle of %d!", handle );

	// delete the client
	clients[handle] = NULL;
	pollArray[handle].fd = -1;

	// update max handle
	if ( handle == maxHandle ) {
		maxHandle--;

		if ( maxHandle < 0 )
			maxHandle = 0;
	}
}
