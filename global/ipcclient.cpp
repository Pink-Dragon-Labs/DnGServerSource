//
// ipcclient
//
// This module contains the IPCClient class and supporting structures.
//
// author: Stephen Nichols
//

#include "system.hpp"

//
//	default constructor
//
IPCClient::IPCClient()
{
	reset();
}

//
//	destructor
//
IPCClient::~IPCClient()
{
	if ( newMsg )
		delete newMsg;

	if ( m_pDecrypt )
		delete m_pDecrypt;

	if ( m_pEncrypt )
		delete m_pEncrypt;
	
	close();
	reset();
}

//
//	reset data members 
//
void IPCClient::reset ( void )
{
	setPtrType ( this, _MEM_CLIENT );

	handle() = -1;
	newMsg = NULL;
	player = NULL;
	isDead = 0;
	server = NULL;
	revents = 0;
	maxMsgSize = 10000;

	m_pDecrypt = NULL;
	m_pEncrypt = NULL;

	m_nIP = 0;
}

// close this client socket
void IPCClient::close ( void )
{
	if ( handle() != -1 ) {
		::close ( handle() );
		gFileHandles.decrement();
		gIPCPollMgr.delClient ( this );
		handle() = -1;
	}
}

//
//	handle making a client-server connection to a well known point 
//
int IPCClient::makeConnection ( char *machineName, char *name )
{
	int fd = ConnectPortTCP ( machineName, name );

	if ( fd != -1 ) {
		handle() = fd;
		gFileHandles.increment();
		gIPCPollMgr.addClient ( this );

		SetPortTCP ( fd, O_NONBLOCK );
	}

	// toss any old incoming message on reconnect
	if ( newMsg ) {
		delete newMsg;
		newMsg = NULL;
	}

	return fd;
}

//
//	send a message to an ipc server 
//
int IPCClient::send ( char *buffer, int size )
{
	return send ( _IPC_CLIENT_SEND, buffer, size );
}

//
// send a typed message to the server
//
int IPCClient::send ( int type, void *buffer, int size )
{
	IPCStats::Msgs[ type ].addOutbound( size, (int*) buffer );

	IPCMessage *message = new IPCMessage ( type, (char *)buffer, size, this );

	if ( m_pEncrypt )
		m_pEncrypt->Encode ( message );

	addMsg ( message );

	return 0;
}

//
// send a packed data to the server
//
int IPCClient::send ( int type, PackedData *packet )
{
	return send ( type, (void *)packet->data(), packet->size() );
}

//
//	send a message to an ipc server 
//
int IPCClient::BLE_send ( char *buffer, int size )
{
	return BLE_send ( _IPC_CLIENT_SEND, buffer, size );
}

//
// send a typed message to the server
//
int IPCClient::BLE_send ( int type, void *buffer, int size )
{
	IPCStats::Msgs[ type ].addOutbound( size, (int*) buffer );

	IPCMessage *message = new IPCBLE_Message ( type, (char *)buffer, size, this );

	if ( m_pEncrypt )
		m_pEncrypt->Encode ( message );

	addMsg ( message );

	return 0;
}

//
// send a packed data to the server
//
int IPCClient::BLE_send ( int type, PackedData *packet )
{
	return BLE_send ( type, (void *)packet->data(), packet->size() );
}

//
// this member reads the next message from the socket
//
int IPCClient::getNextMsg ( void )
{
	if ( !newMsg ) {
		newMsg = new IPCMessage;
		newMsg->from() = this;
	}

	int nRet = newMsg->read ( handle(), maxMsgSize );

	if ( !nRet && m_pDecrypt ) {
//		newMsg->Dump( "ENC RECV" );
				
		newMsg = m_pDecrypt->Decode ( newMsg );

//		newMsg->Dump( "DEC RECV" );
	}

	return nRet;
}

//
// this member receives a message from the connected socket
//
IPCMessage *IPCClient::receive ( void )
{
	IPCMessage *message = new IPCMessage;

	while ( 1 ) {
		int result = message->read ( handle(), maxMsgSize );

		if ( result == -2 ) {
			delete message;
			message = NULL;
			break;
		} 

		if ( result == 0 )
			break;
	}

	return ( message );
}

//
// this member handles incoming messages
//
void IPCClient::handleMessage ( IPCMessage *message )
{
	switch ( message->type() ) {
		case _IPC_SERVER_HUNG_UP: {
			close();
		}

		break;
	}
}

// process messages on this client
void IPCClient::doit ( void )
{
	if ( handle() != -1 ) {
		// check for hang up
		if ( revents & (POLLHUP|POLLERR|POLLNVAL) ) {
			IPCMessage *msg = new IPCMessage ( _IPC_SERVER_HUNG_UP, NULL, 0, this );
			handleMessage ( msg );
			delete msg;
			close();
		} 

		else { // if ( revents & POLLIN ) {
			while ( 1 ) {
				int result = getNextMsg();

				// if this happens, the connection is dead
				if ( (result == -1) && (errno == EINTR) ) {
					result = -2;
				} 

				if ( result == -3 )
					result = -2;

				if ( result == -1 ) {
					break;
				}

				else if ( result == -2 ) {
					IPCMessage *msg  = new IPCMessage ( _IPC_SERVER_HUNG_UP, NULL, 0, this );
					handleMessage ( msg );
					delete msg;
					close();

					break;
				}

				else if ( result == 0 ) {
					handleMessage ( newMsg );
					delete newMsg;
					newMsg = NULL;
				}
			}
		}

		// send the next outgoing message
		int val = sendNextMsg();

		if ( val == -2 ) {
			IPCMessage *msg  = new IPCMessage ( _IPC_SERVER_HUNG_UP, NULL, 0, this );
			handleMessage ( msg );
			delete msg;
			close();
		}
	}

	revents = 0;
}

//
// this member writes a formatted string over the connected socket
//
void IPCClient::printf ( const char *format, ... )
{
	va_list args;
	char output[1024];

	va_start ( args, format );
	vsprintf ( sizeof ( output ), output, format, args );
	va_end ( args );

	::write ( handle(), output, strlen ( output ) );
}

//
// this member adds a message to the message queue
//
void IPCClient::addMsg ( IPCMessage *msg )
{
	msgQueue.add ( msg );
}

//
// this member tries to send the next message on the message queue
//
int IPCClient::sendNextMsg ( void )
{
	int retVal = -1;

	if ( msgQueue.size() ) {
		LinkedElement *element = msgQueue.head();

		IPCMessage *msg = (IPCMessage *)element->ptr();

		if ( msg ) {
			retVal = msg->write ( handle() );

			if ( retVal == 0 ) {
				msgQueue.delElement ( element );
				delete msg;
			}
		}
	}

	return retVal;
}

void IPCClient::setIP() {
	struct sockaddr_in sock;
	int size = sizeof ( sock );

	memset( &sock, 0, size );

	getpeername ( handle(), (struct sockaddr *)&sock, (socklen_t*) &size );
  
	m_nIP = htonl( sock.sin_addr.s_addr );
}

void IPCClient::secure() {
	m_pDecrypt = IPCEncryption::Secure( this, FALSE );
	m_pEncrypt = IPCEncryption::Secure( this, TRUE );

	PackedMsg response;
	response.putACKInfo ( _IPC_CLIENT_CONNECTED );

	IPCMessage* pMsg = new IPCMessage ( _IPC_PLAYER_ACK, (char *)response.data(), response.size(), this );

//	pMsg->Dump( "Before encrypt" );

	if ( m_pEncrypt )
		m_pEncrypt->Encode ( pMsg );

//	pMsg->Dump( "After  encrypt" );

	addMsg ( pMsg );
}
