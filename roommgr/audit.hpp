//
// audit
//
// File that handle generically stepping through all accounts on the system and
// alowing the progammer to do *something* to the characters.
//
// author: Bubba
//

#ifndef _AUDIT_HPP_
#define _AUDIT_HPP_

#include <dirent.h>
#include "../global/list.hpp"

class WorldObject;
class RMPlayer;

class AuditProcessor : public ListObject
{
public:
	AuditProcessor();
	virtual ~AuditProcessor();

	void open ( char *directory );
	void close ( void );

	int next ( void );

	WorldObject *player;
	LinkedList characters;
	LinkedList houses;

	DIR *dir;
	char *directory;
	char login[256], password[256];
	RMPlayer *rmplayer;
	int loadHouses, loadCharacters, doDisplay;
};

#endif
