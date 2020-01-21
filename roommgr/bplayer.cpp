/*
	BPlayer class	
	author: Stephen Nichols
*/

#include "bplayer.hpp"
#include "roommgr.hpp"

BPlayer::BPlayer ( WorldObject *obj ) : WorldObjectBase ( obj )
{
	strcpy ( loginName, "" );
	type = _BPLAYER;
	beenPurged = FALSE;
	login = password = NULL;
}

BPlayer::~BPlayer()
{
	setLoginName ( NULL );
	setPassword ( NULL );
}

void BPlayer::copy ( WorldObjectBase *theBase )
{
	BPlayer *base = (BPlayer *)theBase;

	strcpy ( loginName, base->loginName );

	/* copy the character list */
	LinkedElement *element = base->characterList.head();

	while ( element ) {
		WorldObject *object = (WorldObject *)element->ptr();

		WorldObject *character = new WorldObject ( object );
		characterList.add ( character );

		element = element->next();
	}	
}

void BPlayer::buildCharacterInfoPacket ( PackedData *packet )
{
	/* put this characterList size into the packet */
	packet->putByte ( characterList.size() );

	/* put each character in the packet */
	LinkedElement *element = characterList.head();

	while ( element ) {
		WorldObject *object = (WorldObject *)element->ptr();
		object->buildPacket ( packet, 1 );

		element = element->next();
	}
}

void BPlayer::addCharacter ( WorldObject *object )
{
	characterList.add ( object );
}

void BPlayer::delCharacter ( WorldObject *object )
{
	characterList.del ( object );

	LinkedElement *element = charNames.head();

	char *name = object->getName();

	while ( element ) {
		StringObject *str = (StringObject *)element->ptr();

		if ( !strcasecmp ( str->data, name ) ) {
			charNames.del ( str );
			delete str;

			break;
		}
			
		element = element->next();
	}
}

int BPlayer::ownsCharacter ( WorldObject *object )
{
	int retVal = characterList.contains ( object );
	return retVal;
}

int BPlayer::ownsCharacter ( int servID )
{
	int retVal = ownsCharacter ( roomMgr->findObject ( servID ) );
	return retVal;
}

void BPlayer::purgeAllBut ( WorldObject *character )
{
	LinkedElement *element = characterList.head();
	beenPurged = TRUE;

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		element = element->next();

		if ( obj != character ) {
			obj->player = NULL;
			characterList.del ( obj );

			delete obj;
		}		
	}
}

void BPlayer::setLoginName ( char *str )
{
	if ( login ) {
		free ( login );
		login = NULL;
	}

	if ( str ) 
		login = strdup ( str );
}

void BPlayer::setPassword ( char *str )
{
	if ( password ) {
		free ( password );
		password = NULL;
	}

	if ( str )
		password = strdup ( str );
}

void BPlayer::addCharName ( char *str )
{
	char *name = strdup ( str );
	strlower ( name );

	StringObject *strObj = new StringObject ( name );
	charNames.add ( strObj );

	free ( name );
}
