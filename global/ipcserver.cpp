//
// ipcserver
//
// This module contains the IPCServer class and supporting structures.
//
// author: Stephen Nichols
//

#include "system.hpp"

// 
//	default constructor
//
IPCServer::IPCServer()
{
	reset();
}

//
// this constructor passes the name on to init
//
IPCServer::IPCServer ( char *name )
{
	reset();
	init ( name );
}

//
// destructor
//
IPCServer::~IPCServer()
{
	// toss the name if we have one
	if ( name() ) {
		free ( name() );
		name() = NULL;
	}

	if ( handle() != -1 ) {
		close ( handle() );
		gFileHandles.decrement();
	}

	LinkedElement *element = _clientList.head();

	while ( element != NULL ) {
		IPCClient *client = (IPCClient *)element->ptr();
		element = element->next();

		delete client;
	}

//	_outgoingClientList.release();
	_incomingClientList.release();
	_deadClientList.release();

	reset();
}


// this member resets all data members
void IPCServer::reset ( void )
{
	name() = NULL;
	handle() = -1;

	_connections.decrement ( _connections.val() );
}

// this inits the server to wait for incoming connections from the given port
void IPCServer::init ( char *port, char* host )
{
	this->name() = strdup ( port );
	handle() = OpenPortTCP ( port, host );
	SetPortTCP ( handle(), O_NONBLOCK );
	gFileHandles.increment();
}

// this member brings in a new connection and creates a client for it
IPCClient *IPCServer::waitForConnection ( void )
{
	IPCClient *retVal = NULL;

	int clientFD = AcceptPortTCP ( handle() );

	if ( clientFD != -1 ) {
		SetPortTCP ( clientFD, O_NONBLOCK );
		IPCClient *client = new IPCClient;
		client->handle() = clientFD;
		gFileHandles.increment();

		addClient ( client );

		retVal = client;
	}

	return retVal;
}

// this member adds a client to the client list
void IPCServer::addClient ( IPCClient *client )
{
	client->server = this;
	client->setIP();

	_connections.increment();
}

// this member deletes a client from the client list
void IPCServer::delClient ( IPCClient *client )
{
	client->server = NULL;
	_connections.decrement();
}

// this member sends a raw message to a client
int IPCServer::send ( void *buffer, int size, IPCClient *client )
{
	return send ( _IPC_SERVER_SEND, buffer, size, client );
}

int IPCServer::send ( int type, PackedData *packet, IPCClient *client )
{
	return send ( type, packet->data(), packet->size(), client );
}

// this member sends a typed message to a client
int IPCServer::send ( int type, void *buffer, int size, IPCClient *client )
{
	return client->send( type, buffer, size );

	client->addMsg ( new IPCMessage ( type, buffer, size, this, client ) );

//	if ( !client->outgoingClientListElement )
//		client->outgoingClientListElement = _outgoingClientList.add ( client );

	return 1;
}

// this member sends a raw message to a client
int IPCServer::BLE_send ( void *buffer, int size, IPCClient *client )
{
	return BLE_send ( _IPC_SERVER_SEND, buffer, size, client );
}

int IPCServer::BLE_send ( int type, PackedData *packet, IPCClient *client )
{
	return BLE_send ( type, packet->data(), packet->size(), client );
}

// this member sends a typed message to a client
int IPCServer::BLE_send ( int type, void *buffer, int size, IPCClient *client )
{
	return client->BLE_send( type, buffer, size );
}

// this member function is called to maintain this server's connection
void IPCServer::doit ( void )
{
	if ( handle() != -1 ) {
		// handle any incoming connections
		while ( gFileHandles.val() < gMaxConnectCount ) {
			IPCClient *theClient = waitForConnection();

			if ( !theClient ) 
				break;

			addMessage ( new IPCMessage ( _IPC_CLIENT_CONNECTED, NULL, 0, theClient, (void *)-1 ) );
		}
	}

	// handle any messages that are in the queue
	dispatchMessages();

	// send outgoing messages
//	LinkedElement *element = _outgoingClientList.head();
	LinkedElement *element = _clientList.head();

	while ( element ) {
		IPCClient *client = (IPCClient *)element->ptr();
		element = element->next();

		if ( client->handle() != -1 ) {
			while ( client->msgQueueSize() ) {
				int result = client->sendNextMsg();

				// if we could not write to this client, stop trying to send messages
				if ( result == -1 )
					break;
	
				// if the connection is deemed dead, close it down
				else if ( result == -2 ) {
					delClient ( client );
					addMessage ( new IPCMessage ( _IPC_CLIENT_HUNG_UP, NULL, 0, client, (void *)-1 ) );
	
					LinkedElement *msgElement = client->msgQueue.head();

					while ( msgElement ) {
						LinkedElement *next = msgElement->next();
						delete msgElement->ptr();
						msgElement = next;
					}
	
					client->msgQueue.release();
					client->close();
	
					break;
				}
			}
		}

//		client->outgoingClientListElement = NULL;
	}

//	_outgoingClientList.release();
	
	// process the dead clients
	element = _deadClientList.head();

	while ( element ) {
		IPCClient *client = (IPCClient *)element->ptr();
		element = element->next();

		if ( client->handle() != -1 ) {
			delClient ( client );

			addMessage ( new IPCMessage ( _IPC_CLIENT_HUNG_UP, NULL, 0, client, (void *)-1 ) );

			client->close();
		}					
	}

	_deadClientList.release();

	// process all clients with incoming messages
	element = _incomingClientList.head();

	while ( element ) {
		IPCClient *client = (IPCClient *)element->ptr();
		element = element->next();

		if ( (client->handle() != -1) ) {
			// read all incoming messages
			while ( 1 ) {
				int nextMsg = client->getNextMsg();

				if ( nextMsg == -1 )
					break;

				if ( nextMsg == -3 ) {
					delClient ( client );

					PackedData packet;
					packet.putByte ( 3 );

 					addMessage ( new IPCMessage ( _IPC_CLIENT_HACKED_MSG, (IPCPMMessage *)packet.data(), packet.size(), client, (void *)-1 ) );
					addMessage ( new IPCMessage ( _IPC_CLIENT_HUNG_UP, NULL, 0, client, (void *)-1 ) );
					client->close();

					break;
				} 

				// if the message could not be read, the client must have hung up
				else if ( nextMsg == -2 ) {
					delClient ( client );
					addMessage ( new IPCMessage ( _IPC_CLIENT_HUNG_UP, NULL, 0, client, (void *)-1 ) );

					client->close();
					break;
				}

				// everything is cool, let's put the new found message on the queue
				else if ( nextMsg == 0 ) {
					addMessage ( client->newMsg );
					client->newMsg = NULL;
				}
			}
		}	
	}

	_incomingClientList.release();

	// handle any messages that are in the queue
	dispatchMessages();
}

// dispatch the messages in the queue
int IPCServer::dispatchMessages ( void )
{
	LinkedElement *element = _msgQueue.head();

	while ( element ) {
		IPCMessage *msg = (IPCMessage *)element->ptr();

		handleMessage ( msg );
		delete msg;

		element = element->next();
	}

	_msgQueue.release();
	
	return TRUE;
}

// this member handles incoming messages
void IPCServer::handleMessage ( IPCMessage *message )
{
	switch ( message->type() ) {
		case _IPC_CLIENT_CONNECTED: {
			IPCClient *client = (IPCClient *)message->from();
			_clientList.addToEnd ( client );
			gIPCPollMgr.addClient ( client );
		}

		break;

		case _IPC_CLIENT_HUNG_UP: {
			IPCClient *client = (IPCClient *)message->from();

			_clientList.del ( client );
//			_outgoingClientList.del ( client );
			_incomingClientList.del ( client );
			_deadClientList.del ( client );

			delete client;
		}			

		break;
	}
}

