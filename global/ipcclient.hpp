//
// ipcclient
//
// This module contains the IPCClient class and supporting structures.
//
// author: Stephen Nichols
//

#ifndef _IPCCLIENT_HPP_
#define _IPCCLIENT_HPP_

#include "ipcmsghandler.hpp"

// declare these classes so compile errors are avoided
class IPCServer;
class PackedData;
class IPCEncryption;

class RMPlayer;
//
// The IPCClient class represents a single connected client to a server.
//

class IPCClient : public IPCMsgHandler
{
	unsigned int lastRecvTime;
	int _uid, _handle;

	IPCEncryption*	m_pDecrypt;
	IPCEncryption*	m_pEncrypt;

	int	m_nIP;

public:
	IPCClient();
	virtual ~IPCClient();

	// this member provides access to the _uid property
	inline int &uid ( void ) { return ( _uid ); };

	// this member provides access to the _handle property
	inline int &handle ( void ) { return ( _handle ); };

	// this member initializes the ip property
	void setIP ( void );

	// this member provides access to the ip property
	int IP ( void ) { return m_nIP; }

	// this member provides access to the message queue's size
	inline int msgQueueSize ( void ) { return msgQueue.size(); };

	// this member resets all of the properties of this client
	void reset ( void );

	// this member makes a socket connection to the provided machine
	int makeConnection ( char *machine, char *serviceName );

	// this member sends a message via the provided memory
	int send ( char *buffer, int size );

	// this member sends a typed message via the provided memory
	int send ( int type, void *buffer, int size );

	// this member sends a PackedData
	int send ( int type, PackedData *packet );

	// this member sends a message via the provided memory
	int BLE_send ( char *buffer, int size );

	// this member sends a typed message via the provided memory
	int BLE_send ( int type, void *buffer, int size );

	// this member sends a PackedData
	int BLE_send ( int type, PackedData *packet );

	// this member receives a message from the socket
	IPCMessage *receive ( void );

	// this function is called to process this client connection
	virtual void doit ( void );

	// this member is called to process a message
	virtual void handleMessage ( IPCMessage *msg );

	// this member sends a printf style string over the connected socket
	void printf ( const char *format, ... );

	// this member closes the connection
	void close ( void );

	// this member adds a message to the message queue
	void addMsg ( IPCMessage *msg );

	// this member tries to send the next message on the message queue
	int sendNextMsg ( void );

	// this member tries to read the next incoming message from the socket
	int getNextMsg ( void );

	// this member is the queue of outgoing messages for this client
	LinkedList msgQueue;

	// this is the pointer to the message that is currently being received
	IPCMessage *newMsg;

	// this is used by application code to associate this client with a player object
	RMPlayer* player;

	// this member indicates when a client is deemed dead and should be hung up on
	int isDead;

	// this member points to the server that this client belongs to
	IPCServer *server;

	// this member is set if this client is added to an outgoingClientList
	LinkedElement *outgoingClientListElement;

	// this is the poll result
	int revents;

	// maximum incoming message size from this client (-1 unlimited)
	int maxMsgSize;

	// This secures this client with encryption.
	void secure();
};

#endif
