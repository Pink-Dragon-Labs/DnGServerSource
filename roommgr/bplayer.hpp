/*
	BPlayer class
	author: Stephen Nichols
*/

#ifndef _BPLAYER_HPP_
#define _BPLAYER_HPP_

#include "wobjectbase.hpp"

class WorldObject;

#define _MAX_CHARACTERS	4

typedef struct {
	char loginName[17];
	int numCharacters;
	int characters[_MAX_CHARACTERS];
	int owner;
} DBPlayer;

class BPlayer : public WorldObjectBase
{
public:
	LinkedList characterList, charNames;
	char loginName[17];
	int beenPurged;

	char *login, *password;

	BPlayer ( WorldObject *obj );
	virtual ~BPlayer();

	void copy ( WorldObjectBase *base );

	/* add a new character to my list of characters */
	void addCharacter ( WorldObject *object );

	/* delete a character from my list */
	void delCharacter ( WorldObject *object );

	/* miscellaneous members */
	void buildCharacterInfoPacket ( PackedData *packet );
	int ownsCharacter ( WorldObject *object );
	int ownsCharacter ( int servID );

	/* purge all characters but the specified one */
	void purgeAllBut ( WorldObject *character );

	void setLoginName ( char * );
	void setPassword ( char * );

	void addCharName ( char * );
};

#endif
