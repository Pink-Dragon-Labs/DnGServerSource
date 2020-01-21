//
// audit
//
// File that handle generically stepping through all accounts on the system and
// alowing the progammer to do *something* to the characters.
//
// author: Bubba
//

#include "audit.hpp"
#include "roommgr.hpp"

AuditProcessor::AuditProcessor()
{
	player = NULL;
	dir = NULL;
	directory = NULL;
	loadHouses = 1;
	loadCharacters = 1;
	doDisplay = 1;

	rmplayer = new RMPlayer;
}

AuditProcessor::~AuditProcessor()
{
	characters.release();
	houses.release();

	rmplayer->character = NULL;
	rmplayer->player = NULL;
	delete rmplayer;

	close();
}

void AuditProcessor::open ( char *directory )
{
	close();

	if ( directory ) {
		dir = opendir ( directory );
		this->directory = strdup ( directory );

		if ( strstr ( directory, "12Month" ) )
			rmplayer->accountType = _12MONTH;
		else
			rmplayer->accountType = _TRIAL;
	}
}

void AuditProcessor::close ( void )
{
	if ( dir ) {
		closedir ( dir );
		dir = NULL;
	}

	if ( directory ) {
		free ( directory );
		directory = NULL;
	}
}

int AuditProcessor::next ( void )
{
#if 0
	struct dirent *dirp;
	char *name;
	File file;
	LinkedElement *element;

	rmplayer->tossing = 1;

	//
	// cleanup old loaded folks
	//
	if ( player ) {
		delete player;
		player = NULL;
	}

	if ( loadCharacters ) {
		element = characters.head();

		while ( element ) {
			LinkedElement *next = element->next();
			delete element->ptr();
			element = next;
		}
	}

	element = houses.head();

	while ( element ) {
		LinkedElement *next = element->next();

		if ( element->ptr() )
			delete element->ptr();

		element = next;
	}

	characters.release();
	houses.release();

	rmplayer->tossing = 0;

	while ( (dirp = readdir ( dir )) != NULL ) {
		name = dirp->d_name;
			
		if ( name[0] == '.' )
			continue;

		break;
	}

	if ( !dirp )
		return -1;

	char *dotPtr = strchr ( name, '.' );	

	if ( !dotPtr )
		return next(); 

	*dotPtr = 0;

	char *login = name;
	char *password = dotPtr+1;

	strcpy ( this->login, login );
	strcpy ( this->password, password );

	if ( doDisplay )
		logDisplay ( "got player: '%s', '%s'", login, password );

	char filename[1024];
	sprintf ( sizeof ( filename ), filename, "%s%s.%s", directory, login, password );

	file.open ( filename );
	int result = loadValidatedPlayer ( rmplayer, login, password, &file, 0 ); 
	file.close();

	if ( result < 0 ) {
		logDisplay ( "got %d from loadValidatedPlayer!", result );
		return next(); 
	}

	// set player object pointer
	player = rmplayer->player;

	BPlayer *bplayer = (BPlayer *)player->getBase ( _BPLAYER ); 

	if ( bplayer ) {
		element = bplayer->charNames.head(); 

		if ( doDisplay ) 
			logDisplay ( "got %d characters %d", bplayer->charNames.size(), gFileHandles.val() );

		while ( element ) {
			StringObject *obj = (StringObject *)element->ptr();
			element = element->next();

			sprintf ( sizeof ( filename ), filename, "../data/characters/%s", obj->data );

			if ( loadCharacters ) {
				WorldObject *character = loadCharacterData ( filename, login );

				if ( doDisplay )
					logDisplay ( "\tcharacter '%s', 0x%p", obj->data, character );

				if ( character ) {
					characters.add ( character );
				}
				
			} else {
				characters.add ( obj );
			}

			if ( loadHouses ) {
				Building *house = findHouse ( obj->data );

				if ( house ) {
					houses.add ( house );
				} else {
					houses.add ( NULL );
				}
			}
		}
	} else {
		logDisplay ( "no bplayer!" );
	}
#endif

	return 0;
}
