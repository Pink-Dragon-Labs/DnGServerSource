#include "zone.hpp"
#include "roommgr.hpp"
#include "globals.hpp"
#include "ambushgroup.hpp"
#include "zonescripts.hpp"

char gInstanceTbl[_MAX_INSTANCE];

int allocateInstance ( void )
{
	static int initted = 0;

	if ( !initted ) {
		for ( int i=0; i<_MAX_INSTANCE; i++ ) 
			gInstanceTbl[i] = 0;

		initted = 1;
	}
		
	for ( int i=0; i<_MAX_INSTANCE; i++ ) {
		if ( !gInstanceTbl[i] ) {
			gInstanceTbl[i] = 1;
			return i;
		}
	}

	fatal ( "ran out of zone instances" );
	return 0;
}

void freeInstance ( int instance )
{
	gInstanceTbl[instance] = 0;
}

//
// class Zone
//
// A zone is a list of rooms that have shared properties.  For instance,
// if a player is in a special area he may not be able to cast magic
// spells.  If the implementor wants that ability, he should make
// a zone that contains all of the rooms that he wants to affect
// and set the properties.  Also, a zone is useful in that each one
// has it's own reset time and object age allowance.  When the zone
// is reset, all of the rooms are reloaded from the world library and
// each dynamic object's age is checked against it's allowance.  If 
// it is older than allowed, it is destroyed.  One important note is
// that a zone WILL NOT reset if players are present within it.  However,
// once the last player leaves the zone, it will automatically reset.
//

Zone::Zone()
{
	sprintf ( sizeof ( _name ), _name, "<no name>" );
	allowCombat() = _ALLOW_NO_COMBAT;
	allowEntry ( 1 );
	pkgInfo = _PKG_BASIC;
	midiFile = -1;
	groupChance = 0;
	resetPending = 0;
	homeTown = -1;
	isDungeon = 0;
	isRandom = 0;
	didClose = 0;
	instance = -1;
	numMonsters = 0;
	numGroups = 0;
	worldData = NULL;
	worldSize = 0;
	lastAmbushTime = getsecondsfast();
	ambushInterval = 30;
	m_pScript = NULL;
	m_pScriptName = NULL;
	dungeonShutdownTimer = 0;

	resetAction = _RESET_NORMAL;
	entrance = NULL;
	entranceName = NULL;
	id = 0;

	title = NULL;
	setTitle ( "The Realm" );
}

Zone::~Zone()
{
	roomMgr->delZone ( this );

	if ( m_pScriptName ) {
		free ( m_pScriptName );
		m_pScriptName = NULL;
	}

	if ( m_pScript ) {
		delete m_pScript;
		m_pScript = NULL;
	}

	players.release();
	objects.release();
	npcs.release();
	externalNPCs.release();
	worldFiles.release();

	LinkedElement *element = monsterTypes.head();

	while ( element ) {
		MonsterType *type = (MonsterType *)element->ptr();
		type->monsters.release();
		element = element->next();
	}
	monsterTypes.release();

	//mike-groupspawn
	//release all monster group definitions
	LinkedElement* groupElement = monsterGroups.head();
	while ( groupElement ) {
		//release all monster type definitions
		MonsterGroup* group = (MonsterGroup*)groupElement->ptr();

		LinkedElement* monsterElement = group->monsterTypes.head();

		while ( monsterElement ) {
			//release all monster definitions
			MonsterType* monsterType = (MonsterType*) monsterElement->ptr();
			monsterType->monsters.release();
			monsterElement = monsterElement->next();
		}
		group->monsterTypes.release();

		groupElement = groupElement->next();
	}
	monsterGroups.release();

	rooms.release();

	setTitle ( NULL );

	if ( instance > -1 )
		freeInstance ( instance );

	if ( entrance ) {
			entrance->intelligence = -1;
	}

	if ( entranceName ) {
		free ( entranceName );
		entranceName = NULL;
	}

	gZones.del ( this );
	ambushGroups.release();
}

// find and return a room by number
RMRoom *Zone::findRoom ( int number )
{
	return (RMRoom *)gRoomArray->lookup ( number );
}

// find an object in this zone
WorldObject *Zone::findObject ( char *name )
{
	LinkedElement *element = objects.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		element = element->next();

		if ( name && !strcmp ( name, obj->classID ) ) 
			return obj;
	}

	element = npcs.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		element = element->next();

		if ( name && !strcmp ( name, obj->name ) ) 
			return obj;
	}

	return NULL;
}

int Zone::queueStatus ( RMPlayer *player )
{
	int tRetVal = 0;

	if ( player && player->dungeonEntrance ) {

		LinkedList *pQ = dungeonEntranceArray[ player->dungeonEntrance->strength ];

		if ( pQ && pQ->contains ( player ) ) {
			tRetVal = 1 + pQ->indexOf ( player );
		}
	}

	return tRetVal;
}

// add a player to the queue
int Zone::addPlayerToQueue ( RMPlayer *player )
{
	int tRetVal = 0;

	Zone *entranceZone = entrance->room->zone;

	LinkedList *pQ = entranceZone->dungeonEntranceArray[entrance->strength];

	if ( pQ && player ) {

		if ( !pQ->contains ( player ) ) {
			pQ->add ( player );
			roomMgr->sendPlayerText ( player, "|c43|Info> You are number %d in line to enter the Dungeon.\n", pQ->size() );
		} else
	 		roomMgr->sendPlayerText ( player, "|c43|Info> You are already in line!\n" );

		tRetVal = 1;

	} else {
		if (!pQ ) {
			roomMgr->sendSystemMsg ( "No Entry", player, "That doesn't lead anywhere you can go." );
		}
	}
	return tRetVal;
}

// delete a player from the queue
void Zone::delPlayerFromQueue ( RMPlayer *player )
{
	// if my player has an entrance, get him out of the queue

	if ( player && player->dungeonEntrance ) {

		LinkedList *pQ = dungeonEntranceArray[ player->dungeonEntrance->strength ];

		if ( pQ && pQ->contains ( player ) ) {
   		player->dungeonEntrance = NULL;
	  		pQ->del ( player );
		}
	}
}

// send the player in from the queue
int Zone::enterFromQueue( )
{
	// if return value is 1, the dungeon can reset (in main)
	// otherwise same dungeon will be reused.
	int tRetVal = 0;

	if ( !entrance ) {
		logInfo ( _LOG_ALWAYS, "enterFromQueue() called on zone with no entrance" );
		return tRetVal;
	}

	Zone *entranceZone = entrance->room->zone;
	LinkedList *pQ = entranceZone->dungeonEntranceArray[entrance->strength];
	BEntry *base = (BEntry *)entrance->getBase ( _BENTRY );

	if ( pQ && base && base->room ) {
		LinkedElement *element = pQ->head();
			
   	if ( element ) {
   		RMPlayer *player = (RMPlayer *)element->ptr();
			
			if ( player ) {
				if ( player->groupLeader ) {
					LinkedElement *groupMember = player->groupLeader->group.head();
			
					// tell all group members to teleport
					while ( groupMember ) {
						RMPlayer *member = (RMPlayer *)groupMember->ptr();
			
						if ( member ) {
							if ( member->character && member->room && member->teleportRoomNum == -1 && !member->character->combatGroup && (member->room == entrance->room)  ) {
								member->character->teleport ( base->room, NULL );
								tRetVal = 1; // at least one member made it
							} 

							// delete this player from the queue if they were in it!
							delPlayerFromQueue ( member );
						}

						groupMember = groupMember->next();
					}

				} else { // if no group
					if ( player->character && player->room && player->teleportRoomNum == -1 && !player->character->combatGroup && (player->room == entrance->room) ) {
						player->character->teleport( base->room, NULL );
						tRetVal = 1; // made it
					}
				}

				delPlayerFromQueue ( player );
			} else {
				// delete the element since we don't know what it was
				pQ->delElement ( element );
			}

			if ( !tRetVal ) 
				entrance->intelligence = -1;
		}
	} 

	return tRetVal;
}

// add a player to this zone
void Zone::addPlayer ( RMPlayer *player )
{
	if ( player ) {

		// if dungeon add to queue
		if ( isDungeon && entrance && player->dungeonEntrance == entrance ) 
			addPlayerToQueue ( player );
		else {

			BCharacter *bchar = (BCharacter *)player->character->getBase(_BCHARACTER );
				
			if ( bchar ) 
				bchar->lastDungeon = id;

			if ( !players.contains ( player ) )
				players.add ( player );
		}
	}
}

// delete a player from this zone
void Zone::delPlayer ( RMPlayer *player ) 
{
	// clear out lastDungeon on the character if need be
	if ( isDungeon && player && player->character ) {
		BCharacter *bchar = (BCharacter *)player->character->getBase(_BCHARACTER );
				
		if ( bchar ) 
			bchar->lastDungeon = 0;
	}

	players.del ( player );
}

// add an external NPC to this zone
void Zone::addExternalNPC ( WorldObject *npc )
{
	if ( externalNPCs.contains ( npc ) ) 
		return;

	if ( npc && !npc->summoned )
		numMonsters++;

	externalNPCs.add ( npc );
}

// add an NPC to this zone
void Zone::addNPC ( WorldObject *npc )
{
	if ( npcs.contains ( npc ) ) 
		return;

	if ( npc && !npc->summoned )
		numMonsters++;

	npcs.add ( npc );
}

// delete an NPC from this zone
void Zone::delNPC ( WorldObject *npc )
{
	if ( npc && !npc->summoned )
		numMonsters--;

	npcs.del ( npc );
	externalNPCs.del ( npc );
}

// add an object to this zone
void Zone::addObject ( WorldObject *object )
{
	if ( objects.contains ( object ) )
		return;

	objects.add ( object );
}

// delete an object from this zone
void Zone::delObject ( WorldObject *object )
{
	objects.del ( object );
}

// add a world file to this zone
void Zone::addWorldFile ( char *name )
{
	worldFiles.add ( new StringObject ( name ) );
}

// parse all of the world files in the world file list
int Zone::parseWorldFiles ( void )
{
	LinkedElement *element = worldFiles.head();
	int retVal = 0;

	while ( element ) {
		StringObject *name = (StringObject *)element->ptr();
		char filename[1024];
		sprintf ( sizeof ( filename ), filename, "%s.tok", name->data );
		if ( parseWorldFile ( filename ) == -1 ) {
			retVal = -1;
			break;
		}
		element = element->next();
	}

	// step through all objects and handle linkTo's
	element = objects.head();
	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();

		if ( obj ) {
			if ( obj->linkToStr ) {
				TreeNode *node = gObjectTree.find ( obj->linkToStr );

				if ( !node ) {
					obj->linkToStr[strlen ( obj->linkToStr ) - 3] = 0;
					node = gObjectTree.find ( obj->linkToStr );
				}

				WorldObject *objA = node? (WorldObject *)node->data : NULL;

				if ( objA ) {
					objA->linkWith ( obj );

					if ( objA->linkToStr ) {
						free ( objA->linkToStr );
						objA->linkToStr = NULL;
					}
				}

				if ( obj->linkToStr ) {
					free ( obj->linkToStr );
					obj->linkToStr = NULL;
				}
			}

			else if ( obj->linkToRoom != -1 ) {
				LinkedElement *elementA = objects.head();

				while ( elementA ) {
					WorldObject *door = (WorldObject *)elementA->ptr();

					if ( door->room->number == obj->linkToRoom && door->linkToRoom == obj->room->number ) {
						if ( !door->linkTo )
							door->linkWith ( obj );

						door->linkToRoom = -1;
						obj->linkToRoom = -1;

						break;
					}

					elementA = elementA->next();
				}
			}
		}

		element = element->next();
	}
	return retVal;
}

// create this zone from the internal buffer
int Zone::loadFromBuffer ( void )
{
	return -1;
}

// parse one world file
int Zone::parseWorldFile ( char *name )
{
	int retVal = 0, lineNum = 0;

	WorldInfoParser parser;

	char str[10240];

	if ( isDungeon ) {
		instance = allocateInstance();
		parser.instanceNum = instance;
		parser.parser.instanceNum = instance;
	}

	File *file = new File ( name );
	if ( file->isOpen() ) {
		parser.zone = this;

		int bufferSize = file->size();
		char *buffer = (char *)malloc ( bufferSize );
		file->read ( buffer, bufferSize );

		parser.input = buffer;
		parser.index = 0;

		if ( isDungeon ) {
			worldData = buffer;
			worldSize = bufferSize;
		}

		char *ptr = buffer;
		while ( retVal != -1 && (parser.index < bufferSize) ) {

			lineNum++; 

			switch ( parser.parseLine() ) {
				case -1: {
					logInfo ( _LOG_ALWAYS, "\"%s\", line %d: %s", name, lineNum, parser.error() );
					retVal = -1;
				}

				break;

				case 2: {
					RMRoom *room = parser.room;
					if ( room ) {
						if ( room->midiFile == -1 )
							room->midiFile = midiFile;

						if ( room->groupChance == -1 )
							room->groupChance = groupChance;

						gRoomArray->add ( room, room->number );

						rooms.add ( room );
					}
				}
			}
		}

		if ( buffer )
			free ( buffer );

		file->close();
	} else {
		char cwd[1024];
		getcwd ( cwd, 1024 );

		roomMgr->sendListText ( &roomMgr->_guides, "-dError> Zone load failed for '%s' from working directory of '%s' (error = %d)\n", name, cwd, errno );

	}
	delete file;
	return retVal;
}

// load this zone's dynamic object database
void Zone::loadObjects ( void )
{
}

// reset this zone
int Zone::reset ( void )
{
	int retVal = 0;

	resetAction = _RESET_NORMAL;


	if ( !players.size() ) {
	 	tossRooms();
		if ( parseWorldFiles() == -1 )
			fatal ( " zone::parse Invalid World File " );

		generateMonsters();

		groupChance = ( ( numMonsters * groupChance ) / 100 ) / 3;

		retVal = 1;
	}


	if ( !entrance && entranceName ) {
		TreeNode *node = gObjectTree.find ( entranceName );
		entrance = node? (WorldObject *)node->data : NULL;


		if ( !entrance->servID ) {
			fatal ( "can not find entrance (%s) on zone reset", entranceName );
		}

		entrance->physicalState |= _STATE_OCCUPIED;
		entrance->beOpened ( NULL );

		if ( entrance->room ) {
			entrance->room->SetDungeonEntrance();

			PackedMsg msg;
			long servID = entrance->servID;

			msg.putLong ( servID ); 
			msg.putLong ( entrance->room->number ); 

			msg.putByte ( _MOVIE_CLOSE );
			msg.putLong ( servID ); 
			msg.putLong ( servID ); 
	
			msg.putByte ( _MOVIE_OPEN );
			msg.putLong ( servID ); 
			msg.putLong ( servID ); 

			msg.putByte ( _MOVIE_END );
			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, msg.data(), msg.size(), entrance->room );

		}
		entrance->intelligence = -1;
	}

	return retVal;
}

// toss all of the rooms in this zone that match a specific instance
void Zone::tossRooms ( void )
{

	LinkedElement *element = rooms.head();
	while ( element ) {
		RMRoom *room = (RMRoom *)element->ptr();
		element = element->next();

		// test this!!!
		gRoomArray->del ( room->number );
		rooms.del ( room );
		delete room;
	}
}

// go through the dynamic object database and throw away any objects that
// need to be thrown away

void Zone::purgeObjects ( void )
{
	// step through all objects and toss them 

	LinkedElement *element = objects.head();

	while ( element ) {
		WorldObject *obj = (WorldObject *)element->ptr();
		delObject ( obj );
	}
}

void Zone::initDatabase ( void )
{
}

// spawn a new copy of this zone
Zone *Zone::spawn ( void )
{
	Zone *theZone = new Zone;

	theZone->_resetInterval = _resetInterval;
	theZone->_allowCombat = _allowCombat;
	theZone->isDungeon = isDungeon;
	theZone->worldData = worldData;
	theZone->worldSize = worldSize;
	strcpy ( theZone->_name, _name );

	if ( entranceName )
		theZone->entranceName = strdup ( entranceName );

	if ( entrance )
		theZone->entrance = entrance;

	theZone->setTitle ( title );
	theZone->midiFile = midiFile;

	// don't copy world files, no need
	LinkedElement *element = worldFiles.head();

	while ( element ) {
		StringObject *str = (StringObject *)element->ptr();
		element = element->next();

		theZone->addWorldFile ( str->data );
	}

	// copy the ambush groups...
	element = ambushGroups.head();

	while ( element ) {
		CAmbushGroup *pGroup = (CAmbushGroup *)element->ptr();
		element = element->next();

		theZone->addAmbushGroup ( pGroup );
	}

	theZone->ambushInterval = ambushInterval;

	gZones.add ( theZone );
	roomMgr->addZone ( theZone );

	gSpawnCount++;

	theZone->id = gSpawnTotal++;
	theZone->reset();

	if ( m_pScriptName )
		theZone->SetScript ( m_pScriptName );

	entrance->physicalState |= _STATE_OCCUPIED;
	entrance->beOpened ( NULL );

	if ( entrance->room ) {
		entrance->room->SetDungeonEntrance();

		PackedMsg msg;
		long servID = entrance->servID;

		msg.putLong ( servID ); 
		msg.putLong ( entrance->room->number ); 

		msg.putByte ( _MOVIE_CLOSE );
		msg.putLong ( servID ); 
		msg.putLong ( servID ); 
	
		msg.putByte ( _MOVIE_OPEN );
		msg.putLong ( servID ); 
		msg.putLong ( servID ); 

		msg.putByte ( _MOVIE_END );
	
		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, msg.data(), msg.size(), entrance->room );
	}

	entrance->intelligence = -1;

	return theZone;
}

// handle setting the title
void Zone::setTitle ( char *str )
{
	if ( title ) {
		free ( title );
		title = NULL;
	}

	if ( str ) {
		title = strdup ( str );
	}
}

// add a new monster type
void Zone::addMonsterType ( char *name, int population )
{
	MonsterType *info = new MonsterType ( name, population );
	monsterTypes.add ( info );
}

//mike-groupspawn
void Zone::addMonsterGroup( MonsterGroup* newMonsterGroup )
{
	monsterGroups.add( newMonsterGroup );
}

// ambush players
void Zone::ambushPlayers ( void )
{
	// don't ambush unless there are ambush groups...
	if ( !ambushGroups.size() )
		return;

	int nCurTime = getsecondsfast();

	// step through all of the players in the zone...
	LinkedElement *element = players.head();

	while ( element ) {
		RMPlayer *pPlayer = (RMPlayer *)element->ptr();
		element = element->next();

		WorldObject *pCharacter = pPlayer->character;

		// only try to ambush players that are not already fighting...
		if ( pCharacter && !pPlayer->isNPC && !pCharacter->combatGroup && (pCharacter->health > 0) && !pCharacter->room->isDungeonEntrance && pCharacter->room->bAllowAmbush && !pCharacter->hidden && !pPlayer->isTeleporting && !pPlayer->checkAccess( _ACCESS_AMBUSH ) ) {
			int bDoAmbush = 0;

			if ( !pPlayer->groupLeader ) {
				if ( ( (nCurTime - pPlayer->lastAmbushTime) >= ambushInterval ) && !pPlayer->checkAccess( _ACCESS_AMBUSH ) ) {
					bDoAmbush = 1;
				}
			} else {
				// make sure all group members can be ambushed...
				LinkedElement *pElement = pPlayer->groupLeader->group.head();
				bDoAmbush = 1;

				while ( pElement ) {
					RMPlayer *pThePlayer = (RMPlayer *)pElement->ptr();
					pElement = pElement->next();

					if ( (pThePlayer->room == pPlayer->room) && ( ((nCurTime - pThePlayer->lastAmbushTime) < ambushInterval) || pThePlayer->checkAccess( _ACCESS_AMBUSH ) ) ) {
						bDoAmbush = 0;
						break;
					}
				}
			}

			// only ambush players if the time is right...
			if ( bDoAmbush ) {
				int nRoll = random ( 1, 100 );

				// check each ambush group to see which one gets to run...
				CAmbushGroup *pActiveGroups[100];
				int nGroupCount = 0, nSmallestChance = 100;

				LinkedElement *pAmbushElement = ambushGroups.head();

				while ( pAmbushElement ) {
					CAmbushGroup *pGroup = (CAmbushGroup *)pAmbushElement->ptr();
					pAmbushElement = pAmbushElement->next();

					// if this one works, add it to the list
					int nChance = pGroup->GetChance();

					// get the group level for comparison
					int nLevel = pGroup->GetLevel();

					// dungeons always ambush, no matter what...
					if ( pCharacter->room->zone->isDungeon )
						nLevel = 2000;

					if ( (pCharacter->level < (nLevel * 2)) && (nRoll <= nChance) ) {
						pActiveGroups[nGroupCount++] = pGroup;

						if ( nChance < nSmallestChance ) {
							nSmallestChance = nChance;
						}
					}
				}

				if ( nGroupCount ) {
					CAmbushGroup *pGroupSet[100];
					int nSetCount = 0;

					for ( int i=0; i<nGroupCount; i++ ) {
						CAmbushGroup *pGroup = pActiveGroups[i];

						if ( pGroup->GetChance() == nSmallestChance ) {
							pGroupSet[nSetCount++] = pGroup;
						}
					}

					// pick a random set group
					pGroupSet[random ( 0, nSetCount - 1 )]->SpawnAndAttack ( pCharacter );
				}
			}
		}
	}
}

// generate a new set of monsters
void Zone::generateMonsters ( void )
{
	//generate single monsters...
	LinkedElement *element = monsterTypes.head();

	while ( element ) {
		MonsterType *type = (MonsterType *)element->ptr();

		while ( type->monsters.size() < type->population ) {
			NPC *npc = type->makeMonster();

			if ( !npc )
				break;

			RMRoom* room = NULL;
			
			do {
				room = (RMRoom*)  rooms.at ( random ( 0, rooms.size() - 1 ) );
			} while ( room->isDungeonEntrance );

			npc->newRoom ( room );

			// enable monster AI
			npc->aiReady = 1;
		}
		element = element->next();
	}


	//mike-groupspawn
	//mike - now generate grouped monsters...
	LinkedElement *groupElement = monsterGroups.head();

	while( groupElement ) {
		//for each monster group that is in the zone, create 
		// the group if needed

		MonsterGroup* monsterGroup = (MonsterGroup*) groupElement->ptr();
		
		while( monsterGroup->groups.size() < monsterGroup->population ) {
			//lets bring the number of instanciated groups of this type up to
			//it's proper population

			//lets pick a random room in the zone for our new group to appear in
			RMRoom* room = NULL;
			do	{
				room = (RMRoom*)  rooms.at ( random ( 0, rooms.size() - 1 ) );
			} while ( room->isDungeonEntrance );

			//get the list of npc's for this new group
			LinkedList* npcList = monsterGroup->makeMonsterGroup();
			
			LinkedElement* npcElement = npcList->head();
	
			//but wait, they're not grouped! lets handle that...
			
			//the group leader is always going to be the first element in the npc list
			NPC* groupLeader = NULL;

			while( npcElement ) {
				//lets go and place each NPC in the room, adding it to the
				//group at the same time

				NPC* npc = (NPC*)npcElement->ptr();
				if( !npc ) break;

				//ok, we have an npc, lets add it to the group and
				//throw it in the room.
			
				if ( groupLeader ) npc->joinGroup ( groupLeader );
				else groupLeader = npc;

				npc->newRoom ( room );
				npc->aiReady = 1;

				npcElement = npcElement->next();
			}

			//tie the group leader's group into our living group list

			// lets tell the groupleader where to find this MonsterGroup::groups
			// so that it can remove it's RMPlayer::group from the list, and lets
			// also make sure we add the group leader's RMPlayer::group to our 
			// MonsterGroup::groups list so there is something for it to remove!
			groupLeader->groupList = &monsterGroup->groups;
			monsterGroup->groups.add( &groupLeader->group );

			//release this LinkedList object of <NPC*> definitions
			npcList->release();
			delete npcList;
		}

		//done with that group type! move on and consider another type.
		groupElement = groupElement->next();
	}
}

// add a new ambush group to the zone
void Zone::addAmbushGroup ( CAmbushGroup *pGroup )
{
	ambushGroups.add ( pGroup );
}

// set the script of this zone
CZoneScript *Zone::SetScript ( char *pScriptName )
{
	CZoneScript *pScript = NULL;

	if ( m_pScriptName )
		free ( m_pScriptName );

	m_pScriptName = strdup ( pScriptName );

	// SNTODO: replace this with a nice lookup table...
	if ( !strcmp ( pScriptName, "enidDungeon" ) ) {
		pScript = new CEnidDngZoneScript;
		pScript->SetClient ( this );
	}

	m_pScript = pScript;
	return pScript;
}


//mike-groupspawn
MonsterGroup::MonsterGroup( void )
{
	population = 0;
}

//mike-groupspawn
MonsterGroup::~MonsterGroup( void )
{
	//make sure we clear all of this nifty stuff out
	
	//hopefully these release functions like destructors for what they
	//point to.
	LinkedElement* monsterTypeElement = monsterTypes.head();

	while ( monsterTypeElement ) {
		MonsterType *mType = (MonsterType *)monsterTypeElement->ptr();
		mType->monsters.release();

		monsterTypeElement = monsterTypeElement->next();
	}

	monsterTypes.release();
}

//mike-groupspawn
void MonsterGroup::addMonsterType ( char *name, int population )
{
	MonsterType* newMonsterType = new MonsterType ( name, population );
	monsterTypes.add ( newMonsterType );
}

MonsterType::MonsterType ( char *name, int population )
{
	this->name = strdup ( name );
	this->population = population;
}

MonsterType::~MonsterType()
{
	if ( name ) {
		free ( name );
		name = NULL;
	}
}

//mike-groupspawn
LinkedList* MonsterGroup::makeMonsterGroup(void)
{
	//we need a nice flat list of NPC's that are in the group.
	LinkedList* NPCList = new LinkedList();

	LinkedElement* monsterTypeElement = monsterTypes.head();

	//bool groupLeader = true;

	while( monsterTypeElement ) {
		//for each type, there is probably multiple of each type we need
		//to form the group.
		MonsterType *monsterType = (MonsterType *)monsterTypeElement->ptr();

		int thisTypePopulation = monsterType->population;
		
		while(thisTypePopulation--) {
			NPC* newNPC = monsterType->makeMonster();
			//later on, we will add a pointer to the group leaders
			// LinkedList RMPlayer::group  member, so that the leader
			// can delete it's group from this  LinkedList MonsterGroup::groups
			
			//if ( groupLeader ) {
			//	groupLeader = false;
			//	newNPC->groupList = &groups;
			//}

			NPCList->add( newNPC );
		}
		
		monsterTypeElement = monsterTypeElement->next();
	}

	return NPCList;
}

NPC *MonsterType::makeMonster ( void )
{
	NPC *npc = NULL;

	WorldObject *super = roomMgr->findClass ( name );

	if ( super ) {
		WorldObject *object = super->clone();
		object->superObj = super;

		object->setClassID ( name );
		object->setSuper ( super->classID );

		//mike-groupspawn - added this randomness
		object->x = 320 + random(-75, 75);
		object->y = 250 + random(-75, 75);
		object->addToDatabase();

		npc = makeNPC ( object );
		npc->monsterList = &monsters;

		monsters.add ( npc );
	}

	return npc;
}

ResetAction::ResetAction ( LinkedList *list, int theType, int theDur )
{
	list->add ( this );
	type = theType;
	duration = theDur;
}

ResetAction::~ResetAction()
{
}
