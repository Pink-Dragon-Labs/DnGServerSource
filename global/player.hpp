/*
	player class and supporting code
	author: Stephen Nichols
*/

#ifndef _PLAYER_HPP_
#define _PLAYER_HPP_

#include "ipc.hpp"
#include "hash.hpp"

#include "new.hpp"

#define _PLAYER_NAME_SIZE	32

class PackedData;
class PackedMsg;

class Player : public HashableObject
{
public:
	Player();
	virtual ~Player();

	char name[_PLAYER_NAME_SIZE];
	int servID, handlesMsgs;
	IPCClient *owner;

#if 0
	LinkedList msgQueue;
#endif

	/* hash class interface */
	virtual void setIntHashValue ( void ) { _intHashValue = servID; };
	virtual void setCharHashValue ( void ) { _charHashValue = hashString ( name ); };
	virtual int hashMatch ( int val ) { return servID == val; };
	virtual int hashMatch ( char *ptr ) { return !strcmp ( name, ptr ); };

	/* message handling */
#if 0
	void addMsg ( IPCMessage *msg );
	IPCMessage *getMsg ( void );

	virtual void handleMsg ( IPCMessage *msg );
#endif
};

#define _REGISTRY_HASH_SIZE	256

class PlayerRegistry
{
protected:
	HashTable *_hashTable;
	LinkedList _players;

public:
	PlayerRegistry();
	virtual ~PlayerRegistry();

	// return the number of players
	inline int numPlayers ( void ) { return _players.size(); };

	// return the list of players
	inline LinkedList *players ( void ) { return &_players; };

	/* handle player list management */
	virtual void addPlayer ( Player *player );
	virtual void deletePlayer ( Player *player );

	/* handle searching for a player */
	Player *findPlayer ( char *name );
	Player *findPlayer ( int servID );
};

class PlayerServer : public IPCServer, public PlayerRegistry 
{
public:
//	// this member sends a typed message to a client
//	virtual int send ( int type, void *buffer, int size, IPCClient *client );

	/* special message sending functions */
	virtual void sendTo ( int command, void *msg, int size, Player *player );
	virtual void sendTo ( int command, void *msg, int size, char *name );
	virtual void sendTo ( int command, void *msg, int size, int servID );
	virtual void sendTo ( int command, PackedData *msg, Player *player );
	virtual void sendToAll ( int command, void *msg, int size );
	virtual void sendToAll ( int command, void *msg, int size, Player *player );

	virtual void sendACK ( int command, Player *player );
	virtual void sendACK ( int command, char *name );
	virtual void sendACK ( int command, int servID );

	virtual void sendACK ( int command, int info, Player *player );
	virtual void sendACK ( int command, int info, char *name );
	virtual void sendACK ( int command, int info, int servID );

	virtual void sendNAK ( int command, int why, Player *player );
	virtual void sendNAK ( int command, int why, char *name );
	virtual void sendNAK ( int command, int why, int servID );
};

#endif
