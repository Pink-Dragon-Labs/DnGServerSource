/*
	player class and supporting code
	author: Stephen Nichols
*/

#include "system.hpp"

Player::Player()
{
	handlesMsgs = 0;
	name[0] = 0;
	owner = NULL;
	servID = -1;
}

Player::~Player()
{
}

PlayerRegistry::PlayerRegistry()
{
	_hashTable = new HashTable ( _REGISTRY_HASH_SIZE );
}

PlayerRegistry::~PlayerRegistry()
{
	if ( _hashTable ) {
		_hashTable->release();
		delete _hashTable;
		_hashTable = NULL;
	}
}

/* add a player to the registry */
void PlayerRegistry::addPlayer ( Player *player )
{
	if ( _players.contains ( player ) ) 
		crash();

	_hashTable->add ( player );
	_players.add ( player );
}

/* delete a player from the registry */
void PlayerRegistry::deletePlayer ( Player *player )
{
	_hashTable->del ( player );
	_players.del ( player );
}

/* find a player based on name */
Player *PlayerRegistry::findPlayer ( char *name )
{
	return (Player *)_hashTable->findByStr ( name );
}

/* find a player based on servID */
Player *PlayerRegistry::findPlayer ( int servID ) 
{
	return (Player *)_hashTable->findByInt ( servID );
}

//	// this member sends a typed message to a client
//	int PlayerServer::send ( int type, void *buffer, int size, IPCClient *client )
//	{
//		IPCStats::Msgs[ type ].addOutbound( size, (int*) buffer );
//	
//		client->addMsg ( new IPCMessage ( type, buffer, size, this, client ) );
//	
//		return 1;
//	}

/* send a message to a player object */
void PlayerServer::sendTo ( int command, void *msg, int size, Player *player )
{
	IPCPMMessage theMsg;

	if ( msg == NULL ) {
		msg = &theMsg;
		size = sizeof ( theMsg );
	}

	((IPCPMMessage*) msg)->to = player->servID;

	// handle message redirection here
	if ( !player->handlesMsgs ) {
		send ( command, (char *)msg, size, player->owner );
	}	
}

/* send a message to a player by name */
void PlayerServer::sendTo ( int command, void *msg, int size, char *name )
{
	Player *player = findPlayer ( name );
	if ( player )
		sendTo ( command, msg, size, player );
}

/* send a message to a player by servID */
void PlayerServer::sendTo ( int command, void *msg, int size, int servID )
{
	Player *player = findPlayer ( servID );
	if ( player )
		sendTo ( command, msg, size, player );
}

/* send a message to a player object */
void PlayerServer::sendTo ( int command, PackedData *packet, Player *player )
{
	sendTo ( command, packet->data(), packet->size(), player );
}

/* send a message to every player in the registry */
void PlayerServer::sendToAll ( int command, void *msg, int size )
{
	LinkedElement *element = _players.head();

	while ( element != NULL ) {
		Player *player = (Player *)element->ptr();
		sendTo ( command, msg, size, player );
		
		element = element->next();
	}
}

/* send a message to every player except... */
void PlayerServer::sendToAll ( int command, void *msg, int size, Player *exclusion )
{
	LinkedElement *element = _players.head();

	while ( element != NULL ) {
		Player *player = (Player *)element->ptr();

		if ( player != exclusion )
			sendTo ( command, msg, size, player );

		element = element->next();
	}
}

/* send an ACK to a player */
void PlayerServer::sendACK ( int command, Player *player )
{
	IPCPMAckMsg ack;

	ack.from = 0;
	ack.to = player->servID;
	ack.command = command;

	sendTo ( _IPC_PLAYER_ACK, &ack, sizeof ( ack ), player );
}

void PlayerServer::sendACK ( int command, char *name )
{
	Player *player = findPlayer ( name );

	if ( player )
		sendACK ( command, player );
}

void PlayerServer::sendACK ( int command, int servID )
{
	Player *player = findPlayer ( servID );

	if ( player )
		sendACK ( command, player );
}

/* send an ACK to a player */
void PlayerServer::sendACK ( int command, int info, Player *player )
{
	IPCPMAckMsg ack;

	ack.from = 0;
	ack.to = player->servID;
	ack.command = command;
	ack.info = info;

	sendTo ( _IPC_PLAYER_ACK, &ack, sizeof ( ack ), player );
}

void PlayerServer::sendACK ( int command, int info, char *name )
{
	Player *player = findPlayer ( name );

	if ( player )
		sendACK ( command, info, player );
}

void PlayerServer::sendACK ( int command, int info, int servID )
{
	Player *player = findPlayer ( servID );

	if ( player )
		sendACK ( command, info, player );
}
/* send a NAK to a player */
void PlayerServer::sendNAK ( int command, int why, Player *player )
{
	IPCPMNakMsg nak;

	nak.from = 0;
	nak.to = player->servID;
	nak.command = command;
	nak.info = why;

	sendTo ( _IPC_PLAYER_NAK, &nak, sizeof ( nak ), player );
}

void PlayerServer::sendNAK ( int command, int why, char *name )
{
	Player *player = findPlayer ( name );

	if ( player )
		sendNAK ( command, why, player );
}

void PlayerServer::sendNAK ( int command, int why, int servID )
{
	Player *player = findPlayer ( servID );
	
	if ( player )
		sendNAK ( command, why, player );
}
