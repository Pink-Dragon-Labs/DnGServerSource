#include "rmroom.hpp"
#include "roommgr.hpp"
#include "globals.hpp"

char gTownNames [ _TOWN_MAX ][ 15 ] = {
		"Leinster",
		"East Leinster",
		"West Leinster",
		"Kurz",
		"Usk",
		"Asgard",
		"Murias",
		"Drune",
		"Silverbrook",
		"Wen",
		"Caer Fandry",
		"Monmouth",
		"Arimathor"
};

LinkedList gEmptyBuildings, gBuildings, gLoadingHouses;

MemoryAllocator gATPInfoAllocator ( sizeof ( ATPInfo ), 100000 );

int gRoomIDTbl[_ROOM_ID_TBL_SIZE], gATPCount = 0;

int allocateRoomID ( void )
{
	for ( int i=0; i<_ROOM_ID_TBL_SIZE; i++ ) {
		if ( gRoomIDTbl[i] != 0xFFFFFFFF ) {
			int value = gRoomIDTbl[i];

			for ( int j=0; j<32; j++ ) {
				int mask = 1 << j;

				if ( !(value &mask) ) {
					gRoomIDTbl[i] |= mask;
					int retVal = ((i << 5) + j) + _DBR_OFFSET;

					return retVal;
				}
			}
		}	
	}

	return -1;
}

void freeRoomID ( int roomID ) 
{
	roomID -= _DBR_OFFSET;

	if ( roomID < 0 ) 
		return;

	int i = roomID >> 5;
	int j = roomID - (i << 5);

	int mask = ~(1 << j);
	gRoomIDTbl[i] &= mask;
}

RMRoom::RMRoom()
{
	number = -1;
	isDungeonEntrance = 0;
	bRubberWalled = 0;
	instanceNum = -1;
	north = south = east = west = up = down = -1;
	exits = 0;
	type = 0;
	flags = 0;
	servID = -1;
	isDead = FALSE;
	title = NULL;
	building = NULL;
	midiFile = -1;
	groupChance = -1;
	numPlayers = 0;
	bAllowAmbush = 1;
	zone = NULL;
	tempID = 0;
	picture = 0;
	for ( int i = 0; i < 6; i++ ) {
		exitCoords[i][0] = -1;
		exitCoords[i][1] = -1;
	}
	setPtrType ( this, _MEM_ROOM );
}

RMRoom::~RMRoom()
{
	gRoomArray->del ( number );

	if ( building && !building->destructing ) {
		building->delRoom ( this );
	}

	building = NULL;

	if ( isDead && servID != -1 ) {
		isDead = FALSE;
	}

	roomMgr->_rooms.del ( this );

	LinkedElement *element = head();

	while ( element ) {
		LinkedElement *next = element->next();

		RMPlayer *player = (RMPlayer *)element->ptr();

		if ( !player->isNPC && gShutdownTimer >= -1 ) {
			fatal ( "trying to destroy a player on disposal of room %d", number );
		} else {
			// clean up combat groups 
			CombatGroup *combatGroup = player->character->combatGroup;

			if ( combatGroup ) {
				gDeadCombats.del ( combatGroup );
				delete combatGroup;
			}

			player->tossing = 1;
			delete player;
		}

		element = next;
	}

	// step through and delete all objects
	element = objects.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		element = element->next();

		if ( zone ) 
			zone->delObject ( obj );
	}

	gCrowdedRooms.del ( this );

	setTitle ( NULL );

	freeRoomID ( number );

	release();
}

// set this room to be a dungeon entrance
void RMRoom::SetDungeonEntrance() {
	isDungeonEntrance = 1;

	// Force dungeon entrances to be no cast/drop/use zones
	flags |= _RM_FLAG_NO_MAGIC | _RM_FLAG_NO_DROP | _RM_FLAG_NO_USE;
}

void RMRoom::delPlayer ( RMPlayer *player, int updateZone )
{
	if ( !player->isNPC ) {
		numPlayers--;
	}

	if ( zone )
		zone->delPlayerFromQueue ( player );
	
	// remove the player from the list
	del ( player );

	if ( zone && updateZone ) {
		if ( player->isNPC )	
			zone->delNPC ( player->character );
		else {
			zone->delPlayer ( player );
		}
	}

	if ( updateZone && building )
		building->delPlayer ( player );

	if ( player->character->sittingOn )
		player->character->sittingOn->beStoodUpOn ( player->character->sittingOn );

	if ( numPlayers ) {
		// tell the rest of the room to destroy the player
		PackedMsg packet;

		packet.putLong ( number );

		// put the servID of the object to toast
		packet.putLong ( player->character->servID );

		roomMgr->sendToRoom ( _IPC_PLAYER_DESTROY_OBJECT, (IPCPMMessage *)packet.data(), packet.size(), this );
	}
}
 
void RMRoom::addPlayer ( RMPlayer *player, PackedData *thePacket, int updateZone )
{
	if ( player->isNPC ) {
		// Force gate keepers to be no cast/drop/use zones
		if ( player->character->getBase ( _BGATE ) ) {
			flags |= _RM_FLAG_NO_MAGIC | _RM_FLAG_NO_DROP | _RM_FLAG_NO_USE;
		}
	} else {
		numPlayers++;
	}

	if ( numPlayers ) {
		PackedMsg msg;
		PackedData *packet = thePacket? thePacket : &msg;

		player->setRoomTitle ( title );
	
		if ( !thePacket ) {
			// make sure this movie packet is attached to ego
			packet->putLong ( -1 );

			// put the room number so the client can validate "stray" movie commands
			packet->putLong ( number );
		}

		packet->putByte ( _MOVIE_CREATE_CHAR );

		// put the size of the packet, so the movie command can be skipped
		packet->putWord ( player->character->calcPacketSize() );

		// put the servID of the character being made
		packet->putLong ( player->character->servID );

		// tack on the actual object information
		player->character->buildPacket ( packet );

		if ( !thePacket ) {
			packet->putByte ( _MOVIE_END );
			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, packet->data(), packet->size(), this );
		}
	}

	if ( zone && updateZone ) {
		if ( player->isNPC ) {
			BNPC *base = (BNPC *)player->character->getBase ( _BNPC );

			if ( base && base->isExternal ) {
				zone->addExternalNPC ( player->character );
			} else {
				zone->addNPC ( player->character );
			}
		} else {
			zone->addPlayer ( player );
		}
	}

	if ( updateZone && building )
		building->addPlayer ( player );

	// keep the room number up to date
	player->character->roomNumber = number;

	objects.del ( player->character );

	add ( player );

	// added to clear teleport
	player->setTeleportRoomNum ( -1 );

	if ( player->isTeleporting )
		player->isTeleporting--;
}

void RMRoom::addObject ( WorldObject *object, int notify )
{
	if ( !object )
		return;

	if ( !objects.contains ( object ) ) {
		object->room = this;
		object->isVisible = 1;

		if ( object->physicalState & _STATE_RIDDLED )
			riddledObjects.add ( object );

		if ( zone )
			zone->addObject ( object );

		if ( building )
			building->addObject ( object );

		objects.add ( object );
		//object->creationTime = getseconds();

		if ( objects.size() >= 64 && number < _DBR_THRESHOLD && gCrowdedRooms.contains ( this ) ) 
			gCrowdedRooms.add ( this );

		if ( notify && numPlayers ) 
			sendObjInfo ( object );
	}
}

WorldObject *RMRoom::addObject ( char *name, int x, int y, int loop, int notify )
{
//	WorldObject *object = new WorldObject ( roomMgr->findClass ( name ) );
	
	WorldObject *object2 = roomMgr->findClass ( name );

	WorldObject *object = new WorldObject ( object2 );

	object->x = x;
	object->y = y;
	object->loop = loop;
	object->addToDatabase();

	addObject ( object, notify );

	return object;
}

void RMRoom::delObject ( WorldObject *object, int notify )
{
	if ( objects.contains ( object ) ) {
		object->room = NULL;

		if ( object->physicalState & _STATE_RIDDLED )
			riddledObjects.del ( object );

		if ( zone )
			zone->delObject ( object );

		if ( building )
			building->delObject ( object );

		objects.del ( object );

		if ( objects.size() < 64 && number < _DBR_THRESHOLD )
			gCrowdedRooms.del ( this );

		if ( notify && numPlayers ) 
			sendObjDestroy ( object, NULL );
	}
}

void RMRoom::sendObjInfo ( WorldObject *obj, RMPlayer *exclusion, PackedData *thePacket )
{
	PackedMsg staticPacket;
	PackedData *packet = thePacket? thePacket : &staticPacket;

	WorldObject *owner = obj->getOwner();
	WorldObject *baseOwner = obj->getBaseOwner();

	if ( !thePacket ) {
		// put the currently active player's servID -- it's a dependency
//		packet->putLong ( (gActivePlayer && gActivePlayer->character)? gActivePlayer->character->servID : -1 );
		packet->putLong ( (baseOwner)? baseOwner->servID : -1 );

		// put the room number so the client can validate "stray" movie commands
		packet->putLong ( number );
	}

	packet->putByte ( _MOVIE_CREATE_OBJ );

	// put the size of the packet, so the movie command can be skipped
	packet->putWord ( obj->calcPacketSize() );

	// put the current owner's servID -- it's a dependency
	packet->putLong ( owner == obj? 0 : owner->servID );

	// tack on the actual object information
	obj->buildPacket ( packet );

	if ( !thePacket ) {
		packet->putByte ( _MOVIE_END );
		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, packet->data(), packet->size(), this, exclusion );
	}
}

void RMRoom::sendObjDestroy ( WorldObject *obj, RMPlayer *exclusion )
{
	if ( obj->servID != -1 ) {
		WorldObject *baseOwner = obj->getBaseOwner();

		PackedMsg packet;
		packet.putLong ( number );
		packet.putLong ( obj->servID );
//		packet.putLong ( (gActivePlayer && gActivePlayer->character)? gActivePlayer->character->servID : -1 );
		packet.putLong ( (baseOwner)? baseOwner->servID : -1 );

		roomMgr->sendToRoom ( _IPC_PLAYER_DESTROY_OBJECT, (IPCPMMessage *)packet.data(), packet.size(), this, exclusion );
	}
}

void RMRoom::buildPacket ( PackedData *packet )
{
	// put the basic room information 
	packet->putLong ( number );
	packet->putWord ( picture );
	packet->putByte ( (unsigned char)type );
	packet->putByte ( (unsigned char)exits );
	packet->putWord ( (unsigned short)flags );

	// put in the ATP information
	packet->putByte ( atpList.size() );

	if ( atpList.size() ) {
		LinkedElement *element = atpList.head();

		while ( element ) {
			ATPInfo *info = (ATPInfo *)element->ptr();

			packet->putWord ( info->type );
			packet->putWord ( info->x );
			packet->putWord ( info->y );
			packet->putWord ( info->z );

			element = element->next();
		}
	}

	// put the object information
	packet->putWord ( objects.size() );

	LinkedElement *element = objects.head();

	while ( element ) {
		WorldObject *object = (WorldObject *)element->ptr();
		object->buildPacket ( packet );
		element = element->next();
	}

	// put the player information
	packet->putWord ( size() );
	element = head();

	while ( element ) {
		WorldObject *object = (WorldObject *)((RMPlayer *)element->ptr())->character;
		object->buildPacket ( packet );
		element = element->next();
	}
}

void RMRoom::fromPacket ( PackedData *packet )
{
	logInfo ( _LOG_ALWAYS, "RMRoom::fromPacket called and is unimplemented! You BOOB!" );
}

int RMRoom::mapExitToRoom ( unsigned char exit )
{
	int retVal = -1;

	if ( exit & _ROOM_NORTH_BIT )
		retVal = north;

	else if ( exit & _ROOM_SOUTH_BIT )
		retVal = south;

	else if ( exit & _ROOM_EAST_BIT )
		retVal = east;

	else if ( exit & _ROOM_WEST_BIT )
		retVal = west;

	else if ( exit & _ROOM_UP_BIT )
		retVal = up;

	else if ( exit & _ROOM_DOWN_BIT )
		retVal = down;

	return retVal;
}

int RMRoom::isExitBlocked ( unsigned char exit )
{
	static int nBlockedFlagCheck = (_RM_FLAG_BLOCK_N | _RM_FLAG_BLOCK_S | _RM_FLAG_BLOCK_E | _RM_FLAG_BLOCK_W);

	int retVal = 0;

	if ( flags & nBlockedFlagCheck ) {
		// check for an NPC in the room...
		LinkedElement *element = head();
		int bNPC = 0;

		while ( element ) {
			RMPlayer *pPlayer = (RMPlayer *)element->ptr();
			element = element->next();

			if ( pPlayer->isNPC ) {
				BNPC *pNPC = (BNPC *)pPlayer->character->getBase ( _BNPC );

				if ( pNPC && !pNPC->isExternal ) {
					bNPC = 1;
					break;
				}
			}
		}

		if ( bNPC ) {
			if ( (exit & _ROOM_NORTH_BIT) && (flags & _RM_FLAG_BLOCK_N) ) 
				retVal = 1;

			else if ( (exit & _ROOM_SOUTH_BIT) && (flags & _RM_FLAG_BLOCK_S) ) 
				retVal = 1;

			else if ( (exit & _ROOM_EAST_BIT) && (flags & _RM_FLAG_BLOCK_E) ) 
				retVal = 1;

			else if ( (exit & _ROOM_WEST_BIT) && (flags & _RM_FLAG_BLOCK_W) ) 
				retVal = 1;
		}
	}

	return retVal;
}

RMPlayer *RMRoom::findPlayerByName ( char *name )
{
	RMPlayer *ret = NULL;

	if ( !name ) {
		logInfo ( _LOG_ALWAYS, "Invalid findPlayerByName: (NULL)" );
		return ret;
	}

	int len = strlen ( name );

	if ( len < 1 || len > _MAX_CHAR_NAME ) {
		return ret;
	}

	int isBad = 0;

	for ( int i=0; i<len; i++ ) {
		if ( !isalnum ( name[i] ) ) {
			isBad = 1;											
			break;
		}
	}

	if ( isBad ) 
		return ret;

	LinkedElement *element = head();

	while ( element ) {
		RMPlayer *player = (RMPlayer *)element->ptr();

		if ( !strcasecmp ( player->getName(), name ) ) {
			ret = player;
			break;
		}

		element = element->next();
	}

	return ret;
}

void RMRoom::addATP ( int type, int x, int y, int z )
{
	atpList.add ( new ATPInfo ( type, x, y, z ) );
}

void RMRoom::copyOther ( RMRoom* pRoom )
{
	LinkedElement* element;

	// Remove all the ATPs from my current room
	if ( atpList.size() ) {
		element = atpList.head();

		while ( element ) {
			ATPInfo *info = (ATPInfo *)element->ptr();
			element = element->next();

			atpList.del( info );
			delete info;
		}
	}

	// Remove all the objects currently in my room
	element = objects.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		element = element->next();

		if ( zone ) 
			zone->delObject ( obj );
	}

	// Copy the new room picture/changes
	picture = pRoom->picture;
	north = pRoom->north;
	south = pRoom->south;
	east = pRoom->east;
	west = pRoom->west;
	up = pRoom->up;
	down = pRoom->down;

	memcpy ( exitCoords, pRoom->exitCoords, sizeof ( exitCoords ) );

	// Copy the new ATPs
	element = pRoom->atpList.head();

	while ( element ) {
		ATPInfo *info = (ATPInfo *)element->ptr();
		addATP ( info->type, info->x, info->y, info->z );

		element = element->next();
	}

	// Copy the new Objects
	element = pRoom->objects.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		element = element->next();

		addObject( obj, 0 );
	}
}

RMRoom *RMRoom::clone ( void )
{
	RMRoom *room = new RMRoom;

	room->picture = picture;
	room->north = north;
	room->south = south;
	room->east = east;
	room->west = west;
	room->up = up;
	room->down = down;

	memcpy ( room->exitCoords, exitCoords, sizeof ( exitCoords ) );

	LinkedElement *element = atpList.head();

	while ( element ) {
		ATPInfo *info = (ATPInfo *)element->ptr();
		room->addATP ( info->type, info->x, info->y, info->z );

		element = element->next();
	}

	return room;
}

int RMRoom::playerCount ( void )
{
	return building? building->playerCount() : size();
}

void RMRoom::setAntiMagic ( int percent, int duration )
{
}

void RMRoom::doit ( void )
{
}

void RMRoom::setTitle ( char *str )
{
	if ( title ) {
		free ( title );
		title = NULL;
	}

	if ( str )
		title = strdup ( str );
}

//
// Building class
//

Building::Building()
{
	setPtrType ( this, _MEM_BUILDING );

	servID = -1;
	destructing = 0;
	disposeDelay = 5;
	_owner = NULL;
	homeTown = 0;
	isDead = 0;
	resetHouse = 0;
	changed = 0;
	sqlID = -1;
	accountID = -1;

	setOwnerName ( "nobody" );
	gBuildings.add ( this );
}

Building::~Building()
{
	destructing = 1;

	players.release();
	objects.release();
	doors.release();

	setOwnerName ( NULL );

	gBuildings.del ( this );
	gEmptyBuildings.del ( this );
}

void Building::setOwnerName ( char *str )
{
	if ( _owner ) {
		free ( _owner );
		_owner = NULL;
	}

	if ( str ) {
		_owner = strdup ( str );
		strlower ( _owner );
	}
}

void Building::addRoom ( RMRoom *room )
{
	rooms.add ( room );
	gRoomArray->add ( room, room->number );
	room->building = this;

	roomMgr->_rooms.add ( room );

	// step through the room and add the players and objects
	LinkedElement *element = room->head();

	while ( element ) {
		RMPlayer *player = (RMPlayer *)element->ptr();
		addPlayer ( player );

		element = element->next();
	}

	element = room->objects.head();

	while ( element ) {
		WorldObject *object = (WorldObject *)element->ptr();
		addObject ( object );

		element = element->next();
	}
}

void Building::delRoom ( RMRoom *room )
{
	rooms.del ( room );
	room->building = NULL;

	roomMgr->_rooms.del ( room );

	// step through the room and delete the players and objects
	LinkedElement *element = room->head();

	while ( element ) {
		RMPlayer *player = (RMPlayer *)element->ptr();
		delPlayer ( player );

		element = element->next();
	}

	element = room->objects.head();

	while ( element ) {
		WorldObject *object = (WorldObject *)element->ptr();
		delObject ( object );

		element = element->next();
	}

	if ( !rooms.size() )
		delete this;
}

RMRoom *Building::findRoom ( int number )
{
	// find the room in the rooms list
	LinkedElement *element = rooms.head();

	while ( element ) {
		RMRoom *room = (RMRoom *)element->ptr();

		if ( room->number == number )
			return room;

		element = element->next();
	}

	return NULL;
}

void Building::addObject ( WorldObject *object )
{
	changed = 1;
	objects.add ( object );

	if ( object->getBase ( _BENTRY ) )
		doors.add ( object );
}

void Building::delObject ( WorldObject *object )
{
	if ( object ) {
		changed = 1;
		objects.del ( object );
		
		if ( object->getBase ( _BENTRY )) 
			doors.del ( object );
	}
}

void Building::addPlayer ( RMPlayer *player )
{
	if ( !players.size() )
		gEmptyBuildings.del ( this );

	players.add ( player );
}

void Building::delPlayer ( RMPlayer *player )
{
	players.del ( player );

	if ( !players.size() ) {
		if ( !gEmptyBuildings.contains ( this ) ) {
			disposeDelay = 5;
			gEmptyBuildings.add ( this );

			// step through and close all openable items
			LinkedElement *element = objects.head();

			while ( element ) {
				WorldObject *obj = (WorldObject *)element->ptr();
				element = element->next();

				// close the object
				obj->beClosed ( obj );
			}
		}
	}
}

void Building::writeHouseData ( void )
{
	if ( !rooms.size() || isDead )
		return;

	PackedData *data = new PackedData;
	data->grow ( 300000 );

	// write out the homeTown of this building
	data->printf ( "%d\n", homeTown );

	// write out the number of rooms in this house
	data->printf ( "%d\n", rooms.size() );

	// tag each door pair with an id
	LinkedElement *element = doors.head();
	int id = 0;

	while ( element ) {
		WorldObject *door = (WorldObject *)element->ptr();
		door->tempID = id++;

		element = element->next();
	}

	// write out all of the basic visual information for each room in the house
	element = rooms.head();

	while ( element ) {
		RMRoom *room = (RMRoom *)element->ptr();

		room->tempID = id;
		id++;

		// put the picture number
		data->printf ( "%d\n", room->picture );

		// put the room name
		data->printf ( "%s\n", room->title? room->title : "" );

		// put the ATPs
		data->printf ( "%d\n", room->atpList.size() );

		LinkedElement *elementA = room->atpList.head();

		while ( elementA ) {
			ATPInfo *info = (ATPInfo *)elementA->ptr();
			data->printf ( "%d\n%d\n%d\n%d\n", info->type, info->x, info->y, info->z );

			elementA = elementA->next();
		}

		data->printf ( "%d\n", room->objects.size() );

		// go through all objects in the room and write them out
		elementA = room->objects.head();

		while ( elementA ) {
			WorldObject *obj = (WorldObject *)elementA->ptr();
			elementA = elementA->next();

			obj->writeToBuffer ( data );
		}

		element = element->next();
	}

	data->printf ( "%d\n", resetHouse );

	// send the house to be saved
	if ( sqlID == -1 ) 
		gDataMgr->newHouse ( this, data );
	else
		gDataMgr->writeHouse ( this, data );

	delete data;
}

int Building::loadHouseData ( char *buffer, int bufferSize )
{
	// flag for bad house
	int isFunkHouse = 0;

	char str[1024];

	if ( !gEmptyBuildings.contains ( this ) ) {
		disposeDelay = 5;
		gEmptyBuildings.add ( this );
	}

	char *ptr = buffer;

	// get the town number for this house
	homeTown = bufgetint ( str, &ptr, &bufferSize );

	// get the number of rooms in the house
	int numRooms = bufgetint ( str, &ptr, &bufferSize );

	/*if ( numRooms != 4 )
		isFunkHouse = 1;*/

	while ( !isFunkHouse && numRooms > 0 ) {
		RMRoom *room = new RMRoom;
		room->number = allocateRoomID();

		addRoom ( room );

		// get the picture
		room->picture = bufgetint ( str, &ptr, &bufferSize );

		// get the title
		bufgets ( str, &ptr, &bufferSize, sizeof ( str ) );

		//force uppercase
		str[0] = toupper( str[0] );
		char* pName = strchr( str, ' ' ) + 1;
		*pName = toupper( *pName );

		room->setTitle ( str );

		// get the ATPs
		int ATPCount = bufgetint ( str, &ptr, &bufferSize );

		if ( ATPCount > _DBR_MAX_ATPS ) {
			logInfo ( _LOG_ALWAYS, "(%s) House Load Error: Too many add-to-pics (%d)", _owner, ATPCount );
			isFunkHouse = 1;
		}

		while ( !isFunkHouse && ATPCount > 0 ) {
			int type = bufgetint ( str, &ptr, &bufferSize );
			int x = bufgetint ( str, &ptr, &bufferSize );
			int y = bufgetint ( str, &ptr, &bufferSize );
			int z = bufgetint ( str, &ptr, &bufferSize );

			room->addATP ( type, x, y, z );

			ATPCount--;
		}

		// get the number of objects to load and load them in
		int objCount = bufgetint ( str, &ptr, &bufferSize );

		if ( objCount >= 1000 ) {
			logInfo ( _LOG_ALWAYS, "(%s) House Load Error: Too many objects on floor (%d)", _owner, objCount );
			isFunkHouse = 1;
		}

		while ( !isFunkHouse && objCount ) {
			WorldObject *obj = new WorldObject;
			obj->room = room;

			if ( obj->loadFromBuffer ( NULL, &ptr, &bufferSize ) ) {
				// close this object if it is openable
				obj->beClosed ( obj );

				room->addObject ( obj );
			} else {
				obj->room = NULL;
				delete obj;
			} 

			objCount--;

			if ( !bufferSize ) {
				logInfo ( _LOG_ALWAYS, "(%s) house object read past end of buffer!", _owner );
				isFunkHouse = 1;
			} 
		}

		numRooms--;
	}

	if ( bufferSize ) 
		resetHouse = bufgetint ( str, &ptr, &bufferSize );

	if ( isFunkHouse ) {
			logInfo ( _LOG_ALWAYS, "(%s) house corrupted!", _owner );
			isDead = TRUE;
			return -1;
	}

	// give each door a tempID 
	LinkedElement *element = doors.head();
	int id = 0;

	while ( element ) {
		WorldObject *door = (WorldObject *)element->ptr();
		element = element->next();

		door->tempID = id++;

		// fix the invalid house files that have no links!
		if ( ( (intptr_t) door->linkTo ) == -1 ) {
			logInfo ( _LOG_ALWAYS, "(%s) house had an unlinked door!", _owner );

			switch ( door->x ) {
				case 328:
					door->linkTo = (WorldObject*) 0;
					break;

				case 461:
					door->linkTo = (WorldObject*) 3;
					break;

				case 348:
					door->linkTo = (WorldObject*) 2;
					break;

				case 548:
					door->linkTo = (WorldObject*) 1;
					break;
			};
		}
	}

	// go through and link each door based on it's tempID
	element = doors.head();

	while ( element ) {
		WorldObject *door = (WorldObject *)element->ptr();
		element = element->next();

		LinkedElement *savedElement = element;

		// find the door that has a linkTo equal to my tempID
		element = doors.head();

		while ( element ) {
			WorldObject *theDoor = (WorldObject *)element->ptr();
			element = element->next();

			if ( theDoor->linkTo == (WorldObject *)door->tempID ) {
				theDoor->linkWith ( door );
				break;
			}
		}

		element = savedElement;
	}

	element = doors.head();

	while ( element ) {
		WorldObject *door = (WorldObject *)element->ptr();
		element = element->next();

		if ( ((intptr_t)door->linkTo) < 10000 ) {
			door->linkTo = NULL;
		}
	}

	RMRoom *room = (RMRoom *)rooms.at ( 3 );
		
	if ( room ) { 
		room->south = -2;
		room->exits |= _ROOM_SOUTH_BIT;
	}

	// default to not changed
	changed = 0;

	return 0;
}

Building *findHouse ( char *name )
{
	Building *retVal = NULL;

	LinkedElement *element = gBuildings.head();

	while ( element ) {
		Building *building = (Building *)element->ptr();

		if ( building->_owner && !strcasecmp ( building->_owner, name ) && !building->isDead ) {
			retVal = building;
			break;
		}

		element = element->next();
	}

	if ( retVal )
		retVal->disposeDelay = 5;

	return retVal;
}

// this function is called to create a house from a loaded buffer
Building *processHouseData ( char *name, char *buffer, int size, int houseID, int accountID )
{
	// toss the top gLoadingHouses object
	StringObject *obj = (StringObject *)gLoadingHouses.at ( 0 );

	if ( obj ) {
		delete obj;
		gLoadingHouses.delElement ( gLoadingHouses.head() );
	}

	// don't handle invalid sizes
	if ( size < 1 )
		return NULL;

	Building *house = new Building;
	house->setOwnerName ( name );

	if ( house->loadHouseData ( buffer, size ) == -1 ) {
		delete house;
		return NULL;
	}

	house->sqlID = houseID;
	house->accountID = accountID;

	return house;
}

// this function is called to start the house loading process
void loadHouse ( char *name, hlcallback_t callback, int context )
{
	// check to see if this house is already loading
	LinkedElement *element = gLoadingHouses.head();

	while ( element ) {
		StringObject *strObj = (StringObject *)element->ptr();
		element = element->next();

		// we are loading, exit
		if ( !strcasecmp ( strObj->data, name ) )
			return;
	}

	// add a new marker to loading houses
	StringObject *obj = new StringObject ( name );	
	gLoadingHouses.add ( obj );

	gDataMgr->loadHouse ( name, callback, context );
}

