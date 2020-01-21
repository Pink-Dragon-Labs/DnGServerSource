//
// ipcserver
//
// This module contains the IPCServer class and supporting structures.
//
// author: Stephen Nichols
//

#ifndef _IPCSERVER_HPP_
#define _IPCSERVER_HPP_

#include "counter.hpp"

class IPCServer : public IPCMsgHandler
{
	// this is the socket that this server is listening on
	int _handle;

	// this is the name of the port that this server is listening on
	char *_name;

	// this is the list of connected clients
	LinkedList _clientList;

	// this is the number of connections
	Counter _connections;

public:
	IPCServer();
	IPCServer ( char *name );
	virtual ~IPCServer();

	// this member provides access to the _handle property
	inline int &handle ( void ) { return ( _handle ); };

	// this member provides access to the _name property
	inline char *&name ( void ) { return ( _name ); };

	// this member provides access to the number of connections
	inline int connections ( void ) { return _connections.val(); };

	// this member provides access to the client list
	inline LinkedList *clientList ( void ) { return &_clientList; };

	// this member adds a new message to the message queue
	inline void addMessage ( IPCMessage *msg ) { _msgQueue.add ( msg ); };

	// this inits the server to wait for incoming connections from the given port
	void init ( char *port, char* host = NULL );

	// this member resets all data members
	void reset ( void );

	// this member brings in a new connection and creates a client for it
	virtual IPCClient *waitForConnection ( void );

	// this member sends a raw message to a client
	virtual int send ( void *buffer, int size, IPCClient *client );

	// this member sends a typed message to a client
	virtual int send ( int type, void *buffer, int size, IPCClient *client );

	// this member sends a packed data to a client
	virtual int send ( int type, PackedData *packet, IPCClient *client );

	// this member sends a raw message to a client
	virtual int BLE_send ( void *buffer, int size, IPCClient *client );

	// this member sends a typed message to a client
	virtual int BLE_send ( int type, void *buffer, int size, IPCClient *client );

	// this member sends a packed data to a client
	virtual int BLE_send ( int type, PackedData *packet, IPCClient *client );
	
	// this member dispatches any incoming messages
	int dispatchMessages ( void );

	// this member handles incoming messages
	virtual void handleMessage ( IPCMessage *msg );

	// this member adds a client to the client list
	void addClient ( IPCClient *client );

	// this member deletes a client from the client list
	void delClient ( IPCClient *client );

	// This member function processes this server's connection.
	virtual void doit ( void );

	// this is the list of clients with incoming messages
	LinkedList _incomingClientList;

	// this is the list of clients with outgoing messages
	LinkedList _outgoingClientList;

	// this is the list of clients with defunct connections
	LinkedList _deadClientList;

	// this is the message queue
	LinkedList _msgQueue;
};

#endif

