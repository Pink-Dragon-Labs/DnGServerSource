#include <algorithm>
#include "roommgr.hpp"
#include "rmplayer.hpp"
#include "rmroom.hpp"
#include "globals.hpp"
#include "callbacks.hpp"
#include "../global/datamgrdefs.hpp"
#include "friendmgr.hpp"

#define MAX_GOLD_HELD 2000000000
#define MAX_MANA_HELD  400000000

char *playerPronouns[] = { "him", "her", "it", "his", "her", "its", "he", "she", "it" }; 

LinkedList gRequestQueue, gDeadRooms, gDemoPlayers;
int gHighConnections = 0, gDemoCount = 0, gFatalCount = 0, gLimboCount = 0;

BinaryTree gCharacterTree;
BinaryTree gLoginTree;

void describeObject ( WorldObject *character, WorldObject *object, const char *str, char *output, int outputSize );

void logStats ( const char *format, ... )
{
	char filename[1024];
	return;

	va_list args;
	char output[5000];
	char outputA[6000];

	va_start ( args, format );
	vsprintf ( sizeof ( output ), output, format, args );

	char *timeAsStr = timeToStr();
	sprintf ( sizeof ( outputA ), outputA, "%s: ", timeAsStr );
	free ( timeAsStr );

	strcat ( outputA, output );

	time_t time = getseconds();
	struct tm *curTime;        
   curTime = localtime( &time ); 

	sprintf ( sizeof ( filename ), filename, "../stat/stat%d%d.dat", curTime->tm_mon + 1, curTime->tm_year );

	File *file = new File ( filename );

	if ( file->isOpen() ) {
		file->seek ( file->size() );
		file->printf ( "%s\n", outputA );
		file->close();
	}
	delete file;
}

void logHack ( const char *format, ... )
{
	va_list args;
	char output[5000];
	char outputA[6000];
			
	char *timeAsStr = timeToStr();
	sprintf ( sizeof ( outputA ), outputA, "%s: ", timeAsStr );
	free ( timeAsStr );

	va_start ( args, format );
	vsprintf ( sizeof ( output ), output, format, args );

	strcat ( outputA, output );

	File *file = new File ( "../logs/permanent/hacks.log" );

	if ( file->isOpen() ) {
		file->seek ( file->size() );
		file->printf ( "%s\n", outputA );
		file->close();
	}

	delete file;
}

RMPlayer::RMPlayer()
{
	setPtrType ( this, _MEM_PLAYER );

	badPingCount = 0;
	lastWriteTime = getseconds();

	secured = FALSE;
	isTeleporting = 0;
	validated = 0;

	loginContext = NULL;

	talkTree = NULL;
	talkTreeTopic = NULL;
	talkTreePage = -1;
	dungeonEntrance = NULL;
	m_pFriend = NULL;

	engraveName = NULL;
	accountTypeStr = NULL;

	// SNTODO: make this text logging a nice class...
	textLogs = NULL;
	textCount = 0;
	m_pLastComplain = NULL;

	combatAction = nextAction = NULL;

	accountType = -1;
	firstLogin = 0;
	serial = 0;

	quests = NULL;
	teleportRoomNum = -1;
	teleportDestBuildingOwner = NULL;

	room = NULL;
	zone = NULL;
	player = NULL;
	character = NULL;
	isNPC = 0;
	tossing = 0;
	delay = 0;
	badGossipTime = 0;
	badGossipCount = 0;
	_lockedObj = NULL;
	seed = 0;
	roomTitle = NULL;
	lastTalkTarget = NULL;
	lastTalkSequence = 0;
	channel = NULL;
	monsterList = NULL;
	groupList = NULL;
	loginMsg = NULL;
	ping = 3;
	pingTime = 0;
	pingClientTime = 0;

	// cmd options
	allowJoin = 0;
	autoGive = 0;
	invite = 1;

	creationTime = 0;
	expirationTime = 0;

	rights = _ACCESS_NORMAL;
	pkgInfo = _PKG_BASIC;
	gossipBanTime = 0;

	ignoreList = NULL;

	accountID = -1;
	groupLeader = NULL;
	waitingMember = NULL;

	lastAmbushTime = 0;
	lastJumpTime = 0;

	nCountWantingToJoin = 0;

	m_pCrimes = NULL;
}

RMPlayer::~RMPlayer() {
	// toss the account information
	if ( accountTypeStr ) {
		free ( accountTypeStr );
		accountTypeStr = NULL;
	}

	// toss the teleport building owner
	if ( teleportDestBuildingOwner ) {
		free ( teleportDestBuildingOwner );
		teleportDestBuildingOwner = NULL;
	}

	// toss the login context
	if ( loginContext ) {
		delete loginContext;
	}

	if ( dungeonEntrance ) {
		if ( character ) {
			if ( dungeonEntrance->intelligence == character->servID ) 
				dungeonEntrance->intelligence = -1;
		}
		if ( zone ) {
			zone->delPlayerFromQueue ( this );
		}

		dungeonEntrance = NULL;
	}

	// pull this player out of his/her combat group
	if ( character && character->combatGroup ) {
		character->combatGroup->rewardCombatants( character );
		character->combatGroup->deleteCharacter( character );
	}

	if ( ignoreList ) {
		delete ignoreList;
		ignoreList = NULL;
	}

	if ( channel ) 
		channel->delPlayer ( this );

	// unlock any locked object
	if ( _lockedObj ) {
		_lockedObj->_locked--;
		_lockedObj = NULL;
	}

	// leave my group
	if ( groupLeader != NULL ) {
		leaveGroup();
	}

	if ( monsterList ) {
		monsterList->del ( this );
		monsterList = NULL;
	}

	group.release();

	if ( room ) 
		room->delPlayer ( this );

	roomMgr->deletePlayer ( this );

	if ( character ) {
		gWantedList.del ( character );
	}

	if ( player ) {
		WorldObject *thePlayer = player;
		player = NULL;

		delete thePlayer;
	} 

	else if ( character ) {
		WorldObject *theCharacter = character;
		character = NULL;

		delete theCharacter;
	}

	setRoomTitle ( NULL );

	if ( quests ) {
		delete quests;
		quests = NULL;
	}

	setEngraveName ( NULL );
	setPtrType ( this, _MEM_ARRAY );

	if ( textLogs ) {
		delete [] textLogs;
		textLogs = NULL;
	}

	if ( m_pLastComplain ) {
		delete [] m_pLastComplain;
		m_pLastComplain = NULL;
	}

	if ( m_pCrimes ) {
		delete m_pCrimes;
		m_pCrimes = NULL;
	}
}

// get the pointer to this player's friend object...
CFriend *RMPlayer::GetFriendEntry ( void ) {
	return m_pFriend;
}

// add a new friend reference to this player...
void RMPlayer::AddFriend ( char *pPlayerName ) {
	CFriend *pFriend = g_pFriendMgr->FindFriend ( pPlayerName );
	pFriend->AddWatcher ( this );
}

// delete a friend reference from this player...
void RMPlayer::DelFriend ( char *pPlayerName ) {
        gDataMgr->DelFriend( this, pPlayerName );

	CFriend *pFriend = g_pFriendMgr->FindFriend ( pPlayerName );
	pFriend->DelWatcher ( this );
}

// check to see if they are a friend already
bool RMPlayer::IsFriend( char* pPlayerName ) {
	CFriend *pFriend = g_pFriendMgr->FindFriend ( pPlayerName );

	return pFriend->beingWatchedBy( this );
}

CrimeData* RMPlayer::getCrimeData() {
	if ( !m_pCrimes ) {
		m_pCrimes = new CrimeData;
		memset( m_pCrimes, 0, sizeof( CrimeData ) );
	}

	return m_pCrimes;
}

char *RMPlayer::getAccountType( void ) {
	char *ret = "<unknown>";

	switch ( accountType ) {
		case _TRIAL: 
	  		ret = "30Day";
		  	break;
		case _90DAY:
	  		ret = "90Day";
		  	break;
		case _12MONTH:
	  		ret = "12Month";
	}
	return ret;
}

void RMPlayer::setTeleportRoomNum ( int num )
{
	if ( num == -1 ) {
		if ( teleportDestBuildingOwner ) {
			free ( teleportDestBuildingOwner );
			teleportDestBuildingOwner = NULL;
		}

		teleportRoomNum = num;
	} else {
		setTeleportRoomNum ( -1 );

		RMRoom *room = roomMgr->findRoom ( num );

		if ( room && room->building )
			teleportDestBuildingOwner = strdup ( room->building->_owner );

		teleportRoomNum = num;
	}
}

char *RMPlayer::setEngraveName ( char *str )
{
	if ( engraveName ) {
		free ( engraveName );
		engraveName = NULL;
	}

	if ( str ) {
		int len = strlen ( str );

		if ( len < 1 || len > 32 )
			return NULL;

		for ( int i=0; i<len; i++ ) {
			if ( str[i] == '%' )
				str[i] = '$';
			
			if ( !isprint ( str[i] ) || str[i] == '|') 
				str[i] = '$';
		}

		engraveName = strdup ( str );
	}

	return engraveName;
}

void RMPlayer::newRoom ( RMRoom *theRoom )
{
	Zone *oldZone = zone, *newZone = theRoom->zone;
	RMRoom *oldRoom = room, *newRoom = theRoom;

	if ( !oldRoom && character )
		character->AddToAffectedObjects();

	int doPoof = (room == NULL) && character;

	if ( isNPC ) {
		doPoof = 0;
	}

	if ( !isNPC && character && character->sittingOn ) {
		character->sittingOn->beStoodUpOn ( character );
		character->sittingOn = NULL;
	}

	if ( doPoof ) 
		character->hidden++;

	int changingZone = (zone == theRoom->zone)? 0 : 1;

	// handle marking changing zone if going to a building
	if ( theRoom->building && (!room || (room->building != theRoom->building)) )
		changingZone = 1;

	// handle switching between buildings
	if ( room && (theRoom->building != room->building) )
		changingZone = 1;

	if ( room ) 
		room->delPlayer ( this, changingZone );

	room = theRoom;
	zone = theRoom->zone;

	// update my character -- he's in limbo
	if ( character ) 
		character->room = theRoom;

	room->addPlayer ( this, NULL, changingZone );

	if ( doPoof ) {
		character->hidden--;

		PackedMsg response;
		response.putLong ( character->servID );
		response.putLong ( theRoom->number );

		response.putByte ( _MOVIE_SPECIAL_EFFECT );
		response.putLong ( character->servID );
		response.putByte ( _SE_POOF );
		response.putByte ( 1 );
		response.putLong ( character->servID );

		response.putByte ( _MOVIE_SHOW );
		response.putByte ( _MOVIE_END );

		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), theRoom );
	}

	// inform all NPC followers that I have changed rooms
	if ( isNPC && (groupLeader == this) ) {
		LinkedElement *element = group.head();

		while ( element ) {
			RMPlayer *player = (RMPlayer *)element->ptr();

			if ( player->isNPC && player != this && ( player->character->combatGroup == character->combatGroup ) ) {
				NPC *npc = (NPC *)player;
				npc->newRoom ( theRoom );
			}

			element = element->next();
		}
	}
}

void RMPlayer::setRoomTitle ( char *str )
{
	if ( roomTitle ) {
		free ( roomTitle );
		roomTitle = NULL;
	}

	if ( str )
		roomTitle = strdup ( str );
}

void RMPlayer::sendRoomChat ( const char *format, ... ) {
	char text[1024];

	if ( !room )
		return;

	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	PackedData response;

	response.putLong ( -1 );
	response.putLong ( servID );
	response.putArray ( text, strlen ( text ) + 1 );

	LinkedList *list = room->copy();
	char *login = "";

	BPlayer *base = (BPlayer *)player->getBase ( _BPLAYER );

	if ( base ) 
		login = base->login;

	// filter out all of the people that are ignoring this player
	LinkedElement *element = list->head();

	if (this->checkAccess ( _ACCESS_MODERATOR ) || ! *login) {
		while ( element ) {
			RMPlayer *target = (RMPlayer *)element->ptr();
			element = element->next();

			if ( target->isIgnoring( getName() ) ) {
				list->del ( target );
			} else {
				target->addText ( this, text );
			}
		}
	} else {
		while ( element ) {
			RMPlayer *target = (RMPlayer *)element->ptr();
			element = element->next();

			if ( target->isIgnoring( getName() ) ) {
				list->del ( target );
			} 
			
			else if (target->checkAccess ( _ACCESS_MODERATOR ) ) {
				list->del ( target );
				roomMgr->sendPlayerChat ( this, target, "(%s) %s", login, text);
			} 
			
			else {
				target->addText ( this, text );
			}
		}
	}

	roomMgr->sendToList ( _IPC_PLAYER_TEXT, response.data(), response.size(), list, NULL );

	list->release();
	delete list;
}

void RMPlayer::sendRoomText ( const char *format, ... )
{
	char text[1024];

	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	PackedData response;

	response.putLong ( -1 );
	response.putLong ( servID );
	response.putArray ( text, strlen ( text ) + 1 );

	LinkedList *list = room->copy();
	char *login = "";

	BPlayer *base = (BPlayer *)player->getBase ( _BPLAYER );

	if ( base ) 
		login = base->login;

	// filter out all of the people that are ignoring this player
	LinkedElement *element = list->head();

	if (this->checkAccess ( _ACCESS_MODERATOR ) || ! *login) {
		while ( element ) {
			RMPlayer *target = (RMPlayer *)element->ptr();
			element = element->next();

			if ( target->isIgnoring( getName() ) ) {
				list->del ( target );
			}
		}
	} else {
		while ( element ) {
			RMPlayer *target = (RMPlayer *)element->ptr();
			element = element->next();

			if ( target->isIgnoring( getName() ) ) {
				list->del ( target );
			} 
			
			else if (target->checkAccess ( _ACCESS_MODERATOR ) ) {
				list->del ( target );
				roomMgr->sendPlayerText ( target, "(%s) %s", login, text);
			} 
		}
	}

	roomMgr->sendToList ( _IPC_PLAYER_TEXT, response.data(), response.size(), list, NULL );

	list->release();
	delete list;
}

void RMPlayer::loadQuestData ( int* pData ) {
	// load quest data for my character -- if any
	if ( character && pData ) {
		int nQuests;

		for (nQuests = 0;nQuests < pData[0];nQuests++) {
			// Calculate the index into this quest
			int nQuest = nQuests * 4 + 1;

			long nNumber = (long) pData[ nQuest + 0 ];
// Unused - space held	int nFlags = pData[ nQuest + 1 ];
			int nStartTime = pData[ nQuest + 2 ];
			int nEndTime = pData[ nQuest + 3 ];

			// check for quest expiration
			int time = getseconds();
			int defunct = 0;

			if ( nEndTime != 0 && (nEndTime + _QUEST_DELAY) < time )  
				defunct = 1;
			else if ( nEndTime == 0 && (nStartTime + _QUEST_DELAY) < time )
				defunct = 1;

			Quest *quest = ::findQuest ( nNumber );

			if ( quest && !defunct ) {
				QuestInfo *info = new QuestInfo ( quest );
				info->startTime = nStartTime;
				info->endTime = nEndTime;

				addQuest ( info );
			}
		}
	}
}

void RMPlayer::addText ( RMPlayer *thePlayer, char *text )
{
	if ( isNPC || !text )
		return;

	// skip out if tossing
	if ( tossing ) {
		return;
	}

	if ( !textLogs ) {
		textLogs = new chatLog[ 128 ];
	}

	int nLoc = textCount & 0x0000007F;

	strncpy ( textLogs[nLoc].sText, text, 360 );
	strncpy ( textLogs[nLoc].sToon, thePlayer->getName(), 36 );
	strncpy ( textLogs[nLoc].sAccount, thePlayer->getLogin(), 36 );
	textLogs[nLoc].nRoom = thePlayer->room ? thePlayer->room->number : -1;
	
	if( thePlayer->channel ) {
		if( thePlayer->channel->getPassword() == NULL ) {
			textLogs[nLoc].isPrivateChannel = false;
		} else {
			textLogs[nLoc].isPrivateChannel = true;
		}
		textLogs[nLoc].nChannel = thePlayer->channel->number;
	} else {
		textLogs[nLoc].nChannel = -1;
		textLogs[nLoc].isPrivateChannel = false;
	}

	textCount++;
}

void RMPlayer::reportText ( char* name, char* str, char* pReason ) {
	static int sendCount = 0;
	int i, nIndex;

	char filename[1024];
	sprintf ( sizeof ( filename ), filename, "/tmp/report.%d.%d", gServerID, sendCount );
	sendCount++;

	// complain if the name is unknown
	if ( !checkAccess ( _ACCESS_GUIDE ) ) {
		// check the names of the chat messages and make sure that the named person
		// is in there
		int isThere = 0;

		if ( textCount & 0xffffff80 ) {
			for (i = 0;i < 128; i++ ) {
				if ( !strcasecmp ( textLogs[ ( ( textCount + i ) & 0x0000007F ) ].sToon, name ) ) {
					isThere = 1;
					break;
				}
			}
		} else {
			for (i = 0;i < textCount;i++) {
				if ( !strcasecmp ( textLogs[i].sToon, name ) ) {
					isThere = 1;
					break;
				}
			}
		}

		if ( !isThere ) {
			roomMgr->sendSystemMsg ( "Unable To File A Report", this, "We can not find a chat message from '%s' in your chat history.  Check the spelling and try again.\n\nIf the last 100 lines of chat text in your chat history do not include any messages from the person you are reporting, the report will not be filed.\n\nPlease only file reports when a player policy violation has occured.  You can find the current player policy at http://www.realmserver.com/policy.  Be aware that repeated filing of false reports is cause for disciplinary action.  Please only report offensive actions.", name );
			return;
		}
	}

	unlink ( filename );

	File file;
	file.open ( filename );

	file.printf ( "From: complaint@realmserver.com\n" );
	file.printf ( "To: complaint@realmserver.com\n" );
	file.printf ( "Subject: Server %d %s (%s by %s)\n", gServerID + 1, name, pReason, getName() );

	file.printf ( "----==== comment ====----\n%s\n\n", str );

	if ( name ) {
		file.printf ( "----==== Chat Messages From '%s' ====----\n\n", name );

		if ( textCount & 0xffffff80 ) {
			for (i = 0;i < 128; i++ ) {
				nIndex = ( ( textCount + i ) & 0x0000007F );

				if ( !strcasecmp ( textLogs[ nIndex ].sToon, name ) ) {
					file.printf ( "(% 8s, c% 3d [%s], r% 6d) %s", textLogs[nIndex].sAccount, textLogs[nIndex].nChannel, textLogs[nIndex].isPrivateChannel ? "prv" : "pub", textLogs[nIndex].nRoom, textLogs[nIndex].sText );
				}
			}
		} else {
			for (i = 0;i < textCount;i++) {
				if ( !strcasecmp ( textLogs[i].sToon, name ) ) {
					file.printf ( "(% 8s, c% 3d [%s], r% 6d) %s", textLogs[i].sAccount, textLogs[i].nChannel, textLogs[i].isPrivateChannel ? "prv" : "pub", textLogs[i].nRoom, textLogs[i].sText );
				}
			}
		}
	}

	file.printf ( "\n----==== Complete Chat History ====----\n\n" );

	if ( m_pLastComplain ) {
		delete [] m_pLastComplain;
		m_pLastComplain = NULL;
	}

	m_pLastComplain = new char[56000];
	m_pLastComplain[0] = 0;

	char sOutput[1000];

	if ( textCount & 0xffffff80 ) {
		for (i = 0;i < 128; i++ ) {
			nIndex = ( ( textCount + i ) & 0x0000007F );

			file.printf ( "(% 8s, c% 3d [%s], r% 6d) %s", textLogs[nIndex].sAccount, textLogs[nIndex].nChannel, textLogs[nIndex].isPrivateChannel ? "prv" : "pub", textLogs[nIndex].nRoom, textLogs[nIndex].sText );

			sprintf( sizeof(sOutput), sOutput, "(% 8s, c% 3d [%s], r% 6d) %s", textLogs[nIndex].sAccount, textLogs[nIndex].nChannel, textLogs[nIndex].isPrivateChannel ? "prv" : "pub", textLogs[nIndex].nRoom, textLogs[nIndex].sText );
			strcat( m_pLastComplain, sOutput );
			m_pLastComplain[55999] = '\0';
		}
	} else {
		for (i = 0;i < textCount;i++) {
			file.printf ( "(% 8s, c% 3d [%s], r% 6d) %s", textLogs[i].sAccount, textLogs[i].nChannel, textLogs[i].isPrivateChannel ? "prv" : "pub", textLogs[i].nRoom, textLogs[i].sText );

			sprintf( sizeof(sOutput), sOutput, "(% 8s, c% 3d [%s], r% 6d) %s", textLogs[i].sAccount, textLogs[i].nChannel, textLogs[i].isPrivateChannel ? "prv" : "pub", textLogs[i].nRoom, textLogs[i].sText );
			strcat( m_pLastComplain, sOutput );
			m_pLastComplain[55999] = '\0';
		}
	}

	char command[1024];

	if ( checkAccess ( _ACCESS_GUIDE ) ) {
		sprintf ( sizeof ( command ), command, "nohup sendgmcomplaint %s &", filename );
		system ( command );
	} else {
		sprintf ( sizeof ( command ), command, "nohup sendcomplaint %s &", filename );
		system ( command );
	}
}

void RMPlayer::addQuest ( QuestInfo *info )
{
	if ( !quests ) 
		quests = new LinkedList;

	quests->add ( info );
}

void RMPlayer::delQuest ( QuestInfo *info )
{
	quests->del ( info );

	if ( !quests->size() ) {
		delete quests;
		quests = NULL;
	}
}

QuestInfo *RMPlayer::findQuest ( long number )
{
	if ( !quests )
		return NULL;

	LinkedElement *element = quests->head();

	while ( element ) {
		QuestInfo *quest = (QuestInfo *)element->ptr();

		if ( quest->quest->number == number )
			return quest;

		element = element->next();
	}

	return NULL;
}

void RMPlayer::writeQuestData ( void ) {
	// write quest data for my character -- if any
	if ( character ) {
		if ( quests ) {
			int* pData = (int*) malloc( ( quests->size() * 16 ) + 4 );

			pData[0] = quests->size();

			LinkedElement *element = quests->head();

			int nQuests = 0;

			while ( element ) {
				int nQuest = nQuests * 4 + 1;

				QuestInfo *quest = (QuestInfo *)element->ptr();

				pData[ nQuest + 0 ] = (int) quest->quest->number;
				pData[ nQuest + 1 ] = 0;
				pData[ nQuest + 2 ] = quest->startTime;
				pData[ nQuest + 3 ] = quest->endTime;

				element = element->next();
			}

			gDataMgr->writeQuests( character, ( ( quests->size() * 16 ) + 4 ), pData );

			free( pData );
		}
	}
}

void RMPlayer::writeCrimes() {
	if ( m_pCrimes )
		gDataMgr->writeCrimes( character, sizeof(CrimeData), m_pCrimes );
}

void RMPlayer::ignore( char* pName ) {
        if ( !pName )
                return;

        if ( isIgnoring( pName ) )
                return;

        if ( !ignoreList )
                ignoreList = new LinkedList;

        ignoreList->add ( new StringObject ( pName ) );
}

void RMPlayer::unignore( char* pName ) {
        if ( !ignoreList || !pName )
                return;

        LinkedElement *element = ignoreList->head();

        while ( element ) {
                StringObject *name = (StringObject *)element->ptr();

                if ( !strcasecmp ( name->data, pName ) ) {
                        ignoreList->delElement ( element );

                        gDataMgr->DelFoe( this, name->data );

                        delete name;
                        break;
                }

                element = element->next();
        }

        if ( !ignoreList->size() ) {
                delete ignoreList;
                ignoreList = NULL;
        }
}

int RMPlayer::isIgnoring( char* pName ) {
        if ( !ignoreList || !pName )
                return 0;

	RMPlayer* target = findPlayer ( pName, this );

	if ( target && target->checkAccess( _ACCESS_PRIVILEGED ) ) {
		unignore( pName );
		return 0;
	}

        LinkedElement *element = ignoreList->head();

        while ( element ) {
                StringObject *name = (StringObject *)element->ptr();

                if ( !strcasecmp ( name->data, pName ) )
                        return 1;

                element = element->next();
        }

        return 0;
}

int RMPlayer::canAddMember() {
	if ( groupLeader == NULL )
		return 1;

	if ( group.size() < _MAX_GROUP_SIZE )
		return 1;

	return 0;
}

int RMPlayer::addGroupMember ( RMPlayer *member )
{
	if ( groupLeader == NULL && character ) {

		if ( isNPC ) 
			zone->numGroups++;
		
		groupLeader = this;
		group.add ( this );
		
		PackedMsg response;
		response.putLong ( character->servID );
		response.putString ( getName() );
	
		roomMgr->sendTo ( _IPC_GROUP_JOIN, response.data(), response.size(), this );
	}
		
	if ( isNPC ) {
		if ( group.size() < _MAX_NPC_GROUP_SIZE ) {
			group.add ( member );
			member->groupLeader = this;
			return 1;
		}
	} else {
		if ( group.size() < _MAX_GROUP_SIZE ) {
			group.add ( member );
			member->groupLeader = this;
			return 1;
		}
	}

	return 0;
}

int RMPlayer::delGroupMember ( RMPlayer *member )
{
	if ( group.contains ( member ) ) {
		member->groupLeader = NULL;
		group.del ( member );

		if ( group.size() == 1 ) {
			//if we are left as the only person in our group,
			//and somehow we happen to be an NPC, we become the leader,
			//but the group dissolves, so if we were spawned as a group, we should
			//go back and delete that group.
			if( groupList )
				groupList->del( (ListObject*)&group );
			groupList = NULL;
			groupLeader = NULL;
			group.release();

			if ( isNPC ) 
				zone->numGroups--;
		}
		return 1;
	}
	return 0;
}

int RMPlayer::isGroupMember ( RMPlayer *member )
{
	if ( !groupLeader )
		return 0;

	return groupLeader->group.contains ( member );
}

//mike
bool RMPlayer::isGroupLeader ( void )
{
	return ( groupLeader == this );
}

void RMPlayer::sendGroupText ( const char *format, ... )
{
	if ( !groupLeader )
		return;

	char text[1024];

	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	PackedData response;

	response.putLong ( -1 );
	response.putLong ( servID );
	response.putArray ( text, strlen ( text ) + 1 );

	sendToGroup ( _IPC_PLAYER_TEXT, response.data(), response.size() );
}

void RMPlayer::sendGroupChat ( const char *format, ... )
{
	if ( !groupLeader )
		return;

	char text[1024];

	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	LinkedElement *element = groupLeader->group.head();

	while ( element ) {
		RMPlayer *member = (RMPlayer *)element->ptr();
		element = element->next();

		member->addText ( this, text );
	}

	char* pText = text;

	while ( *pText && *pText != ':' ) pText++;

	if ( *pText )
		pText++;

	logChatData ( "%s:%s:G %s:%s", getLogin(), getName(), groupLeader->getName(), pText );

	PackedData response;

	response.putLong ( -1 );
	response.putLong ( servID );
	response.putArray ( text, strlen ( text ) + 1 );

	sendToGroup ( _IPC_PLAYER_TEXT, response.data(), response.size() );
}


int RMPlayer::joinGroup ( RMPlayer *player )
{
	if ( player->addGroupMember ( this ) && character ) {
		PackedMsg response;
		response.putLong ( character->servID );
		response.putString ( getName() );

		sendToGroup ( _IPC_GROUP_JOIN, response.data(), response.size(), this );

		return 1;
	}

	return 0;
}

void RMPlayer::leaveGroup ( void )
{
	if ( groupLeader == this )	{
		groupLeader->disbandGroup();
	} else	{
		//mike-groupspawn - if I leave a group, and i'm not the leader, I
		// shouldn't be worrying about groupList anyway, and it should already
		// BE NULL, but lets reset to NULL just in case something weird happens
		// and I end up telling the zone that my group was destroyed, when it wasn't
		groupList = NULL;
		PackedMsg response;
		response.putLong ( character->servID );
		sendToGroup ( _IPC_GROUP_LEAVE, response.data(), response.size() );

		groupLeader->delGroupMember ( this );	
	}
}

void RMPlayer::kickGroupMember ( RMPlayer *player )
{
	PackedMsg response;
	response.putLong ( player->character->servID );
	groupLeader->sendToGroup ( _IPC_GROUP_KICK, response.data(), response.size() );

	groupLeader->delGroupMember ( player );

	//player is no longer in a group.
	player->groupList = NULL;
}

void RMPlayer::disbandGroup ( void )
{
	//mike-groupspawn
	if( groupList ) {
		//if here, then I am the leader of my group, and I am also part
		//of a group that was spawned, as a group, by a zone. I need to go and
		//tell the zone group manager ( MonsterGroup object ) that my group
		//has been destroyed, so it can recreate a group just like mine later on
		groupList->del( (ListObject*)&group );
	}

	groupList = NULL;

	PackedMsg response;
	response.putLong ( character->servID );
	sendToGroup ( _IPC_GROUP_LEAVE, response.data(), response.size() );

	LinkedElement *element = group.head();

	while ( element ) {
		RMPlayer *member = (RMPlayer *)element->ptr();
		element = element->next();

		if ( member != this )
			delGroupMember ( member );
	}

	delGroupMember ( this );
}

void RMPlayer::sendToGroup ( int type, unsigned char *buffer, int size, RMPlayer *exclusion )
{
	LinkedElement *element = groupLeader->group.head();

	while ( element ) {
		RMPlayer *member = (RMPlayer *)element->ptr();

		if ( member != exclusion )
			roomMgr->sendTo ( type, buffer, size, member );

		element = element->next();
	}
}

int RMPlayer::processRequest ( IPCMessage *message )
{
	int retVal = 1;

	// handle non-authenticated messages
	switch ( message->type() ) {
		case _IPC_TERMINATED: 
			return process_IPC_TERMINATED ( message );

		case _IPC_LOGIN_DONE: 
			return process_IPC_LOGIN_DONE ( message );

		case _IPC_PLAYER_LOGOUT: {
			// this player quit in combat, minor reaming to discourage "plug".
			// REMOVED PER ORDERS
//			if (  character && character->combatGroup && ( character->health > 0 ) && character->opposition->size() ) {
//				character->physicalState |= _STATE_WEINER;
//				character->changeHealth ( -character->health, character, 1, 1 );
//			}

			return retVal;
		}

		break;

		// handle logging out
		case _IPC_CLIENT_HUNG_UP: {
			if ( message->to() != (void *)-1 ) {
				BPlayer *bplayer = player? (BPlayer *)player->getBase ( _BPLAYER ) : NULL;
				logInfo ( _LOG_ALWAYS, "***** HEY STUPID ***** %s: got _IPC_CLIENT_HUNG_UP", bplayer? bplayer->login : "unknown" );
				return retVal;
			}

			return process_IPC_CLIENT_HUNG_UP ( message );
		}

		// handle hacked message information
		case _IPC_CLIENT_HACKED_MSG: {
			if ( player ) {
				PackedMsg packet ( message->data(), message->size() );
				int nReason = packet.getByte();

				BPlayer *bplayer = (BPlayer *)player->getBase ( _BPLAYER );

				switch ( nReason ) {
					case 1:
						logHack ( "%s: got hacked msg indicator on receiving a message", bplayer->login );
						forceLogout();

					break;

					case 2:
						logHack ( "%s: got hacked msg indicator on sending a message", bplayer->login );
						forceLogout();

					break;
				
					case 3:
						logHack ( "%s: got hacked msg indicator on msg size", bplayer->login );
						forceLogout();

					break;

					default:
						logHack ( "%s: got hacked msg indicator with %d", bplayer->login, nReason );
						forceLogout();

					break;
				}
			}

			return retVal;
		}

		break;

		// handle logging in
		case _IPC_INN_LOGIN: {
			retVal = process_IPC_INN_LOGIN ( message );
		}

		break;

		// handle character login
		case _IPC_PLAYER_CHARACTER_LOGIN: {
			retVal = process_IPC_PLAYER_CHARACTER_LOGIN ( message );
		}

		break;

		// handle character login
		case _IPC_PLAYER_OLD_CHARACTER_LOGIN: {
			retVal = process_IPC_PLAYER_OLD_CHARACTER_LOGIN ( message );
		}

		break;

		// handle this player changing to a new room (absolutely)
		case _IPC_PLAYER_CHANGE_ROOM: {
			retVal = process_IPC_PLAYER_CHANGE_ROOM ( message );
		}

		break;

		// handle getting the room number for a house
		case _IPC_GET_HOUSE: {
			retVal = process_IPC_GET_HOUSE ( message );
		}

		break;

		// handle getting the entry information of an object
		case _IPC_GET_ENTRY_INFO: {
			retVal = process_IPC_GET_ENTRY_INFO ( message );
		}

		break;

		// talk to an NPC
		case _IPC_TALK: {
			retVal = process_IPC_TALK ( message );
		}

		break;

		// handle changing to an adjacent room
		case _IPC_PLAYER_SHIFT_ROOM: {
			retVal = process_IPC_PLAYER_SHIFT_ROOM ( message );
		}

		break;			

		// handle this player's movie message
		case _IPC_PLAYER_MOVIE: {
			retVal = process_IPC_PLAYER_MOVIE ( message );
		}

		break;

		// handle a chat message that goes to this player's room
		case _IPC_PLAYER_ROOM_CHAT: {
			retVal = process_IPC_PLAYER_ROOM_CHAT ( message );
		}

		break;

		// handle system notify messages
		case _IPC_PLAYER_SYSTEM_MSG: {
			retVal = process_IPC_PLAYER_SYSTEM_MSG ( message );
		}

		break;

		// handle a player to player chat
		case _IPC_PLAYER_CHAT: {
			retVal = process_IPC_PLAYER_CHAT ( message );
		}

		break;

		// handle creating a new character for this BPlayer
		case _IPC_PLAYER_CREATE_CHARACTER: {
			retVal = process_IPC_PLAYER_CREATE_CHARACTER ( message );
		}

		break;

		// handle destroying an existing character for this BPlayer
		case _IPC_PLAYER_DESTROY_CHARACTER: {
			retVal = process_IPC_PLAYER_DESTROY_CHARACTER ( message );
		}

		break;

		// handle querying this BPlayer for it's characters
		case _IPC_PLAYER_QUERY_CHARACTERS: {
			retVal = process_IPC_PLAYER_QUERY_CHARACTERS ( message );
		}

		break;

		// handle getting extended properties of an object
		case _IPC_PLAYER_GET_EXTENDED_PROPS: {
			retVal = process_IPC_PLAYER_GET_EXTENDED_PROPS ( message );
		}

		break;

		// handle setting head appearance data
		case _IPC_PLAYER_SET_HEAD_DATA: {
			retVal = process_IPC_PLAYER_SET_HEAD_DATA ( message );
		}

		break;

		// handle updating attributes
		case _IPC_UPDATE_ATTRIBUTES: {
			retVal = process_IPC_UPDATE_ATTRIBUTES ( message );
		}

		break;

		// handle getting the text description of an object
		case _IPC_PLAYER_GET_DESCRIPTION: {
			retVal = process_IPC_PLAYER_GET_DESCRIPTION ( message );
		}

		break;

		// handle getting the text biography for an object
		case _IPC_PLAYER_GET_BIOGRAPHY: {
			retVal = process_IPC_PLAYER_GET_BIOGRAPHY ( message );
		}

		break;

		// handle setting the text biography for an object
		case _IPC_PLAYER_SET_BIOGRAPHY: {
			retVal = process_IPC_PLAYER_SET_BIOGRAPHY ( message );
		}

		break;

		// handle taking an object
		case _IPC_VERB_GET: {
			retVal = process_IPC_VERB_GET ( message );
		}

		break;

		// handle dyeing an object
		case _IPC_VERB_DYE: {
			retVal = process_IPC_VERB_DYE ( message );
		}

		break;

		// handle "using" an object
		case _IPC_VERB_USE: {
			retVal = process_IPC_VERB_USE ( message );
		}

		break;

		// handle giving an object
		case _IPC_VERB_GIVE: {
			retVal = process_IPC_VERB_GIVE ( message );
		}

		break;

		case _IPC_GET_QUEST_LIST: {
			retVal = process_IPC_GET_QUEST_LIST ( message );
		}

		break;

		case _IPC_VERB_DROP: {
			retVal = process_IPC_VERB_DROP ( message );
		}

		break;

		case _IPC_VERB_PUT_ON: {
			retVal = process_IPC_VERB_PUT_ON ( message );
		}

		break;

		case _IPC_VERB_TAKE_OFF: {
			retVal = process_IPC_VERB_TAKE_OFF ( message );
		}

		break;

		case _IPC_VERB_PUT_IN: {
			retVal = process_IPC_VERB_PUT_IN ( message );
		}

		break;

		case _IPC_MIX_OBJECT: {
			retVal = process_IPC_MIX_OBJECT ( message );
		}

		break;

		case _IPC_MAIL_LIST_GET: {
			retVal = process_IPC_MAIL_LIST_GET ( message );
		}

		break;

		case _IPC_MAIL_MESSAGE_GET: {
			retVal = process_IPC_MAIL_MESSAGE_GET ( message );
		}

		break;

		case _IPC_MAIL_MESSAGE_DELETE: {
			retVal = process_IPC_MAIL_MESSAGE_DELETE ( message );
		}

		break;

		case _IPC_MAIL_MESSAGE_SEND: {
			retVal = process_IPC_MAIL_MESSAGE_SEND ( message );
		}

		break;

		case _IPC_MAIL_MESSAGE_ARCHIVE: {
			retVal = process_IPC_MAIL_MESSAGE_ARCHIVE ( message );
		}

		break;

		case _IPC_MAIL_MESSAGE_COMPLAIN: {
			retVal = process_IPC_MAIL_MESSAGE_COMPLAIN ( message );
		}

		break;

		case _IPC_SEND_REG: {
			retVal = process_IPC_SEND_REG ( message );
		}

		break;

		case _IPC_VERB_OPEN: {
			retVal = process_IPC_VERB_OPEN ( message );
		}

		break;

		case _IPC_VERB_CLOSE: {
			retVal = process_IPC_VERB_CLOSE ( message );
		}

		break;

		case _IPC_VERB_LOCK: {
			retVal = process_IPC_VERB_LOCK ( message );
		}

		break;

		case _IPC_VERB_UNLOCK: {
			retVal = process_IPC_VERB_UNLOCK ( message );
		}

		break;

		case _IPC_COMBAT_ACTION: {
			retVal = process_IPC_COMBAT_ACTION ( message );
		}

		break;

		// handle engaging in combat with another WorldObject
		case _IPC_VERB_ENGAGE: {
			retVal = process_IPC_VERB_ENGAGE ( message );
		}

		break;

		// handle consuming a WorldObject
		case _IPC_VERB_CONSUME: {
			retVal = process_IPC_VERB_CONSUME ( message );
		}

		break;

		// handle sitting on an object
		case _IPC_VERB_SIT: {
			retVal = process_IPC_VERB_SIT ( message );
		}

		break;

		// handle memorizing a scroll
		case _IPC_VERB_MEMORIZE: {
			retVal = process_IPC_VERB_MEMORIZE ( message );
		}

		break;

		// handle robbing someone
		case _IPC_VERB_ROB: {
			retVal = process_IPC_VERB_ROB ( message );
		}

		break;

		// handle standing back up
		case _IPC_VERB_STAND: {
			retVal = process_IPC_VERB_STAND ( message );
		}

		break;

		// handle a request to flee from combat
		case _IPC_COMBAT_FLEE: {
			retVal = process_IPC_COMBAT_FLEE ( message );
		}

		break;

		// handle a request to leave combat
		case _IPC_COMBAT_EXIT: {
			retVal = process_IPC_COMBAT_EXIT ( message );
		}

		break;

		// handle requesting shop inventory information
		case _IPC_GET_SHOP_INFO: {
			retVal = process_IPC_GET_SHOP_INFO ( message );
		}

		break;

		// handle setting the title of a character
		case _IPC_SET_TITLE: {
			retVal = process_IPC_SET_TITLE ( message );
		}

		break;

		// handle selling something to a shop
		case _IPC_SHOP_SELL: {
			retVal = process_IPC_SHOP_SELL ( message );
		}

		break;

		// handle getting what a shop will pay for an object
		case _IPC_SHOP_GET_PRICE: {
			retVal = process_IPC_SHOP_GET_PRICE ( message );
		}

		break;

		// handle getting the price to recharge a useable item
		case _IPC_SHOP_GET_RECHARGE_PRICE: {
			retVal = process_IPC_SHOP_GET_RECHARGE_PRICE ( message );
		}

		break;

		// handle getting the price to recharge a useable item
		case _IPC_SHOP_RECHARGE: {
			retVal = process_IPC_SHOP_RECHARGE ( message );
		}

		break;

		// handle getting the repair price of a piece of armor
		case _IPC_GET_REPAIR_PRICE: {
			retVal = process_IPC_GET_REPAIR_PRICE ( message );
		}

		break;

		// handle getting the repair price of a piece of armor
		case _IPC_GET_REPAIR_PRICES: {
			retVal = process_IPC_GET_REPAIR_PRICES ( message );
		}

		break;
		
		// handle repairing an item of armor
		case _IPC_REPAIR: {
			retVal = process_IPC_REPAIR ( message );
		}

		break;

		// handle repairing a item(s)
		case _IPC_MASS_REPAIR: {
			retVal = process_IPC_MASS_REPAIR ( message );
		}

		break;
		
		// handle buying something from a shop
		case _IPC_SHOP_BUY: {
			retVal = process_IPC_SHOP_BUY ( message );
		}

		break;

		// handle examining something from a shop
		case _IPC_SHOP_EXAMINE: {
			retVal = process_IPC_SHOP_EXAMINE ( message );
		}

		break;

		// handle dropping some money
		case _IPC_MONEY_DROP: {
			retVal = process_IPC_MONEY_DROP ( message );
		}

		break;

		// handle putting money in a container
		case _IPC_MONEY_PUT: {
			retVal = process_IPC_MONEY_PUT ( message );
		}

		break;

		// handle taking money 
		case _IPC_MONEY_TAKE: {
			retVal = process_IPC_MONEY_TAKE ( message );
		}

		break;

		// handle giving money to another object
		case _IPC_MONEY_GIVE: {
			retVal = process_IPC_MONEY_GIVE ( message );
		}

		break;

		// handle casting a spell
		case _IPC_CAST_SPELL: {
			retVal = process_IPC_CAST_SPELL ( message );
		}

		break;

		case _IPC_GET_BOOK_INFO: {
			retVal = process_IPC_GET_BOOK_INFO ( message );
		}

		break;

		case _IPC_GET_BOOK_PAGE: {
			retVal = process_IPC_GET_BOOK_PAGE ( message );
		}

		break;

		// get the top page for a talk tree
		case _IPC_TREE_GET: {
			retVal = process_IPC_TREE_GET ( message );
		}

		break;

		// choose a topic for the current page of a talk tree
		case _IPC_TREE_CHOOSE_TOPIC: {
			retVal = process_IPC_TREE_CHOOSE_TOPIC ( message );
		}

		break;

		// get some text from the current talk tree page
		case _IPC_TREE_GET_TEXT: {
			retVal = process_IPC_TREE_GET_TEXT ( message );
		}

		break;

		// go back one page on the talk tree
		case _IPC_TREE_BACK: {
			retVal = process_IPC_TREE_BACK ( message );
		}

		break;

		// handle accepting a quest
		case _IPC_QUEST_ACCEPT: {
			retVal = process_IPC_QUEST_ACCEPT ( message );
		}

		break;

		// handle declining a quest
		case _IPC_QUEST_DECLINE: {
			retVal = process_IPC_QUEST_DECLINE ( message );
		}

		break;

		case _IPC_FATAL_DATA: {
			retVal = process_IPC_FATAL_DATA ( message );
		}

		break;

		// handle locking access to an object
		case _IPC_LOCK_OBJ: {
			retVal = process_IPC_LOCK_OBJ ( message );
		}

		break;

		// handle unlocking access to an object
		case _IPC_UNLOCK_OBJ: {
			retVal = process_IPC_UNLOCK_OBJ ( message );
		}

		break;

		// handle joining a player's group
		case _IPC_GROUP_JOIN: {
			retVal = process_IPC_GROUP_JOIN ( message );
		}

		break;

		// handle leaving a player's group
		case _IPC_GROUP_LEAVE: {
			retVal = process_IPC_GROUP_LEAVE ( message );
		}

		break;

		// handle kicking out a group member
		case _IPC_GROUP_KICK: {
			retVal = process_IPC_GROUP_KICK ( message );
		}

		break;

		// handle kicking out a group member
		case _IPC_GROUP_QUESTION: {
			retVal = process_IPC_GROUP_QUESTION ( message );
		}

		break;

		// handle getting the position information for an object
		case _IPC_GET_POSN: {
			retVal = process_IPC_GET_POSN ( message );
		}

		break;

		// handle changing the password associated with an object
		case _IPC_CHANGE_PASSWORD: {
			retVal = process_IPC_CHANGE_PASSWORD ( message );
		}

		break;

		// handle creating a new object
		case _IPC_CREATE_OBJECT: {
			retVal = process_IPC_CREATE_OBJECT ( message );
		}

		break;

		// handle finding out what's new
		case _IPC_WHATS_NEW: {
			retVal = process_IPC_WHATS_NEW ( message );
		}

		break;

		// handle setting the engrave name
		case _IPC_SET_ENGRAVE_NAME: {
			retVal = process_IPC_SET_ENGRAVE_NAME ( message );
		}

		break;

		// handle getting the look info
		case _IPC_GET_LOOK_INFO: {
			retVal = process_IPC_GET_LOOK_INFO ( message );
		}

		break;

		// handle selling crystals
		case _IPC_SELL_CRYSTALS: {
			retVal = process_IPC_SELL_CRYSTALS ( message );
		}

		break;

		// handle processing a mass sell request
		case _IPC_MASS_SELL: {
			retVal = process_IPC_MASS_SELL ( message );
		}

		break;

		// handle processing a mass buy request
		case _IPC_MASS_BUY: {
			retVal = process_IPC_MASS_BUY ( message );
		}

		break;

		// handle getting the prices of items in the player's inventory
		case _IPC_GET_SELL_PRICES: {
			retVal = process_IPC_GET_SELL_PRICES ( message );
		}

		break;

		// handle creating the channel
		case _IPC_CREATE_CHANNEL: {
			retVal = process_IPC_CREATE_CHANNEL ( message );
		}

		break;

		case _IPC_WRITE_SPELLS: {
			gDataMgr->WriteSpells( this, message );
		}

		break;

		default:
			logHack ( "unhandled message type %d of %d", message->type(), message->size() );
			break;
	}

#if 0
	if ( character ) {

		BPlayer *bplayer = player? (BPlayer *)player->getBase ( _BPLAYER ) : NULL;

		if ( !character || !character->player  ) 
			logInfo ( _LOG_ALWAYS, "character player does not match after message %d, %s", message->type(), bplayer? bplayer->login : "unknown" );
			forceLogout();

			return retVal;
		}
		
		if ( !character->getBase ( _BCHARACTER ) ) {
			logInfo ( _LOG_ALWAYS, "character does not have BCHARACTER base on message %d, %s", message->type(), bplayer? bplayer->login : "unknown" );
			forceLogout();

			return retVal;
		}
	}
#endif

	return retVal;
}

int RMPlayer::process_IPC_WHATS_NEW ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;

	gDataMgr->whatsNew( character );

	roomMgr->sendPlayerText ( this, "|c65|File Manager> Retrieving what's new message...\n" );

	handsOn();
		
	return retVal;
}

int RMPlayer::process_IPC_SELL_CRYSTALS ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	packet.getLong(); // unused data
	long numToSell = packet.getLong();

	if ( numToSell < 0)	{
		character->manaValue = 0;
		character->value = 0;
//		disable ( "character %s sold %d mana in room %s(%d)", getName(), numToSell, roomTitle? roomTitle : "???", room->number );
		return retVal;
	}

	if ( character->manaValue >= numToSell ) {
		character->manaValue -= numToSell;
		character->value += numToSell * 5;

		PackedMsg response;

		// put ACK info
		response.putACKInfo ( message->type() );
		response.putLong ( numToSell * 5 );
		response.putLong ( numToSell );

		roomMgr->sendTo ( _IPC_PLAYER_ACK, &response, this );
	} else 
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );

	return retVal;
}

int RMPlayer::process_IPC_GET_LOOK_INFO ( IPCMessage *message )
{
	int retVal = 1;

	// no character->player test here.

	PackedMsg packet ( message->data(), message->size() );
	WorldObject *obj = NULL;

	// get the object to get the info on
	WorldObject *object = roomMgr->findObject ( packet.getLong() );

	if ( !object ) {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		handsOn();
		return retVal;
	}

	BCharacter *bChar = (BCharacter *)object->getBase ( _BCHARACTER );

	if ( bChar ) {

		PackedMsg response;
		response.putACKInfo ( message->type() );

		int areas[] = { _WEAR_HEAD, _WEAR_NECK, _WEAR_CHEST, _WEAR_BANDS, _WEAR_LEGS, _WEAR_FEET, _WEAR_SHIELD, -1 };

		// put armor information first
		int index = 0;

		while ( areas[index] != -1 ) {
			if ( (obj = object->getWornOn ( areas[index] )) ) {
				response.putString ( obj->getName() );
				response.putWord ( obj->armorRating() );
				response.putWord ( (obj->health * 100) / obj->healthMax );
			} else {
				response.putString ( "None" );
				response.putWord ( 0 );
				response.putWord ( 0 );
			}

			index++;
		}

		int minDamage = 1, maxDamage = 4, strength = object->calcStrength(), strDiff = 0, percent = 100;
		
		strDiff = strength - 10;

		if ( strDiff < 0 ) {
			percent = 100 + (5 * strDiff);
		}

		else if ( strDiff > 0 ) {
			percent = 100 + (13 * strDiff);
		}

		WorldObject *curWeapon = object->curWeapon;

		if ( curWeapon ) {
			BWeapon *weapon = (BWeapon *)curWeapon->getBase ( _BWEAPON );

			if ( weapon ) {
				int healthPct = 100; //(curWeapon->health * 100) / curWeapon->healthMax;

				if ( healthPct >= 50 ) {
					healthPct = 100 - ((100 - healthPct) / 10);
				} 
				else {
					healthPct = 60 - (50 - healthPct);
				}

				minDamage = std::max( 1, ((weapon->minDamage * healthPct) / 100 ));//1 >? ( (weapon->minDamage * healthPct) / 100 );
				maxDamage = std::max( 1, ( (weapon->maxDamage * healthPct) / 100 ));//1 >? ( (weapon->maxDamage * healthPct) / 100 );

				response.putString ( curWeapon->getName() );
				response.putWord ( minDamage );
				response.putWord ( maxDamage );
				response.putWord ( (curWeapon->health * 100) / curWeapon->healthMax );
			} else {
				response.putString ( "Bad" );
				response.putWord ( 0 );
				response.putWord ( 0 );
				response.putWord ( 0 );
			}
		} else {
			response.putString ( "None" );
			response.putWord ( 0 );
			response.putWord ( 0 );
			response.putWord ( 0 );
		}

		// put combat info
		response.putByte ( object->calcNumAttacks() );
		response.putByte ( object->calcNumDodges() );
		response.putByte ( object->calcNumBlocks() );
		response.putWord ( std::max(1, (minDamage * percent) / 100) ); //1 >? ( (minDamage * percent) / 100 )
		response.putWord ( std::max(1, (maxDamage * percent) / 100) ); //1 >? ( (maxDamage * percent) / 100 )
		response.putByte ( object->calcNumMoves() );

		// put carry capacity
		BContainer *bcontain = (BContainer *)object->getBase ( _BCONTAIN );

		if ( bcontain ) {
			response.putWord ( bcontain->weight / 10 );
			response.putWord ( bcontain->weightCapacity / 10 );
		} else {
			response.putWord ( -1 );
			response.putWord ( -1 );
		}

		// if we haven't entered the game yet, we do not know if we are criminal by this method
		// since a non-object cannot load a file

		CrimeData* crime = getCrimeData();
		response.putLong ( crime->bountyOnHead );
		response.putByte ( crime->criminal );
	
		BCharacter *bChar = (BCharacter *)object->getBase ( _BCHARACTER );
		
		if ( bChar ) {
			response.putLong ( bChar->npcKills );
			response.putLong ( bChar->playerKills );

			response.putLong( (int) ( bChar->m_fManaDrain * 100 ) );
			response.putLong( bChar->m_nMeleeArmorPiercing );
		        response.putLong( bChar->m_nEvilMDMMod );
			response.putLong( bChar->m_nGoodMDMMod );

			char sTemp[100];
			
			sprintf( sizeof(sTemp), sTemp, "S:%d E:%d M:%d T:%d N:%d", bChar->m_anCastResistance[0], bChar->m_anCastResistance[1], bChar->m_anCastResistance[2], bChar->m_anCastResistance[3], bChar->m_anCastResistance[4] );
			response.putString( sTemp );
			
			sprintf( sizeof(sTemp), sTemp, "S:%d E:%d M:%d T:%d N:%d", bChar->m_anSpellResistance[0], bChar->m_anSpellResistance[1], bChar->m_anSpellResistance[2], bChar->m_anSpellResistance[3], bChar->m_anSpellResistance[4] );
			response.putString( sTemp );
		} else {
			response.putLong ( 0 );
			response.putLong ( 0 );

			response.putLong ( 0 );
			response.putLong ( 0 );
			response.putLong ( 0 );
			response.putLong ( 0 );
			response.putString( "" );
			response.putString( "" );
		}

		if ( object->player ) {
			response.putString( object->player->accountTypeStr );
			response.putLong( object->player->billingDate );
			response.putLong( object->player->nCredits );
			response.putLong( object->player->nCoppers );
			
			referralID nID( object->player->accountID );
			response.putString( nID.getpID() );

			if ( object->player->checkAccess( _ACCESS_IMPLEMENTOR | _ACCESS_PUBLICRELATIONS ) ) {
				response.putByte( 1 );
			} else if ( object->player->checkAccess( _ACCESS_PRIVILEGED ) ) {
				response.putByte( 2 );
			} else {
				response.putByte( 0 );
			}
		} else {
			response.putString( "" );
			response.putLong( 0 );
			response.putLong( 0 );
			response.putLong( 0 );
			response.putString( "" );
			response.putByte( 0 );
		}

		// put strength bonus
		response.putWord ( percent - 100 );

		// put spell bonus
		response.putWord ( calcSpellMod ( object ) - 100 );

		// put old level
		response.putWord ( object->oldLevel );

		roomMgr->sendTo ( _IPC_PLAYER_ACK, &response, this );

	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		handsOn();
	}

	return retVal;
}

int RMPlayer::process_IPC_SET_ENGRAVE_NAME ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );
	char *name = packet.getString();
	setEngraveName ( name );

	// new
	if ( name )
		free ( name );

	return retVal;
}

int RMPlayer::process_IPC_CREATE_OBJECT ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	if ( isTeleporting ) {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		return retVal;
	}

	PackedMsg packet ( message->data(), message->size() );

	// get the object that is being used to create with
	WorldObject *tool = roomMgr->findObject ( packet.getLong() );

	if ( tool ) {

		WorldObject *owningObj = tool->getBaseOwner();
		
		if ( owningObj && owningObj->player && owningObj->player != this ) {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
			return retVal;
		}

		BCharacter *bchar = (BCharacter *)character->getBase ( _BCHARACTER );
		
		if ( !character ) {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
			return retVal;
		}

		BMix *bmix = (BMix *)tool->getBase ( _BMIX );

		if ( !bmix ) {
//			disable ( "%s used invalid tool for object create using %s.", getName(), tool->getName());
			return retVal;
		}
		
		// get the name of the object to create
		char *name = packet.getString();

		if ( !name ) {
			roomMgr->sendSystemMsg ( "What Was That?", this, "You cannot make a 'nothing'!" );
			handsOn();
			return retVal;
		}
		
		int len = strlen ( name );

		if ( (len < 1) || (len > 320) ) {
			free ( name );
			roomMgr->sendSystemMsg ( "What Was That?", this, "You cannot make a 'nothing'!" );
			handsOn();
			return retVal;
		}

		// find the object to create
		WorldObject *object = roomMgr->findComponentObject ( tool->skill, name );

		int skillLevel = character->getSkill ( tool->skill );

		if ( !skillLevel ) {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		}
		else if ( !object || ( object && !object->components ) ) {
			roomMgr->sendSystemMsg ( "What Was That?", this, "Your specification '%s' is not something that can be built.", name );
		}
		else  if ( object == (WorldObject *)-1 ) {
			roomMgr->sendSystemMsg ( "Be More Specific", this, "The specification '%s' is too vague! Please be more specific.", name );
		}
		else {
			int diff = object->strength - skillLevel;
			int delta = random ( 0, 15 );
			
			if ( diff > 0 ) 
				delta = random ( 20 * diff, 30 * diff );

			else if ( diff < 0 ) 
				delta = random ( 0, 15 / (abs ( diff ) * 3) );

			int percent = 100 - delta;

			if ( percent < 1 )
				percent = 1;

			if ( percent > 100 )
				percent = 100;

			int healthPct = 100;
			
			if ( tool->healthMax) 
				healthPct = (tool->health * 100) / tool->healthMax;

			if ( healthPct < 50 ) {
				healthPct = 100 - (50 - healthPct);
				percent = (percent * healthPct) / 100;
			}

			//calculate the stamina cost
			int staminaCost = object->strength;

			if ( diff > 0 )
				staminaCost *= (diff + 1);

			LinkedList items, components;

			LinkedList *neededComponents = new LinkedList ( object->components );

			character->makeItemList ( &items, -1 );

			LinkedElement *element = items.head();

			while ( element ) {

				WorldObject *item = (WorldObject *)element->ptr();
				element = element->next();

				LinkedElement *elementA = neededComponents->head();

				while ( elementA ) {
					WorldObject *component = (WorldObject *)elementA->ptr();
					elementA = elementA->next();

					if ( component->classID && item->classID ) {
						if ( !strcmp ( component->classID, item->classID ) ) {
							components.add ( item );
							items.del ( item );
							neededComponents->del ( component );
							break;
						}
					}
				}
			}

			char missingText[1024] = "";

			element = neededComponents->head();

			while ( element ) {
				int itemCount = 1;

				WorldObject *item = (WorldObject *)element->ptr();
				element = element->next();

				while ( element && ((WorldObject *)element->ptr() == item) ) {
					itemCount++;
					element = element->next();
				}

				char str[64];
				sprintf ( sizeof ( str ), str, "%d %s%s", itemCount, item->name, (itemCount == 1)? "" : "s" );

				if ( !element && (neededComponents->size() - itemCount) )
					strcat ( missingText, "and " );

				strcat ( missingText, str );

				if ( element )
					strcat ( missingText, ", " );
			}

			if ( neededComponents->size() ) {
				roomMgr->sendSystemMsg ( "Missing Components", this, "You can not create the %s because you do not have all of the necessary components.  You still need to get %s before you can create that object.", object->name, missingText );
			} else {

				if( tool->skill != _SKILL_WEAPONSMITH && tool->skill != _SKILL_ARMORER ) {
					int theDiff = character->stamina - staminaCost;
					
					if ( theDiff < 0 ) {
						int waitTime = abs ( theDiff ) * 2;
						roomMgr->sendSystemMsg ( "Too Tired", this, "You are too tired to make that difficult of an item.  You should rest about %d minutes before you try again.", waitTime );
					
						goto createEnd;
					}

					character->stamina -= staminaCost;
				}

				element = components.head();

				PackedMsg response;
				response.putLong ( character->servID );
				response.putLong ( character->room->number );

				while ( element ) {
					WorldObject *object = (WorldObject *)element->ptr();
					element = element->next();

					response.putByte ( _MOVIE_TOSS_OBJECT );
					response.putLong ( character->servID );
					response.putLong ( object->servID );

					roomMgr->destroyObj ( object, 0, __FILE__, __LINE__ );
				}

				WorldObject *result = character->addObject ( object->classID ); 

				result->addAffect ( _AFF_IDENTIFIED, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, -1, 0, NULL, 0 );
				result->makeVisible ( 1 );					  
				result->health = (result->healthMax * percent) / 100;

				character->room->sendObjInfo ( result, NULL, &response );

				// put the object in a worn pack (if any)
				WorldObject *pBackPack = character->getWornOn ( _WEAR_PACK );

				if ( pBackPack ) {
					character->putIn ( result, pBackPack, &response );
				}

				putMovieText ( character, &response, "|c43|%s makes one %s.\n", getName(), object->getName() );	

  				bchar->gainExperience ( random ( 15 * object->strength, 40 * object->strength ), &response );

				if ( !random ( 0, 1 ) ) {
					int pctTbl[] = { 1, 2, 5, 7, 10, 20 };

					int theStr = object->strength;

					if ( theStr < 0 )
						theStr = 0;

					if ( theStr > 5 )
						theStr = 5;

					int pct = pctTbl[theStr];
					tool->health -= (tool->healthMax * pct) / 100;

					if ( tool->health <= 0 ) {
						putMovieText ( character, &response, "|c43|%s's %s breaks.\n", getName(), tool->getName() );

						response.putByte ( _MOVIE_TOSS_OBJECT );
						response.putLong ( character->servID );
						response.putLong ( tool->servID );

						roomMgr->destroyObj ( tool, 0, __FILE__, __LINE__ );
					} 

					else if ( tool->health < (tool->healthMax / 2) ) {
						putMovieText ( character, &response, "|c43|%s's %s is in pretty bad shape.\n", getName(), tool->getName() );	
					}
				}

				response.putByte ( _MOVIE_END );
				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), character->room );
			}

createEnd:
			neededComponents->release();
			delete neededComponents;

			components.release();
			items.release();
		}

		// toss that name
		if ( name ) 
			free ( name );
	}

	handsOn();

	return retVal;
}

int RMPlayer::process_IPC_CHANGE_PASSWORD ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );
	// get the object to change the password of
	WorldObject *obj = roomMgr->findObject ( packet.getLong() );

	if ( !obj )
		return retVal;

	BPassword *base = obj? (BPassword *)obj->getBase ( _BPASSWORD ) : NULL;

	if ( obj == character ) {
		char *oldPass = packet.getString();
		char *newPass = packet.getString();

		int valid = TRUE;

		if ( oldPass && newPass ) {
			int len = strlen ( newPass );

			if ( len < 1 || len > _MAX_PASSWORD_SIZE ) {
				roomMgr->sendSystemMsg ( "Invalid New Password", this, "Your new Password must be between 1 and %d characters long.", _MAX_PASSWORD_SIZE );
				valid = FALSE;
			}

			if ( valid ) {
				strlower ( oldPass );
				strlower ( newPass );
		
				for ( int i=0; i<len; i++ ) {
					if ( !isalnum ( newPass[i] ) ) {
						roomMgr->sendSystemMsg ( "Invalid New Password", this, "Your Password must contain letters and numbers only." );
						valid = FALSE;
						break;
					}
				}
			}
		} else {
			valid = FALSE;
		}
				
		if ( valid ) {
			BPlayer *bplayer = (BPlayer *)player->getBase ( _BPLAYER );

			if ( strcmp ( bplayer->password, oldPass ) ) {
				roomMgr->sendNAK ( message->type(), _ERR_BAD_PASSWORD, this );
			} else {
				if ( !strcmp ( oldPass, newPass ) ) {
					roomMgr->sendSystemMsg ( "Invalid new Password", this, "New password entries do not match." );
					return retVal;
				}

				// update our password
   			bplayer->setPassword ( newPass );
				gDataMgr->setpass ( this, newPass );
   			
  				gDataMgr->logPermanent ( getLogin(), getName(), _DMGR_PLOG_SETPASS, "%s", newPass );
  				roomMgr->sendACK ( message->type(), this );
			}
		} else 
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );

		if ( oldPass ) 
			free ( oldPass );

		if ( newPass )
			free ( newPass );
	} 

	else if ( base ) {
		Building *building = room->building;

		if ( !building || !strcasecmp ( getName(), building->_owner ) ) {
			char *oldPass = packet.getString();
			char *newPass = packet.getString();

			int isValid = TRUE;

			if ( oldPass && newPass ) {
				int len = strlen ( newPass );
	
				if ( len < 1 || len > _MAX_PASSWORD_SIZE ) {
					roomMgr->sendSystemMsg ( "Invalid New Password", this, "Your new Password must be between 1 and %d characters long.", _MAX_PASSWORD_SIZE );
					isValid = FALSE;
				}
	
				if ( isValid ) {

					char *ptr = newPass;
		
					while ( *ptr ) {
						if ( !isalnum ( *ptr ) ) {
							roomMgr->sendSystemMsg ( "Invalid New Password", this, "Your Password must contain letters and numbers only." );
							isValid = 0;
							break;
						}
						ptr++;
					}
				}
			}
			else 
				isValid = FALSE;

			if ( isValid ) {
				if ( strcmp ( oldPass, base->password ) ) {
					roomMgr->sendNAK ( message->type(), _ERR_BAD_PASSWORD, this );
				} else {
					strcpy ( base->password, newPass );
					roomMgr->sendACK ( message->type(), this );

					// password is changed, mark building as changed
					if ( building )
						building->changed = 1;
				}
			} else 
				roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );

			if ( oldPass ) 
				free ( oldPass );

			if ( newPass )
				free ( newPass );
		} else {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
			roomMgr->sendSystemMsg ( "You Can't Do That", this, "You are not allowed to change the password on anything unless it is in your own house." );
		}
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}
	return retVal;
}

// callback to complete the GET_POSN call
void cbGetPosnHouseLoad ( int result, int context, int houseID, int accountID, char *name, int size, char *data )
{
	WorldObject *obj = roomMgr->findObject ( context );
	Building *house = NULL;

	if ( !obj )
		return;

	// we have a house, let's process it
	if ( result == _DATAMGR_OKAY ) {
		// process the loaded house
		house = processHouseData ( name, data, size, houseID, accountID ); 
	} else {
		if ( gLoadingHouses.head() ) {
			StringObject *obj = (StringObject *)gLoadingHouses.head()->ptr();

			if ( obj ) {
				delete obj;
				gLoadingHouses.delElement ( gLoadingHouses.head() );
			}
		}
	}

	PackedMsg ack;
	ack.putACKInfo ( _IPC_GET_POSN );

	if ( house && house->rooms.size() ) {
  		RMRoom *theRoom = (RMRoom *)house->rooms.at ( 0 );
 	 	obj->roomNumber = theRoom->number;
  		ack.putLong ( obj->roomNumber );

		// Give permenant house if necessary.
		if ( house->homeTown == 1 && obj->player ) {
			BCharacter* bChar = (BCharacter *)obj->getBase ( _BCHARACTER );

			if ( bChar ) {
				int level = bChar->getLevel();

				if ( level >= 50 ) {
					house->homeTown = random ( _TOWN_LEINSTER_WEST, _TOWN_ARIMATHOR );
					house->changed = 1;

					roomMgr->sendSystemMsg ( "Congratulations!", obj->player, "Your permenant house is in %s.", gTownNames[ house->homeTown ] );
				}
			}
		}
	} else {
		obj->roomNumber = random ( 5000, 5089 );
  		ack.putLong ( obj->roomNumber );
	}

	if ( obj->player )
		obj->player->setTeleportRoomNum ( obj->roomNumber );

	obj->x = ( std::min(std::max(obj->x, 1), 639) );
//( ( obj->x >? 1 ) <? 639 );
	obj->y = ( std::min(std::max(obj->y, 1), 479) );
//( ( obj->y >? 1 ) <? 479 );

	ack.putWord ( obj->x );
	ack.putWord ( obj->y );
	ack.putByte ( obj->loop );

	roomMgr->sendTo ( _IPC_PLAYER_ACK, &ack, obj->player );
}

int RMPlayer::process_IPC_GET_POSN ( IPCMessage *message )
{
	int retVal = 1;

	// get the object to return info on
	PackedMsg packet ( message->data(), message->size() );
	WorldObject *obj = roomMgr->findObject ( packet.getLong() );

	if ( !obj || !player ) {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		return retVal;
	}

	BPlayer *base = (BPlayer *)player->getBase ( _BPLAYER );

	if ( !base || !base->ownsCharacter ( obj ) || player->player != this ) {
//		disable ( "character %s misdirected get position for %s", getName(), obj->getName() );
		return retVal;
	}

	BCharacter *bchar = (BCharacter *)obj->getBase ( _BCHARACTER );

	if ( !bchar ) {
		logHack ( "Logged in character is not base _BCHARACTER!", getName() );
		return retVal;
	}

	PackedMsg ack;
	ack.putACKInfo ( message->type() );

	//
	// ## UGLY ## Fix later
	//
  	RMRoom *theRoom = roomMgr->findRoom ( obj->roomNumber );

	int printDungeonMsg = 0;

	if ( theRoom && theRoom->zone ) {
	
		Zone *theZone = theRoom->zone;
	
		if ( theZone->isDungeon ) {
			if ( bchar->lastDungeon && ( theZone->id != bchar->lastDungeon ) )
				printDungeonMsg = 1;
		}
	} else {
		if ( bchar->lastDungeon )
			printDungeonMsg = 1;
	}

	if ( printDungeonMsg ) {
		roomMgr->sendSystemMsg ( "You're At Home", this, "You have been returned to your house because the dungeon that you were in when you logged out last has been reset.  You can avoid this if you make sure that you log off when you are outside of a dungeon." );
		theRoom = NULL;
		bchar->lastDungeon = 0;
	} else if ( theRoom && theRoom->bRubberWalled ) {
		theRoom = NULL;
	}

  	if ( obj->physicalState & _STATE_WEINER ) {
  		roomMgr->sendSystemMsg ( "Oh No!", this, "Your character died when you logged out last.  This is because you WILLFULLY terminated The Realm during a combat session.  Please rely on the Flee combat option instead of terminating The Realm during combat." ); 
  		obj->physicalState &= ~_STATE_WEINER;
  	}

  	// put the position information
  	if ( obj->player && ( obj->roomNumber > _DBR_THRESHOLD || !theRoom ) ) {
  		Building *house = findHouse ( obj->getName() );

		if ( house ) {
			if ( house->rooms.size() ) {
  				RMRoom *theRoom = (RMRoom *)house->rooms.at ( 0 );
 	 			obj->roomNumber = theRoom->number;
  				ack.putLong ( obj->roomNumber );

				// Give permenant house if necessary.
				if ( house->homeTown == 1 && obj->player ) {
					BCharacter* bChar = (BCharacter *)obj->getBase ( _BCHARACTER );

					if ( bChar ) {
						int level = bChar->getLevel();

						if ( level >= 50 ) {
							house->homeTown = random ( _TOWN_LEINSTER_WEST, _TOWN_ARIMATHOR );
							house->changed = 1;

							roomMgr->sendSystemMsg ( "Congratulations!", obj->player, "Your permanent house is in %s.", gTownNames[ house->homeTown ] );
						}
					}
				}
			} else {
  				obj->roomNumber = random ( 5000, 5089 );
  				ack.putLong ( obj->roomNumber );
			}
  		} else {
			loadHouse ( obj->getName(), cbGetPosnHouseLoad, obj->servID );
			return 1;
  		}
  	} else {
  		ack.putLong ( obj->roomNumber );
  	}

  	// set the destination room
	setTeleportRoomNum ( obj->roomNumber );

	// safety to make sure still on screen
	obj->x = ( std::min(std::max(obj->x, 1), 639) );
//( ( obj->x >? 1 ) <? 639 );
	obj->y = ( std::min(std::max(obj->y, 1), 479) );
//( ( obj->y >? 1 ) <? 479 );

  	ack.putWord ( obj->x );
  	ack.putWord ( obj->y );
  	ack.putByte ( obj->loop );

  	roomMgr->sendTo ( _IPC_PLAYER_ACK, &ack, this );

	return retVal;
}

int RMPlayer::process_IPC_GROUP_KICK ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		
	PackedMsg packet ( message->data(), message->size() );

	// get the object to kick out
	WorldObject *obj = roomMgr->findObject ( packet.getLong() );

	if ( obj && obj->player ) {

		if ( obj->player->isNPC ) {
//			disable ( "character %s tried to kick NPC %s from group.", getName(), obj->getName() );
			return retVal;
		}

		// make sure i'm the group leader
		if ( groupLeader == this ) {
			if ( isGroupMember ( obj->player ) ) {
				roomMgr->sendACK ( _IPC_GROUP_KICK, this );
				kickGroupMember ( obj->player );
			} else {
				roomMgr->sendNAK ( _IPC_GROUP_KICK, _ERR_REDUNDANT, this );
			}
		} else {
			roomMgr->sendNAK ( _IPC_GROUP_KICK, _ERR_SERVICE_NOT_AVAILABLE, this );
		}
	} else {
		roomMgr->sendNAK ( _IPC_GROUP_KICK, _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_GROUP_LEAVE ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	if ( groupLeader ) {
		roomMgr->sendACK ( _IPC_GROUP_LEAVE, this );
		leaveGroup();
	} else {
		roomMgr->sendNAK ( _IPC_GROUP_LEAVE, _ERR_REDUNDANT, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_GROUP_QUESTION ( IPCMessage *message ) {
	int retVal = 1;

	if ( !character || !character->player || !waitingMember )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	int nServID = packet.getLong();

	// get the object to join
	WorldObject *obj = roomMgr->findObject ( nServID );

	if ( obj && obj->player && !obj->player->tossing ) {
		int nResponse = packet.getByte();

		switch ( nResponse ) {
			case 1: {	//	Asked leader and they agreed - add to the group
				RMPlayer* target = obj->player;

				if ( target->joinGroup ( this ) ) {
					PackedMsg ack;
					ack.putACKInfo ( _IPC_GROUP_JOIN );

					LinkedElement *element = this->group.head();

					while ( element ) {
						RMPlayer *member = (RMPlayer *)element->ptr();

						if ( member ) {
							ack.putLong ( member->character->servID );
							ack.putString ( member->getName() );
						}

						element = element->next();
					}
			
					ack.putLong ( -1 );

					roomMgr->sendTo ( _IPC_PLAYER_ACK, &ack, target );
				} else {
					roomMgr->sendNAK ( _IPC_GROUP_JOIN, _ERR_TOO_BULKY, target );
				}
			}	  

			break;

			case 0: {	//	Asked leader and they refused
				roomMgr->sendNAK ( _IPC_GROUP_JOIN, _ERR_TOO_HEAVY, obj->player );
			}	  

			break;
		}

		waitingMember = NULL;
		obj->player->waitingMember = NULL;
	} else {
		waitingMember = NULL;
	}

	return retVal;
}

int RMPlayer::process_IPC_GROUP_JOIN ( IPCMessage *message ) {
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the object to join
	WorldObject *obj = roomMgr->findObject ( packet.getLong() );

	if ( obj && obj->player && ( obj->health > 0 ) && !obj->combatGroup && !character->combatGroup && !obj->player->isIgnoring( getName() ) ) {
		if ( obj->player->isNPC ) {
			return retVal;
		}

		// find the leader of this object
		RMPlayer *leader = obj->player->groupLeader? obj->player->groupLeader : obj->player;

		if ( !leader ) {
			roomMgr->sendNAK ( _IPC_GROUP_JOIN, _ERR_BAD_SERVID, this );
		} else if ( leader->waitingMember ) {
			if ( leader->waitingMember == this ) {
				//	Close group leader question box
				PackedMsg response;
				response.putLong( servID );
				response.putString( "CLOSE" );

				roomMgr->sendTo ( _IPC_GROUP_QUESTION, &response, leader );

				leader->waitingMember = NULL;
				waitingMember = NULL;
			} else {
				roomMgr->sendNAK ( _IPC_GROUP_JOIN, _ERR_TOO_EXPENSIVE, this );
			}
		} else if ( !leader->allowJoin ) {
			roomMgr->sendNAK ( _IPC_GROUP_JOIN, _ERR_BAD_SERVID, this );
			roomMgr->sendSystemMsg ( "You Can't Do That", this, "%s's group is currently closed to new members.  Try again later.", leader->getName() ); 

			if ( !group.size() )
				roomMgr->sendPlayerText ( leader, "|c43|Info> %s has just tried to join your closed group.  Type /open to open your group to allow people to join.\n", getName() );
		// make sure we are not already in a group
		} else if ( groupLeader != NULL ) {
			roomMgr->sendNAK ( _IPC_GROUP_JOIN, _ERR_MUST_REMOVE, this );

		// make sure we are not trying to join ourself
		} else if ( leader == this ) {
			roomMgr->sendNAK ( _IPC_GROUP_JOIN, _ERR_REDUNDANT, this );

		// add this player into the group if there is room
		} else if ( leader->canAddMember() ) {
			//	ask group leader if I can join???
			PackedMsg response;
			response.putLong( servID );

			char sMessage[1024];

			if ( leader == obj->player )
				sprintf( sizeof( sMessage ), sMessage, "Do you want to permit %s to join your group?", getName() );
			else
				sprintf( sizeof( sMessage ), sMessage, "Do you want to permit %s to join your group through %s?", getName(), obj->player->getName() );

			response.putString( sMessage );

			roomMgr->sendTo ( _IPC_GROUP_QUESTION, &response, leader );

			waitingMember = leader;
			leader->waitingMember = this;
		} else {
			roomMgr->sendNAK ( _IPC_GROUP_JOIN, _ERR_TOO_BULKY, this );
		}
	} else {
		roomMgr->sendNAK ( _IPC_GROUP_JOIN, _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_UNLOCK_OBJ ( IPCMessage *message )
{
	int retVal = 1;

	PackedMsg packet ( message->data(), message->size() );

	// get the object to unlock
	WorldObject *obj = roomMgr->findObject ( packet.getLong() );

	if ( obj ) {
		this->_lockedObj = NULL;
		obj->_locked--;
		roomMgr->sendACK ( _IPC_UNLOCK_OBJ, this );
	} else {
		roomMgr->sendNAK ( _IPC_UNLOCK_OBJ, _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_LOCK_OBJ ( IPCMessage *message )
{
	int retVal = 1;

	PackedMsg packet ( message->data(), message->size() );

	// get the object to lock
	WorldObject *obj = roomMgr->findObject ( packet.getLong() );

	if ( obj ) {
		this->_lockedObj = obj;
		obj->_locked++;
		roomMgr->sendACK ( _IPC_LOCK_OBJ, this );
	} else {
		roomMgr->sendNAK ( _IPC_LOCK_OBJ, _ERR_BAD_SERVID, this );
	}
	return retVal;
}

int RMPlayer::process_IPC_FATAL_DATA ( IPCMessage *message )
{
	int retVal = 1;

	PackedMsg packet ( message->data(), message->size() );

	char *name = packet.getString(), *timeStr = timeToStr();

	if ( !name )
		return retVal;

	int majorVer = packet.getWord();
	int minorVer = packet.getWord();
	int numLines = packet.getByte();

	if ( numLines ) {
		File *file = new File ( "../logs/fatal.log" );

		if ( file->isOpen() ) {
			file->seek ( file->size() );

			if ( numLines ) {
				file->printf ( "login name : %s\n", name );
				file->printf ( "version    : %d.%d\n", majorVer, minorVer );
				file->printf ( "time       : %s\n", timeStr );
				file->printf ( "-----------------------------------------------------\n" );
	
				while ( numLines-- ) {
					char *str = packet.getString();
					
					if ( str ) {
						file->printf ( "%s\n", str );
						free ( str );
					}
				}

				file->printf ( "-----------------------------------------------------\n\n" );
			}

			file->close();
		}

		delete file;
	}

	free ( name );

	if ( timeStr )
		free ( timeStr );

	roomMgr->sendACK ( _IPC_FATAL_DATA, this );

	return retVal;
}

int RMPlayer::process_IPC_QUEST_DECLINE ( IPCMessage *message )
{
	int retVal = 1;

	PackedMsg packet ( message->data(), message->size() );

	long number = packet.getLong();
	Quest *quest = ::findQuest ( number );

	if ( quest ) {
		PackedMsg ack;
		ack.putACKInfo ( message->type() );
		ack.putString ( quest->declined );

		roomMgr->sendTo ( _IPC_PLAYER_ACK, &ack, this );
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_QUEST_ACCEPT ( IPCMessage *message )
{
	int retVal = 1;

	PackedMsg packet ( message->data(), message->size() );

	long number = packet.getLong();
	Quest *quest = ::findQuest ( number );

	if ( quest ) {
		if ( findQuest ( number ) ) {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		} else {
			if ( !quest->item && quest->reward ) {
				PackedMsg ack;
				ack.putACKInfo ( message->type() );
				ack.putString ( quest->accepted );

				roomMgr->sendTo ( _IPC_PLAYER_ACK, &ack, this );

				WorldObject *reward = new WorldObject;
				reward->copy ( quest->reward );
				reward->addToDatabase();
				reward->forceIn ( character );
				reward->makeVisible ( 1 );

				QuestInfo *info = new QuestInfo ( quest );
				info->startTime = getseconds();
				info->endTime = getseconds();

				addQuest ( info );
	
				writeQuestData();
			} else {
				PackedMsg ack;
				ack.putACKInfo ( message->type() );
				ack.putString ( quest->accepted );

				roomMgr->sendTo ( _IPC_PLAYER_ACK, &ack, this );

				QuestInfo *info = new QuestInfo ( quest );
				info->startTime = getseconds();

				addQuest ( info );
	
				writeQuestData();
			}
		}
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_TREE_BACK ( IPCMessage *message )
{
	int retVal = 1;

	int type = _TREE_ROOT;

	if ( talkTreeTopic ) {
		TalkTreeTopicOwner *owner = talkTreeTopic->owner;

		if ( owner->type == _TREE_ROOT ) {
			type = _TREE_ROOT;
			talkTreeTopic = NULL;
		} else {
			talkTreeTopic = (TalkTreeTopic *)owner;
			type = _TREE_NODE;
		}
	}

	talkTreePage = -1;

	TalkTreeTopicOwner *page = talkTreeTopic? (TalkTreeTopicOwner *)talkTreeTopic : (TalkTreeTopicOwner *)talkTree;

	PackedMsg ack;
	ack.putACKInfo ( message->type() );
	ack.putByte ( type );

	page->buildPacket ( this, &ack );

	roomMgr->sendTo ( _IPC_PLAYER_ACK, &ack, this );

	return retVal;
}

int RMPlayer::process_IPC_TREE_GET_TEXT ( IPCMessage *message )
{
	int retVal = 1;

	PackedMsg packet ( message->data(), message->size() );
	int direction = packet.getWord(), type = _TREE_ROOT;

	PackedMsg ack;
	ack.putACKInfo ( message->type() );

	while ( 1 ) {
		talkTreePage += direction;

		if ( talkTreePage < 0 ) 
			talkTreePage = 0;

		else if ( talkTreePage >= talkTreeTopic->text.size() )
			talkTreePage = talkTreeTopic->text.size() - 1;

		TalkTreeData *obj = (TalkTreeData *)talkTreeTopic->text.at ( talkTreePage );

		if ( obj->type == _TREE_TEXT ) {
			TalkTreeText *str = (TalkTreeText *)obj;
			ack.putByte ( _TREE_NODE );

			int flags = _TALK_LAST_MENU;

			if ( talkTreePage < talkTreeTopic->text.size() - 1 )
				flags |= _TALK_GO_ON;

			if ( talkTreePage > 0 )
				flags |= _TALK_GO_BACK;

			ack.putByte ( flags );	
			ack.putString ( str->data );

			break;
		} else {
			TalkTreeQuest *quest = (TalkTreeQuest *)obj;
			QuestInfo *myQuest = findQuest ( quest->quest->number );

			if ( myQuest ) {
				if ( !myQuest->endTime ) {
					ack.putByte ( _TREE_QUEST_REMINDER );
					ack.putString ( quest->quest->reminder );
					break;
				}
			} else {
				ack.putByte ( _TREE_QUEST_PROPOSAL );
				ack.putLong ( quest->quest->number );
				ack.putString ( quest->quest->proposal );
				break;
			}
		}
	}

	roomMgr->sendTo ( _IPC_PLAYER_ACK, &ack, this );

	return retVal;
}

int RMPlayer::process_IPC_TREE_CHOOSE_TOPIC ( IPCMessage *message )
{
	int retVal = 1;

	PackedMsg packet ( message->data(), message->size() );

	int index = packet.getWord();

	if ( talkTree ) {

		if ( talkTreeTopic ) {
			talkTreeTopic = (TalkTreeTopic *)talkTreeTopic->topics.at ( index );
		} else {
			talkTreeTopic = (TalkTreeTopic *)talkTree->topics.at ( index );
		}

		if ( talkTreeTopic ) {
			
			PackedMsg ack;
			ack.putACKInfo ( message->type() );

			if ( talkTreeTopic->topics.size() ) {
				ack.putByte ( _TREE_ROOT );
				talkTreeTopic->buildPacket ( this, &ack );
			} else {
				talkTreePage = 0;
				LinkedElement *element = talkTreeTopic->text.head();
		
				while ( element ) {
					TalkTreeData *obj = (TalkTreeData *)element->ptr();

					if ( obj->type == _TREE_TEXT ) {
						TalkTreeText *str = (TalkTreeText *)obj;
						ack.putByte ( _TREE_NODE );

						int flags = _TALK_LAST_MENU;

						if ( element->next() )
							flags |= _TALK_GO_ON;

						ack.putByte ( flags );	
						ack.putString ( str->data );

						break;
					} else {
						TalkTreeQuest *quest = (TalkTreeQuest *)obj;
						QuestInfo *myQuest = findQuest ( quest->quest->number );

						if ( myQuest ) {
							if ( !myQuest->endTime ) {
								ack.putByte ( _TREE_QUEST_REMINDER );
								ack.putString ( quest->quest->reminder );
								break;
							}
						} else {
							ack.putByte ( _TREE_QUEST_PROPOSAL );
							ack.putLong ( quest->quest->number );
							ack.putString ( quest->quest->proposal );
							break;
						}
					}

					talkTreePage++;
					element = element->next();
				}
			}

			roomMgr->sendTo ( _IPC_PLAYER_ACK, &ack, this );
		} else {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		}
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_TREE_GET ( IPCMessage *message )
{
	int retVal = 1;

	PackedMsg packet ( message->data(), message->size() );

	// get the servID of the object to use as the owner of the talk tree
	long servID = packet.getLong();

	WorldObject *obj = roomMgr->findObject ( servID );

	if ( obj ) {

		BTalk *base = (BTalk *)obj->getBase ( _BTALK );

		if ( base ) {
			LinkedElement *element = gTalkTrees.head();
			talkTree = NULL;
			talkTreeTopic = NULL;
			talkTreePage = -1;

			while ( element ) {
				TalkTree *tree = (TalkTree *)element->ptr();

				if ( tree->id == base->talkTreeID ) {
					talkTree = tree;
					break;
				}

				element = element->next();
			}

			if ( talkTree ) {
				PackedMsg ack;
				ack.putACKInfo ( message->type() );

				talkTree->buildPacket ( this, &ack );

				roomMgr->sendTo ( _IPC_PLAYER_ACK, &ack, this );
			} else {
				roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
			}
		} else {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		}
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_GET_BOOK_PAGE ( IPCMessage *message )
{
	int retVal = 1;

	PackedMsg packet ( message->data(), message->size() );

	packet.getLong();

	PackedMsg ack;
	ack.putACKInfo ( message->type() ); 

	char page[1024];
	sprintf ( sizeof ( page ), page, "The quick brown fox jumped over the lazy dogand by the way, this is page %d\n", packet.getWord() );

	ack.putString ( page );

	roomMgr->sendTo ( _IPC_PLAYER_ACK, &ack, this );

	return retVal;
}

int RMPlayer::process_IPC_GET_BOOK_INFO ( IPCMessage *message )
{
	int retVal = 1;

	PackedMsg ack;
	ack.putACKInfo ( message->type() ); 
	ack.putWord ( 10 );

	roomMgr->sendTo ( _IPC_PLAYER_ACK, &ack, this );

	return retVal;
}

int RMPlayer::process_IPC_CAST_SPELL ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the spell number to cast
	int spellID = packet.getWord();
	long caster = packet.getLong();
	long target = packet.getLong();
	int targetX = packet.getWord();
	int targetY = packet.getWord();

	if ( spellID < 0 || spellID > _SPELL_MAX )
		return retVal;

	if ( targetY < 0 || targetX < 0 )
		return retVal;

	PackedMsg response;
	int isCharacter = 0;
	spell_info *spell; 
	BCharacter *bchar; 
	BPlayer *bplayer; 
	BUse *buse; 
	int knowsSpell = 0;

	WorldObject *pCaster = roomMgr->findObject ( caster );
	WorldObject *targetObj, *owningObj; 

	// no caster, no-magic rooms or in teleport phase
	if ( (room->flags & _RM_FLAG_NO_MAGIC) && !checkAccess (_ACCESS_MODERATOR ) ) {
		if ( pCaster && ((pCaster->player && !pCaster->player->isNPC) || !pCaster->player) ) {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
			goto castSpellEnd;
		}
	}
	
	// validate caster
	owningObj = pCaster->getBaseOwner();

	if ( owningObj && owningObj->player && !owningObj->player->isNPC && owningObj->player != this )
		goto castSpellEnd;

	// spells cast on a character head ID are never valid

	targetObj = roomMgr->findObject ( target );

	if ( targetObj && targetObj->getBase ( _BHEAD ) )
		goto castSpellEnd;

	// check for using artifacts
	if ( pCaster != character ) {
		buse = (BUse *)pCaster->getBase ( _BUSE );	

		if ( !buse ) {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
			goto castSpellEnd;
		}

		if ( spellID != buse->spell )
			goto castSpellEnd;
	}

	// check to see if player knows the spell
	bchar = (BCharacter *)pCaster->getBase ( _BCHARACTER );

	if ( !pCaster->playerControlled )
		knowsSpell = 1;
	else 
		knowsSpell = bchar? bchar->knowsSpell ( spellID ) : 0;

	if ( bchar && (!knowsSpell || (spellID < 0 || spellID >= _SPELL_MAX)) ) {
		bplayer = player? (BPlayer *)player->getBase ( _BPLAYER ) : 0;
		goto castSpellEnd;
	}

	spell = &gSpellTable[spellID];

	isCharacter = pCaster->getBase ( _BCHARACTER )? 1 : 0;

	//can't cast any spells in combat - such is handled through IPC_COMBAT_ACTION
	if  ( isCharacter && spell && character->combatGroup && ( character->opposition && character->opposition->size() ) ) {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		goto castSpellEnd;
	}

	// don't cast combat spells out of combat
	if ( spell && spell->isCombat == _COMBAT_SPELL ) {
		logHack( "cast of combat spell outside combat. SpellID %d, Playername %s", spellID, character->getName(), character->player->getLogin() );
		goto castSpellEnd;
	}

	// put who movie is for
	response.putLong ( character->servID );
	response.putLong ( character->room->number );

	if ( isCharacter ) {
		response.putByte ( _MOVIE_CAST_BEGIN );
		response.putLong ( pCaster->servID );
	}

	castSpell ( spell, pCaster, target, targetX, targetY, &response );

	if ( isCharacter ) {
		response.putByte ( _MOVIE_CAST_END );
		response.putLong ( pCaster->servID );
	}

	response.putByte ( _MOVIE_END );
	
	roomMgr->sendACK ( message->type(), this );
	roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), character->room );

castSpellEnd:

	// give the client control again
	handsOn();

	return retVal;
}

void RMPlayer::castSpell ( spell_info *spell, WorldObject *pCaster, long targetServID, int targetX, int targetY, PackedData *packet ) 
{
	WorldObject *casterOwner = pCaster->getBaseOwner();
	WorldObject *targetObj = roomMgr->findObject ( targetServID );
	WorldObject *owningObj = NULL;

	if ( targetObj ) {
		owningObj = targetObj->getBaseOwner();

		// must be in same room or in same combat
		if ( owningObj && casterOwner ) {
			if	( casterOwner->getRoom() != owningObj->getRoom() || ( casterOwner->combatGroup != owningObj->combatGroup ) )
				return;

			// check to see if the target object is owned by another player
			if ( !targetObj->player && (owningObj != targetObj) && (casterOwner != owningObj) )
				return;

			// combat only spell can not be cast on a friend
			if ( targetObj->player && (targetObj->player != this) && (spell->isCombat == _COMBAT_SPELL) && isGroupMember ( targetObj->player ) ) 
				return;
		}
	}

	char output[10240] = "";

	BUse *buse = (BUse *)pCaster->getBase ( _BUSE );

	if ( pCaster ) {

		if ( targetObj ) {

			if ( !pCaster->getBase ( _BCHARACTER ) ) {
				if ( pCaster->physicalState & _STATE_TRAPPED ) {
					sprintf ( sizeof ( output ), output, "|c60|A trap is tripped when %s opens the %s!", getName(), pCaster->getName() );
				} 
	
				else if ( pCaster == casterOwner ) 
					sprintf ( sizeof ( output ), output, "The %s glows for a moment.", pCaster->getName() );
				else
					sprintf ( sizeof ( output ), output, "%s's %s glows for a moment.", casterOwner->getName(), pCaster->getName() );
			}
	
			else if ( pCaster == targetObj )
				sprintf ( sizeof ( output ), output, "|c57|%s mumbles '%s' under %s breath. ", pCaster->getName(), spell->verbal, pCaster->getPronoun ( _PRONOUN_HIS ) );
	
			else if ( owningObj == pCaster )
				sprintf ( sizeof ( output ), output, "|c57|%s says '%s', pointing at %s %s. ", pCaster->getName(), spell->verbal, pCaster->getPronoun ( _PRONOUN_HIS ), targetObj->basicName );
	
			else if ( owningObj == targetObj )
				sprintf ( sizeof ( output ), output, "|c57|%s says '%s', pointing at %s%s. ", pCaster->getName(), spell->verbal, targetObj->player? "" : "the ", targetObj->player? targetObj->getName() : targetObj->basicName );
	
			else {
				sprintf ( sizeof ( output ), output, "|c57|%s says '%s', pointing at %s's %s. ", pCaster->getName(), spell->verbal, owningObj->getName(), targetObj->basicName );
			}
		} else {
			if ( pCaster->getBase ( _BCHARACTER ) ) {
				sprintf ( sizeof ( output ), output, "|c57|%s says '%s'. ", pCaster->getName(), spell->verbal );
			} else {
				if ( pCaster == casterOwner ) 
					sprintf ( sizeof ( output ), output, "The %s glows for a moment.", pCaster->getName() );
				else
					sprintf ( sizeof ( output ), output, "%s's %s glows for a moment.", casterOwner->getName(), pCaster->getName() );			
			}
		}

		int manaCost = spell->manaCost;
		int doSpell = 1;
		int agressive = spell->agressive;
		int nSDMMod = (casterOwner && casterOwner->character)? casterOwner->character->GetSDM ( spell->skillType - _SKILL_SORCERY ) : 0;

		// adjust the mana cost if the caster has an inflated magical SDM...
		// manaCost will not increase until level 1000+
		if ( nSDMMod && pCaster->level >= 1000) {
			// only inflate power for damaging mysticism spells...
			if ( (spell->skillType == _SKILL_MYSTICISM) && !spell->isDamaging )
				nSDMMod = 0;

			manaCost += (manaCost * nSDMMod) / 200;
			//manaCost += (manaCost * nSDMMod) / 100;

			if ( manaCost < 1 )
				manaCost = 1;
		}
	
		if ( pCaster->player && !pCaster->player->isNPC && (manaCost > pCaster->manaValue) ) {
			char str[1024];
			sprintf ( sizeof ( str ), str, "|c248|%s does not have the %d %s to cast the spell!", getName(), manaCost, (manaCost > 1)? "|c248|crystals" : "|c248|crystal" );
			strcat ( output, str );
			doSpell = 0;
		}

		affect_t *spellBlastAff = pCaster->hasAffect ( _AFF_SPELL_BLAST );

		if ( doSpell && pCaster->player && spellBlastAff ) {
			strcat ( output,  "|c22|The spell is countered and fails!" );
			pCaster->delAffect ( spellBlastAff, packet );

  			if ( !pCaster->player->isNPC )
  				pCaster->changeMana ( -manaCost, packet );

			doSpell = 0;
		}

		if ( doSpell && pCaster->player && pCaster->combatGroup && pCaster->combatGroup->cloud && pCaster->combatGroup->cloud->hasAffect ( _AFF_ANTI_MAGIC ) ) {
			strcat ( output,  "|c22|An anti-magic aura suppresses the spell!" );
  		
			if ( !pCaster->player->isNPC )
  				pCaster->changeMana ( -manaCost, packet );

			doSpell = 0;
		}
		// Added Heal, GHeal, Cure Poison, Empower and Nimbility to global 'ignore spell resistance' list
		// This will allow NPCS like Pixie, Warden etc to cast buffs on fully magic protected players out of combat.
		if ( (spell != &gSpellTable[_SPELL_HOME]) && (spell != &gSpellTable[_SPELL_TELEPORT]) && (spell != &gSpellTable[_SPELL_GATHER_THE_FELLOWSHIP]) && (spell != &gSpellTable[_SPELL_SHIFT]) && (spell != &gSpellTable[_SPELL_GREATER_HEAL]) && (spell != &gSpellTable[_SPELL_HEAL]) && (spell != &gSpellTable[_SPELL_EMPOWER]) && (spell != &gSpellTable[_SPELL_NIMBILITY]) && (spell != &gSpellTable[_SPELL_CURE_POISON]) ) {
			
			// Removed Cast Resistance - Zach
			// check for spell circle denial...
			/*if ( doSpell && casterOwner->character && casterOwner->character->TestCastResistance ( spell->skillType - _SKILL_SORCERY ) ) {
  				if ( pCaster && pCaster->player && !pCaster->player->isNPC )
  					pCaster->changeMana ( -manaCost, packet );

				strcat ( output,  "|c22|Cast resisted!" );
				doSpell = 0;

				packet->putByte ( _MOVIE_SPECIAL_EFFECT );
				packet->putLong ( casterOwner->servID );
				packet->putByte ( _SE_SPELL_BLAST );
				packet->putByte ( 0 );
				packet->putByte ( 1 );
				packet->putLong ( casterOwner->servID );
			}*/

			// check for spell resistance  on the target...
			if ( doSpell && targetObj && targetObj->character && targetObj->character->TestSpellResistance ( spell->skillType - _SKILL_SORCERY ) ) {
				
				// if the caster is a player, and target is not an NPC, ignore magic resistances
				if ( pCaster && pCaster->player && !pCaster->player->isNPC && !targetObj->player->isNPC ){
					char buf[1024];
					sprintf ( sizeof ( buf ), buf, "|c21|%s broke through %s magic resistance! ", pCaster->getName(), pCaster->getPronoun (_PRONOUN_HIS ) );
					strcat ( output, buf );
					pCaster->changeMana ( -manaCost, packet );
					doSpell = 1;

				// if the caster is player, and the target is an NPC, calculate magic resistances
				} else if ( pCaster && pCaster->player && targetObj->player->isNPC ){
  					pCaster->changeMana ( -manaCost, packet );

					strcat ( output,  "|c248|Target resists!" );
					doSpell = 0;

					packet->putByte ( _MOVIE_SPECIAL_EFFECT );
					packet->putLong ( targetObj->servID );
					packet->putByte ( _SE_SPELL_BLAST );
					packet->putByte ( 0 );
					packet->putByte ( 1 );
					packet->putLong ( targetObj->servID );
				}
			}
		}

		// check for enchant resistance...
		if ( doSpell && targetObj && targetObj->m_nEnchantResistance ) {
			int nChance = targetObj->m_nEnchantResistance;

			if ( spell == &gSpellTable[_SPELL_GREATER_IDENTIFY] )
				nChance /= 2;

			else if ( spell == &gSpellTable[_SPELL_ENGRAVE] )
				nChance = 0;

			if ( random ( 1, 100 ) <= nChance ) {
  				if ( pCaster && pCaster->player && !pCaster->player->isNPC )
  					pCaster->changeMana ( -manaCost, packet );

				strcat ( output,  "|c248|Target resists!" );
				doSpell = 0;

				packet->putByte ( _MOVIE_SPECIAL_EFFECT );
				packet->putLong ( owningObj->servID );
				packet->putByte ( _SE_SPELL_BLAST );
				packet->putByte ( 0 );
				packet->putByte ( 1 );
				packet->putLong ( owningObj->servID );
			}
		}

		// set the global SDM modifier...
		gTempSDM = nSDMMod;

		putMovieText ( character, packet, "%s", output );
		output[0] = 0;
	
		if ( doSpell ) {
			if ( pCaster->player && !pCaster->player->isNPC ) {
				pCaster->changeMana ( -manaCost, packet );
				spell->castCount++;
			}

			affect_t *affect = spell->function ( 0, pCaster, targetServID, targetX, targetY, output, packet, this );

			if ( affect && targetObj ) {
				affect_t *permAffect = targetObj->hasAffect ( _AFF_PERMANENCY, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );
		
				if (affect && permAffect ) {
					targetObj->delAffect ( permAffect, packet );
		
					if ( targetObj->hasAffect ( affect->id, affect->type, _AFF_SOURCE_PERMANENT ) )	{
						targetObj->delAffect ( affect, packet );
						affect = NULL;
						permAffect = NULL;
		
						char buf[1024];
						sprintf ( sizeof ( buf ), buf, "|c71|The permanency aura on the %s is destroyed because it was already enchanted with that! ", targetObj->getName() );
						strcat ( output, buf );
					} else {
						affect->duration = -1;
						affect->source = _AFF_SOURCE_PERMANENT;
	
						char buf[1024];
						sprintf ( sizeof ( buf ), buf, "|c21|The permanency aura on the %s has absorbed the spell, making it permanent. ", targetObj->getName() );
						strcat ( output, buf );
					}
				}
		
				affect_t *enchantAff = targetObj->hasAffect ( _AFF_ENCHANT_ITEM, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL );
		
				if ( affect && enchantAff ) {
					targetObj->delAffect ( enchantAff, packet );
	
					if ( targetObj->hasAffect ( affect->id, affect->type, _AFF_SOURCE_WEARER ) ) {
						targetObj->delAffect ( affect, packet );
						affect = NULL;
						enchantAff = NULL;

						char buf[1024];
						sprintf ( sizeof ( buf ), buf, "|c71|The enchantment aura on the %s is destroyed because it was already enchanted with that! ", targetObj->getName() );
						strcat ( output, buf );
					} else {
						affect->duration = -1;
						affect->source = _AFF_SOURCE_WEARER;
	
						char buf[1024];
						sprintf ( sizeof ( buf ), buf, "|c21|The enchantment aura on the %s has absorbed the spell, making it permanent. ", targetObj->getName() );
						strcat ( output, buf );
					}
				}
			}
		}		

		putMovieText ( character, packet, "%s", output );
		
		// clear the global SDM modifier...
		gTempSDM = 0;

		if ( buse && !pCaster->player ) {
			buse->uses--;
	
			if ( buse->uses < 1 ) {
				packet->putByte ( _MOVIE_TOSS_OBJECT );
				packet->putLong ( character->servID );
				packet->putLong ( pCaster->servID );
	
				if ( !pCaster->getBase ( _BCHARACTER ) ) {
					if ( pCaster == casterOwner ) 
						sprintf ( sizeof ( output ), output, "|c60|The %s is spent and crumbles to dust.", pCaster->getName() );
					else
						sprintf ( sizeof ( output ), output, "|c60|%s's %s is spent and crumbles to dust.", casterOwner->getName(), pCaster->getName() );
	
					putMovieText ( character, packet, "%s", output );
				}
	
				roomMgr->destroyObj ( pCaster, 0, __FILE__, __LINE__ );
			}
			else{
				char* urgencyColor;

				if(buse->uses > 20){
					urgencyColor = "c86";
				}
				else if(buse->uses <= 20 && buse->uses > 10){
					urgencyColor = "c13";
				}
				else{
					urgencyColor = "c60";
				}

				sprintf ( sizeof ( output ), output, "The %s has |%s|(%d)|c43| charges remaining.", pCaster->getName(), urgencyColor, buse->uses );				
				putMovieText ( character, packet, "%s", output );
			}
		}

		// if this was cast on an NPC, and it was agressive, attack the caster
		if ( agressive && pCaster && casterOwner && targetObj && owningObj && owningObj->player && owningObj->player->isNPC && !casterOwner->combatGroup && zone && zone->allowCombat() ) {
			owningObj->player->engage ( casterOwner, packet );

			if ( owningObj->combatGroup )
				owningObj->combatGroup->freeFlee = 0;
		}
	}
}

// Performs a proc
void RMPlayer::castProc ( spell_info *spell, WorldObject *pCaster, long targetServID, int targetX, int targetY, PackedData *packet )
{
    WorldObject *casterOwner = pCaster->getBaseOwner();
    WorldObject *targetObj = roomMgr->findObject ( targetServID );
    WorldObject *owningObj = NULL;

    if ( targetObj )
    {
        owningObj = targetObj->getBaseOwner();

        // must be in same room or in same combat
        if ( owningObj && casterOwner )
        {
            if    ( casterOwner->getRoom() != owningObj->getRoom() || ( casterOwner->combatGroup != owningObj->combatGroup ) )
                return;

            // check to see if the target object is owned by another player
            if ( !targetObj->player && (owningObj != targetObj) && (casterOwner != owningObj) )
                return;

            // combat proc can not be cast on a friend
            if ( targetObj->player && (targetObj->player != this) && (spell->isCombat == _COMBAT_SPELL) && isGroupMember ( targetObj->player ) )
                return;
        }
    }

    char output[10240] = "";

    if ( pCaster )
    {
        int doSpell = 1;
        int agressive = spell->agressive;
        int nSDMMod = (casterOwner && casterOwner->character)? casterOwner->character->GetSDM ( spell->skillType - _SKILL_SORCERY ) : 0;

        affect_t *spellBlastAff = pCaster->hasAffect ( _AFF_SPELL_BLAST );

        // Check for spellblast on target
        if ( doSpell && pCaster->player && spellBlastAff )
        {
            strcat ( output,  "|c22|The spell is countered and fails!" );
            pCaster->delAffect ( spellBlastAff, packet );

            doSpell = 0;
        }

		 // Check for anti magic aura
        if ( doSpell && pCaster->player && pCaster->combatGroup && pCaster->combatGroup->cloud && pCaster->combatGroup->cloud->hasAffect ( _AFF_ANTI_MAGIC ) )
        {
            strcat ( output,  "|c22|An anti-magic aura suppresses the attack!" );

            doSpell = 0;
        }

        if ( (spell != &gSpellTable[_SPELL_HOME]) && (spell != &gSpellTable[_SPELL_TELEPORT]) && (spell != &gSpellTable[_SPELL_GATHER_THE_FELLOWSHIP]) && (spell != &gSpellTable[_SPELL_SHIFT]) )
        {
            // check for spell circle denial...
            /*if ( doSpell && casterOwner->character && casterOwner->character->TestCastResistance ( spell->skillType - _SKILL_SORCERY ) )
            {
             //   if ( pCaster && pCaster->player && !pCaster->player->isNPC )
               //     pCaster->changeMana ( -manaCost, packet );

                strcat ( output,  "|c22|Cast resisted!" );
                doSpell = 0;

                packet->putByte ( _MOVIE_SPECIAL_EFFECT );
                packet->putLong ( casterOwner->servID );
                packet->putByte ( _SE_SPELL_BLAST );
                packet->putByte ( 0 );
                packet->putByte ( 1 );
                packet->putLong ( casterOwner->servID );
            }*/

            // check for spell resistance  on the target...
            if ( doSpell && targetObj && targetObj->character && targetObj->character->TestSpellResistance ( spell->skillType - _SKILL_SORCERY ) )
            {
                strcat ( output,  "|c22|Target resists!" );
                doSpell = 0;

                packet->putByte ( _MOVIE_SPECIAL_EFFECT );
                packet->putLong ( targetObj->servID );
                packet->putByte ( _SE_SPELL_BLAST );
                packet->putByte ( 0 );
                packet->putByte ( 1 );
                packet->putLong ( targetObj->servID );
            }
        }

		 // set the global SDM modifier...
        gTempSDM = nSDMMod;

        putMovieText ( character, packet, "%s", output );
        output[0] = 0;

        if ( doSpell )
            affect_t *affect = spell->function ( 0, pCaster, targetServID, targetX, targetY, output, packet, this );

        putMovieText ( character, packet, "%s", output );

        // clear the global SDM modifier...
        gTempSDM = 0;
    }
}


int RMPlayer::process_IPC_MONEY_GIVE ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the amount of money to give
	long amount = packet.getLong();

	// get the object to give the money to
	WorldObject *directObject = roomMgr->findObject ( packet.getLong() );

	int isMana = packet.getByte();

	if ( directObject ) {
		BContainer *bcontain = (BContainer *)directObject->getBase ( _BCONTAIN );

		int isContainer = bcontain? 1 : 0;

		if ( !isContainer ) {
			roomMgr->sendPlayerText ( this, "|c43|Info> The %s cannot contain that.", directObject->getName() );
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
			handsOn();
			return retVal;
		}

		if ( directObject->getBase ( _BCHARACTER ) && directObject->player && !directObject->player->isNPC && !directObject->player->autoGive ) { 
			roomMgr->sendPlayerText ( this, "|c65|Info> %s is not accepting items.", directObject->getName() );
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
			handsOn();
			return retVal;
		}

		// don't allow money give to NPC
		if ( directObject->player && directObject->player->isNPC ) {
			roomMgr->sendPlayerText ( this, "|c43|Info> %s has no use for that!", directObject->getName() );
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
			handsOn();
			return retVal;
		}

		if ( isMana ) {
			if ( character->canAffordMana ( amount ) ) {
				if ( ( directObject->manaValue + amount ) > MAX_MANA_HELD ) {
					amount = MAX_MANA_HELD - directObject->manaValue;

					if ( amount < 0 )
						amount = 0;
				}

				character->manaValue -= amount;
				directObject->manaValue += amount;

				logInfo ( _LOG_ALWAYS, "Mana given by %s -- writing character %s", character->getName(), character->getName() );
				character->writeCharacterData();
				logInfo ( _LOG_ALWAYS, "Mana taken by %s -- writing character %s", directObject->getName(), directObject->getName() );
				directObject->writeCharacterData();

				directObject->calcWeight();
				character->calcWeight();

				// build the response
				PackedMsg response;

				// who is this movie for
				response.putLong ( character->servID );
				response.putLong ( character->room->number );

				// put money give junk
				response.putByte ( _MOVIE_MONEY_GIVE );
				response.putLong ( directObject->servID );
				response.putLong ( amount );
				response.putByte ( isMana );

				response.putByte ( _MOVIE_END );

				roomMgr->sendACK ( message->type(), this );
				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), character->room );
			} else {
				roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
			}
		} else {
			if ( character->canAfford ( amount ) ) {
                if ( ( directObject->value + amount ) > MAX_GOLD_HELD ) {
	                amount = MAX_GOLD_HELD - directObject->value;

	                if ( amount < 0 )
	                    amount = 0;
                }

				character->value -= amount;
				directObject->value += amount;

				logInfo ( _LOG_ALWAYS, "Gold given by %s -- writing character %s", character->getName(), character->getName() );
				character->writeCharacterData();
				logInfo ( _LOG_ALWAYS, "Gold taken by %s -- writing character %s", directObject->getName(), directObject->getName() );
				directObject->writeCharacterData();

				directObject->calcWeight();
				character->calcWeight();

				// build the response
				PackedMsg response;

				// who is this movie for
				response.putLong ( character->servID );
				response.putLong ( character->room->number );

				// put money give junk
				response.putByte ( _MOVIE_MONEY_GIVE );
				response.putLong ( directObject->servID );
				response.putLong ( amount );
				response.putByte ( isMana );

				response.putByte ( _MOVIE_END );

				roomMgr->sendACK ( message->type(), this );
				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), character->room );
			} else {
				roomMgr->sendNAK( message->type(), _ERR_TOO_EXPENSIVE, this );
			}
		}
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	// give control back to the user
	handsOn();

	return retVal;
}

int RMPlayer::process_IPC_MONEY_TAKE ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the object to take 
	WorldObject *directObject = roomMgr->findObject ( packet.getLong() );

	if ( directObject && directObject->isMoney() && directObject->getRoom() == character->room ) 
	{
		WorldObject *owningObj = directObject->getBaseOwner();

		if ( owningObj && owningObj->player && owningObj->player != this ) {
			goto endMoneyTake;
		}

		if ( owningObj ) {
			BOpenable *bOpen = (BOpenable *)owningObj->getBase ( _BOPEN );
			if ( bOpen && !bOpen->isOpen() )
				goto endMoneyTake;
		}

		// mark the building as changed
		directObject->markBuildingAsChanged();

		if ( directObject->view == _MANA_VIEW ) {
			if ( ( character->manaValue + directObject->manaValue ) > MAX_MANA_HELD || ( character->manaValue + directObject->manaValue ) < 0 ) {
	            roomMgr->sendNAK ( message->type(), _ERR_NO_ROOM, this );

	            handsOn();

	            return retVal;
	        }

			character->manaValue += directObject->manaValue;
		} else {
			if ( ( character->value + directObject->value ) > MAX_GOLD_HELD || ( character->value + directObject->value ) < 0 ) {
				roomMgr->sendNAK ( message->type(), _ERR_NO_ROOM, this );
				
				handsOn();
			
				return retVal;
			}

			character->value += directObject->value;
		}

		character->calcWeight();

		WorldObject *owner = directObject->getOwner();
		BContainer *bContain = (BContainer *) owner->getBase ( _BCONTAIN );

		PackedMsg response;

		// who this movie is for
		response.putLong ( character->servID );
		response.putLong ( character->room->number );

		// put movie data in response
		response.putByte ( _MOVIE_MONEY_TAKE );
		response.putLong ( directObject->servID );

		if ( owner && (owner != directObject) && (owner->physicalState & _STATE_TOSS) ) {
			if ( bContain && bContain->contents.size() == 1 ) {
				response.putByte ( _MOVIE_TOSS_OBJECT );
				response.putLong ( character->servID );
				response.putLong ( owner->servID );
			}
		}
		
		response.putByte ( _MOVIE_END );

		roomMgr->sendACK ( message->type(), this );

		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), character->room );

		// destroy the object
		roomMgr->destroyObj ( directObject, 0, __FILE__, __LINE__ );

		if ( owner && (owner != directObject) && (owner->physicalState & _STATE_TOSS) ) 
			if ( bContain && bContain->contents.size() == 1 )
				roomMgr->destroyObj ( owner, 0, __FILE__, __LINE__ );

	} else {
endMoneyTake:
		roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
	}

	// give the client control again
	handsOn();

	return retVal;
}

int RMPlayer::process_IPC_MONEY_PUT ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the amount of money to put
	long amount = packet.getLong();

	// get the object to put the money into
	WorldObject *directObject = roomMgr->findObject ( packet.getLong() );

	// get the type of money to put
	int isMana = packet.getByte();

	if ( directObject ) {
		BContainer *bcontain = (BContainer *)directObject->getBase ( _BCONTAIN );
		WorldObject *owningObj = directObject->getBaseOwner();

		if ( owningObj && owningObj->player && owningObj->player != this )
			goto endMoneyPut;

		WorldObject *super = roomMgr->findClass ( isMana ? (char*) "ManaBag" : (char*) "MoneyBag" );

		if ( super ) {
			if ( isMana ) {
				// see if the character has the passed amount
				if ( character->canAffordMana ( amount ) ) {
					character->manaValue -= amount;
					character->calcWeight();

					PackedMsg response;

					// who this movie is for
					response.putLong ( character->servID );
					response.putLong ( character->room->number );

					// put movie data in response
					response.putByte ( _MOVIE_MONEY_PUT );
					response.putLong ( directObject->servID );

					WorldObject *money = new WorldObject ( super );
					money->x = character->x;
					money->y = character->y;
					money->manaValue = amount;
					money->physicalState |= _STATE_MONEY;

					money->addToDatabase();

					money->forceIn ( directObject );
					money->isVisible = 1;

					money->hidden++;
					BCarryable *bcarry = (BCarryable *)money->getBase ( _BCARRY );
					bcarry->show = 0;
					money->makeVisible ( 1 );
					bcarry->show = 1;

					// put money object's servID
					response.putLong ( money->servID );

					response.putByte ( _MOVIE_END );

					roomMgr->sendACK ( message->type(), this );

					roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), character->room );
				} else {
					roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
				}
			} else {
				// see if the character has the passed amount
				if ( character->canAfford ( amount ) ) {
					character->value -= amount;
					character->calcWeight();

					PackedMsg response;

					// who this movie is for
					response.putLong ( character->servID );
					response.putLong ( character->room->number );

					// put movie data in response
					response.putByte ( _MOVIE_MONEY_PUT );
					response.putLong ( directObject->servID );

					WorldObject *money = new WorldObject ( super );
					money->x = character->x;
					money->y = character->y;
					money->value = amount;
					money->physicalState |= _STATE_MONEY;

					money->addToDatabase();

					money->forceIn ( directObject );
					money->isVisible = 1;

					money->hidden++;
					BCarryable *bcarry = (BCarryable *)money->getBase ( _BCARRY );
					bcarry->show = 0;
					money->makeVisible ( 1 );
					bcarry->show = 1;

					// put money object's servID
					response.putLong ( money->servID );

					response.putByte ( _MOVIE_END );

					roomMgr->sendACK ( message->type(), this );

					roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), character->room );
				} else {
					roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
				}
			}
		} else {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		}
	} else {
endMoneyPut:
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	// give the client control again
	handsOn();

	return retVal;
}

int RMPlayer::process_IPC_MONEY_DROP ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	if ( room && (room->objects.size() > _DBR_MAX_OBJECTS) ) {
		roomMgr->sendSystemMsg ( "Too Much Stuff", this, "You can't drop any more things in this room." ); 
		return retVal;
	}

	PackedMsg packet ( message->data(), message->size() );

	// get the amount of money to drop
	long amount = packet.getLong();
	int isMana = packet.getByte();

	WorldObject *super = roomMgr->findClass ( isMana ? (char*) "ManaBag" : (char*) "MoneyBag" );

	if ( super ) {
		if ( isMana ) {
			// see if the character has the passed amount
			if ( character->canAffordMana ( amount ) ) {
				if ( amount > MAX_MANA_HELD )
					amount = MAX_MANA_HELD;

				character->manaValue -= amount;
				character->calcWeight();
			
	   			PackedMsg response;
	   		
	   			// who this movie is for
	   			response.putLong ( character->servID );
	   			response.putLong ( character->room->number );
	   	
	   			// put movie data in response
	   			response.putByte ( _MOVIE_MONEY_DROP );
	   			response.putLong ( character->servID );
	   			response.putLong ( amount );
	   		
	   			// create the new money object that will be on the ground
	   			WorldObject *money = new WorldObject ( super );
	   		
	   			BCarryable *bcarry = (BCarryable *)money->getBase ( _BCARRY );
	   			bcarry->show = 0;
	   		
	   			money->x = character->x;
	   			money->y = character->y;
	   			money->manaValue = amount;
	   			money->physicalState |= _STATE_MONEY;
	   		
	   			money->addToDatabase();
	   			money->isVisible = 1;
	   		
	   			money->hidden++;
	   		
	   			if ( character->combatGroup )
	   				character->combatGroup->addObject ( money );
	   	
	   			character->room->addObject ( money );
	   		
	   			money->hidden--;
	   	
	   			// put the servID of the new money object
	   			response.putLong ( money->servID );
	   	
	   			// end the movie
	   			response.putByte ( _MOVIE_END );
	   	
				roomMgr->sendACK ( message->type(), this );
	   	
				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), character->room );
	   		} else {
   				roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
   			}
	   	} else {
	   		// see if the character has the passed amount
	   		if ( character->canAfford ( amount ) ) {
	   			character->value -= amount;
				character->calcWeight();
   
  				PackedMsg response;
   	
  				// who this movie is for
  				response.putLong ( character->servID );
  				response.putLong ( character->room->number );
  	
  				// put movie data in response
  				response.putByte ( _MOVIE_MONEY_DROP );
  				response.putLong ( character->servID );
  				response.putLong ( amount );
  	
  				// create the new money object that will be on the ground
  				WorldObject *money = new WorldObject ( super );
  	
  				BCarryable *bcarry = (BCarryable *)money->getBase ( _BCARRY );
  				bcarry->show = 0;
  	
  				money->x = character->x;
  				money->y = character->y;
  				money->value = amount;
  				money->physicalState |= _STATE_MONEY;
  	
  				money->addToDatabase();
  				money->isVisible = 1;
  	
  				money->hidden++;
  	
  				if ( character->combatGroup )
  					character->combatGroup->addObject ( money );
  	
  				character->room->addObject ( money );
  	
  				money->hidden--;
  	
  				// put the servID of the new money object
  				response.putLong ( money->servID );
  	
  				// end the movie
  				response.putByte ( _MOVIE_END );
  	
  				roomMgr->sendACK ( message->type(), this );
  	
  				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), character->room );
  			} else {
  				roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
  			}
  		}
	} else 
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );

	// give client control again
	handsOn();
	return retVal;
}

int RMPlayer::process_IPC_SHOP_EXAMINE ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the servID of the store to examine from
	long servID = packet.getLong();

	// get the ID of the item to examine
	long id = packet.getLong();

	// find the store
	WorldObject *shop = roomMgr->findObject ( servID );
	BShop *bshop = shop? (BShop *)shop->getBase ( _BSHOP ) : NULL;

	if ( shop && bshop ) {

		ShopItem *item = bshop->at ( id );

		if ( item ) {
			PackedMsg response;

			// put ACK info
			response.putACKInfo ( message->type() );

			if ( item->type == _SHOP_OBJECT || item->type == _SHOP_CRYSTALS ) {
				ShopObject *obj = (ShopObject *)item;

				char str[10240];
				describeObject ( character, obj->object, item->description(), str, sizeof ( str ) );

				response.putString ( str );
			} else {
				response.putString ( item->description() );
			}
			roomMgr->sendTo ( _IPC_PLAYER_ACK, &response, this );
		} else {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		}
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_SHOP_BUY ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the servID of the store to purchase from
	long servID = packet.getLong();

	// get the ID of the item to purchase
	long id = packet.getLong();

	// find the store
	WorldObject *shop = roomMgr->findObject ( servID );
	BShop *bshop = shop? (BShop *)shop->getBase ( _BSHOP ) : NULL;

	if ( !shop || !bshop )
		return retVal;

	if ( !bshop->sellMarkup && !bshop->buyMarkup && !checkAccess ( _ACCESS_BUY_STORE ) ) {

		BCharacter *bchar = (BCharacter *)character->getBase ( _BCHARACTER );

		if ( bchar ) {
			bchar->experience = 0;
			bchar->buildPoints = 0;
			memset ( bchar->spells, 0, sizeof ( bchar->spells ) );
			memset ( bchar->skills, 0, sizeof ( bchar->skills ) );
		}

		character->level = 1;
		character->strength = 1;
		character->intelligence = 1;
		character->dexterity = 1;
		character->endurance = 1;
		character->value = 0;
		character->manaValue = 0;

		roomMgr->destroyObj ( shop, 1, __FILE__, __LINE__ );

//		disable ( "character %s bought from a GOD BALDRIC", getName() );
	} 

	else if ( character->room == shop->getRoom() ) {

		ShopItem *item = bshop->at ( id );

		if ( item ) {
			PackedMsg response;

			// put ACK info
			response.putACKInfo ( message->type() );

			// put the item type
			response.putByte ( item->type );

			switch ( item->type ) {
				case _SHOP_OBJECT: {
					if ( character->canAfford( item->price(), bshop->currency )  || IsThisATestServer() ) {
						WorldObject *object = new WorldObject ( ((ShopObject *)item)->object );

						object->addToDatabase();

						// set the room of the object temporarily
						object->room = room;
						object->addAffect ( _AFF_IDENTIFIED, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, 0, 0, NULL, 0 );
						object->room = 0;

						int result = object->forceIn ( character, -1 );

						if ( result == _WO_ACTION_HANDLED ) {

							object->buildPacket ( &response, 1 );

							switch( bshop->currency ) {
								case BShop::Gold:
									character->value -= item->price();
									response.putLong ( item->price() );
									break;
								case BShop::Copper:
									nCoppers -= item->price();
									gDataMgr->copper( this, nCoppers );
									response.putLong ( 0 ); //subtract zero gold from inv
									break;
								default:
									logDisplay("%s:%d currency type not supported for BShop", __FILE__, __LINE__ );
									response.putLong ( 0 );
							};
							
							//log a priveleged store purchase
							if ( !bshop->sellMarkup && !bshop->buyMarkup ) {
								if( object && object->classID )
									gDataMgr->logPermanent( getLogin(),  getName(), _DMGR_PLOG_UNKNOWN, "purchased one %s from a priveleged store", object->classID );
							}


							roomMgr->sendTo ( _IPC_PLAYER_ACK, response.data(), response.size(), this );
						} else {
							roomMgr->sendNAK ( message->type(), result, this );
							// toss the object we just made
							roomMgr->destroyObj ( object, 1, __FILE__, __LINE__ );
						}

					} else {
						roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
					}
				}

				break;

				case _SHOP_CRYSTALS: {
					long numToBuy = packet.getLong();

					if ( (numToBuy < 1) || (numToBuy > 9999999) ) {
						roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
					} else {
						long price = item->price() * numToBuy;

						if ( price <= character->value || IsThisATestServer() ) {
							character->value -= price;
							character->manaValue += numToBuy;
							character->calcWeight();

							response.putLong ( numToBuy );
							response.putLong ( price );

							roomMgr->sendTo ( _IPC_PLAYER_ACK, &response, this );
						} else {
							roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
						}
					}
				}

				break;

				default: {
						roomMgr->sendNAK ( message->type(), _ERR_SERVICE_NOT_AVAILABLE, this );
				}
			}
		} else {
			roomMgr->sendNAK ( message->type(), _ERR_SERVICE_NOT_AVAILABLE, this );
		}
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_GET_REPAIR_PRICE ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the shop to repair at
	WorldObject *shop = roomMgr->findObject ( packet.getLong() );
	BShop *bshop = shop? (BShop *)shop->getBase ( _BSHOP ) : NULL;

	if ( !shop || !bshop )
		return retVal;

	// get the object to repair
	WorldObject *object = roomMgr->findObject ( packet.getLong() );

	if ( object ) {
		if ( !bshop->buyMarkup || strstr ( object->classID, "Newbie" ) ) {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		}

		else if ( object->health < object->healthMax ) {
			int cost = object->calcRepairCost();
			roomMgr->sendACK ( message->type(), cost, this );
		} else {
			roomMgr->sendNAK ( message->type(), _ERR_REDUNDANT, this );
		}
	}

	return retVal;
}

int RMPlayer::process_IPC_GET_REPAIR_PRICES ( IPCMessage *message )
{
	int retVal = 1;

	// no character, or no player is ignored...
	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the shop
	int	nShopID = packet.getLong();
	WorldObject *shop = roomMgr->findObject ( nShopID );
	BShop *bshop = shop? (BShop *)shop->getBase ( _BSHOP ) : NULL;

	// no shop or invalid shop is ignored...
	if ( !shop || !bshop )
		return retVal;

	//
	// step through the character's inventory and put repairable items
	// in a response message...
	//
	BContainer *pContainer = (BContainer *)character->getBase ( _BCONTAIN );

	// no container is ignored...
	if ( !pContainer )
		return retVal;

	// ready the response message...
	PackedMsg response;
	response.putACKInfo ( message->type() );

	response.putLong( nShopID );

	LinkedElement *pElement = pContainer->contents.head();

	int nCount = 0;

	while ( pElement ) {
		WorldObject *pObject = (WorldObject *)pElement->ptr();
		pElement = pElement->next();

		// ignore non-carryable...
		if ( !pObject->getBase ( _BCARRY ) )
			continue;

		// ignore heads...
		if ( pObject->getBase ( _BHEAD ) )
			continue;

		if ( pObject->health < pObject->healthMax ) {
			int cost = pObject->calcRepairCost();

			// put the servID of the item to sell...
			response.putLong ( pObject->servID );

			// put the repair cost of the item...
			response.putLong ( cost );

			nCount++;
		}
	}

	if ( !nCount ) {
		roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );

		return retVal;
	}

	// put a terminator...
	response.putLong ( -1 );

	// send the response...
	roomMgr->sendTo ( _IPC_PLAYER_ACK, response.data(), response.size(), this );

	return retVal;
}

int RMPlayer::process_IPC_MASS_REPAIR ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the shop to repair at
	WorldObject *shop = roomMgr->findObject ( packet.getLong() );
	BShop *bshop = shop? (BShop *)shop->getBase ( _BSHOP ) : NULL;

	if ( !shop || !bshop )
		return retVal;

	int nServID;
	int nTotalCost = 0;

	while ( (nServID = packet.getLong() ) != -1 ) {
		// get the object to repair
		WorldObject *object = roomMgr->findObject ( nServID );

		if ( object && character->owns ( object ) ) {
			if ( object->health < object->healthMax ) {
				int cost = object->calcRepairCost();

				if ( cost > character->value ) {
					roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
					return retVal;
				} else {
					nTotalCost += cost;
					character->value -= cost;
					object->health = object->healthMax;
				}
			} else {
				roomMgr->sendNAK ( message->type(), _ERR_REDUNDANT, this );
				return retVal;
			}
		}
	}

	roomMgr->sendACK ( message->type(), nTotalCost, this );

	return retVal;
}

int RMPlayer::process_IPC_REPAIR ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the shop to repair at
	WorldObject *shop = roomMgr->findObject ( packet.getLong() );
	BShop *bshop = shop? (BShop *)shop->getBase ( _BSHOP ) : NULL;

	if ( !shop || !bshop )
		return retVal;

	// get the object to repair
	WorldObject *object = roomMgr->findObject ( packet.getLong() );

	if ( object && object->getBase ( _BCARRY ) && character->owns ( object ) ) {

		if ( strstr ( object->classID, "Newbie" ) ) {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		}

		else if ( object->health < object->healthMax ) {
			int cost = object->calcRepairCost();

			if ( cost > character->value ) {
				roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
			} else {
				character->value -= cost;
				object->health = object->healthMax;
				roomMgr->sendACK ( message->type(), this );
			}
		} else {
			roomMgr->sendNAK ( message->type(), _ERR_REDUNDANT, this );
		}
	}

	return retVal;
}

int RMPlayer::process_IPC_SHOP_RECHARGE ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the shop to recharge at 
	WorldObject *shop = roomMgr->findObject ( packet.getLong() );
	BShop *bshop = shop? (BShop *)shop->getBase ( _BSHOP ) : NULL;

	if ( !shop || !bshop )
		return retVal;

	// get the object to recharge
	WorldObject *object = roomMgr->findObject ( packet.getLong() );

	if ( object && object->getBase ( _BCARRY ) && character->owns ( object ) && !object->getBase ( _BHEAD ) ) {

		BUse *buse = (BUse *)object->getBase ( _BUSE );

		if ( !buse || buse->usesMax == -1 || buse->uses == buse->usesMax ) {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		} else {
			int cost = (buse->usesMax - buse->uses) * buse->useCost;	
			cost = (cost * bshop->sellMarkup) / 100;

			if ( character->value >= cost ) {
				character->value -= cost;
				buse->uses = buse->usesMax;

				roomMgr->sendACK ( message->type(), this );
			} else {
				roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
			}
		}
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_SHOP_GET_RECHARGE_PRICE ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the shop to recharge at 
	WorldObject *shop = roomMgr->findObject ( packet.getLong() );
	BShop *bshop = shop? (BShop *)shop->getBase ( _BSHOP ) : NULL;

	if ( !shop || !bshop )
		return retVal;

	// get the object to recharge
	WorldObject *object = roomMgr->findObject ( packet.getLong() );

	if ( object && object->getBase ( _BCARRY ) && character->owns ( object ) && !object->getBase ( _BHEAD ) ) {

		BUse *buse = (BUse *)object->getBase ( _BUSE );

		if ( !buse || buse->usesMax == -1 || buse->uses == buse->usesMax ) {
			roomMgr->sendACK ( message->type(), -2, this );
		} else {
			int cost = (buse->usesMax - buse->uses) * buse->useCost;	
			cost = (cost * bshop->sellMarkup) / 100;
			roomMgr->sendACK ( message->type(), cost, this );
		}
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_SHOP_GET_PRICE ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the shop
	WorldObject *shop = roomMgr->findObject ( packet.getLong() );
	BShop *bshop = shop? (BShop *)shop->getBase ( _BSHOP ) : NULL;

	if ( !shop || !bshop )
		return retVal;

	// get the object to check
	WorldObject *object = roomMgr->findObject ( packet.getLong() );

	if ( object && character->room == shop->getRoom() && object->getBase ( _BCARRY ) && 
		character->owns ( object ) && !object->getBase ( _BHEAD ) ) {

		if ( checkAccess ( _ACCESS_MODERATOR ) ) {
			roomMgr->sendACK ( message->type(), 0, this );
		} else {
			if ( !bshop->buyMarkup || (object->physicalState & _STATE_WORTHLESS) ) {
				roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
			} else {
				if ( !IsThisATestServer() )
					roomMgr->sendACK ( message->type(), bshop->price ( object ), this );
				else
					roomMgr->sendACK ( message->type(), 0, this );
			}
		}
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}
	return retVal;
}

int RMPlayer::process_IPC_SHOP_SELL ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the shop
	WorldObject *shop = roomMgr->findObject ( packet.getLong() );
	BShop *bshop = shop? (BShop *)shop->getBase ( _BSHOP ) : NULL;

	if (!shop || !bshop )
		return retVal;

	// get the object to sell
	WorldObject *object = roomMgr->findObject ( packet.getLong() );

	// sell the object to the shop
	if ( object &&	character->owns ( object ) && object->getBase ( _BCARRY ) && !object->getBase ( _BHEAD ) ) {

		int	price = 0;
		int result = 0;
	
		if ( checkAccess ( _ACCESS_MODERATOR ) ) {
			result = bshop->toss ( object );
		} else {
			price = bshop->price ( object );
			result = bshop->buy ( object, character );
		}
	
		if ( result == _WO_ACTION_HANDLED ) {
			roomMgr->sendACK ( message->type(), price, this );
		} else {
			roomMgr->sendNAK ( message->type(), result, this );
		}
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_SET_TITLE ( IPCMessage *message )
{
	int retVal = 1;

	PackedMsg packet ( message->data(), message->size() );
	WorldObject *obj = roomMgr->findObject ( packet.getLong() );

	if ( !obj || !player ) {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		return retVal;
	}

	BPlayer *base = (BPlayer *)player->getBase ( _BPLAYER );

	if ( !base || !base->ownsCharacter ( obj ) || player->player != this ) {
//		disable ( "misdirected set title for %s", obj->getName() );
		return retVal;
	}

	char *title = packet.getString();

	if ( title ) {
		int len = strlen ( title );

		if ( len > 30 ) {
			roomMgr->sendSystemMsg ( "Invalid Title", this, "The title you submitted is invalid." );
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
			free ( title );
			return retVal;
		}

		// make title valid
		char *ptr = title;

		while ( *ptr ) {
			if ( !isprint ( *ptr ) || *ptr == '|')
				*ptr = ' ';

			ptr++;
		}

		BPlayer *base = (BPlayer *)player->getBase ( _BPLAYER );
		
		if ( base && obj->character && strcmp( obj->character->title, title ) ) {
  			gDataMgr->logPermanent ( getLogin(), obj->getName(), _DMGR_PLOG_SETTITLE, "%s", title );
			strcpy ( obj->character->title, title );
		}

	 	free ( title );

		// must write character data. Could be at menu!
		obj->writeCharacterData();
		roomMgr->sendACK ( message->type(), this );
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}
	return retVal;
}

int RMPlayer::process_IPC_GET_SHOP_INFO ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		
	PackedMsg packet ( message->data(), message->size() );

	// get the servID of the shop
	long servID = packet.getLong();

	WorldObject *shop = roomMgr->findObject ( servID ); 
	BShop *bshop = shop? (BShop *)shop->getBase ( _BSHOP ) : NULL;

	if ( !shop ) 
		return retVal;

	if ( !bshop ) {
		return retVal;
	}

	PackedMsg response;

	// put ACK info
	response.putACKInfo ( message->type() );

	BCharacter *bchar = (BCharacter *)character->getBase ( _BCHARACTER );
	response.putWord ( bchar->buildPoints );

	response.putByte( bshop->currency );

	if( bshop->currency == BShop::Copper )
		response.putLong( nCoppers );

	// put the shop information
	bshop->buildShopPacket ( &response, character );

	// put the weight capacity of the character
	response.putLong ( character->calcWeightCap() - character->getWeight() );

	// put the weight information
	bshop->buildWeightPacket ( &response );

	roomMgr->sendTo ( _IPC_PLAYER_ACK, &response, this );

	return retVal;
}

int RMPlayer::process_IPC_COMBAT_EXIT ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		
	PackedMsg packet ( message->data(), message->size() );

	// for now, assume we can exit always
	int canExit = character->combatGroup? 1 : 0;

	if ( canExit ) {
		roomMgr->sendACK ( message->type(), this );
		exitCombat();
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_REDUNDANT, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_COMBAT_FLEE ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		
	PackedMsg packet ( message->data(), message->size() );

	// for now, assume we can flee
	int canFlee = ( character->combatGroup && character->health > 0 )? 1 : 0;

	if ( canFlee ) {
		PackedMsg response;
		response.putACKInfo ( message->type() );

		roomMgr->sendTo ( _IPC_PLAYER_ACK, &response, this );

		int chance = random ( 0, 100 );
		int moneyDropped = 0;

		// drop money
		if ( chance < 61 ) {
			int value = character->value / 2;	

			if ( value ) {
				moneyDropped = 1;

				WorldObject *money = new WorldObject ( roomMgr->findClass ( "MoneyBag" ) );

				money->x = character->x;
				money->y = character->y;
				money->physicalState = _STATE_MONEY;
				money->isVisible = 1;
				money->value = value;
				money->addToDatabase();

				money->hidden++;

				if ( character->combatGroup )
					character->combatGroup->addObject ( money );

				character->room->addObject ( money );

				character->value -= value;
				character->calcWeight();

				money->hidden--;

				PackedMsg movie;
				movie.putLong ( character->servID );
				movie.putLong ( character->room->number );
				movie.putByte ( _MOVIE_MONEY_DROP );
				movie.putLong ( character->servID );
				movie.putLong ( value );
				movie.putLong ( money->servID );
				movie.putByte ( _MOVIE_END );

				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, movie.data(), movie.size(), character->room );

				roomMgr->sendSystemMsg ( "Oh No!", this, "In your haste to run away, you dropped %d gold coins!", value );
			}
		}

		if ( !moneyDropped ) {
			PackedMsg response;
			response.putLong ( character->servID );
			response.putLong ( room->number );

			LinkedList items;

			BContainer *bcontain = (BContainer *)character->getBase ( _BCONTAIN );

			LinkedElement *element = bcontain->contents.head();

			while ( element ) {
				WorldObject *obj = (WorldObject *)element->ptr();
				element = element->next();

				if ( !obj->getBase ( _BHEAD ) && obj->objectWornOn == -1 && obj->objectWieldedOn == -1 ) 
					items.add ( obj );
			}

			if ( items.size() ) {
				WorldObject *item = (WorldObject *)items.at ( random ( 0, items.size() - 1 ) );

				roomMgr->sendSystemMsg ( "Oh No!", this, "In your haste to run away, you dropped your %s!", item->name );

				if ( character->drop ( item, &response ) )
					roomMgr->destroyObj ( item, 1, __FILE__, __LINE__ );

				response.putByte ( _MOVIE_END );

				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), room );
			}

			items.release();
		}

		exitCombat();
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_VERB_STAND ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		
	PackedData movie;
	WorldObject *directObject, *indirectObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;

	if ( character->sittingOn ) {
		result = character->stand ( &movie );
	}

	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	return retVal;
}

void RMPlayer::rob ( WorldObject *directObject, PackedData *movie )
{
	if ( directObject->playerControlled ) {
		character->character->peaceful = 0;
		character->lastCrimeTime = getseconds();
	}

	int chanceToSteal = calcStealChance ( character, directObject ) - directObject->thiefProtection;

	int pickedPocket = (random ( 1, 100 ) <= chanceToSteal);
	int gotCaught = (random ( 1, 100 ) > chanceToSteal);

   	affect_t *affect = NULL; 
   	affect = directObject->hasAffect (_AFF_IMMOLATION_FIRE, _AFF_TYPE_NORMAL);

   	if ( affect && !character->hasAffect ( _AFF_DAMAGE_FIRE, _AFF_TYPE_RESISTANCE ) )
   	{
   		// burn the poor thief's fingers.
   		int theDamage = random ( 1, 6 ) + (affect->value / 2);

   		theDamage = character->takeDamage ( _AFF_DAMAGE_FIRE, character, theDamage, NULL, movie, 1 );

   		if ( theDamage > 0 )
   		{
   			putMovieText (character, movie, "|c60|%s's fingers are burned while trying to pick %s's pockets!|c43|", getName(), directObject->getName() );
//   		pickedPocket = 0;
   			gotCaught = 1;
   		}
   	}

	//Zach - lightning immolation addition
 	affect = directObject->hasAffect (_AFF_IMMOLATION_LIGHTNING, _AFF_TYPE_NORMAL);
	
   	if ( affect && !character->hasAffect ( _AFF_DAMAGE_LIGHTNING, _AFF_TYPE_RESISTANCE ) )
   	{
   		// electrify the poor thief's fingers.
		//note this is the standard immolation formula
   		int theDamage = random ( 1, 6 ) + (affect->value / 2);
   		theDamage = character->takeDamage ( _AFF_DAMAGE_LIGHTNING, character, theDamage, NULL, movie, 1 );
	
   		if ( theDamage > 0 )
   		{
   			putMovieText (character, movie, "|c12|%s's fingers are zapped while trying to pick %s's pockets!|c43|", getName(), directObject->getName() );
   			gotCaught = 1;
   		}
   	}
	//Zach - cold immolation addition
	affect = directObject->hasAffect (_AFF_IMMOLATION_COLD, _AFF_TYPE_NORMAL);
	
   	if ( affect && !character->hasAffect ( _AFF_DAMAGE_COLD, _AFF_TYPE_RESISTANCE ) )
   	{
   		// freeze the poor thief's fingers.
		//note this is the standard immolation formula
   		int theDamage = random ( 1, 6 ) + (affect->value / 2);
   		theDamage = character->takeDamage ( _AFF_DAMAGE_COLD, character, theDamage, NULL, movie, 1 );
	
   		if ( theDamage > 0 )
   		{
   			putMovieText (character, movie, "|c55|%s's fingers are frozen while trying to pick %s's pockets!|c43|", getName(), directObject->getName() );
   			gotCaught = 1;
   		}
   	}

	int moneyTaken = 0, manaTaken = 0, robResult = 0;

	movie->putByte ( _MOVIE_ROB );
	movie->putLong ( directObject->servID );

	#define _THEFT_SUCCESS 1
	#define _THEFT_DETECT 2

	if ( pickedPocket ) {

		robResult |= _THEFT_SUCCESS;

		int skillDiff = (character->getSkill ( _SKILL_PICK_POCKETS ) - directObject->getSkill ( _SKILL_PICK_POCKETS )) + 1;

		int amount = random ( 1, 10 );

		if ( skillDiff > 0 )
			amount = random ( skillDiff * 25, skillDiff * 250 );
			
		// if the target has more money than mana, take the money then the mana
		if ( directObject->value > (directObject->manaValue * 5) ) {
			moneyTaken = std::min( directObject->value, amount);
//( directObject->value <? amount );
			amount -= moneyTaken;

			manaTaken = std::min( directObject->manaValue, ( amount / 5 ) );
//( directObject->manaValue <? ( amount / 5 ) );
			amount -= manaTaken * 5;
		} else {
			manaTaken = std::min( directObject->manaValue, ( amount / 5 ) );
//( directObject->manaValue <? ( amount / 5 ) );
			amount -= manaTaken * 5;

			moneyTaken = std::min( directObject->value, amount);
//( directObject->value <? amount );
			amount -= moneyTaken;
		}
	}

	// make sure the magistrate knows about this player
	if ( gotCaught ) {
		
		robResult |= _THEFT_DETECT;
		directObject->thiefProtection += 25;

		if ( !isNPC && directObject->playerControlled )  {
			character->updateWanted ( character, _CRIME_THEFT );
		}
	}

	movie->putLong ( moneyTaken );
	movie->putLong ( manaTaken );
	movie->putByte ( robResult );

	if ( moneyTaken ) {
		directObject->value -= moneyTaken;
		character->value += moneyTaken;
	}

	if ( manaTaken ) {
		directObject->manaValue -= manaTaken;
		character->manaValue += manaTaken;
	}

	if ( character->pvpGrace > 0 ) {
		roomMgr->sendPlayerText ( this, "|c43|Info>  Your player killer grace period has ended because you have exercised your thief skills." );
		character->pvpGrace = 0;
	}
}

int RMPlayer::process_IPC_VERB_ROB ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	int allowed = 1;

	PackedData movie;
	WorldObject *directObject, *indirectObject;

	PackedData packet ( message->data(), message->size() );

	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;

	// no pickpocket in tourney, arena area, dungeon entrance
	// guides and dead characters or self!
	if ( zone && directObject && directObject->character && directObject->getRoom () == character->getRoom() && !directObject->player->checkAccess ( _ACCESS_GUIDE ) && !room->isDungeonEntrance && character->health > 0 && directObject->health > 0 ) {
		// if the zone allows full PvP, robbing is allowed
		allowed = ((zone->allowCombat() & _ALLOW_PLAYER_COMBAT) || directObject->player->isNPC)? 1 : 0;

		if ( allowed ) {
			rob ( directObject, &movie );
			result = _WO_ACTION_HANDLED;
		}
	}

	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	return retVal;
}

int RMPlayer::process_IPC_VERB_MEMORIZE ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	PackedData movie;
	WorldObject *directObject, *indirectObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;

	// make sure we own the object
	if ( directObject && character->owns ( directObject ) ) {

		result = character->memorize ( directObject, &movie );
	}

	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	// if we memorized it, destroy it
	if ( result == _WO_ACTION_HANDLED ) 
		roomMgr->destroyObj ( directObject, FALSE, __FILE__, __LINE__ );

	return retVal;
}

int RMPlayer::process_IPC_VERB_SIT ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	PackedData movie;
	WorldObject *directObject, *indirectObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;

	// make sure we can sit on the object

	if ( directObject && character->health > 0 && directObject->room == character->room && (character->view < 300) ) {

		if ( !directObject->getBase ( _BSIT ) ) {
//			disable ( "character %s diverted sit object to %s", getName(), directObject->getName() );
		} else {
			result = character->sit ( directObject, &movie );
		}
	}

	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	return retVal;
}

int RMPlayer::process_IPC_VERB_CONSUME ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	PackedData movie;
	WorldObject *directObject, *indirectObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;

	// make sure we can consume the directObject
	if ( directObject && character->health > 0 && 
		directObject->getRoom() == character->room && directObject->getBase ( _BCONSUME ) ) 
	{
		directObject->makeVisible ( 1 );

		// have the character consume the directObject
		result = character->consume ( directObject, &movie );

		// if the result is positive, destroy the consumed object
		if ( result == _WO_ACTION_HANDLED ) {
			roomMgr->destroyObj ( directObject, FALSE, __FILE__, __LINE__ );
		}
	}

	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	return retVal;
}

int RMPlayer::process_IPC_VERB_ENGAGE ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	PackedData movie;
	WorldObject *directObject, *indirectObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int theX = packet.getWord();
	int theY = packet.getWord();

	int result = _WO_ACTION_PROHIBITED;
	int canFight = 1;

	if ( !directObject || !directObject->player || directObject->getRoom() != room || 
		directObject->player->isTeleporting || isTeleporting || directObject->health < 1 || 
		character->health < 1 || isGroupMember ( directObject->player ) || 
		directObject->character->peaceful )
		canFight = 0;

	if ( canFight ) {
		// attempt to engage self in combat
		if ( directObject->player == this ) {
			canFight = 0;
			goto engageEnd;
		}

		if ( zone ) {
			int allow = zone->allowCombat();

			if ( directObject->player->isNPC ) {
				if ( !(allow & _ALLOW_NPC_COMBAT) )
					canFight = 0;
			} else {
				if ( !(allow & _ALLOW_PLAYER_COMBAT) ) 
					canFight = 0;
			}
		} else {
			canFight = 0;
		}
	}

	if ( canFight && room && !directObject->player->isNPC && room->isDungeonEntrance )
		canFight = 0;

	if ( canFight ) {
		if ( directObject->combatGroup || character->combatGroup ) {
			result = _ERR_REDUNDANT;
			goto engageEnd;
		}

		if ( directObject->pvpGrace > 0 ) {
			result = _ERR_TOO_HEAVY;
			goto engageEnd;
		}

		if ( !directObject->player->isNPC && !(room->zone->allowCombat() & _COMBAT_TOURNEY) ) {
			character->character->peaceful = 0;
			character->lastCrimeTime = getseconds();
		}

		if ( engage ( directObject, &movie, theX, theY ) )
			result = _WO_ACTION_HANDLED;

	} else {
		result = _WO_ACTION_PROHIBITED;

		if ( room->isDungeonEntrance ) {
			roomMgr->sendPlayerInfo ( this, "You cannot fight other players in entrance zones.\n" );
			result = _ERR_TOO_HEAVY;
		}

		if ( character && character->health < 1 )
			result = _ERR_DEAD;

		if ( directObject && directObject->character && directObject->character->peaceful )
			result = _ERR_TOO_EXPENSIVE;
	}

engageEnd:
	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	return retVal;
}

int RMPlayer::process_IPC_COMBAT_ACTION ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		
	if ( !character->combatGroup ) {
//		roomMgr->sendSystemMsg ( "Oops!", this, "Somehow you chose a combat action out of combat.  This is a bug. ;)  Send us mail describing what you did!" );
		if ( character->room ) 
			handsOn();

		return retVal;
	}

	if ( character->health < 1 ) {
		return retVal;
	}

	PackedMsg packet ( message->data(), message->size() );

	int type = packet.getByte();

	switch ( type ) {
		case _CA_MOVE: {
			int x = packet.getByte();
			int y = packet.getByte();
			
			setAction ( new CombatMove ( character, x, y, 1  ) );
		}

		break;

		case _CA_ATTACK: {
			setAction ( new CombatAttack ( character, roomMgr->findObject ( packet.getLong() ) ) );
		}

		break;

		case _CA_EQUIP: {
			setAction ( new CombatEquip ( character, roomMgr->findObject ( packet.getLong() ) ) );
		}

		break;

		case _CA_CHARGE: {
			setAction ( new CombatCharge ( character, roomMgr->findObject ( packet.getLong() ), 1 ) );
		}

		break;

		case _CA_FLEE: {
			setAction ( new CombatFlee ( character ) );
		}

		break;

		case _CA_EAT: {
			setAction ( new CombatEat ( character, roomMgr->findObject ( packet.getLong() ) ) );
		}

		break;

		case _CA_GUARD: {
			setAction ( new CombatGuard ( character ) );
		}

		break;

		case _CA_CAST_SPELL: {
			int spellID = packet.getWord();
			long casterID = packet.getLong();
			long targetID = packet.getLong();
			int targetX = packet.getByte();
			int targetY = packet.getByte();

			if ( (spellID < 0) || (spellID > _SPELL_MAX) ) {
				setAction ( new CombatFlee ( character ) );
				return 1;
			}

			if ( targetY < 0 || targetX < 0 ) {
				setAction ( new CombatFlee ( character ) );
				return 1;
			}

			WorldObject *casterObj = roomMgr->findObject ( casterID );
			BCharacter *bchar = NULL;
			BUse *buse = NULL;
			int knowsSpell = 0;
			BPlayer *bplayer = NULL;

			if ( !casterObj ) {
				setAction ( new CombatFlee ( character ) );
				return 1;
			}

			spell_info *spell = &gSpellTable[spellID];

			// if trying to cast a non-combat spell, choke
			if ( spell->isCombat == _NON_COMBAT_SPELL ) {
				setAction ( new CombatFlee ( character ) );
				return 1;
			}

			// check for using artifacts
			if ( character && casterObj != character ) {
				buse = (BUse *)casterObj->getBase ( _BUSE );	

				if ( !buse ) {
					setAction ( new CombatFlee ( character ) );
					return retVal;
				}

				if ( spellID != buse->spell ) {
					setAction ( new CombatFlee ( character ) );
					return retVal;
				}
			}

			// check to see if player knows the spell
			bchar = (BCharacter *)casterObj->getBase ( _BCHARACTER );

			if ( !casterObj->playerControlled )
				knowsSpell = 1;
			else 
				knowsSpell = bchar? bchar->knowsSpell ( spellID ) : 0;

			if ( bchar && (!knowsSpell || (spellID < 0 || spellID >= _SPELL_MAX)) ) {
				setAction ( new CombatFlee ( character ) );
				return retVal;
			}

			// validate targets 

			WorldObject *target = roomMgr->findObject ( targetID );
			WorldObject *owningObj = NULL;

			if ( target ) {
				if ( target->getBase ( _BHEAD ) ) {
					setAction ( new CombatFlee ( character ) );
					return retVal;
				}

				owningObj = target->getBaseOwner();
				
				if ( owningObj && owningObj->combatGroup != character->combatGroup ) {
					setAction ( new CombatFlee ( character ) );
					return retVal;
				} else {
					if ( !owningObj ) {
						setAction ( new CombatFlee ( character ) );
						logInfo ( _LOG_ALWAYS, "%s: casting at combat target %s with no owner object", getName(), target->getName() );
						return retVal;
					}
				}
			}

			// validate caster
			owningObj = casterObj->getBaseOwner();

			if ( owningObj && owningObj->player && owningObj->player != this ) {
				setAction ( new CombatFlee ( character ) );
				return retVal;
			}

			if ( !target || targetID < 1 )
				targetID = 0;

			if ( character )
				setAction ( new CombatCastSpell ( character, spellID, casterID, targetID, targetX, targetY ) );
		}

		break;

		// NEW.. TEST!
		default: {
//			setAction ( new CombatFlee ( character ) );
			logInfo ( _LOG_ALWAYS, "invalid action type received from %s", character->getName() );
		}
	}
	return retVal;
}

int RMPlayer::process_IPC_VERB_UNLOCK ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	PackedData movie;
	WorldObject *directObject, *indirectObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;

	// check to see if a key is being used
	if ( directObject && directObject->getRoom() == character->room && directObject->getBase ( _BLOCK ) )
	{
		if ( indirectObject && !indirectObject->getBase ( _BKEY ) ) {
//			disable ( "character %s unlock using misdirected object %s", getName(), indirectObject->getName() );
			return retVal;
		}
		else
			result = character->unlock ( directObject, indirectObject, &movie );

		WorldObject *linkObj = directObject->linkTo;

		if ( result == _WO_ACTION_HANDLED && linkObj ) {
			if ( linkObj->beUnlocked ( linkObj, indirectObject ) == _WO_ACTION_HANDLED ) {
				PackedMsg response;

				// put who the movie is for
				response.putLong ( linkObj->servID );
				response.putLong ( linkObj->getRoom()->number );

				// put the fact that this object locked
				response.putByte ( _MOVIE_UNLOCK );
				response.putLong ( linkObj->servID );
				response.putLong ( linkObj->servID );
				response.putLong ( indirectObject? indirectObject->servID : -1 );

				response.putByte ( _MOVIE_END );

				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), linkObj->getRoom() );
			}
		}

		// toss tossable used keys
		if ( (result == _WO_ACTION_HANDLED) && indirectObject && (indirectObject->physicalState & _STATE_TOSS) ) {
			roomMgr->sendRoomText ( room, "|c248|Info> As soon as the %s was used to unlock the %s, it disappeared!\n", indirectObject->name, directObject->name ); 
			roomMgr->destroyObj ( indirectObject, 1, __FILE__, __LINE__ );
		}
	}
	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	return retVal;
}

int RMPlayer::process_IPC_VERB_LOCK ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	PackedData movie;
	WorldObject *directObject, *indirectObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;

	// check to see if a key is being used
	if ( directObject && directObject->getRoom() == character->room && directObject->getBase ( _BLOCK ) )
	{
		if ( indirectObject && !indirectObject->getBase ( _BKEY ) ) {
//			disable ( "character %s trying to lock using invalid key %s.", getName(), indirectObject->getName() );
			return retVal;
		}
		else
			result = character->lock ( directObject, indirectObject, &movie );

		WorldObject *linkObj = directObject->linkTo;

		if ( result == _WO_ACTION_HANDLED && linkObj ) {
			if ( linkObj->beLocked ( linkObj, indirectObject ) == _WO_ACTION_HANDLED ) {
				PackedMsg response;

				// put who the movie is for
				response.putLong ( linkObj->servID );
				response.putLong ( linkObj->getRoom()->number );

				// put the fact that this object locked
				response.putByte ( _MOVIE_LOCK );
				response.putLong ( linkObj->servID );
				response.putLong ( linkObj->servID );
				response.putLong ( indirectObject? indirectObject->servID : -1 );

				response.putByte ( _MOVIE_END );

				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), linkObj->getRoom() );
			}
		}			
	}

	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	return retVal;
}

int RMPlayer::process_IPC_VERB_CLOSE ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	PackedData movie;
	WorldObject *directObject, *indirectObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;

	// do this so it does not interrupt a teleport script.

	if ( zone && dungeonEntrance ) {
		zone->delPlayerFromQueue ( this );
		roomMgr->sendSystemMsg ( "Out of Line", this, "Selecting 'Close' exits you from the Queue." );
		goto closeEnd;
	}

	if ( directObject && directObject->getRoom() == character->room && 
		directObject->getBase ( _BOPEN ) ||	directObject->getBase ( _BSWITCH ) && 
		!directObject->_locked )
	{
		if ( directObject->physicalState & _STATE_OCCUPIED ) {
			result = _ERR_TOO_HEAVY;
			goto closeEnd;
		}

		result = character->close ( directObject, &movie );

		WorldObject *linkObj = directObject->linkTo;

		if ( result == _WO_ACTION_HANDLED && linkObj ) {
			if ( linkObj->beClosed ( linkObj ) == _WO_ACTION_HANDLED ) {
				PackedMsg response;

				// put who the movie is for
				response.putLong ( linkObj->servID );
				response.putLong ( linkObj->getRoom()->number );

				// put the fact that this object closed
				response.putByte ( _MOVIE_CLOSE );
				response.putLong ( linkObj->servID );
				response.putLong ( linkObj->servID );

				response.putByte ( _MOVIE_END );

				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), linkObj->getRoom() );
			}
		}
	}

	if ( directObject && directObject->_locked )
		result = _ERR_ACCESS_LOCKED;

closeEnd:
	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	return retVal;
}

int RMPlayer::process_IPC_VERB_OPEN ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	PackedData movie;
	WorldObject *directObject, *indirectObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;

	if ( directObject && directObject->getRoom() == character->room && 
		directObject->getBase ( _BOPEN ) || directObject->getBase ( _BSWITCH ) ) 
	{
		WorldObject *linkObj = directObject->linkTo;

		//don't allow an open if it's not on the ground
		BCarryable* bcarry = static_cast< BCarryable*>( directObject->getBase( _BCARRY ) );

		if ( directObject->ownerID != -1 ) {
			result = _ERR_TOO_EXPENSIVE;
			goto openEnd;
		}	

		if ( directObject->physicalState & _STATE_OCCUPIED ) {
			result = _ERR_TOO_HEAVY;
			goto openEnd;
		}

		if ( directObject->physicalState & _STATE_RIDDLED ) {
			result = _ERR_TOO_EXPENSIVE;
			goto openEnd;
		}	

		BEntry *entry = (BEntry *)directObject->getBase ( _BENTRY );

		if ( entry && !entry->room ) {
			result = _ERR_BAD_SERVID;
			goto openEnd;
		}

		// see if this is password protected
		BPassword *base = (BPassword *)directObject->getBase ( _BPASSWORD );

		if ( base ) {
			char *password = packet.getString();

			if ( password ) {
				int len = strlen ( password );

				if ( ( len < 1 || len > _MAX_PASSWORD_SIZE ) || strcmp ( base->password, password ) ) 
					result = _ERR_BAD_PASSWORD; 
				else if ( !entry )
					result = character->open ( directObject, &movie );
				else {
					character->teleport( entry->room, &movie );

					result = _WO_ACTION_HANDLED;

					goto openEnd;
				}

				free ( password );
			}
			else
				result = _ERR_BAD_PASSWORD;
	
		} else {
			result = character->open ( directObject, &movie );
		}		

		if ( result == _WO_ACTION_HANDLED && linkObj ) {
			if ( linkObj->beOpened ( linkObj ) == _WO_ACTION_HANDLED ) {
				PackedMsg response;

				// put who the movie is for
				response.putLong ( linkObj->servID );
				response.putLong ( linkObj->getRoom()->number );

				// put the fact that this object opened
				response.putByte ( _MOVIE_OPEN );
				response.putLong ( linkObj->servID );
				response.putLong ( linkObj->servID );

				response.putByte ( _MOVIE_END );

				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), linkObj->getRoom() );
			}
		}			
	}

openEnd:
	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	return retVal;
}

int RMPlayer::process_IPC_SEND_REG ( IPCMessage *message )
{
	int retVal = 1;

	char filename[1024];

	BPlayer *bPlayer = (BPlayer *)player->getBase ( _BPLAYER );
	sprintf ( sizeof ( filename ), filename, "../data/registrations/%s", bPlayer->login );

	strlower ( filename );
	File *file = new File ( filename );

	// start printing data to file.

	if ( file->isOpen() ) {
		PackedData *data = new PackedData();
		data->grow ( 5000 );

		// parse incoming packet

		PackedMsg packet ( message->data(), message->size() );

		for ( int i=0; i<7; i++ ) {
			char *tStr = packet.getString();

			if ( tStr ) {
				int len = strlen ( tStr );
				int isBad = 0;

				if ( len > 0 && len < 512 )
					data->printf ( "%s\n", tStr );
				else
					isBad = 1;

				if ( !isBad ) {
					for ( int i=0; i<len; i++ ) {
						if ( !isprint ( tStr[i] ) ) {
							isBad = 1;											
							break;
						}
					}
				}

				if ( isBad )
					data->printf ( "<garbage line>\n" );

				free ( tStr );

			} else {
				data->printf ( "<garbage line>\n" );
			}
		}

		file->truncate();
		file->write ( data->data(), data->size() );

		delete data;
	} else {
		fatal ( "unable to open registration file ( '%s' ) errno = %d", filename, errno );
	}

	delete file;

	if (exists ( filename )) 
		roomMgr->sendACK ( _IPC_SEND_REG, this );
	else
		roomMgr->sendNAK ( _IPC_SEND_REG, _IPC_PLAYER_NAK, this );

	return retVal;
}

int RMPlayer::process_IPC_MAIL_MESSAGE_SEND ( IPCMessage *message ) {
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		
	if ( gMagicMailOff != 1 ) {
		gDataMgr->sendMail( character, message->data(), message->size() );
	} else {
		PackedMsg response;

                response.putACKInfo ( message->type(), _ERR_BAD_SERVID );
                response.putString ( "Magic Mail has been removed for the time being." );
                roomMgr->sendTo ( _IPC_PLAYER_NAK, &response, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_MAIL_MESSAGE_DELETE ( IPCMessage *message ) {
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

        if ( gMagicMailOff != 1 ) {
		PackedData packet ( message->data(), message->size() );
		int msgNumber = packet.getLong();

		gDataMgr->deleteMail( character, msgNumber );
	} else {
		roomMgr->sendNAK ( _IPC_MAIL_MESSAGE_DELETE, _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_MAIL_MESSAGE_GET ( IPCMessage *message ) {
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
        if ( gMagicMailOff != 1 ) {
		PackedData packet ( message->data(), message->size() );
		int msgNumber = packet.getLong();

		gDataMgr->getMailMessage( character, msgNumber );
	} else {
                roomMgr->sendNAK ( _IPC_MAIL_MESSAGE_GET, _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_MAIL_MESSAGE_ARCHIVE ( IPCMessage *message ) {
	int retVal = 1;

  	PackedMsg response;

// disable Magic Mail
		response.putACKInfo ( message->type(), _ERR_BAD_SERVID );
		response.putString ( "Archiving Magic Mail is still being worked on." );
		roomMgr->sendTo ( _IPC_PLAYER_NAK, &response, this );
		return retVal;
// --------------------------------------------------
}

int RMPlayer::process_IPC_MAIL_MESSAGE_COMPLAIN ( IPCMessage *message ) {
	static int sendCount = 0;

	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
        if ( gMagicMailOff != 1 ) {
		PackedData packet ( message->data(), message->size() );
		int msgNumber = packet.getLong();

		gDataMgr->deleteMail( character, msgNumber );

		char filename[1024];
		sprintf ( sizeof ( filename ), filename, "/tmp/reportMM.%d.%d", gServerID, sendCount );
		sendCount++;

		unlink ( filename );

		File file;
		file.open ( filename );

		file.printf ( "From: complaint@realmserver.com\n" );
		file.printf ( "To: mm@realmserver.com\n" );
		file.printf ( "Subject: Server %d %s #%d\n", gServerID + 1, getName(), msgNumber );

		file.printf ( "MM #%d is bad", msgNumber );

		char command[1024];

		sprintf ( sizeof ( command ), command, "nohup sendmm %s &", filename );
		system ( command );
	}

	return retVal;
}

int RMPlayer::process_IPC_MAIL_LIST_GET ( IPCMessage *message ) {
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	if ( gMagicMailOff != 1 ) {
		gDataMgr->getMailList( character );
	} else {
		roomMgr->sendNAK ( _IPC_MAIL_LIST_GET, _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_MIX_OBJECT ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		
	// get the object to "mix"
	PackedData packet ( message->data(), message->size() );
	WorldObject *obj = roomMgr->findObject ( packet.getLong() );

	if ( obj ) {
		
		WorldObject *owningObj = obj->getBaseOwner();
		
		if ( owningObj && owningObj->player && owningObj->player != this ) 
			goto endMix;

		BMix *bmix = (BMix *)obj->getBase ( _BMIX );

		if ( bmix ) {
			Recipe *recipe = gRecipeList.matches ( obj );

			if ( recipe ) {
				BContainer *bcontain = (BContainer *)obj->getBase ( _BCONTAIN );

				if ( bcontain ) {
					LinkedElement *element = bcontain->contents.head();

					while ( element ) {
						WorldObject *theObj = (WorldObject *)element->ptr();
						element = element->next();

						roomMgr->destroyObj ( theObj, 1, __FILE__, __LINE__ );
					}
				}

				PackedMsg response;
				response.putACKInfo ( message->type() );

				char str[1024] = "Something happens...";

				if ( recipe->object ) 
					sprintf ( sizeof ( str ), str, "You have created a %s.", recipe->object->name );

				response.putString ( str );

				roomMgr->sendTo ( _IPC_PLAYER_ACK, &response, this );

				recipe->doit ( character );
			} else {
				roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
			}
		} else {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		}
	} else {
endMix:
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_VERB_PUT_IN ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	PackedData movie;
	WorldObject *directObject, *indirectObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;

	if ( directObject && indirectObject && directObject != indirectObject && indirectObject->getBase ( _BCARRY ) 
		&& !indirectObject->getBase ( _BHEAD ) ) {
		
		//if( indirectObject->hasAffect( _AFF_STAFF ) ) {
		//	roomMgr->sendSystemMsg ( "Sorry", this, "The '%s' is a staff-protected item. You may not place it into another object.", indirectObject->name );
		//	goto endPutInVerb;
		//}

		// base types for checks
		WorldObject *owningObj = indirectObject->getBaseOwner();
		WorldObject *owningDirObj = directObject->getBaseOwner();

		if ( ( owningObj && owningObj->player && owningObj->player == this ) || ( owningObj == owningDirObj ) ) {
			BContainer *bcontain = (BContainer *)directObject->getBase ( _BCONTAIN );

			if ( bcontain && directObject->getRoom() == character->room ) {
				if ( bcontain->doesAccept() ) {
					int nResult = 0;

					if ( (nResult = bcontain->accepts( indirectObject )) ) {
						result = _WO_ACTION_HANDLED;
						roomMgr->sendPlayerText( this, "|c65|Info> The %s accepted %d item%s.", directObject->getName(), nResult, nResult > 1 ? "s" : "" );
					}
				} else {
					// put indirectObject into directObject
					result = character->putIn ( indirectObject, directObject, &movie );
				}
			}
		}
	}

endPutInVerb:

	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	return retVal;
}

int RMPlayer::process_IPC_VERB_TAKE_OFF ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	PackedData movie;
	WorldObject *directObject, *indirectObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;

	if ( directObject && directObject->getBase ( _BCARRY ) && 
		( directObject->getBase ( _BWEAR ) || directObject->getBase ( _BWEAPON ) ) &&
		character->owns ( directObject ) ) 
	{
		if ( !checkAccess ( _ACCESS_IMPLEMENTOR ) && directObject->getBase (_BHEAD ) ) {
//			disable ( "character %s attempted to remove head.", getName() ); 
			goto endTakeOff;
		}

		result = character->takeOff ( directObject, &movie );
	}

endTakeOff:

	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	return retVal;
}

int RMPlayer::process_IPC_VERB_PUT_ON ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	PackedData movie;
	WorldObject *directObject, *indirectObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;

	if ( directObject && character->owns ( directObject ) && directObject->getBase ( _BCARRY ) && 
		( directObject->getBase ( _BWEAR ) || directObject->getBase ( _BWEAPON ) ) ) {
		result = character->putOn ( directObject, &movie );
#if 0
		if ( result == _WO_ACTION_HANDLED ) 
			character->dropUntilUnencumbered ( &movie );
#endif
	}

	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	return retVal;
}

int RMPlayer::process_IPC_VERB_DROP ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	PackedData movie;
	WorldObject *directObject, *indirectObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;

	if ( directObject && character->owns ( directObject ) && directObject->getBase ( _BCARRY ) ) 
	{
		if ( directObject->getBase (_BHEAD ) ) {
//			disable ( "character %s attempted to drop head.", getName() ); 
		}
		else if ( room->objects.size() > _DBR_MAX_OBJECTS ) {
			roomMgr->sendSystemMsg ( "Too Much Stuff", this, "You can't drop any more things in this room." ); 
		}
		else if ( room->flags & _RM_FLAG_NO_DROP ) {
			roomMgr->sendSystemMsg ( "Sorry", this, "You can't drop anything in this room." ); 
		}
		  //else if( directObject->hasAffect( _AFF_STAFF ) ) {
		  //	roomMgr->sendSystemMsg ( "Sorry", this, "The '%s' is a staff-protected item. You may not drop it from your character.", directObject->name );
		  //}

		//see if this object can only be picked up in the owners home
		else if( directObject->physicalState & _STATE_HOUSE_DECOR && !room->building && !checkAccess( _ACCESS_IMPLEMENTOR ) )
		{
			//we are not in a house at all...
			roomMgr->sendSystemMsg ( "Sorry", this, "That action is not allowed, the '%s' may only be dropped in your own home.", directObject->name );
		}
		else if( directObject->physicalState & _STATE_HOUSE_DECOR && room->building && strcasecmp ( getName(), room->building->_owner ) && !checkAccess( _ACCESS_IMPLEMENTOR ) )
		{
			//we are not in our house...
			roomMgr->sendSystemMsg ( "Sorry", this, "That action is not allowed, the '%s' may only be dropped in your own home.", directObject->name );
		}
		else result = character->drop ( directObject, &movie );
	}

	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	return retVal;
}

int RMPlayer::process_IPC_GET_QUEST_LIST ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		

	PackedMsg ack;
	ack.putACKInfo ( message->type() );

	ack.putWord ( quests? quests->size() : 0 );

	if ( quests ) {
		// put all the active quests in first
		LinkedElement *element = quests->head();

		while ( element ) {
			QuestInfo *info = (QuestInfo *)element->ptr();

			if ( !info->endTime ) {
				char str[1024];
				sprintf ( sizeof ( str ), str, "%s (active)", info->quest->name? info->quest->name : "Bugged Quest" );
				ack.putString ( str );
			}

			element = element->next();
		}

		element = quests->head();

		while ( element ) {
			QuestInfo *info = (QuestInfo *)element->ptr();

			if ( info->endTime ) {
				char str[1024];
				sprintf ( sizeof ( str ), str, "%s (completed)", info->quest->name? info->quest->name : "Bugged Quest" );
				ack.putString ( str );
			}

			element = element->next();
		}
	}

	roomMgr->sendTo ( _IPC_PLAYER_ACK, &ack, this );

	return retVal;
}

int RMPlayer::process_IPC_VERB_GIVE ( IPCMessage *message ) {
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	int questOwner = 0;

	PackedData movie;
	WorldObject *indirectObject, *directObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;
	int tossIt = 0;

	if ( directObject && indirectObject && directObject->getBase ( _BCONTAIN ) && 
		indirectObject->getBase ( _BCARRY ) && indirectObject->isOwnedBy ( character ) && 
		directObject->getRoom() == character->room ) 
	{
		if ( indirectObject->getBase (_BHEAD ) ) {
//			disable ( "character %s attempted to give head.", getName() ); 
			goto endGiveVerb;
		}

		///if( indirectObject->hasAffect( _AFF_STAFF ) ) {
		///	roomMgr->sendSystemMsg ( "Sorry", this, "The '%s' is a staff-protected item. You may not give it to another character.", indirectObject->name );
		///	goto endGiveVerb;
		///}

		if ( directObject->getBase ( _BCHARACTER ) && directObject->player && !directObject->player->isNPC && !directObject->player->autoGive ) {
			roomMgr->sendPlayerText ( this, "|c65|Info> %s is not accepting items.", directObject->getName() );
			goto endGiveVerb;
		}

		BContainer *bcontain = (BContainer *) directObject->getBase ( _BCONTAIN );
		int nResult = 0;

		if ( (nResult = bcontain->accepts( indirectObject )) ) {
			result = _WO_ACTION_HANDLED;
			roomMgr->sendPlayerText( this, "|c65|Info> %s found %d item%s that %s took.", directObject->getName(), nResult, nResult > 1 ? "s" : "", directObject->getPronoun( _PRONOUN_HE ) );
		} else {
			indirectObject->makeVisible ( 1 );

			BContainer *bcontain = (BContainer *)indirectObject->getBase ( _BCONTAIN );

			if ( bcontain )
				bcontain->makeVisible ( 1 );

			BTalk *btalk = (BTalk *)directObject->getBase ( _BTALK );
			int needsItem = (directObject->player && directObject->player->isNPC)? 0 : 1;

			if ( quests ) {
				if ( btalk ) {
					LinkedElement *element = quests->head();

					while ( element ) {
						QuestInfo *info = (QuestInfo *)element->ptr();
						Quest *quest = info->quest;

						if ( quest && ( btalk->talkTreeID == quest->id ) ) {
							QuestInfo *myQuest = findQuest ( quest->number );

							if ( myQuest && !myQuest->endTime && myQuest->quest->item ) {
								if ( !strcmp ( myQuest->quest->item->classID, indirectObject->super ) || !strcmp ( myQuest->quest->item->classID, indirectObject->classID ) ) {
									movie.putByte ( _MOVIE_QUEST_COMPLETE );
									movie.putLong ( directObject->servID );
									movie.putString ( myQuest->quest->completed );
									movie.putLong ( character->servID );
									needsItem = 1;
									tossIt = 1;

									myQuest->endTime = getseconds();	
									writeQuestData();

									if ( quest->reward && quest->reward->classID ) {

										WorldObject* reward = character->addObject( quest->reward->classID );
										reward->makeVisible( 1 );
									} else {
										roomMgr->sendSystemMsg ( "Quest Error", this, "You have encountered a quest with a missing reward object.  Please report this to the proper people." ); 
									}

									character->writeCharacterData();

									break;
								}
							}
						}

						element = element->next();
					}
				}
			}

			if ( needsItem ) {
				result = character->give ( indirectObject, directObject, &movie );

				//Save Character that is giving the item
				logDisplay("saving character %s", character->getName());
				character->writeCharacterData();

				//If the recieving character is an NPC, let's save them too
				if(!directObject->player->isNPC){
					logDisplay("saving character %s", directObject->getName());
					directObject->writeCharacterData();
				}
			} else {
				roomMgr->sendPlayerText ( this, "|c65|Info> %s has no use for that!", directObject->getName() );
			}
		}
	}

endGiveVerb:

	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	if ( tossIt )
		roomMgr->destroyObj ( indirectObject, 1, __FILE__, __LINE__ );

	return retVal;
}

int RMPlayer::process_IPC_VERB_USE ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	PackedData movie;
	WorldObject *indirectObject, *directObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;
	int tossIt = 0;

	if ( directObject && directObject->getRoom() == character->room )
	{
		WorldObject *owningObj = directObject->getBaseOwner();

		if ( owningObj && ( owningObj->player == NULL || owningObj->player == this ) ) {

			BUse *base = (BUse *)directObject->getBase ( _BUSE );
	
			if ( base ) {
				if ( base->uses ) {
	
					directObject->directObject = character;
					directObject->indirectObject = indirectObject;
	
					result = directObject->processActions ( vBeUsed, &movie );
	
					if ( result == _WO_ACTION_HANDLED ) {
						if ( base->uses > 0 ) {
							base->uses--;
	
							if ( base->uses == 0 ) {
								tossIt = 1;
	
								movie.putByte ( _MOVIE_TOSS_OBJECT );
								movie.putLong ( character->servID );
								movie.putLong ( directObject->servID );
							}
						}
					}
	
					directObject->directObject = NULL;
					directObject->indirectObject = NULL;
				} else {
					result = _ERR_TOO_EXPENSIVE;
				}
			} else {
				result = _ERR_BAD_SERVID;
			}
		}
	}

	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	if ( tossIt )
		roomMgr->destroyObj ( directObject, 0, __FILE__, __LINE__ );

	return retVal;
}

int RMPlayer::process_IPC_VERB_DYE ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	PackedData movie;
	WorldObject *indirectObject, *directObject;

	PackedData packet ( message->data(), message->size() );
	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;

	int didDye = 0;

	if ( directObject && indirectObject ) {

		// check the bases and owner(s) to make sure ok to dye

		WorldObject *owningObj = directObject->getBaseOwner();
		BDye *base = (BDye *)directObject->getBase ( _BDYE );
		BDye *dyeBase = (BDye *)indirectObject->getBase ( _BDYE );
		BWeapon *bweapon = (BWeapon *)directObject->getBase ( _BWEAPON );

		if ( !base && ( directObject->getBase ( _BWEAR ) || bweapon ) 
			&& dyeBase && !dyeBase->nHairDye &&
			!indirectObject->getBase ( _BWEAR ) &&
			character->owns ( indirectObject ) && 
			( directObject->getRoom() == indirectObject->getRoom() ) && 
			( owningObj && ( owningObj->player == NULL || owningObj->player == this ) ) )
		{
			directObject->color = indirectObject->color;

			movie.putByte ( _MOVIE_DYE );
			movie.putLong ( character->servID );
			movie.putLong ( directObject->servID );
			movie.putByte ( directObject->color );
			didDye = 1;

			result = _WO_ACTION_HANDLED;
		}
	}
	else if ( directObject && !indirectObject ) {
		BDye *base = (BDye *)directObject->getBase ( _BDYE );

		BHead* head = character->GetHead();

		if ( head && base && base->nHairDye ) {
			// eye color is really hair color.
			head->eyeColor = base->nHairDye - 1;

			movie.putByte ( _MOVIE_DYE );
			movie.putLong ( character->servID );
			movie.putLong ( character->servID );
			movie.putByte ( head->eyeColor );
			didDye = 2;

			result = _WO_ACTION_HANDLED;
		}
	}

	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	switch ( didDye ) {
		case 1:
			roomMgr->destroyObj ( indirectObject, 1, __FILE__, __LINE__ );
			break;
		case 2:
			roomMgr->destroyObj ( directObject, 1, __FILE__, __LINE__ );
			break;
	}

	return 1;
}

int RMPlayer::process_IPC_VERB_GET ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		
	PackedData movie;

	// objects to process
	WorldObject *indirectObject, *directObject, *pBackPack;

	// object owners
	WorldObject *owner = NULL;
	WorldObject *owningObj = NULL;

	BContainer *bContain = NULL;

	PackedData packet ( message->data(), message->size() );

	preVerbMessage ( &packet, &movie, &directObject, &indirectObject );

	int result = _WO_ACTION_PROHIBITED;

	BSit* bsit = static_cast< BSit*> (directObject->getBase( _BSIT ) );
	BOpenable* bopen = static_cast< BOpenable*> (directObject->getBase( _BOPEN ) );

	// validate object 
	if ( !directObject || !directObject->isWorldObject() ) {
		goto endGetVerb;
	}

	// validate object room
	if ( directObject->getRoom() != character->room ) {
		goto endGetVerb;
	}

	// validate combat cloud
	if ( character->combatGroup != directObject->combatGroup ) {  
		goto endGetVerb;
	}

	// validate bases - carryable, not head
	if ( !directObject->getBase ( _BCARRY ) || directObject->getBase ( _BHEAD ) ) {
		goto endGetVerb;
	}

	if( bsit && bsit->owner ) goto endGetVerb;

	//if( directObject->hasAffect( _AFF_STAFF ) && !checkAccess( _ACCESS_GUIDE | _ACCESS_PROPHET | _ACCESS_EVENT ) ) {
	//	roomMgr->sendSystemMsg ( "Sorry", this, "That action is not allowed, the '%s' is a staff-protected item.", directObject->name );
	//
	//	roomMgr->sendListText ( &roomMgr->_gms, "Info> %s(%s) has attempted to get a '%s'. Room: %d(%s)\n", this->getName(), this->getLogin(), directObject->name, room->number, room->title?room->title:room->zone->title );
	//	
	//	logHack( "%s(%s) attempted VERB_GET on '%s'. Room: %d(%s)", this->getName(), this->getLogin(), directObject->name, room->number, room->title?room->title:room->zone->title );
	//	
	//	goto endGetVerb;
	//}

	//see if this object can only be picked up in the owners home
	if( directObject->physicalState & _STATE_HOUSE_DECOR && room && room->building )
	{
		if ( strcasecmp ( getName(), room->building->_owner ) && !checkAccess( _ACCESS_IMPLEMENTOR ) ) {
			//we are in a house, but it's NOT ours
			roomMgr->sendSystemMsg ( "Sorry", this, "That action is not allowed, the '%s' can only be picked up in your own home.", directObject->name );
			goto endGetVerb;
		}
	}

	// get directObject root owner
	owningObj = directObject->getBaseOwner();

	// if owner is a player it must match
	if ( owningObj && owningObj->player && ( owningObj->player != this ) ) {
		goto endGetVerb;
	}

	// get directObject immediate owner
	owner = directObject->getOwner();

	// validate open container to take from
	if ( owner && ( owner != directObject ) ) {
		BOpenable *bOpen = (BOpenable *)owner->getBase ( _BOPEN );

		if ( bOpen && !bOpen->isOpen() )
			goto endGetVerb;
	}

	if( bopen && bopen->isOpen() ) {
		character->close( directObject, &movie );
	}

	bContain = (BContainer *) owner->getBase ( _BCONTAIN );

	result = character->take ( directObject, &movie );

	// clear the treasure bit if someone picks up an object
	directObject->physicalState &= ~_STATE_TREASURE;

	// put the item in a worn pack, if any...
	pBackPack = character->getWornOn ( _WEAR_PACK );

	if ( pBackPack && (owningObj != character) ) {
		character->putIn ( directObject, pBackPack, &movie );
	}

endGetVerb:
	if ( (result == _WO_ACTION_HANDLED) && owner && directObject && (owner != directObject) && (owner->physicalState & _STATE_TOSS) ) {
		if ( bContain && !bContain->contents.size() ) {
			movie.putByte ( _MOVIE_TOSS_OBJECT );
			movie.putLong ( character->servID );
			movie.putLong ( owner->servID );
		}
	}

	postVerbMessage ( result, message->type(), &movie, directObject, indirectObject );

	if ( (result == _WO_ACTION_HANDLED) && owner && directObject && (owner != directObject) && (owner->physicalState & _STATE_TOSS) ) 
		if ( bContain && !bContain->contents.size() )
			roomMgr->destroyObj ( owner, 0, __FILE__, __LINE__ );

	return retVal;
}

int RMPlayer::process_IPC_PLAYER_SET_BIOGRAPHY ( IPCMessage *message )
{
	int retVal = 1;

	PackedMsg packet ( message->data(), message->size() );

	// get the servID of the object in question
	long servID = packet.getLong();

	// get the actual object that the servID refers to
	WorldObject *obj = roomMgr->findObject ( servID );

	if ( !obj || !player ) {
	//	roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		return retVal;
	}

	BPlayer *base = (BPlayer *)player->getBase ( _BPLAYER );

	if ( !base || !base->ownsCharacter ( obj ) || player->player != this ) {
//		disable ( "invalid or misdirected set biography for %s", obj->getName() );
		return retVal;
	}

	char *text = NULL;
	text = packet.getString();

	obj->setBiography ( text, TRUE );

	obj->playerControlled = 1;
	obj->writeCharacterData();

	if ( text )
		free ( text );

	roomMgr->sendACK ( message->type(), this );

	return retVal;
}

int RMPlayer::process_IPC_PLAYER_GET_BIOGRAPHY ( IPCMessage *message )
{
	int retVal = 1;

	PackedMsg packet ( message->data(), message->size() );

	// prepare the response packet	
	PackedMsg response;
	response.putACKInfo ( message->type() );

	// get the servID of the object in question
	long servID = packet.getLong();

	// get the object to be described
	WorldObject *object = roomMgr->findObject ( servID );

	if ( object ) {
		if ( object->biography )
			response.putString ( object->biography );
		else
			response.putString ( " " );

		roomMgr->sendTo ( _IPC_PLAYER_ACK, &response, this );
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_PLAYER_GET_DESCRIPTION ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		
	PackedMsg packet ( message->data(), message->size() );

	// prepare the response packet
	PackedMsg response;

	// put the ACK type
	response.putACKInfo ( message->type() );

	// get the object to query
	WorldObject *object = roomMgr->findObject ( packet.getLong() );
	WorldObject *owner = object? object->getBaseOwner() : NULL;

	if ( object && (character->room == object->getRoom()) && ((owner == object) || (owner == character) || (owner->player == NULL)) ) {
		BDescribed *base = (BDescribed *)object->getBase ( _BDESCRIBED );

		if ( base ) {
			char str[10240] = "";

			if( base->isBook ) {
				if( base->text ) {
					response.putString( base->text );
				}
				else {
					response.putString( "This is a blank book." );
				}
			}
			else {
				if ( base->acceptText ) {
					sprintf( sizeof( str ), str, base->acceptText,
						gAcceptCounts[ base->acceptDisplay[0] ],
						gAcceptCounts[ base->acceptDisplay[1] ],
						gAcceptCounts[ base->acceptDisplay[2] ],
						gAcceptCounts[ base->acceptDisplay[3] ],
						gAcceptCounts[ base->acceptDisplay[4] ],
						gAcceptCounts[ base->acceptDisplay[5] ],
						gAcceptCounts[ base->acceptDisplay[6] ],
						gAcceptCounts[ base->acceptDisplay[7] ],
						gAcceptCounts[ base->acceptDisplay[8] ],
						gAcceptCounts[ base->acceptDisplay[9] ]
					);
				} else {
					if ( object->hasAffect ( _AFF_ENH_IDENTIFIED ) ) {
						describeObject ( character, object, strlen ( base->idText )? base->idText : base->text, str, sizeof ( str ) );

						// must have more than 1 affect ( identified )
   						if ( object->activeAffects && object->activeAffects->size() > 1 ) {
							strcat(str, "\n\nIdentified properties:\n\n" );
							strcat(str, object->describeAffects() );
						}
					} else { 
						if ( object->hasAffect ( _AFF_IDENTIFIED ) )
							describeObject ( character, object, strlen ( base->idText )? base->idText : base->text, str, sizeof ( str ) );
						else
							describeObject ( character, object, base->text, str, sizeof ( str ) );
					}
				}

				if ( checkAccess ( _ACCESS_MODERATOR ) ) {
					BCarryable *bcarry = (BCarryable *) object->getBase ( _BCARRY );

					if ( bcarry ) {
						char msg[ 256 ];

						if ( bcarry->sLastOwner ) {
							sprintf( sizeof( msg ), msg, "\n\nDropped by toon '%s'", bcarry->sLastOwner ); 
						} else {
							sprintf( sizeof( msg ), msg, "\n\nDropped by Game System" );
						}

						strcat( str, msg );
					}
				}

				if ( checkAccess ( _ACCESS_IMPLEMENTOR ) ) {
					char msg[256];

					sprintf( sizeof(msg), msg, "\n\nphysicalProperties = 0x%08x at (%d,%d)", object->physicalState, object->x, object->y );

					strcat( str, msg );
				}

				response.putString ( str );
			}
		} else {
			response.putString ( "You just looked at an object without a description!" );
		}
	} else {
		response.putString ( "You try to look at it, but you can't get a clear view of it now." );
	}

	roomMgr->sendTo ( _IPC_PLAYER_ACK, &response, this );

	return retVal;
}

int RMPlayer::process_IPC_PLAYER_SET_HEAD_DATA ( IPCMessage *message )
{
	int retVal = 1;

	//
	// If this RMPlayer object does not have a player object attached, then
	// it is not logged in.  If this is the case, ignore the message.
	//
	if ( !player ) {
		logHack ( "_IPC_PLAYER_SET_HEAD_DATA received from player with no player object" );
		return retVal;
	}

	//
	// Get the BPlayer base of the player object.  This contains all of the
	// owned character information for the attached account.  This information
	// will come in handy when we want to handle making sure that this message
	// was not redirected.
	//
	BPlayer *bplayer = (BPlayer *)player->getBase ( _BPLAYER );

	//
	// If there is no BPlayer base, ignore the message.
	//
	if ( !bplayer ) {
		logHack ( "_IPC_PLAYER_SET_HEAD_DATA received from player with no BPLAYER base" );
		return retVal;
	}

	// 
	// If this RMPlayer object has a character assigned then he must not
	// be at the character selection menu.  Naughty!  This message can only be
	// received by a RMPlayer object that has no character.
	//
	if ( character ) {
//		disable ( "_IPC_PLAYER_SET_HEAD_DATA received from logged in player. Character %s.", character->getName() );
		return retVal;
	}

	PackedMsg packet ( message->data(), message->size() );

	// get the object to modify
	int theServID = packet.getLong();
	WorldObject *object = roomMgr->findObject ( theServID );

	if ( object ) {
		// 
		// Now we can check for a redirection of the servID of this message.
		// We do this by asking the BPlayer base to see if it owns the passed
		// character object.  If it does not, we log it and return.
		//
		if ( !bplayer->ownsCharacter ( object ) ) {
			logHack ( "_IPC_PLAYER_SET_HEAD_DATA sent for unowned servID" );
			return retVal;
		}	
		
		//
		// Scan for the head of the object passed in.
		//
		BContainer *bcontain = (BContainer *)object->getBase ( _BCONTAIN );
		BHead *head = NULL;

		if ( bcontain ) {
			LinkedElement *element = bcontain->contents.head();

			while ( element ) {
				WorldObject *obj = (WorldObject *)element->ptr();

				head = (BHead *)obj->getBase ( _BHEAD );

				if ( head )
					break;

				element = element->next();
			}

			if ( head ) {
				// check the head to see if it is the default head, which it must be
				// to handle this message
				if ( head->headNumber || head->eyeNumber || head->hairNumber || head->browNumber || head->noseNumber || head->earNumber || head->mouthNumber || head->faceHairNumber || head->skinColor || head->eyeColor || head->hairColor ) {
					disable ( "SET_HEAD_DATA on non-default head" );
					return retVal;
				}

				head->fromPacket ( &packet );

				// handle invalid head
				if ( !head->valid() || head->eyeColor > 11 ) {  // The hair color is not a standard color.
					head->race = 0;
					head->headNumber = 0;
					head->eyeNumber = 0;
					head->hairNumber = 0;
					head->browNumber = 0;
					head->noseNumber = 0;
					head->earNumber = 0;
					head->mouthNumber = 0;
					head->faceHairNumber = 0;
					head->skinColor = 0;
					head->eyeColor = 0;
					head->hairColor = 0;
				}

				switch ( object->view ) {
					case 100: {
						head->sex = _SEX_MALE;
					}

					break;

					case 200: {
						head->sex = _SEX_FEMALE;
					}

					break;
				}

				BCharacter *bchar = (BCharacter *)object->getBase ( _BCHARACTER );

				if ( bchar ) {
					bchar->sex = head->sex;
					bchar->race = head->race;

					if ( !checkAccess ( _ACCESS_GUIDE ) ) {
						object->validScale();
					}

					object->playerControlled = 1;
					object->writeCharacterData();

					roomMgr->sendACK ( _IPC_PLAYER_SET_HEAD_DATA, this );
				}
			}
		}
	}

	return retVal;
}

int RMPlayer::process_IPC_PLAYER_GET_EXTENDED_PROPS ( IPCMessage *message )
{
	int retVal = 1;

//  cannot be tested against this character's player, may not exist yet

	PackedMsg packet ( message->data(), message->size() );

	// get the object to query
	int nValue = packet.getLong();

//	WorldObject *object = roomMgr->findObject ( packet.getLong() );
	WorldObject *object = roomMgr->findObject ( nValue );

	if ( object ) {

		int isMine = 0;
		
		if ( player ) {

			BPlayer *base = (BPlayer *)player->getBase ( _BPLAYER );

			if ( base && base->ownsCharacter ( object ) && player->player == this ) 
				isMine = 1;
		}

		PackedMsg response;

		// put ACK info
		response.putACKInfo ( message->type() );

		object->buildExtendedPacket ( &response, isMine );
		
		roomMgr->sendTo ( _IPC_PLAYER_ACK, &response, this );
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_PLAYER_QUERY_CHARACTERS ( IPCMessage *message )
{
	int retVal = 1;

	if ( !player ) 
		return retVal;

	BPlayer *base = (BPlayer *)player->getBase ( _BPLAYER );

	if ( base ) {
		PackedMsg packet;

		base->buildCharacterInfoPacket ( &packet );

		roomMgr->sendTo ( _IPC_PLAYER_QUERY_CHARACTERS, &packet, this );
	} else {
		logInfo ( _LOG_DETAILED, "(%d): NAK (currently logged in object is not BPlayer)", player->servID );

		roomMgr->sendNAK ( _IPC_PLAYER_QUERY_CHARACTERS, _ERR_BAD_SERVID, this );
	}	

	if ( gShutdownTimer > -1 ) {

		unsigned int timeHours = gShutdownTimer / 3600;
		unsigned int timeMinutes = ( gShutdownTimer - ( timeHours * 3600) ) / 60;
		unsigned int timeSeconds = gShutdownTimer - ( timeHours * 3600) - ( timeMinutes * 60);

		char timeText[40] = {0};
		char scratch[40] = {0};

		if( timeHours ) {
			sprintf( 40, scratch, "%d hours", timeHours );
			strcat( timeText, scratch );
		}
		if( timeMinutes ) {
			sprintf( 40, scratch, "%d minutes", timeMinutes );
			if( timeHours ) strcat( timeText, ", " );
			strcat( timeText, scratch );
		}
		if( timeSeconds || ( !timeHours && !timeMinutes ) ) {
			sprintf( 40, scratch, "%d seconds", timeSeconds );
			if( timeHours || timeMinutes ) strcat( timeText, ", " );
			strcat( timeText, scratch );
		}

		if ( gShutdownMessage[0] ) {
			roomMgr->sendSystemMsg ( "Shutdown Message", this, "The game will be shutting down in %s.\n%s", timeText, gShutdownMessage );
		} else {
			roomMgr->sendSystemMsg ( "Shutdown Message", this, "The game will be shutting down in %s.", timeText );
		}
	}

	return retVal;
}

int RMPlayer::process_IPC_PLAYER_DESTROY_CHARACTER ( IPCMessage *message )
{
	int retVal = 1;

	if ( !player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	BPlayer *base = (BPlayer *)player->getBase ( _BPLAYER );

	if ( base ) {
		long number = packet.getLong();

		WorldObject *object = roomMgr->findObject ( number );

		if ( object && base->ownsCharacter ( object ) && player->player == this ) {

			long value = object->value + (object->manaValue * 5);
			int time = getseconds() - object->creationTime;

			if ( time < 10 ) {

				time = 10 - time;

				if ( time ) 
					roomMgr->sendSystemMsg ( "Please Wait", this, "You can not erase that character for another %d %s. This prevents the use of macros to rapidly create/destroy characters in an attempt to impact game performance.", time, (time != 1)? "seconds" : "second" ); 
				
				roomMgr->sendNAK ( _IPC_PLAYER_DESTROY_CHARACTER, _ERR_TOO_EXPENSIVE, this );
				return 1;
			}

			if ( value < 1000 && (time < (86400 * 3)) ) {
				time = (86400 * 3) - time;	

				if ( time ) {
					int upDays = time / 86400;
					time -= upDays * 86400;
					int upHours = time / 3600;
					time -= upHours * 3600;
					int upMinutes = time / 60;
					time -= upMinutes * 60;

					roomMgr->sendSystemMsg ( "You Have To Wait", this, "You can not erase that character for another %d %s, %d %s, %d %s and %d %s. This is because you have spent or dropped some of your starting wealth (gold or mana crystals).  If you can recover your starting wealth, you will be able to erase that character.  We do this to assure that players do not give all their starting wealth away to other players to get rich.", upDays, (upDays != 1)? "days" : "day", upHours, (upHours != 1)? "hours" : "hour", upMinutes, (upMinutes != 1)? "minutes" : "minute", time, (time != 1)? "seconds" : "second" );
				}
				roomMgr->sendNAK ( _IPC_PLAYER_DESTROY_CHARACTER, _ERR_TOO_EXPENSIVE, this );
				return 1;
			}

			// don't destroy objects that are in a room 
			if ( object->room ) {
				logHack ( "%s tried to destroy logged in character %s", base->login, object->getName() );
				return 1;
			}

			gDataMgr->logPermanent ( getLogin(), object->getName(), _DMGR_PLOG_ERASECHAR, "%s", object->getName() );

			char *theName = strdup ( object->getName() );
			strlower ( theName );

			//trash the house - log out anyone inside
			Building* house = findHouse( theName );
			if( house ) {

				LinkedElement* roomElement = house->rooms.head();

				while( roomElement ) {
					RMRoom* room = static_cast< RMRoom*>( roomElement->ptr() );
					roomElement = roomElement->next();

					LinkedElement* playerElement = room->head();

					while( playerElement ) {
						RMPlayer* player = static_cast< RMPlayer*>( playerElement->ptr() );
						playerElement = playerElement->next();

						if( player ) {
							roomMgr->sendSystemMsg( "Info", player, "The character of the house you are currently in has been deleted and your connection has been closed. Please log in again to continue play." );
							player->forceLogout();
						}
					}

					//we don't want to delete the players!
					room->release();
				}
			
				house->players.release();
				delete house;
			}

			gDataMgr->delCharacter ( object );

			free ( theName );	  

			base->delCharacter ( object );

			object->player = NULL;
			roomMgr->destroyObj ( object, 1, __FILE__, __LINE__ ); 

			roomMgr->sendACK ( _IPC_PLAYER_DESTROY_CHARACTER, this );
		} else {
			roomMgr->sendNAK ( _IPC_PLAYER_DESTROY_CHARACTER, _ERR_BAD_SERVID, this );
		}
	} else { 
		logInfo ( _LOG_DETAILED, "(%d): NAK (currently logged in object is not BPlayer)", player->servID );
		roomMgr->sendNAK ( _IPC_PLAYER_DESTROY_CHARACTER, _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_UPDATE_ATTRIBUTES ( IPCMessage *message )
{
	int retVal = 1;

	// only have object at this point in some cases. No test for character->player

	PackedMsg packet ( message->data(), message->size() );

	WorldObject *obj = roomMgr->findObject ( packet.getLong() );

	if ( !obj || !player || !character ) {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		return retVal;
	}

	BPlayer *base = (BPlayer *)player->getBase ( _BPLAYER );

	if ( !base ) {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		return retVal;
	}

	if ( !base->ownsCharacter ( obj ) || player->player != this ) {
		return retVal;
	}

	BCharacter *bcharacter = (BCharacter *)obj->getBase ( _BCHARACTER );
	if( reinterpret_cast< int>( character ) == 0x21 ) { logInfo( _LOG_ALWAYS, "%s:%d - BCharacter value corrupted", __FILE__, __LINE__ ); }

	if ( !bcharacter ) {
		return retVal;
	}

	int updateSkills = packet.getByte();

	obj->fromExtendedPacket ( &packet );

	// check to make sure total attribute assignments dont exceed 48 
	int diff = 0;
	diff += obj->strength;
	diff += obj->dexterity;
	diff += obj->intelligence;
	diff += obj->endurance;

	if ( !isNPC && ( diff > 48 ) || ( diff < 40 ) || ( obj->strength < 3 ) || ( obj->dexterity < 6 ) || ( obj->intelligence < 2 ) || ( obj->endurance < 5 ) || (obj->strength > 26 ) || ( obj->dexterity > 25 ) || ( obj->intelligence > 25 ) || ( obj->endurance > 23 ) ) {
		obj->strength = 1;
		obj->dexterity = 1;
		obj->intelligence = 1;
		obj->endurance = 1;
		return retVal;
	}

//	huh?
	obj->delAffect ( _AFF_CONVERTED, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, NULL );

	if ( !obj->hasAffect ( _AFF_RESET_A, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT ) )
		obj->addAffect ( _AFF_RESET_A, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, -1, 0, NULL );

	if ( updateSkills ) {
		switch ( character->profession() ) {
			case _PROF_WARRIOR: {
				bcharacter->skills[_SKILL_SHORT_SWORD] = 1;
				bcharacter->skills[_SKILL_LONG_SWORD] = 2;
				bcharacter->skills[_SKILL_TWOHANDED_SWORD] = 2;
				bcharacter->skills[_SKILL_DAGGER] = 1;
				bcharacter->skills[_SKILL_AXE] = 1;
				bcharacter->skills[_SKILL_CLUB] = 1;
				bcharacter->skills[_SKILL_MACE] = 1;
				bcharacter->skills[_SKILL_UNARMED] = 1;
				bcharacter->skills[_SKILL_THROWING] = 1;
				bcharacter->skills[_SKILL_MAUL] = 1;
				bcharacter->skills[_SKILL_BROADSWORD] = 1;
				bcharacter->skills[_SKILL_SHIELD_USE] = 2;
				bcharacter->skills[_SKILL_ACROBATICS] = 1;
			}
			break;
	
   			case _PROF_WIZARD: {
   				switch ( obj->coarseAlignment() ) {
   					case _ALIGN_GOOD: 
   						bcharacter->skills[_SKILL_THAUMATURGY] = 2;
   						bcharacter->skills[_SKILL_SORCERY] = 1;
   						bcharacter->spells[_SPELL_HEAL] = 1;
   						bcharacter->spells[_SPELL_LIGHT_DART] = 1;
   						bcharacter->spells[_SPELL_HOME] = 1;
   						break;
   
   					case _ALIGN_NEUTRAL:
   						bcharacter->skills[_SKILL_ELEMENTALISM] = 2;
   						bcharacter->skills[_SKILL_SORCERY] = 1;
   						bcharacter->spells[_SPELL_FLAME_ORB] = 1;
   						bcharacter->spells[_SPELL_IMMOLATION] = 1;
   						bcharacter->spells[_SPELL_HOME] = 1;
   						break;
   
   					case _ALIGN_EVIL:
   						bcharacter->skills[_SKILL_NECROMANCY] = 2;
   						bcharacter->skills[_SKILL_MYSTICISM] = 1;
   						bcharacter->spells[_SPELL_HOLD_MONSTER] = 1;
   						bcharacter->spells[_SPELL_ACID_SPHERE] = 1;
   						bcharacter->spells[_SPELL_SUMMON_ZOMBIE] = 1;
   						break;
   				}
   
   				bcharacter->skills[_SKILL_ACROBATICS] = 1;
   				bcharacter->skills[_SKILL_SHORT_SWORD] = 1;
				bcharacter->skills[_SKILL_THROWING] = 2;
   				bcharacter->skills[_SKILL_THEURGISM] = 1;
   			}
   
   			break;
   
   			case _PROF_ADVENTURER: {
   				bcharacter->skills[_SKILL_LONG_SWORD] = 2;
   				bcharacter->skills[_SKILL_ACROBATICS] = 1;
				bcharacter->skills[_SKILL_TWOHANDED_SWORD] = 1;
   				bcharacter->skills[_SKILL_CRITICAL_STRIKING] = 1;
   				bcharacter->skills[_SKILL_SHIELD_USE] = 2;
   				bcharacter->skills[_SKILL_THEURGISM] = 1;
   				bcharacter->skills[_SKILL_SORCERY] = 1;
   				bcharacter->spells[_SPELL_HOME] = 1;
   				bcharacter->spells[_SPELL_KILL_STAR] = 1;
   			}
   
   			break;
   
   			case _PROF_THIEF: {
   				bcharacter->skills[_SKILL_THROWING] = 2;
   				bcharacter->skills[_SKILL_DAGGER] = 1;
   				bcharacter->skills[_SKILL_ACROBATICS] = 2;
   				bcharacter->skills[_SKILL_DAGGER] = 1;
   				bcharacter->skills[_SKILL_PICK_POCKETS] = 1;
   				bcharacter->skills[_SKILL_DETECT_TRAPS] = 1;
   				bcharacter->skills[_SKILL_PICK_LOCKS] = 1;
   				bcharacter->skills[_SKILL_CRITICAL_STRIKING] = 2;
   				bcharacter->skills[_SKILL_SHIELD_USE] = 1;
   			}
   
   			break;
   		}
   	}
	
	bcharacter->setWearMask();
				
	obj->calcHealth();
	obj->health = obj->healthMax;
	obj->hunger = _MAX_HUNGER;
	obj->thirst = _MAX_THIRST;
	bcharacter->peaceful = 1;
	obj->writeCharacterData();

	roomMgr->sendACK ( message->type(), this );

	return retVal;
}

// Racial Passives

int RMPlayer::CheckRace(WorldObject *object, int theRace)
{
	// Logs
    //logDisplay("Checking value of theRace  %d", theRace);
    
    if ( theRace == _RACE_GIANT ) {
        object->addAffect ( _AFF_ENCUMBERANCE_BLESSING, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, -1, 0, NULL );
        //logDisplay ( "Giant created, applied 'Encumbrance Blessing' passive!" );

    } else if ( theRace == _RACE_ELF ) {
        object->addAffect ( _AFF_QUICKEN, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, -1, 0, NULL );
        //logDisplay ( "Elf created, applied 'Quicken' passive!" );

    } else if ( theRace == _RACE_HUMAN ) {
        object->addAffect ( _AFF_EXTRA_ATTACK, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, -1, 0, NULL );
        //logDisplay ( "Human created, applied 'Extra Attack' passive!" );

    } else return 0;

    //delete( theCharacter ); not sure yet
    
    return 1;

}

// Class Passives

int RMPlayer::CheckClass(WorldObject *object, int theClass)
{
	//logDisplay("Checking value of theClass  %d", theClass);
    
    if ( theClass == _PROF_WARRIOR ) {
        object->addAffect ( _AFF_EMPOWER, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, -1, 0, NULL );
        //logDisplay ( "Warrior created, applied 'Empower' passive!" );

    } else if ( theClass == _PROF_ADVENTURER ) {
        object->addAffect ( _AFF_EXTENSION, _AFF_TYPE_NORMAL, _AFF_SOURCE_ARTIFACT, -1, 0, NULL );
        //logDisplay ( "Adventurer created, applied 'Extension' passive!" );

    } else if ( theClass == _PROF_THIEF ) {
        object->addAffect ( _AFF_EXTRA_DODGE, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, -1, 0, NULL );
        //logDisplay ( "Thief created, applied 'Extra Dodge' passive!" );

	} else if ( theClass == _PROF_WIZARD ) {
        object->addAffect ( _AFF_SEE_INVISIBLE, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, -1, 0, NULL );
        //logDisplay ( "Wizard created, applied 'See Invis' passive!" );

    } else return 0;

    
    return 1;

}


int RMPlayer::process_IPC_PLAYER_CREATE_CHARACTER ( IPCMessage *message )
{
	int retVal = 1;

	BPlayer *base = NULL;

	if ( player )
		base = (BPlayer *)player->getBase ( _BPLAYER );

	if ( base ) {
		// check for too many characters already
		if ( base->charNames.size() >= 4 || base->characterList.size() >= 4 ) {
			roomMgr->sendNAK ( _IPC_PLAYER_CREATE_CHARACTER, _ERR_BAD_SERVID, this );	
			return retVal;
		}

		PackedMsg packet ( message->data(), message->size() );

		int len = 0;
		int badTitle = 0;
		char *ptr = " ";
		char *referenceName;

		char *properName = packet.getString();

		// checks for valid character name and title

  		if ( !isValidName ( properName ) ) {
			if ( properName )
				free ( properName );

			return retVal;
  		} else {
  			referenceName = strdup ( properName );
  			strlower ( referenceName );
  		}

		char *title = packet.getString();

		if ( title ) {
			len = strlen ( title );

			if (len < 1 || len > 30 ) {
				return retVal;
			}

			ptr = title;
	
			while ( *ptr ) {
				if ( !isalnum ( *ptr ) && !isspace ( *ptr ) || *ptr == '|' ) {
					badTitle = 1;
					break;
				}
				ptr++;
			}

		} else {
			badTitle = 1;
			return retVal;
		}

		if ( badTitle ) {
			free ( properName );
			free ( referenceName );

			if ( title )
				free ( title );

			return retVal;
		}

		int profession = packet.getByte();
		int race = packet.getByte();
		int sex = packet.getByte();
		int peaceful = packet.getByte();

		int alreadyTaken = 0;

		if ( strchr ( referenceName, '1' ) )
			alreadyTaken = 1;

		if ( alreadyTaken ) {
			roomMgr->sendNAK ( _IPC_PLAYER_CREATE_CHARACTER, _ERR_REDUNDANT, this );	
		} else {
			if ( profession < 0 || profession > 3 ) {
				if ( referenceName )
					free ( referenceName );

				if ( properName )
					free ( properName );

				if ( title )
					free ( title );

				return retVal;
			}				

			WorldObject *object = makeTemplateCharacter ( profession, race, sex, properName, base->password, 0 );

			if ( object ) {
				object->ownerName = strdup ( base->login );
				object->isVisible = 1;
				object->player = this;

				BCharacter *character = (BCharacter *)object->getBase ( _BCHARACTER );
				if( reinterpret_cast< int>( character ) == 0x21 ) { logInfo( _LOG_ALWAYS, "%s:%d - BCharacter value corrupted", __FILE__, __LINE__ ); }

				// set the character's properties
				strcpy ( character->properName, properName );
				strcpy ( character->title, title );
	
				character->peaceful = peaceful;

				object->fromExtendedPacket ( &packet );

				// check to make sure total attribute assignments dont exceed 48 
				//int diff = object->strength + object->dexterity + object->intelligence + object->endurance;

				//if ( !isNPC && diff > 48 ) {
				//	gDataMgr->logPermanent ( base->login, properName, _DMGR_PLOG_IMPCMD, "had hacked stats of S:%d D:%d I:%d E:%d", object->strength, object->dexterity, object->intelligence, object->endurance );
				//	logHack( "1) Account %s - character %s had hacked stats of S:%d D:%d I:%d E:%d", base->login, properName, object->strength, object->dexterity, object->intelligence, object->endurance );
				//
				//	object->strength = 1;
				//	object->dexterity = 1;
				//	object->intelligence = 1;
				//	object->endurance = 1;
				//
				//	diff = 4;
				//}

				character->buildPoints = 3;	

				switch ( character->profession ) {
					case _PROF_WARRIOR: {					
						character->skills[_SKILL_CLUB] = 1;
						character->spells[_SPELL_HOME] = 1;
						//character->skills[_SKILL_SHIELD_USE] = 1;
						//character->skills[_SKILL_ACROBATICS] = 1;
						//character->skills[_SKILL_SHORT_SWORD] = 1;
						//character->skills[_SKILL_LONG_SWORD] = 1;
						//character->skills[_SKILL_TWOHANDED_SWORD] = 1;
						//character->skills[_SKILL_DAGGER] = 1;
						//character->skills[_SKILL_AXE] = 1;
						//character->skills[_SKILL_CLUB] = 2;
						//character->skills[_SKILL_MACE] = 1;
						//character->skills[_SKILL_UNARMED] = 1;
						//character->skills[_SKILL_THROWING] = 1;
						//character->skills[_SKILL_MAUL] = 1;
						//character->skills[_SKILL_BROADSWORD] = 1;

						//if ( diff != 4 ) {
						//	if ( ( object->strength		< minStatValues[ _PROF_WARRIOR ][ race ][ _STAT_STRENGTH ] ) ||
						//		( object->dexterity		< minStatValues[ _PROF_WARRIOR ][ race ][ _STAT_DEXTERITY ] ) ||
						//		( object->intelligence	< minStatValues[ _PROF_WARRIOR ][ race ][ _STAT_INTELLIGENCE ] ) ||
						//		( object->endurance		< minStatValues[ _PROF_WARRIOR ][ race ][ _STAT_ENDURANCE ] ) ) {
						//			gDataMgr->logPermanent ( base->login, properName, _DMGR_PLOG_IMPCMD, "had hacked stats of S:%d D:%d I:%d E:%d", object->strength, object->dexterity, object->intelligence, object->endurance );
						//			logHack( "2) Account %s - character %s had hacked stats of S:%d D:%d I:%d E:%d", base->login, properName, object->strength, object->dexterity, object->intelligence, object->endurance );
						//
						//			object->strength = 1;
						//			object->dexterity = 1;
						//			object->intelligence = 1;
						//			object->endurance = 1;
						//	}
						//}
					}

					break;

					case _PROF_WIZARD: {
						switch ( object->coarseAlignment() ) {
							case _ALIGN_GOOD: 
								character->skills[_SKILL_THAUMATURGY] = 2;
								character->skills[_SKILL_SORCERY] = 1;
								character->spells[_SPELL_HEAL] = 1;
								character->spells[_SPELL_LIGHT_DART] = 1;
								break;

							case _ALIGN_NEUTRAL:
								character->skills[_SKILL_ELEMENTALISM] = 2;
								character->skills[_SKILL_SORCERY] = 1;
								character->spells[_SPELL_FLAME_ORB] = 1;
								character->spells[_SPELL_IMMOLATION] = 1;
								break;

							case _ALIGN_EVIL:
								character->skills[_SKILL_NECROMANCY] = 2;
								character->skills[_SKILL_MYSTICISM] = 1;
								character->spells[_SPELL_HOLD_MONSTER] = 1;
								character->spells[_SPELL_ACID_SPHERE] = 1;
								character->spells[_SPELL_SUMMON_ZOMBIE] = 1;
								break;
						}

						
						character->skills[_SKILL_THROWING] = 1;						
						character->skills[_SKILL_SHIELD_USE] = 1;
						character->spells[_SPELL_HOME] = 1;
						//character->skills[_SKILL_THROWING] = 2;
   						//character->skills[_SKILL_THEURGISM] = 1;
						//character->skills[_SKILL_ACROBATICS] = 1;

						//if ( diff != 4 ) {
						//	if ( ( object->strength		< minStatValues[ _PROF_WIZARD ][ race ][ _STAT_STRENGTH ] ) ||
						//		( object->dexterity		< minStatValues[ _PROF_WIZARD ][ race ][ _STAT_DEXTERITY ] ) ||
						//		( object->intelligence	< minStatValues[ _PROF_WIZARD ][ race ][ _STAT_INTELLIGENCE ] ) ||
						//		( object->endurance		< minStatValues[ _PROF_WIZARD ][ race ][ _STAT_ENDURANCE ] ) ) {
						//			gDataMgr->logPermanent ( base->login, properName, _DMGR_PLOG_IMPCMD, "had hacked stats of S:%d D:%d I:%d E:%d", object->strength, object->dexterity, object->intelligence, object->endurance );
						//			logHack( "3) Account %s - character %s had hacked stats of S:%d D:%d I:%d E:%d", base->login, properName, object->strength, object->dexterity, object->intelligence, object->endurance );
						//
						//			object->strength = 1;
						//			object->dexterity = 1;
						//			object->intelligence = 1;
						//			object->endurance = 1;
						//	}
						//}
					}

					break;

					case _PROF_ADVENTURER: {
						character->skills[_SKILL_LONG_SWORD] = 1; 						 						
						character->skills[_SKILL_SHIELD_USE] = 1;  						
   						character->skills[_SKILL_SORCERY] = 1;
   						character->spells[_SPELL_HOME] = 1;
   						character->spells[_SPELL_KILL_STAR] = 1;
						//character->skills[_SKILL_SHIELD_USE] = 2;
						//character->skills[_SKILL_THEURGISM] = 1;
						//character->skills[_SKILL_ACROBATICS] = 1;
						//character->skills[_SKILL_LONG_SWORD] = 2;  						
						//character->skills[_SKILL_TWOHANDED_SWORD] = 1;
   						//character->skills[_SKILL_CRITICAL_STRIKING] = 1; 

						//if ( diff != 4 ) {
						//	if ( ( object->strength		< minStatValues[ _PROF_ADVENTURER ][ race ][ _STAT_STRENGTH ] ) ||
						//		( object->dexterity		< minStatValues[ _PROF_ADVENTURER ][ race ][ _STAT_DEXTERITY ] ) ||
						//		( object->intelligence	< minStatValues[ _PROF_ADVENTURER ][ race ][ _STAT_INTELLIGENCE ] ) ||
						//		( object->endurance		< minStatValues[ _PROF_ADVENTURER ][ race ][ _STAT_ENDURANCE ] ) ) {
						//			gDataMgr->logPermanent ( base->login, properName, _DMGR_PLOG_IMPCMD, "had hacked stats of S:%d D:%d I:%d E:%d", object->strength, object->dexterity, object->intelligence, object->endurance );
						//			logHack( "4) Account %s - character %s had hacked stats of S:%d D:%d I:%d E:%d", base->login, properName, object->strength, object->dexterity, object->intelligence, object->endurance );
						//
						//			object->strength = 1;
						//			object->dexterity = 1;
						//			object->intelligence = 1;
						//			object->endurance = 1;
						//	}
						//}
					}

					break;

					case _PROF_THIEF: {
						character->skills[_SKILL_THROWING] = 1;			   						
   						character->skills[_SKILL_PICK_POCKETS] = 1;
   						character->skills[_SKILL_DETECT_TRAPS] = 1;
   						character->skills[_SKILL_PICK_LOCKS] = 1;
   						character->skills[_SKILL_SHIELD_USE] = 1;
						character->spells[_SPELL_HOME] = 1;
						//character->skills[_SKILL_THROWING] = 2;
						//character->skills[_SKILL_CRITICAL_STRIKING] = 2;
						//character->skills[_SKILL_DAGGER] = 1;
   						//character->skills[_SKILL_ACROBATICS] = 2;

						//if ( diff != 4 ) {
						//	if ( ( object->strength		< minStatValues[ _PROF_THIEF ][ race ][ _STAT_STRENGTH ] ) ||
						//		( object->dexterity		< minStatValues[ _PROF_THIEF ][ race ][ _STAT_DEXTERITY ] ) ||
						//		( object->intelligence	< minStatValues[ _PROF_THIEF ][ race ][ _STAT_INTELLIGENCE ] ) ||
						//		( object->endurance		< minStatValues[ _PROF_THIEF ][ race ][ _STAT_ENDURANCE ] ) ) {
						//			gDataMgr->logPermanent ( base->login, properName, _DMGR_PLOG_IMPCMD, "had hacked stats of S:%d D:%d I:%d E:%d", object->strength, object->dexterity, object->intelligence, object->endurance );
						//			logHack( "5) Account %s - character %s had hacked stats of S:%d D:%d I:%d E:%d", base->login, properName, object->strength, object->dexterity, object->intelligence, object->endurance );
						//
						//			object->strength = 1;
						//			object->dexterity = 1;
						//			object->intelligence = 1;
						//			object->endurance = 1;
						//	}
						//}
					}

					break;
				}


				//common bonuses for staff characters
				if(	checkAccess( _ACCESS_IMPLEMENTOR ) || checkAccess( _ACCESS_PUBLICRELATIONS ) ||
					checkAccess( _ACCESS_MODERATOR ) ||	checkAccess( _ACCESS_GUIDE ) || checkAccess( _ACCESS_EVENT ) )
				{

					object->addAffect ( _AFF_SEE_INVISIBLE, _AFF_TYPE_NORMAL, _AFF_SOURCE_ARTIFACT, -1, 0, NULL );
					object->addObject( "rEternalNourishment", 1 );
					object->addObject( "bCarrying", 1 );

					character->skills[ _SKILL_SORCERY ] = 5;
					character->skills[ _SKILL_THAUMATURGY ] = 5;
					character->skills[ _SKILL_MEDITATION ] = 5;

					character->spells[ _SPELL_HOME ] = 1;
					character->spells[ _SPELL_ENGRAVE ] = 1;
					character->spells[ _SPELL_DISPEL_MAGIC ] = 1;
					character->spells[ _SPELL_CORNUCOPIA ] = 1;
					character->spells[ _SPELL_TELEPORT ] = 1;
					character->spells[ _SPELL_EXTENSION ] = 1;
					character->spells[ _SPELL_SEE_INVISIBILITY ] = 1;
					character->spells[ _SPELL_IMPROVED_INVISIBILITY ] = 1;

					character->spells[ _SPELL_HEAL ] = 1;
					character->spells[ _SPELL_GREATER_HEAL ] = 1;
					character->spells[ _SPELL_REMOVE_CURSE ] = 1;
					character->spells[ _SPELL_CURE_POISON ] = 1;
					character->spells[ _SPELL_INVULNERABILITY ] = 1;
				}

				if ( checkAccess( _ACCESS_IMPLEMENTOR ) ) {
					object->addObject ( "ImplementorBaldric", 1 );
					object->addObject ( "RareCrown", 1 );

				} else if ( checkAccess( _ACCESS_PUBLICRELATIONS ) ) {
					object->addObject( "CRBaldric", 1 );

				} else if ( checkAccess( _ACCESS_MODERATOR ) ) {
					object->addObject( "SentinelBaldric", 1 );
					object->addObject( "SentinelHelm", 1 );
					//sentinels get level 1000
					character->gainExperience( 999 * 10000 );

				} else if ( checkAccess( _ACCESS_GUIDE ) ) {
					object->addObject( "MentorBaldric", 1 );
					object->addObject( "MentorHelmet", 1 );

					//mentors get level 500
					character->gainExperience( 499 * 10000 );

				} else if ( checkAccess( _ACCESS_EVENT ) ) {
					object->addObject( "EventsBaldric", 1 );

					//event staff get level 1000
					character->gainExperience( 999 * 10000 );
				}

				
				character->setWearMask();

				object->addAffect ( _AFF_RESET, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, -1, 0, NULL );
				object->addAffect ( _AFF_RESET_A, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT, -1, 0, NULL );

				object->calcHealth();
				object->health = object->healthMax;

				base->addCharacter ( object );
				base->addCharName ( properName );

				// write the character data to the database
				object->playerControlled = 1;
				object->writeCharacterData();
				object->playerControlled = 0;
				// apply racial and profession passives
                CheckRace( object, race );
				CheckClass( object, profession );
            } else {
				logInfo ( _LOG_DETAILED, "(%ld): NAK (could not create object)", player->servID );
				roomMgr->sendNAK ( _IPC_PLAYER_CREATE_CHARACTER, _ERR_SERVICE_NOT_AVAILABLE, this );
			}
		}

		if ( referenceName )
			free ( referenceName );

		if ( properName )
			free ( properName );

		if ( title )
			free ( title );
	} else {
		logHack ( "_IPC_PLAYER_CREATE_CHARACTER: Currently logged in object is not BPlayer)" );
		forceLogout();
	}
	
	return retVal;
}

int RMPlayer::process_IPC_PLAYER_CHAT ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	BPlayer *bplayer = (BPlayer *)player->getBase ( _BPLAYER );

	int len = message->size();

	if ( !checkAccess( _ACCESS_IMPLEMENTOR ) && ( len < 1 || len > 320 ) ) {
		return retVal;
	}

	IPCPMChatMsg *msg = (IPCPMChatMsg *)message->data();

	IPCPMMessage *response = (IPCPMMessage *)malloc ( sizeof ( IPCPMMessage ) + message->size() - sizeof ( IPCPMChatMsg ) );
	memcpy ( (char *)response + sizeof ( IPCPMMessage ), message->data() + sizeof ( IPCPMChatMsg ), message->size() - sizeof ( IPCPMChatMsg ) );

	response->to = msg->servID;
	roomMgr->sendTo ( _IPC_PLAYER_CHAT, response, message->size(), msg->servID );

	response->to = servID;
	roomMgr->sendTo ( _IPC_PLAYER_CHAT, response, message->size(), this );

	free ( response );

	return retVal;
}

int RMPlayer::process_IPC_PLAYER_SYSTEM_MSG ( IPCMessage *message )
{
	int retVal = 1;

	roomMgr->sendToAll ( _IPC_PLAYER_SYSTEM_MSG, (IPCPMMessage *)message->data(), message->size() );

	return retVal;
}

int RMPlayer::process_IPC_PLAYER_ROOM_CHAT ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		
	PackedMsg packet ( message->data(), message->size() );
	// extract the chat text
	char *text = packet.getString ( -1 );

	// check for control characters
	if ( text ) {
		int len = strlen ( text );

		if ( !checkAccess( _ACCESS_IMPLEMENTOR | _ACCESS_EVENT ) && ( len < 1 || len > 320 ) ) {
			free ( text );
			return retVal;
		}

		char *ptr = text;

		while ( *ptr ) {
			unsigned char ch = *ptr;

			if ( iscntrl ( ch ) && (ch < 128 || ch > 138) ) { 
				*ptr = '.';
			}
			ptr++;
		}

		// process and distribute the chat message
		processRoomChat ( text );

		// toss the chat text
		free ( text );
	}

	return retVal;
}

int RMPlayer::process_IPC_PLAYER_MOVIE ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	int objServID = packet.getLong();
	int objRoomNum = packet.getLong();

	if ( character->room && (character->room->number != objRoomNum) ) {
		logInfo ( _LOG_ALWAYS, "Potentially Invalid movie info from %s, (objServID == %d, character->servID = %d, objRoomNum = %d, room->number = %d)", character->getName(), objServID, character->servID, objRoomNum, character->room? character->room->number : -1 );
	}

	if ( character->servID == objServID ) {
		// update my state based on this movie

		if ( character->processMovie ( &packet ) ) {
			if ( room ) {
				PackedMsg response;
				response.putArray ( message->data(), message->size() );
				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), room );
			}
			roomMgr->sendACK ( _IPC_PLAYER_MOVIE, this );
		}
	} else {
		logInfo ( _LOG_ALWAYS, "Invalid movie info from %s, (objServID == %d, character->servID = %d, objRoomNum = %d, room->number = %d)", character->getName(), objServID, character->servID, objRoomNum, character->room? character->room->number : -1 );
		disable ( "invalid movie info" );
	}

	return retVal;
}

int RMPlayer::process_IPC_PLAYER_SHIFT_ROOM ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		
	if ( character->playerControlled && zone ) 
		zone->delPlayerFromQueue ( this );

	PackedMsg packet ( message->data(), message->size() );
	unsigned char exit = packet.getByte(); 

	// check for a blocked exit...
	if ( room->isExitBlocked ( exit ) && (!zone || !zone->ambushGroups.size()) ) {
		logHack ( "%s tried to walk past a blocked direction.", character->getName() );
		forceLogout();
	}

	int newNumber = room->hasExit ( exit )? room->mapExitToRoom ( exit ) : -1;
	int houseExit = 0;

	// the more you walk, the more quickly you get ambushed...
	if ( lastAmbushTime ) {
		lastAmbushTime -= random ( 5, 30 );
	}

	// magic house-exit check...
	if ( newNumber == -2 ) {
		newNumber = gHouseExits[room->building->homeTown];

		RMRoom *newRoom = roomMgr->findRoom ( newNumber );
		Zone *zone = newRoom? newRoom->zone : NULL;

		if ( zone ) {
			if ( newRoom->size() > 20 ) {
				while ( newRoom->size() > 20 ) 
					newRoom = (RMRoom *)zone->rooms.at ( random ( 0, zone->rooms.size() - 1 ) );
	
				roomMgr->sendSystemMsg ( "Gate Too Full", this, "Your towns gate keeper was too full, you are being redirected to another room in that area." );
			}
		}

		// SNTODO: remove this 20 Victor kludge
		newNumber = newRoom? newRoom->number : 20;
		houseExit = 1;

		// set destination room
		setTeleportRoomNum ( newNumber );
	}

	if ( !character->combatGroup ) {
		// tell the current room that I'm leaving
		if ( room ) {
			PackedMsg leftRoom;
			leftRoom.putLong ( character->servID );
			leftRoom.putLong ( room->number );
			leftRoom.putByte ( _MOVIE_CHANGE_ROOM );
			leftRoom.putByte ( exit );
			leftRoom.putByte ( _MOVIE_END );

			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, leftRoom.data(), leftRoom.size(), room );
		}

		int exitDir = exitToDir ( exit );

		if ( room->exitCoords[exitDir][0] != -1 ) {
			PackedMsg ack;
			ack.putACKInfo ( message->type() ); 

			ack.putWord ( room->exitCoords[exitDir][0] );
			ack.putWord ( room->exitCoords[exitDir][1] );

			roomMgr->sendTo ( _IPC_PLAYER_ACK, ack.data(), ack.size(), this );
		}

		// make sure we are standing up
		if ( character->sittingOn ) {
			character->sittingOn->beStoodUpOn ( character );
			character->sittingOn = NULL;
		}

		RMRoom *newRoom = roomMgr->findRoom ( newNumber );

		if ( newRoom ) {
			int changingZone = ((zone == newRoom->zone) && (room->building == newRoom->building))? 0 : 1;

			if ( checkAccess ( _ACCESS_MODERATOR | _ACCESS_GUIDE | _ACCESS_PROPHET | _ACCESS_EVENT ) || ( (newRoom->size() < ROOM_LIMIT_SIZE ) && !newRoom->bRubberWalled ) ) {
				if ( room )
					room->delPlayer ( this, changingZone );

				roomMgr->sendRoomInfo ( newNumber, this );

				character->room = newRoom;
				room = newRoom;
				zone = newRoom->zone;
	
				newRoom->addPlayer ( this, NULL, changingZone );
			} else {
				roomMgr->sendNAK ( _IPC_PLAYER_SHIFT_ROOM, _ERR_BAD_ROOM, this );
			}
		} else {
			roomMgr->sendNAK ( _IPC_PLAYER_SHIFT_ROOM, _ERR_BAD_ROOM, this );
		}
	} else {
		roomMgr->sendNAK ( _IPC_PLAYER_SHIFT_ROOM, _ERR_BAD_ROOM, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_TALK ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		
	PackedMsg packet ( message->data(), message->size() );

	WorldObject *object = roomMgr->findObject ( packet.getLong() );

	if ( object ) {

		BTalk *base = (BTalk *)object->getBase ( _BTALK );

		if ( base ) {

			if ( lastTalkTarget != object ) {
				lastTalkTarget = object;
				lastTalkSequence = 0;
			}

			PackedMsg ack;
			ack.putACKInfo ( message->type() );

			File file ( base->file );

			if ( file.isOpen() ) {
				int bufferSize = file.size();
				char *text = (char *)malloc ( bufferSize + 1 );
				text[bufferSize] = 0;
				file.read ( text, bufferSize );

				char *ptr = text;
				char line[1024], lastLine[1024];
				int count = lastTalkSequence + 1;

				while ( count && bufgets ( line, &ptr, &bufferSize, sizeof ( line ) ) ) {
					strcpy ( lastLine, line );
					count--;
				}

				if ( count ) {
					strcpy ( line, lastLine );
				} else {
					lastTalkSequence++;
				}

				ack.putString ( line );

				free ( text );
			} else {
				ack.putString ( "I really have nothing to say." );
			}

			roomMgr->sendTo ( _IPC_PLAYER_ACK, ack.data(), ack.size(), this );
		} else {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		}
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	// give control back to the user
	handsOn();

	return retVal;
}

int RMPlayer::process_IPC_GET_ENTRY_INFO ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		
	int tDungeon = 0;

	Zone *theZone = NULL;

	PackedMsg packet ( message->data(), message->size() );

	WorldObject *object = roomMgr->findObject ( packet.getLong() );

	if ( object && room ) {

		if ( !object->room ) {
			return retVal;
		}

		if ( object->room->number != room->number ) {
			return retVal;
		}

		BEntry *base = (BEntry *)object->getBase ( _BENTRY );

		if ( base && base->room ) {
			BOpenable *bopen = (BOpenable *)object->getBase ( _BOPEN );
			RMRoom *baseRoom = roomMgr->findRoom ( base->room );

			theZone = baseRoom? baseRoom->zone : NULL;
			tDungeon = theZone? theZone->isDungeon : NULL;

			if ( bopen && !bopen->isOpen() ) {
				roomMgr->sendNAK ( message->type(), _ERR_MUST_OPEN, this );
				return retVal;
			}

			// always put on the queue!
			if ( room->isDungeonEntrance && tDungeon && tDungeon != 2 ) {
				dungeonEntrance = object;
				roomMgr->sendNAK ( message->type(), _ERR_NONSENSE, this );
				theZone->addPlayer( this );
				return retVal;									 
			}

			if ( baseRoom->size() > ROOM_LIMIT_SIZE ) {	
				roomMgr->sendNAK ( message->type(), _ERR_TOO_BULKY, this );
				return retVal;									 
			}	

			PackedMsg ack;
			ack.putACKInfo ( message->type() );
			ack.putWord ( base->startingX );
			ack.putWord ( base->startingY );
			ack.putByte ( base->startingLoop );
			ack.putWord ( base->endingX );
			ack.putWord ( base->endingY );
			ack.putByte ( base->endingLoop );
			ack.putLong ( base->room );
			roomMgr->sendTo ( _IPC_PLAYER_ACK, ack.data(), ack.size(), this );

			// set destination room
			setTeleportRoomNum ( base->room );
		} else {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		}
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	return retVal;
}

int RMPlayer::process_IPC_GET_HOUSE ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		
	PackedMsg packet ( message->data(), message->size() );

	char *theName = packet.getString();

	// out of bounds test
	if ( !theName || !isValidName ( theName ) ) {
		if ( theName )
			free ( theName );
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
		return retVal;
	}

	strlower ( theName );
	teleportHouse ( character, theName );

	roomMgr->sendNAK ( message->type(), _ERR_TOO_HEAVY, this );

	free ( theName );

	return retVal;
}

int RMPlayer::process_IPC_PLAYER_CHANGE_ROOM ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;
		
	if ( character->playerControlled && zone ) 
		zone->delPlayerFromQueue ( this );

	PackedMsg packet ( message->data(), message->size() );
	long number = packet.getLong();
	short value = packet.getWord();
	long number2 = packet.getLong();
	long curRoomNum = room? room->number : -1;

	if ( value != 0x00002be2 || number != number2 ) {
		logInfo ( _LOG_ALWAYS, "%s is modifying packets to get to another room.", character->getName() );

		// stop the teleport
		setTeleportRoomNum ( -1 );

		if ( isTeleporting )
			isTeleporting--;

		roomMgr->sendNAK ( _IPC_PLAYER_CHANGE_ROOM, _ERR_BAD_ROOM, this );
		return retVal;
	}

	// check for redirected teleports
	if ( (curRoomNum != -1) && (number != curRoomNum) && (teleportRoomNum != number) ) {
		if ( teleportRoomNum != -1 )
			return retVal;

		setTeleportRoomNum ( -1 );
			
		if ( isTeleporting )
			isTeleporting--;

		roomMgr->sendNAK ( _IPC_PLAYER_CHANGE_ROOM, _ERR_BAD_ROOM, this );
		return retVal;
	}

	// check for redirected teleports from the main screen
	if ( (curRoomNum == -1) && (number != character->roomNumber) ) {
		logInfo ( _LOG_ALWAYS, "%s tried to redirect starting room number (%d, %d)", character->getName(), number, character->roomNumber );
		return retVal;
	}

	// make sure that if the player is going to a house that the dest matches by name
	RMRoom *destRoom = roomMgr->findRoom ( teleportRoomNum );

	if ( destRoom && destRoom->building ) {
		if ( teleportDestBuildingOwner ) {
			if ( strcasecmp ( teleportDestBuildingOwner, destRoom->building->_owner ) ) {
				logInfo ( _LOG_ALWAYS, "%s going to house (%s), but teleportDestBuildingOwner did not match.", character->getName(), teleportDestBuildingOwner );

				// stop the teleport
				setTeleportRoomNum ( -1 );

				if ( isTeleporting )
					isTeleporting--;

				roomMgr->sendNAK ( _IPC_PLAYER_CHANGE_ROOM, _ERR_BAD_ROOM, this );
				return retVal;
			}
		} else {
			logInfo ( _LOG_ALWAYS, "%s going to house with no teleportDestBuildingOwner set.", character->getName() );

			// stop the teleport
			setTeleportRoomNum ( -1 );

			if ( isTeleporting )
				isTeleporting--;

			roomMgr->sendNAK ( _IPC_PLAYER_CHANGE_ROOM, _ERR_BAD_ROOM, this );
			return retVal;
		}
	}

	// logging in, make sure not to go to another's house.
	if (  curRoomNum == -1 && ( number > _DBR_THRESHOLD ) ) {
  		Building *house = findHouse ( character->getName() );
		if ( house && house->rooms.size() ) {
  			RMRoom *theRoom = (RMRoom *)house->rooms.at ( 0 );
  			number = theRoom->number;
  		} else {
  			number = random ( 5000, 5089 );
  		}
	}

	// reset the teleport room number 
	setTeleportRoomNum ( -1 );

	if ( isTeleporting )
		isTeleporting--;

	if ( character->health < 1 ) {
		character->health = 1;
		character->changeHealth ( character->healthMax / 2, NULL );
		character->clearAffect ( _AFF_POISONED, _AFF_TYPE_NORMAL );
		character->clearAffect ( _AFF_ACID_BURN, _AFF_TYPE_NORMAL );
	}

	if ( !this->room ) {
		lastAmbushTime = getsecondsfast() + (60 * 3);

		if ( roomMgr->players()->size() == 1 ) 
			roomMgr->sendPlayerInfo ( this, "You are all by yourself in this great big world.\n" );
	 	else
			roomMgr->sendPlayerInfo ( this, "Users Online: %d\n", roomMgr->players()->size() );

		int upTime = getsecondsfast() - gStartTime;
		double daysUp = (double)upTime / 86400.0;
		double hoursUp = daysUp * 24.0;

		if ( daysUp >= 1 )
			roomMgr->sendPlayerInfo ( this, "Server Uptime: %f days.\n", daysUp );
		else
			roomMgr->sendPlayerInfo ( this, "Server Uptime: %f hours.\n", hoursUp );
			roomMgr->sendPlayerInfo ( this, "The Realms of Serenia v0.23\n" );
			roomMgr->sendPlayerInfo ( this, "Expboost: %.2f times normal.", gExpBoost );
			roomMgr->sendPlayerInfo ( this, "Lootboost: %d times normal.", gLootBoost );

	        if ( gMagicMailOff != 1 ) {
			gDataMgr->getMailList( character );
		}
	}

	if ( !character->combatGroup && (!this->room || this->room->number != number) ) {
		// make sure we are standing up
		if ( character->sittingOn ) {
			character->sittingOn->beStoodUpOn ( character );
			character->sittingOn = NULL;
		}
		
		RMRoom *newRoom = roomMgr->findRoom ( number );

		if ( newRoom ) {
			int changingZone = (zone == newRoom->zone)? 0 : 1;

			// handle marking changing zone if going to a building
			if ( newRoom->building && (!room || (room->building != newRoom->building)) )
				changingZone = 1;

			// handle switching between buildings
			if ( room && (newRoom->building != room->building) )
				changingZone = 1;

			// delete our character from the room
			if ( room ) 
				room->delPlayer ( this, changingZone );
	
			// do the room change within this server
			roomMgr->sendRoomInfo ( number, this );

			character->room = newRoom;
			room = newRoom;
			zone = newRoom->zone;

			room->addPlayer ( this, NULL, changingZone );
		} else {
			roomMgr->sendNAK ( _IPC_PLAYER_CHANGE_ROOM, _ERR_BAD_ROOM, this );
		}
	} else {
		roomMgr->sendNAK ( _IPC_PLAYER_CHANGE_ROOM, _ERR_BAD_ROOM, this );
	}
	return retVal;
}

int RMPlayer::process_IPC_CLIENT_HUNG_UP ( IPCMessage *message )
{
	int retVal = 1;

	char *login = getLogin();

	if ( character ) {
		char theName[1024];
		strcpy ( theName, getName() );
		strlower ( theName );

		// remove this player from the friend manager...
		CFriend *pFriend = GetFriendEntry();

		if ( pFriend ) {
			pFriend->HandleLogout ( this );
		}

		if ( !character->player ) {
			logInfo ( _LOG_ALWAYS, "character %s has no player object", getName() );
			return retVal;
		}

		if ( !character->player->player ) {
			logInfo ( _LOG_ALWAYS, "character %s's player has no player object", getName() );
			return retVal;
		}

		// this player quit in combat, minor reaming to discourage "plug".
		if (  character->combatGroup && ( character->health > 0 ) && character->opposition->size() ) {
			character->changeHealth ( -character->health / 2, character, 1, 1 );
		}

		logStats( "%s: (%s) Logout.", login, getAccountType() );

		gCharacterTree.del ( theName );
		gObjectArray->del ( character->servID );
	}

	gLoginTree.del ( login );

	if ( loginMsg ) {
		roomMgr->_msgQueue.del ( loginMsg );
		delete loginMsg;

		loginMsg = NULL;
	}

	if ( player )
		gDataMgr->logout ( this );

	if ( character ) 
		character->writeCharacterData();

	retVal = 0;

	return retVal;
}

int RMPlayer::process_IPC_PLAYER_OLD_CHARACTER_LOGIN ( IPCMessage *message )
{
	BPlayer *bplayer = (BPlayer *)player->getBase ( _BPLAYER );  

	if ( !bplayer ) {
	 	logHack ( "CHARACTER_LOGIN: No _BPLAYER base on old interpreter" );
	 	forceLogout();
	 	return 1;
	}

	logInfo ( _LOG_ALWAYS, "Account %s is logging in on old interpreter and is DISABLED", bplayer->login );

	disable ( "account AUTO-disabled for logging in with an old interpreter" );

	return 1;
}

int RMPlayer::process_IPC_PLAYER_CHARACTER_LOGIN ( IPCMessage *message ) {
	int retVal = 1;

	// handle banned disk serial numbers...
	if ( !allowedToLogin() ) {
		writeSerialNumber();
		disable ( "account AUTO-disabled for banned access" );
	 	return retVal;
	}

	// new
	if ( !player ) {
	 	logHack ( "CHARACTER_LOGIN: No player." );
	 	forceLogout();
	 	return retVal;
	}

	BPlayer *bplayer = (BPlayer *)player->getBase ( _BPLAYER );  

	if ( !bplayer ) {
	 	logHack ( "CHARACTER_LOGIN: No _BPLAYER base." );
	 	forceLogout();
	 	return retVal;
	}

	PackedMsg packet ( message->data(), message->size() );

	long characterServID = packet.getLong();
	character = roomMgr->findObject ( characterServID ); 

	if ( !character ) {
		logHack ( "%s _IPC_CHARACTER_LOGIN: No character.", bplayer->login );
		forceLogout();	
		return retVal;
	}

	BCharacter *bchar = (BCharacter *)character->getBase ( _BCHARACTER ); 
	
	if ( !bchar ) {
		logHack ( "%s _IPC_CHARACTER_LOGIN: No _BCHARACTER base.", bplayer->login );
		forceLogout();	
		return retVal;
	}

	if ( !isValidName ( character->getName() ) ) {
		logHack ( "%s IPC_CHARACTER_LOGIN: Invalid character name.", bplayer->login );
		forceLogout();
		return retVal;
	}

	character->playerControlled = 1;
	character->player = this;

	bplayer->purgeAllBut ( character );

	char *login = "<unknown>";

	login = bplayer->login;

	if ( !login ) {
		logHack ( "Bad player login base for %s (%d).", character->getName(), characterServID );
		forceLogout();
		return retVal;
	}

	char theName[1024];
	strcpy ( theName, getName() );
	strlower ( theName );

	gObjectArray->add ( character, characterServID );
	gCharacterTree.add ( theName, this );
	character->characterElement = roomMgr->_characters.add ( character );

	// process the friend list data (if any)
	if ( packet.getDataLeft() ) {
		int nFriendListSize = packet.getByte();

		while ( nFriendListSize ) {
			char *pName = packet.getString();

			free ( pName );
		}
	}

	// create the friend list entry for this player
	CFriend *pFriend = GetFriendEntry();

	if ( NULL == pFriend ) {
		m_pFriend = g_pFriendMgr->FindFriend ( getName() );
		m_pFriend->HandleLogin ( this );
	}

	gDataMgr->config( character );

	return retVal;
}

int RMPlayer::process_IPC_LOGIN_DONE ( IPCMessage *message )
{
	int retVal = 1;
	int didReset = 0;
	loginMsg = NULL;

	return retVal;
}

int RMPlayer::process_IPC_TERMINATED ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player ) 
		return retVal;
		
	// this player quit in combat, ream him!
	if ( character && character->combatGroup && (character->health > 0) && character->opposition->size() ) {
		character->physicalState |= _STATE_WEINER;
		character->changeHealth ( -character->health, character, 1, 1 );
	}

	return retVal;
}

int RMPlayer::process_IPC_INN_LOGIN ( IPCMessage *message )
{
	int retVal = 1;

	PackedMsg packet ( message->data(), message->size() );
	packet.getLong();
	packet.getWord();

	serial = packet.getLong();

	ping = 3;
	pingTime = 0;
	pingClientTime = 0;

	validated = 1;

	// if the login context is gone, just ACK it
	if ( !loginContext ) 
		roomMgr->sendACK ( _IPC_PLAYER_LOGIN, this );

	return retVal;
}

// process mass buy request...
int RMPlayer::process_IPC_MASS_BUY ( IPCMessage *message )
{
	const int nMaxObjectCount = 50;
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the servID of the store to purchase from
	long servID = packet.getLong();

	// find the store
	WorldObject *shop = roomMgr->findObject ( servID );
	BShop *bshop = shop? (BShop *)shop->getBase ( _BSHOP ) : NULL;

	// don't process invalid shop objects...
	if ( !shop || !bshop )
		return retVal;

	// don't process GM shop requests if not a GM...
	if ( !bshop->sellMarkup && !bshop->buyMarkup && !checkAccess ( _ACCESS_BUY_STORE ) )
		return retVal;

	// only process buy message if in the same room as the shop...
	if ( character->room == shop->getRoom() ) {
		// list of ShopCartItems to buy...
		LinkedList itemList;

		// aggregate value of all items purchased...
		int nTotalValue = 0;

		// total count of objects...
		int nTotalObjectCount = 0;

		// build a list of each item to buy...
		for ( ;; ) {
			// handle non termination of message...
			if ( packet.getDataLeft() < 1 ) {
				roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
				return retVal;
			}

			// get the index of the item to buy...
			int nItemIndex = packet.getWord();

			// stop when a -1 is found....
			if ( nItemIndex == -1  ) {
				break;
			}

			// get the quantity to buy...
			int nQuantity = packet.getLong();

			// normalize the quantity...
			if ( (nQuantity < 0) || (nQuantity > 9999999) ) {
				nQuantity = 0;
			}

			// get the item to buy...
			ShopItem *pItem = bshop->at ( nItemIndex );

			if ( pItem ) {
				// if not mana, increment the total object count...
				if ( pItem->type != _SHOP_CRYSTALS ) {
					nTotalObjectCount += nQuantity;
				}

				// create the shop cart item...
				ShopCartItem *pCartItem = new ShopCartItem;
				pCartItem->m_nQuantity = nQuantity;
				pCartItem->m_pItem = pItem;

				// add to the item list...
				itemList.add ( pCartItem );

				// increment the value...
				nTotalValue += pItem->price() * nQuantity;
			}
		}

		// see if it is too expensive...
		if ( (nTotalValue < 0) || !character->canAfford( nTotalValue, bshop->currency ) ) {
			roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
		} 
		
		// see if too many objects...
		else if ( nTotalObjectCount > nMaxObjectCount ) {
			roomMgr->sendNAK ( message->type(), _ERR_SERVICE_NOT_AVAILABLE, this );
		}

		// process the buy request...
		else {
			// ready the response message...
			PackedMsg response;
			response.putACKInfo ( message->type() );

			// step through and purchase each item in turn, appending the response to the ACK...
			LinkedElement *pElement = itemList.head();

			while ( pElement ) {
				ShopCartItem *pCartItem = (ShopCartItem *)pElement->ptr();
				pElement = pElement->next();

				if ( pCartItem ) {
					// get the shop item...
					ShopItem *pItem = pCartItem->m_pItem;

					// get the quantity...
					int nQuantity = pCartItem->m_nQuantity;

					// purchase the item...
					switch ( pItem->type ) {
						// handle buying objects...
						case _SHOP_OBJECT: {
							for ( int i=0; i<pCartItem->m_nQuantity; i++ ) {
								// put the item type...
								response.putByte ( pItem->type );

								// create the object...
								WorldObject *pObject = new WorldObject ( ((ShopObject *)pItem)->object );
								pObject->addToDatabase();

								// identify this object...
								pObject->room = room;
								pObject->addAffect ( _AFF_IDENTIFIED, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, 0, 0, NULL, 0 );
								pObject->room = NULL;

								// attempt to force the object into the character...
								int nResult = pObject->forceIn ( character, 1 );

								// reduce the coffers of the character and put the object information in the packet...
								if ( nResult == _WO_ACTION_HANDLED ) {
									int nPrice = pItem->price();

									pObject->buildPacket ( &response, 1 );

									switch( bshop->currency ) {
										case BShop::Gold:
											character->value -= pItem->price();
											response.putLong ( pItem->price() );
											break;
										case BShop::Copper:
											nCoppers -= pItem->price();
											gDataMgr->copper( this, nCoppers );
											response.putLong ( 0 ); //subtract zero gold from inv
											break;
										default:
											logDisplay("%s:%d currency type not supported for BShop", __FILE__, __LINE__ );
											response.putLong ( 0 );
									};

								//log a priveleged store purchase
								if ( !bshop->sellMarkup && !bshop->buyMarkup ) {
									if( pObject && pObject->classID )
										gDataMgr->logPermanent( getLogin(),  getName(), _DMGR_PLOG_UNKNOWN, "purchased %s from a priveleged store", pObject->classID );
								}

								} else {
									// denote invisible object...
									response.putByte ( 0 );
	
									// destroy the object...
									roomMgr->destroyObj ( pObject, 1, __FILE__, __LINE__ );
								}
							}
						}

						break;

						// handle buying mana...
						case _SHOP_CRYSTALS: {
							// put the item type...
							response.putByte ( pItem->type );

							int nPrice = pItem->price() * pCartItem->m_nQuantity;
							character->value -= nPrice;

							if ( ( character->manaValue + pCartItem->m_nQuantity ) > MAX_MANA_HELD )
								pCartItem->m_nQuantity = MAX_MANA_HELD - character->manaValue;

							if ( pCartItem->m_nQuantity < 0 )
								pCartItem->m_nQuantity = 0;

							character->manaValue += pCartItem->m_nQuantity;

							character->calcWeight();

							response.putLong ( pCartItem->m_nQuantity );
							response.putLong ( nPrice );
						}

						break;
					}
				}
			}

			// terminate the item list...
			response.putByte ( 255 );

			roomMgr->sendTo ( _IPC_PLAYER_ACK, response.data(), response.size(), this );
		}
	} else {
		roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
	}

	return retVal;
}

// process mass sell request...
int RMPlayer::process_IPC_MASS_SELL ( IPCMessage *message )
{
	int retVal = 1;

	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	// get the shop
	WorldObject *shop = roomMgr->findObject ( packet.getLong() );
	BShop *bshop = shop? (BShop *)shop->getBase ( _BSHOP ) : NULL;

	// ignore no shop or invalid shop...
	if (!shop || !bshop )
		return retVal;

	// ready the response message...
	PackedMsg response;
	response.putACKInfo ( message->type() );

	// loop and sell each item...
	for ( ;; ) {
		if ( packet.getDataLeft() < 1 ) {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_SERVID, this );
			return retVal;
		}

		int nServID = packet.getLong();

		// if -1, stop processing...
		if ( nServID == -1 )
			break;

		// handle selling mana...
		if ( nServID == 0 ) {
			// get the quantity to sell...
			int nQuantity = packet.getLong();

			// only sell valid amounts of mana...
			if ( (nQuantity >= 0) && (nQuantity <= character->manaValue) ) {
				int nProposedValue = ( character->value + ( nQuantity * 5 ) );
				if ( nProposedValue > MAX_GOLD_HELD || nProposedValue < 0 ) {
					nQuantity = ( MAX_GOLD_HELD - character->value ) / 5;

					if ( nQuantity < 0 )
						nQuantity = 0;
				}

				character->manaValue -= nQuantity;

				// put mana indicator
				response.putLong ( 0 );

				if ( !checkAccess( _ACCESS_BUY_STORE ) ) {
					// put price...
					response.putLong ( nQuantity * 5 );
					character->value += nQuantity * 5;
				} else {
					response.putLong ( 0 );
				}
			}
		} else {
			// get the object to sell...
			WorldObject *pObject = roomMgr->findObject ( nServID );

			// sell the object to the shop
			if ( pObject &&	character->owns ( pObject ) && pObject->getBase ( _BCARRY ) && !pObject->getBase ( _BHEAD ) ) {
				int price = 0;
				int result = 0;
	
				price = bshop->price ( pObject );

				int nProposedValue = character->value + price;

				if ( nProposedValue > 0 && nProposedValue < MAX_GOLD_HELD ) {
					result = bshop->buy ( pObject, character );
		
					if ( result == _WO_ACTION_HANDLED ) {
						// put the servID of the item in the response...
						response.putLong ( nServID );

						if ( !checkAccess( _ACCESS_BUY_STORE ) )
							// put the price...
							response.putLong ( price );
						else
							response.putLong ( 0 );
					}
				}
			}
		}
	}

	// put a terminator...
	response.putLong ( -1 );

	// send the response...
	roomMgr->sendTo ( _IPC_PLAYER_ACK, response.data(), response.size(), this );

	return retVal;
}

// handle getting the prices of items in the player's inventory
int RMPlayer::process_IPC_GET_SELL_PRICES ( IPCMessage *message )
{
	int retVal = 1;

	// no character, or no player is ignored...
	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	int nShopID = packet.getLong();

	// get the shop
	WorldObject *shop = roomMgr->findObject ( nShopID );
	BShop *bshop = shop? (BShop *)shop->getBase ( _BSHOP ) : NULL;

	// no shop or invalid shop is ignored...
	if ( !shop || !bshop )
		return retVal;

	if ( !bshop->buyMarkup ) {
		roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
		return retVal;
	}

	//
	// step through the character's inventory and put unworn object prices
	// in a response message...
	//
	BContainer *pContainer = (BContainer *)character->getBase ( _BCONTAIN );

	// no container is ignored...
	if ( !pContainer )
		return retVal;

	// ready the response message...
	PackedMsg response;
	response.putACKInfo ( message->type() );

	response.putLong( nShopID );

	LinkedElement *pElement = pContainer->contents.head();

	while ( pElement ) {
		WorldObject *pObject = (WorldObject *)pElement->ptr();
		pElement = pElement->next();

		// ignore non-carryable...
		if ( !pObject->getBase ( _BCARRY ) )
			continue;

		// ignore heads...
		if ( pObject->getBase ( _BHEAD ) )
			continue;

		// ignore worn items...
		if ( pObject->isWornBy ( character ) )
			continue;

		// ignore unsellable objects...
		if ( pObject->physicalState & _STATE_WORTHLESS )
			continue;

		// put the servID of the item to sell...
		response.putLong ( pObject->servID );

		// put the sell price of the item...
		response.putLong ( bshop->price ( pObject ) );
	}

	// put a terminator...
	response.putLong ( -1 );

	// send the response...
	roomMgr->sendTo ( _IPC_PLAYER_ACK, response.data(), response.size(), this );

	return retVal;
}

// handle creating the channel
int RMPlayer::process_IPC_CREATE_CHANNEL ( IPCMessage *message )
{
	int retVal = 1;

	// no character, or no player is ignored...
	if ( !character || !character->player )
		return retVal;

	PackedMsg packet ( message->data(), message->size() );

	int nChannel = packet.getWord();

	Channel* pChannel;

	if ( nChannel == -1 ) {
		nChannel = 0;

		pChannel = gChannels [ nChannel ];

		while ( ( pChannel->isSystem || !pChannel->isEmpty() ) && nChannel < 1000 ) {
			pChannel = gChannels [ ++nChannel ];
		}

		if ( nChannel == 1000 ) {
			roomMgr->sendNAK ( message->type(), _ERR_TOO_EXPENSIVE, this );
			return retVal;
		}
	} else {
		pChannel = gChannels [ nChannel ];

		if ( ! pChannel->isEmpty() || pChannel->isSystem ) {
			roomMgr->sendNAK ( message->type(), _ERR_BAD_ROOM, this );
			return retVal;
		}
	}

	if ( channel ) {
		channel->delPlayer ( this );
	}

	pChannel->addPlayer( this );

	char*	pChannelName = packet.getString();
										  
	if ( pChannelName ) {
		pChannel->setName( pChannelName );
		delete pChannelName;
	}

	char*	pChannelTopic = packet.getString();

	if ( pChannelTopic ) {
		pChannel->setTopic( pChannelTopic );
		delete pChannelTopic;
	}

	char*	pChannelPass = packet.getString();

	if ( pChannelPass ) {
		if ( *pChannelPass )
			pChannel->setPassword( pChannelPass );
		delete pChannelPass;
	}

	PackedMsg response;

	// put ACK info
	response.putACKInfo ( message->type() );

	// put channel number.
	response.putWord ( nChannel );

	roomMgr->sendTo ( _IPC_PLAYER_ACK, &response, this );

	return retVal;
}

int RMPlayer::allowedToLogin ( void )
{
	char filename[1024];
	sprintf ( sizeof ( filename ), filename, "../data/banned/0x%x", serial );

	if ( exists ( filename ) )
		return 0;

	return 1;
}

void RMPlayer::writeSerialNumber ( void )
{
	char filename[1024];
	sprintf ( sizeof ( filename ), filename, "../data/banned/0x%x", serial );

	if ( player && exists ( filename ) ) {
		File *serialFile = new File ( filename );
		serialFile->open ( filename );

		if ( serialFile->isOpen() ) {
			char* login = getLogin();
			char* name = getName();

			gDataMgr->logPermanent ( login, name, _DMGR_PLOG_BANNED, "Attempted login by disk id 0x%08x.", serial );

			serialFile->seek ( serialFile->size() );

			time_t time = getseconds();
			struct tm *curTime;        
			curTime = localtime( &time ); 

			serialFile->printf ( "%s logged in on %02d/%02d/%04d %02d:%02d:%02d\n", login, curTime->tm_mon, curTime->tm_mday, ( curTime->tm_year + 1900 ), curTime->tm_hour, curTime->tm_min, curTime->tm_sec );
		}

		delete serialFile;
	}
	return;
}

void RMPlayer::setAction ( CombatAction *action )
{
	int doCheck = 0;

	if ( combatAction ) {
		delete combatAction;
		combatAction = NULL;
	} else {
		doCheck = 1;
	}

	combatAction = action;
		
	if ( doCheck ) 
		character->combatGroup->makeTurnReady ( character );
}

void RMPlayer::forceLogout ( void )
{
	if ( isValidPtr ( owner ) && getPtrType ( owner ) == _MEM_CLIENT ) {
		tossing = 1;
		owner->isDead = TRUE;
	} 	
}

void RMPlayer::preVerbMessage ( PackedData *message, PackedData *movie, WorldObject **directObject, WorldObject **indirectObject ) 
{
	long dObj = message->getLong();
	long iObj = message->getLong();

	// extract the direct and indirect objects from the packet

	*directObject = roomMgr->findObject ( dObj );
	*indirectObject = roomMgr->findObject ( iObj );

	// process the rest of the packet as if it were a movie
	if ( character && character->player == this ) {
		movie->putLong ( 0 );
		movie->putLong ( 0 );
		movie->putLong ( character->servID );
		movie->putLong ( room->number );

		character->processMovie ( message, movie );
	} 
}

void RMPlayer::postVerbMessage ( int result, int verbMsg, PackedData *movie, WorldObject *directObject, WorldObject *indirectObject )
{
	// make sure the client gets control
	movie->putByte ( _MOVIE_HANDS_ON );

	// put the terminator on the movie
	movie->putByte ( _MOVIE_END );

	if ( !directObject && !indirectObject ) {
		roomMgr->sendNAK ( verbMsg, _ERR_BAD_SERVID, this );
		goto sendMovie;
	}

	switch ( result ) {
		case _WO_ACTION_HANDLED: {
			roomMgr->sendACK ( verbMsg, this );
		}

		break;

		default: {
			roomMgr->sendNAK ( verbMsg, result, this );	
		}

		break;
	}

sendMovie:

	if ( !character || !character->player ) 
		return;
		
#if 0
		PackedMsg response;

		// put character servID
		response.putLong ( character->servID );
		response.putLong ( room->number );

		// put movie data
		response.putArray ( movie->data(), movie->size() );

		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), room );
#endif

	roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, movie->data(), movie->size(), room );
}

// process a room chat string, handling any special commands
void RMPlayer::processRoomChat ( char *text )
{
	if ( !text ) {
		logHack ( "Invalid Room Chat: (NULL) received from %s", getName() );
		return;
	}

	int len = strlen ( text );

	if ( !checkAccess( _ACCESS_IMPLEMENTOR | _ACCESS_EVENT ) && ( len < 1 || len > 320 ) ) {
		logHack ( "Invalid Room Chat: string length of %d received from %s", len, getName() );
		return;
	
	}

	if ( checkTimedAccess ( _ACCESS_GAGGED ) && *text != '/' ) {
		roomMgr->sendPlayerText ( this, "|c60|Info> You are not allowed to speak.\n" );
		return;
	}

	if ( strstr ( text, "Info>" ) || strstr ( text, "info>" ) ) 
		roomMgr->sendPlayerText ( this, "|c60|Info> No fake info messages allowed.\n" );
	
	else if ( strstr ( text, "Gossip>" ) || strstr ( text, "gossip>" ) ) 
		roomMgr->sendPlayerText ( this, "|c60|Info> No fake gossip messages allowed.\n" );

	else if ( strstr ( text, "Tell>" ) || strstr ( text, "tell>" ) || strstr ( text, "TELL>" ) )
		roomMgr->sendPlayerText ( this, "|c60|Info> No fake tell messages allowed.\n" );

	else if ( ( strstr ( text, "|f") - text + 1 ) == 1 )
		roomMgr->sendPlayerText ( this, "|c60|Info> No phoney funky fonts, fool!\n" );

	else if ( !parseCommand ( text, this ) ) {

		if ( *text != '/' ) {
			// check for multiple '|' symbols in the text... two are allowed
			char* ptr = text;
			int pipeCount = 0;
			char* lastPipe = text;

			while ( *ptr ) {

				if ( *ptr == '|' ) {

					lastPipe = ptr;

					if ( pipeCount == 2 ) {
						roomMgr->sendPlayerText ( this, "|c60|Info> No embedded color changes allowed.\n" );
						return;
					}
					pipeCount++;
				}
				ptr++;
			}

			lastPipe++;

			if( pipeCount == 1 || lastPipe > ptr ) {
				roomMgr->sendPlayerText ( this, "|c60|Info> No embedded color changes allowed.\n" );
				return;
			}

			if ( room ) {
				logChatData ( "%s:%s:R %d: %s", getLogin(), getName(), room->number, lastPipe );
			}
			else {
				logChatData ( "%s:%s: %s", getLogin(), getName(), lastPipe );
			}

  			sendRoomChat ( "%s: %s\n", getName(), text ); 

			if ( character->room && character->room->riddledObjects.size() ) {
				LinkedElement *element = character->room->riddledObjects.head();

				while ( element ) {
					WorldObject *obj = (WorldObject *)element->ptr();

					BDescribed *base = (BDescribed *)obj->getBase ( _BDESCRIBED );

					if ( base ) {
						strlower ( text );

						LinkedList *tokens = buildTokenList ( text, " \t\n\r|" );
						LinkedElement *tok = tokens->head();

						while ( tok ) {
							StringObject *str = (StringObject *)tok->ptr();

							if ( strstr ( str->data, base->riddleAnswer ) == str->data ) {
								PackedMsg packet;
								packet.putLong ( character->servID );
								packet.putLong ( character->room->number );

								obj->processActions ( vBeAnswered, &packet );		

								packet.putByte ( _MOVIE_END );

								roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, packet.data(), packet.size(), character->room );

								break;
							}
							tok = tok->next();
						}
						delete tokens;
					}

					element = element->next();
				}
			}	
		}
	}
}

char *RMPlayer::getName ( void )
{
	char *ret = NULL;

	if ( character ) 
		ret = character->getName();
	else 
		ret = getLogin();

	return ret;
}

char *RMPlayer::getLogin ( void )
{
	if ( player ) {
		BPlayer *bplayer = (BPlayer *)player->getBase ( _BPLAYER );

		if ( bplayer ) 
			return bplayer->login;
	}

	return "unknown login";
}

char *RMPlayer::getPassword ( void )
{
	if ( player ) {
		BPlayer *bplayer = (BPlayer *)player->getBase ( _BPLAYER );

		if ( bplayer ) 
			return bplayer->password;
	}

	return "password";
}

char *RMPlayer::getPronoun ( int plural )
{
	if ( character && character->character ) 
		return playerPronouns[character->character->sex + (plural * 3)];

	return playerPronouns[0];
}

void RMPlayer::exitCombat ( PackedData *thePacket )
{
	if ( !character || !character->player || !character->combatGroup || !character->combatGroup->cloud ) 
		return;
		
	PackedMsg response;

	// put the servID of the movie owner
	if ( !thePacket ) {
		response.putLong ( character->servID );
		response.putLong ( room->number );
	}

	PackedData *packet = thePacket? thePacket : &response;

	// put the combat exit info
	packet->putByte ( _MOVIE_COMBAT_EXIT );
	packet->putLong ( character->servID );
	packet->putWord ( character->combatGroup->cloud->x );
	packet->putWord ( character->combatGroup->cloud->y );
	packet->putLong ( room->number );

	// put the end of the movie
	if ( !thePacket ) {
		response.putByte ( _MOVIE_END );

		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), room );
	}

	character->x = character->combatGroup->cloud->x;
	character->y = character->combatGroup->cloud->y;

	if ( character->combatGroup->pvp && character->combatGroup->defenders.contains ( character ) ) {
		CrimeData* charCrime = getCrimeData();

		if ( !character->pvpGrace ) {
			if ( charCrime->criminal ) {
				roomMgr->sendPlayerText ( this, "|c60|Info>  You do not get a player-killer grace period because you have criminal status." );
			} else {
				roomMgr->sendPlayerText ( this, "|c21|Info>  You are now immune from player killers and can not be jumped for 5 minutes." );
				character->pvpGrace = 20;
			}
		}
	}
	// remove the character from the group
	character->combatGroup->deleteCharacter ( character );

//  new kludge!!
	character->combatGroup = NULL;
}

int RMPlayer::engage ( WorldObject *directObject, PackedData *movie, int theX, int theY ) {
	if ( !character || !character->player || !directObject )
		return 0;
		
	WorldObject *attackLeader = NULL, *defendLeader = NULL;

	int allowCombat = room->zone->allowCombat();

	int attackerIsPlayer = 0, defenderIsPlayer = 0;
	int addedAttacker = 0, addedDefender = 0;

	attackLeader = character;
	defendLeader = directObject;

	if ( groupLeader && !groupLeader->character->combatGroup )
		attackLeader = groupLeader->character;

	if ( directObject->player->groupLeader && !directObject->player->groupLeader->character->combatGroup )
		defendLeader = directObject->player->groupLeader->character;

	CombatGroup *group = new CombatGroup;

	// setup the attacker's side
	if ( groupLeader ) {
		// go through the attacker side and set them all up
		LinkedElement *element = groupLeader->group.head();

		while ( element ) {
			RMPlayer *member = (RMPlayer *)element->ptr();
			WorldObject *obj = member->character;

			if ( obj ) {
				// Prevent group from turning on your player attack (ignore this
				// check for tournament regions
				if ( !(allowCombat & _COMBAT_TOURNEY) && obj->character->peaceful && obj->playerControlled && !defendLeader->player->isNPC ) {
					roomMgr->sendPlayerText ( groupLeader->character->player, "|c60|Info> Group member %s refuses to fight!", obj->getName() );
					element = element->next();
					continue;
				}

				if ( obj->health > 0 && !member->isTeleporting && member->room == room && !obj->combatGroup ) {
					if ( obj->playerControlled && zone ) {
						zone->delPlayerFromQueue ( member );
					}
	
					obj->combatX = -1;
					obj->combatY = -1;

					group->addCharacter ( obj, _COMBAT_ATTACKER );

					// do not turn this on unless they actually made it in.
					if ( obj->combatGroup ) {
						addedAttacker++;

						if ( obj->playerControlled ) {
							attackerIsPlayer = 1;
						}

						if ( attackerIsPlayer ) {
							if ( !(allowCombat & _COMBAT_TOURNEY) && !defendLeader->player->isNPC ) {
								obj->character->peaceful = 0;
								obj->lastCrimeTime = getseconds();
							}
						}

						if ( obj->pvpGrace ) {
							roomMgr->sendPlayerText ( obj->player, "|c60|Info>  Your player killer grace period has ended because you are entering combat." );
							obj->pvpGrace = 0;
						}
					}
				}
			}
			element = element->next();
		}
	} else {
		if ( character->playerControlled && zone )
			zone->delPlayerFromQueue ( character->player );

		character->combatX = -1;
		character->combatY = -1;

		group->addCharacter ( character, _COMBAT_ATTACKER );

		if ( character->combatGroup  ) {
			addedAttacker++;

			if ( character->playerControlled ) {
				attackerIsPlayer = 1;
			}

			if ( character->pvpGrace ) {
				roomMgr->sendPlayerText ( character->player, "|c60|Info>  Your player killer grace period has ended because you are entering combat." );
				character->pvpGrace = 0;
			}
		} 
	}

	// setup the defender's side
	if ( directObject->player->groupLeader ) {
		LinkedElement *element = directObject->player->groupLeader->group.head();

		while ( element ) {
			RMPlayer *member = (RMPlayer *)element->ptr();
			WorldObject *obj = member->character;

			if ( obj ) {
				// Prevent group from turning on your player attack
				if ( obj->character->peaceful && obj->playerControlled && !attackLeader->player->isNPC ) {
					roomMgr->sendPlayerText ( directObject->player, "|c60|Info> Group member %s refuses to fight!", obj->getName() );
					element = element->next();
					continue;
				}

				if ( obj->health > 0 && !member->isTeleporting && member->room == room && !obj->combatGroup ) {
					if ( obj->playerControlled && zone ) {
						zone->delPlayerFromQueue ( member );
					}

					obj->combatX = -1;
					obj->combatY = -1;

					group->addCharacter ( obj, _COMBAT_DEFENDER );

					if ( obj->combatGroup ) {
						addedDefender++;

						if ( obj->playerControlled ) 
							defenderIsPlayer = 1;

						if ( obj->pvpGrace ) {
							roomMgr->sendPlayerText ( obj->player, "|c60|Info>  Your player killer grace period has ended because you are entering combat." );
							obj->pvpGrace = 0;
						}
					}
				}
			}

			element = element->next();
		}
	} else {
		if ( directObject->playerControlled && zone ) {
			zone->delPlayerFromQueue ( directObject->player );
		}

		directObject->combatX = -1;
		directObject->combatY = -1;

		group->addCharacter ( directObject, _COMBAT_DEFENDER );

		if ( directObject->combatGroup  ) {

			addedDefender++;

			if ( directObject->playerControlled ) {
				defenderIsPlayer = 1;
			}

			if ( directObject->pvpGrace ) {
				roomMgr->sendPlayerText ( directObject->player, "|c60|Info>  Your player killer grace period has ended because you are entering combat." );
				character->pvpGrace = 0;
			}
		}
	}

	if ( !addedAttacker || !addedDefender ) 
		return 0;

	if ( attackerIsPlayer && defenderIsPlayer ) {
		group->pvp = 1;
	}

	// create a combat cloud object and put it in the room
	WorldObject *super = roomMgr->findClass ( "CombatCloud" );

	if ( super ) {
		WorldObject *cloud = new WorldObject ( super );
		cloud->isZoneObject = 1;

		if ( theX == -1 ) {
			cloud->x = directObject->x;
			cloud->y = directObject->y;
		} else {
			cloud->x = theX;
			cloud->y = theY;
		}

		cloud->addToDatabase();

		room->addObject ( cloud );

		group->cloud = cloud;
		cloud->combatGroup = group;
	}

	LinkedElement *element = group->combatants.head();

	while ( element ) {
		WorldObject *object = (WorldObject *)element->ptr();

		if ( object->sittingOn )
			object->stand ( movie );

		element = element->next();
	}

	movie->putByte ( _MOVIE_START_COMBAT );
	movie->putWord ( character->room->picture );
	movie->putWord ( group->obstacleSeed );

	// put the attackers
	movie->putByte ( group->attackers.size() );

	element = group->attackers.head();

	while ( element ) {
		WorldObject *combatant = (WorldObject *)element->ptr();

		movie->putLong ( combatant->servID );
		movie->putByte ( combatant->combatX );
		movie->putByte ( combatant->combatY );

		element = element->next();
	}

	// put the defenders
	movie->putByte ( group->defenders.size() );

	element = group->defenders.head();

	while ( element ) {
		WorldObject *combatant = (WorldObject *)element->ptr();

		movie->putLong ( combatant->servID );
		movie->putByte ( combatant->combatX );
		movie->putByte ( combatant->combatY );

		element = element->next();
	}

	movie->putWord ( directObject->x );
	movie->putWord ( directObject->y );
	movie->putLong ( group->cloud->servID );
	movie->putByte ( group->pvp );

	element = group->combatants.head();

	while ( element ) {
		WorldObject *combatant = (WorldObject *)element->ptr();
		element = element->next();

		if ( !combatant->playerControlled ) {
			group->makeCombatReady ( combatant );
			movie->putByte ( _MOVIE_COMBAT_BEGIN );
			movie->putLong ( combatant->servID );
		}
	}
	return 1;
}

int processDamage ( int affect, int var, WorldObject *weapon, WorldObject *target, int activeDmgType ) 
{
	if ( !weapon->hasAffect ( affect ) )	 
		return 0;

	return var;
}

#define SET_DAMAGE(affect,var) var = processDamage ( affect, averageDamage, theWeapon, obj, activeDamageType )

int RMPlayer::attack ( WorldObject *obj, PackedData *movie, int retaliate, int canDodge, int canBlock ) 
{
	if ( !character || !character->player || !obj )
		return 0;
		
	char *dodgeSuccessTbl[] = { "|c50|nimbly avoids", "|c50|easily dodges", "|c50|dodges", "|c50|avoids", "|c50|barely avoids", "|c50|hardly dodges" };

	BCharacter *attacker = (BCharacter *)character->getBase ( _BCHARACTER );
	BCharacter *defender = (BCharacter *)obj->getBase ( _BCHARACTER );

	if ( !defender || !attacker )
		return 0;

	WorldObject *theWeapon = NULL;
	int hasWeapon = 0;
	Weapon weapon; // weapon object

	if ( character->curWeapon && !character->curWeapon->destructing ) {

    	BWeapon *bWeapon = (BWeapon *)character->curWeapon->getBase ( _BWEAPON );

		if ( bWeapon ) {
			weapon = *(Weapon *)bWeapon;
		 	theWeapon = character->curWeapon;
			hasWeapon = 1;
		}
		else 
			character->curWeapon = NULL;
	}

	if ( !character->curWeapon || character->curWeapon->destructing ) {

		if ( character->hands ) {
			weapon = *character->hands;
			theWeapon = character;
			hasWeapon = 0;
		}
		else {
			theWeapon = character;
			hasWeapon = 0;
		}
	}

	char output[1024] = "", str[1024] = "";

	int distance = getDistance ( character->combatX, character->combatY, obj->combatX, obj->combatY );
	int range = character->combatRange();

	if ( character->canMove() && (distance < range) ) {
		int numToMove = range;

		int startX = character->combatX - numToMove, endX = character->combatX + numToMove;
		int startY = character->combatY - numToMove, endY = character->combatY + numToMove;

		int destX = -1, destY = -1, bestDist = 0;

		for ( int theX=startX; theX<endX; theX++ ) {
			for ( int theY=startY; theY<endY; theY++ ) {
				if ( !character->combatGroup->isOccupied ( theX, theY ) ) {
					int theDist = getDistance ( obj->combatX, obj->combatY, theX, theY );

					if ( (theDist > bestDist && theDist <= range) || ((theDist == bestDist) && random ( 0, 1 )) ) {
						bestDist = theDist;
						destX = theX;
						destY = theY;
					}
				}
			}					
		}

		if ( destX != -1 && destY != -1 ) {
			CombatMove *move = new CombatMove ( character, destX, destY, 0 );
			move->doit ( movie );
			delete move;
		}
	}

	int damage = 0, fireDamage = 0, coldDamage = 0, acidDamage = 0, lightningDamage = 0, poisonDamage = 0, staminaDamage = 0, stealStaminaDamage = 0, experienceDamage = 0, stealExperienceDamage = 0, stealLifeDamage = 0, rustDamage = 0, etherealDamage = 0, stunDamage = 0;

	RMPlayer *targetPlayer = obj->player;

	int weaponSkill = character->getSkill ( weapon.skillType );
	int criticalHitSkill = character->getSkill ( _SKILL_CRITICAL_STRIKING );
	int dodgeSkill = canDodge? obj->getSkill ( _SKILL_ACROBATICS ) : 0;
	int blockSkill = canBlock? obj->getSkill ( _SKILL_SHIELD_USE ) : 0;
	int attackerSkill = character->canAttack()? gSkillInfo[weapon.skillType][weaponSkill].ability : 0;
	int defenderSkill = obj->canDodge()? gSkillInfo[_SKILL_ACROBATICS][dodgeSkill].ability : 0;
	int blockerSkill = obj->canBlock()? gSkillInfo[_SKILL_SHIELD_USE][blockSkill].ability : 0;

	// see if we miss outright...
	int nMissSkill = 15;

	int didMiss = 0; //opposedRoll ( nMissSkill, attackerSkill );

	// can we see our target?
	if ( !didMiss && !character->CanSee ( obj ) ) {
		if ( random ( 1, 100 ) >= 50 )
			didMiss = 1;
	}

	// give exceptional dexterity bonuses to defender
	int dex = obj->calcDexterity();

	// for each dexterity point over 10 add a 15 percent bonus to skill
	if ( dex > 10 ) {
		int percent = (dex - 10) * 15;
		defenderSkill += (defenderSkill * percent) / 100;
		blockerSkill += (blockerSkill * percent) / 100;
	} else if ( dex < 10 ) {
		int percent = (10 - dex) * 15;
		defenderSkill -= (defenderSkill * percent) / 100;
		blockerSkill -= (blockerSkill * percent) / 100;
	}

	// give exceptional dexterity bonuses to attacker
	dex = character->calcDexterity();

	// for each dexterity point over 10 add a 15 percent bonus to skill
	if ( dex > 10 ) {
		int percent = (dex - 10) * 15;
		attackerSkill += (attackerSkill * percent) / 100;
	} else if ( dex < 10 ) {
		int percent = (10 - dex) * 15;
		attackerSkill -= (attackerSkill * percent) / 100;
	}

	int didHit = opposedRoll ( attackerSkill, defenderSkill );
	int criticalHit = opposedRoll ( gSkillInfo[_SKILL_CRITICAL_STRIKING][criticalHitSkill].ability, obj->calcDexterity() + defenderSkill );

	damage += random ( weapon.minDamage, weapon.maxDamage );

	// factor in weapon health

#if 0
	if ( hasWeapon && !weapon.isMissile ) {
		int percent = (theWeapon->health * 100) / theWeapon->healthMax;

		if ( percent >= 50 ) {
			percent = 100 - ((100 - percent) / 10);
		} 
		else {
			percent = 60 - (50 - percent);
		}
		damage = (damage * percent) / 100;
	}
#endif

	if ( damage < 1 ) 
		damage = 1;

	// handle increased damage for berserked attackers
	int isBerserked = character->hasAffect ( _AFF_BERSERK )? 1 : 0;

	if ( character->playerControlled ) 
		{
		int charInt = character->calcIntelligence();
		int charStr = character->calcStrength();
		int charDex = character->calcDexterity();
		int charEnd = character->calcEndurance();

		switch ( weapon.skillType ) {
   				case _SKILL_SHORT_SWORD: {
   					int diff = charInt - 5;		// 10

   					if ( diff < 0 ) 
   					{
					int percent = 100 + (8 * diff);		// 5
					damage = (damage * percent) / 100;
					} 
		
					else if ( diff > 0 ) {
					int percent = 100 + (13 * diff);	// 13
					damage = (damage * percent) / 100;
					}
   				}
   				break;

   				case _SKILL_LONG_SWORD: {
   						int diff = (charStr + charDex) - 30; // 20

   					if ( diff < 0 ) {
					int percent = 100 + (7 * diff);		// 4
					damage = (damage * percent) / 100;
					} 
		
					else if ( diff > 0 ) {
					int percent = 100 + (13 * diff);	// 10
					damage = (damage * percent) / 100;
					}
   				}
   				break;

   				case _SKILL_TWOHANDED_SWORD: {
   						int diff = (charStr + charDex) - 30; // 20

   					if ( diff < 0 ) {
					int percent = 100 + (8 * diff);		// 5
					damage = (damage * percent) / 100;
					} 
		
					else if ( diff > 0 ) {
					int percent = 100 + (14 * diff);	// 10
					damage = (damage * percent) / 100;
					}
   				}
   				break;
   
   				case _SKILL_DAGGER: {
   						int diff = charDex - 10;

   					if ( diff < 0 ) {
					int percent = 100 + (7 * diff);		// 5
					damage = (damage * percent) / 100;
					} 
		
					else if ( diff > 0 ) {
					int percent = 100 + (16 * diff);	// 13
					damage = (damage * percent) / 100;
					}
   				}
   				break;
   
   				case _SKILL_AXE: {
   						int diff = charStr - 10;

   					if ( diff < 0 ) {
					int percent = 100 + (9 * diff);		// 5
					damage = (damage * percent) / 100;
					} 
		
					else if ( diff > 0 ) {
					int percent = 100 + (14 * diff);	// 13
					damage = (damage * percent) / 100;
					}
   				}
   				break;
   
   				case _SKILL_CLUB: {
   					int	diff = charStr - 10;

   					if ( diff < 0 ) {
					int percent = 100 + (10 * diff);	// 5
					damage = (damage * percent) / 100;
					} 
		
					else if ( diff > 0 ) {
					int percent = 100 + (18 * diff);	// 13
					damage = (damage * percent) / 100;
					}
   				}
   				break;
   
   				case _SKILL_MACE: {
   						int diff = charStr - 10;

   					if ( diff < 0 ) {
					int percent = 100 + (8 * diff);		// 5
					damage = (damage * percent) / 100;
					} 
		
					else if ( diff > 0 ) {
					int percent = 100 + (15 * diff); 	// 13
					damage = (damage * percent) / 100;
					}
   				}
   				break;
   
   				case _SKILL_UNARMED: {
   						int diff = charEnd - 5;			// 8

   					if ( diff < 0 ) {
					int percent = 100 + (8 * diff);		// 6
					damage = (damage * percent) / 100;
					} 
		
					else if ( diff > 0 ) {
					int percent = 100 + (16 * diff);	// 16
					damage = (damage * percent) / 100;
					}
   				}
   				break;
   
   				case _SKILL_THROWING: {
   						int diff = charDex - 10;

   					if ( diff < 0 ) {
					int percent = 100 + (7 * diff);		// 5
					damage = (damage * percent) / 100;
					} 
		
					else if ( diff > 0 ) {
					int percent = 100 + (15 * diff);	// 13
					damage = (damage * percent) / 100;
					}
   				}
   				break;
   
   				case _SKILL_MAUL: {
   						int diff = (charStr + charDex) - 30; // 20

   					if ( diff < 0 ) {
					int percent = 100 + (9 * diff);		// 8
					damage = (damage * percent) / 100;
					} 
		
					else if ( diff > 0 ) {
					int percent = 100 + (18 * diff);	// 16
					damage = (damage * percent) / 100;
					}
   				}
   				break;

				case _SKILL_BROADSWORD: {
   						int diff = (charStr) - 10;

   					if ( diff < 0 ) {
					int percent = 100 + (9 * diff);		// 7
					damage = (damage * percent) / 100;
					} 
		
					else if ( diff > 0 ) {
					int percent = 100 + (18 * diff);	// 15
					damage = (damage * percent) / 100;
					}
   				}
   				break;
   			}
		}


	// count the number of damage types that the weapon does 
	int numDamages = 0;

	// this table holds all of the _AFF_DAMAGE_XXX types that this weapon has
	int extraDmgTable[_AFF_DAMAGE_MAX];

	for ( int i=_AFF_DAMAGE_NORMAL; i<_AFF_DAMAGE_MAX; i++ ) {
		if ( theWeapon->hasAffect ( i ) ) {
			extraDmgTable[numDamages] = i;
			numDamages++;
		}
	}

	// this is the chosen damage type to apply during this attack
	int activeDamageType = _AFF_DAMAGE_NORMAL;

	if ( !numDamages ) 
		numDamages = 1;

	if ( character->playerControlled ) {
		int extraDamage = (damage * (15 * (numDamages - 1))) / 100;
		damage += extraDamage;
	}

	// modify the damage based on alignment...
	switch ( obj->coarseAlignment() ) {
		case _ALIGN_EVIL:
			damage = attacker->ApplyEvilMDMMod ( damage );

			break;
		case _ALIGN_GOOD:
			damage = attacker->ApplyGoodMDMMod ( damage );

			break;
	}

	if ( weapon.isMissile && obj->hasAffect ( _AFF_DAMAGE_MISSILE, _AFF_TYPE_RESISTANCE ) )
		damage /= 2;

	// handle ethereal attacks...
	if ( obj->hasAffect ( _AFF_DAMAGE_ETHEREAL ) )
		didMiss = 1;

	// handle checking for phased attacker...
	if ( attacker->TestMeleePhasing() )
		didMiss = 1;

	if ( defender->TestMeleePhasing() )
		didMiss = 1;

	WorldObject *theShield = obj->getWornOn ( _WEAR_SHIELD );

	if ( didMiss ) {
		movie->putByte ( weapon.isMissile? _MOVIE_MISSILE : _MOVIE_ATTACK );
		movie->putLong ( character->servID );
		movie->putLong ( obj->servID );
		movie->putByte ( _ATTACK_MISS );

		sprintf ( sizeof ( str ), str, "|c248|%s misses %s.\n", getName(), obj->getName() );
		strcat ( output, str );
	} 
	
	else if ( didHit ) {
		if ( canBlock )
			obj->numBlocks++;
			
		int hitArmor = 0;

		// check for shield hit
		if ( theShield && !opposedRoll ( attackerSkill, blockerSkill ) ) {
			movie->putByte ( weapon.isMissile? _MOVIE_MISSILE : _MOVIE_ATTACK );
			movie->putLong ( character->servID );
			movie->putLong ( obj->servID );
			movie->putByte ( _ATTACK_HIT_ARMOR );

			obj->damageArmor ( _AFF_DAMAGE_NORMAL, character, theWeapon, theShield, output, movie );
			int damageMod = 10;
			hitArmor = 2;

			if ( isNPC && (weapon.skillType == _SKILL_UNARMED) ) {
				switch ( weapon.damageType ) {
					case dtPierce: {
						damageMod = 40; //25;
					}
					break;

					case dtSlash: {
						damageMod = 60; //45;
					}
					break;

					case dtBludgeon: {
						damageMod = 80; //65;
					}
					break;
				}
			} else {
   				// handle pass-through damage on the shield
   				switch ( weapon.skillType ) {
   					case _SKILL_SHORT_SWORD: {
   						damageMod = 40; //20;
   					}
   					break;

   					case _SKILL_LONG_SWORD: {
   						damageMod = 45; //25;
   					}
   					break;

   					case _SKILL_TWOHANDED_SWORD: {
   						damageMod = 50; //30;
   					}
   					break;
   
   					case _SKILL_DAGGER: {
   						damageMod = 30; //0;
   					}
   					break;
   
   					case _SKILL_AXE: {
   						damageMod = 55; //35;
   					}
   					break;
   
   					case _SKILL_CLUB: {
   						damageMod = 55; //35;
   					}
   					break;
   
   					case _SKILL_MACE: {
   						damageMod = 50; //30;
   					}
   					break;
   
   					case _SKILL_UNARMED: {
   						damageMod = 25; //0;
   					}
   					break;
   
   					case _SKILL_THROWING: {
   						damageMod = 40; //20;
   					}
   					break;
   
   					case _SKILL_MAUL: {
   						damageMod = 70; //50;
   					}
   					break;

					case _SKILL_BROADSWORD: {
   						damageMod = 50; //35;
   					}
   					break;
   				}

   				damage = (damage * damageMod) / 100;
   			}
   		}

		if ( !hitArmor ) {
			// handle hitting a player
   			if ( obj->playerControlled ) {

   				int bodyAreas[] = { _WEAR_HEAD, _WEAR_NECK, _WEAR_CHEST, _WEAR_CHEST, _WEAR_CHEST, _WEAR_CHEST, _WEAR_CHEST, _WEAR_LEGS, _WEAR_LEGS, _WEAR_LEGS, _WEAR_LEGS, _WEAR_FEET, _WEAR_BANDS };

   				// pick a random area of the body to hit 12 = number of locations above
   				int area = bodyAreas[random ( 0, 12 )];

   				// see if there is any armor there
   				WorldObject *armor = obj->getWornOn ( area );

   				if ( armor ) {
   					int armorRoll = random ( 1, 100 );
   					//this just gets the individual piece AR modified by either IA +10 or defenselessness which is /2.
   					int armorRating = armor->armorRating();
   					//armortype 0 = none, 1 = leather, 2 = chain, 3 = plate
   					int armorType = armor->armorType;

						// apply magical piercing...
						armorRating = attacker->ApplyMeleeArmorPierce ( armorRating );

   					// get the modifiers
   					//removing the armor mod table as we don't want to use it.
   					//int armorMod = gArmorModTbl[weapon.damageType][armorType];

   					//original
   					//int damageMod = gDamagePassTbl[weapon.damageType][armorType];
					//modified to create damage mod based on armortype only
					int damageMod = 0;
					//armortype 0 = none, 1 = leather, 2 = chain, 3 = plate
					switch ( armorType ) {
   					//the damgeMod numbers is the percent that they will take, so 90 means they take 90% of damage.
   						case 0: {
							int roll = random ( 80, 90 );
   							damageMod = roll;
   						}
   						break;

   						case 1: {
							int roll = random ( 65, 75 );
   							damageMod = roll;
   						}
   						break;
   					
   						case 2: {
							int roll = random ( 40, 50 );
   							damageMod = roll;
   						}
   						break;
   					
   						case 3: {
							int roll = random ( 15, 25 );
   							damageMod = roll;
   						}
   						break;
   					}


					/* removing this section since we don't want to mod the armor values.
   					if ( armorMod > 0 ) {
   						armorRating += (armorRating * armorMod) / 100;
   					} 
   					else if ( armorMod < 0 ) {
   						armorMod = abs ( armorMod );
   						armorRating -= (armorRating * armorMod) / 100;
   					}
   					*/

   					// we hit the armor, let's modify the damage done
   					if ( armorRoll <= armorRating ) {
   						damage = (damage * damageMod) / 100;
   						movie->putByte ( weapon.isMissile? _MOVIE_MISSILE : _MOVIE_ATTACK );
   						movie->putLong ( character->servID );
   						movie->putLong ( obj->servID );
   						movie->putByte ( _ATTACK_HIT_ARMOR );

   						obj->damageArmor ( _AFF_DAMAGE_NORMAL, character, theWeapon, armor, output, movie );
   						hitArmor = 1;
   					}
// Handle proc here
                    BWearable *bwear = (BWearable *)armor->getBase(_BWEAR);

                    if(bwear)
                    {
                        WorldObject *target = bwear->owner;

                        if(bwear->reverseProcID != 0)
                        {
                            if(calcChance(target->calcDexterity()) > 75)
                            {
                                spell_info *spell = &gSpellTable[bwear->reverseProcID];

                                if(spell)
                                {
                                    sprintf ( sizeof ( output ), output, "|c13|%s was struck, pulsing with the incantation '%s'!", bwear->self->getName(), spell->verbal );
                                    target->player->castProc(spell, target, target->servID, target->combatX, target->combatY, movie);
                                }
                            }
                        }
                        if(bwear->spellProcID != 0)
                        {
                            if(calcChance(target->calcDexterity()) > 75)
                            {
                                spell_info *spell = &gSpellTable[bwear->spellProcID];

                                if(spell)
                                {
                                    WorldObject *mob = character->player->character;

                                    sprintf ( sizeof ( output ), output, "|c13|%s pulses with the incantation '%s'!", bwear->self->getName(), spell->verbal );
                                    target->player->castProc(spell, mob, mob->servID, mob->combatX, mob->combatY, movie);
                                }
                            }
                        }
                    }

   				}
			} else {
   				// handle NPC hits
   				int armorRoll = random ( 1, 100 );
   				int armorRating = obj->armorRating();
   				int armorType = obj->armorType;

					// apply magical piercing...
					armorRating = attacker->ApplyMeleeArmorPierce ( armorRating );

   			   					// get the modifiers
   					//removing the armor mod table as we don't want to use it.
   					//int armorMod = gArmorModTbl[weapon.damageType][armorType];

   					//original
   					//int damageMod = gDamagePassTbl[weapon.damageType][armorType];
					//modified to create damage mod based on armortype only
					int damageMod = 0;
					//armortype 0 = none, 1 = leather, 2 = chain, 3 = plate
					switch ( armorType ) {
   						case 0: {
							int roll = random ( 80, 90 );
   							damageMod = roll;
   						}
   						break;

   						case 1: {
							int roll = random ( 65, 75 );
   							damageMod = roll;
   						}
   						break;
   					
   						case 2: {
							int roll = random ( 40, 50 );
   							damageMod = roll;
   						}
   						break;
   					
   						case 3: {
							int roll = random ( 15, 25 );
   							damageMod = roll;
   						}
   						break;
   					}


					/* removing this section since we don't want to mod the armor values.
   					if ( armorMod > 0 ) {
   						armorRating += (armorRating * armorMod) / 100;
   					} 
   					else if ( armorMod < 0 ) {
   						armorMod = abs ( armorMod );
   						armorRating -= (armorRating * armorMod) / 100;
   					}
   					*/


   				// we hit the armor, let's modify the damage done
   				if ( armorRoll <= armorRating ) {
   					damage = (damage * damageMod) / 100;
   					movie->putByte ( weapon.isMissile? _MOVIE_MISSILE : _MOVIE_ATTACK );
   					movie->putLong ( character->servID );
   					movie->putLong ( obj->servID );
   					movie->putByte ( _ATTACK_HIT_ARMOR );
   					obj->damageArmor ( _AFF_DAMAGE_NORMAL, character, theWeapon, NULL, output, movie );
   					hitArmor = 1;
					//logDisplay ( "Struck armor! Reducing outgoing damage!" );
   				}
   			}
   		}

		// we got through the armor, damage the man!
		if ( damage ) { 
			// damage the weapon if we did not hit the armor...
			if ( !hitArmor ) {
				sprintf ( sizeof ( output ), output, "|c248|Didn't strike Armor!|c43| " );
				obj->damageArmor ( _AFF_DAMAGE_NORMAL, character, theWeapon, theWeapon, output, movie );
			}else if ( hitArmor ) {
				sprintf ( sizeof ( output ), output, "|c60|Struck Armor!|c43| " );
			}

			if ( criticalHit ) {
					switch ( random ( 0, ( character->getSkill ( _SKILL_CRITICAL_STRIKING ) ) ) )
				{
					case 0:
						// no skill = no crit bonus
						//damage = damage;
						//sprintf ( sizeof ( output ), output, "|c248|%s almost had a critical strike!|c43| ", getName() );
						//logDisplay ( "Failed Crit - No bonus" );

						if ( !hitArmor ) {
							damage = damage;
							sprintf ( sizeof ( output ), output, "|c248|%s didn't strike armor and almost had a critical strike!|c43| ", getName() );
						}else if ( hitArmor ) {
							damage = damage;
							sprintf ( sizeof ( output ), output, "|c60|%s struck armor and almost had a critical strike!|c43| ", getName() );
						}
					break;
					
					case 1:
						// level one = small bonus
						//damage *= 1.5;
						//sprintf ( sizeof ( output ), output, "|c12|%s lands a critical strike! x1.5|c43| ", getName() );
						//logDisplay ( "Level One Crit - x1.5" );

						if ( !hitArmor ) {
							damage *= 1.5;
							sprintf ( sizeof ( output ), output, "|c248|%s didn't strike armor, but landed a critical strike! x1.5|c43| ", getName() );
						}else if ( hitArmor ) {
							damage *= 1.5;
							sprintf ( sizeof ( output ), output, "|c60|%s struck armor and landed a critical strike! x1.5|c43| ", getName() );
						}
					break;

					case 2:
						// level 2 = small bonus
						//damage *= 1.5;
						//sprintf ( sizeof ( output ), output, "|c12|%s lands a critical strike! x1.5|c43| ", getName() );
						//logDisplay ( "Level Two Crit - x1.5" );

						if ( !hitArmor ) {
							damage *= 1.5;
							sprintf ( sizeof ( output ), output, "|c248|%s didn't strike armor, but landed a critical strike! x1.5|c43| ", getName() );
						}else if ( hitArmor ) {
							damage *= 1.5;
							sprintf ( sizeof ( output ), output, "|c60|%s struck armor and landed a critical strike! x1.5|c43| ", getName() );
						}
					break;

					case 3:
						// level 3 = increased damage
						//damage *= 2;
						//sprintf ( sizeof ( output ), output, "|c14|%s lands a moderate critical strike! x2|c43| ", getName() ); 
						//logDisplay ( "Level Three Crit - x2" );

						if ( !hitArmor ) {
							damage *= 2;
							sprintf ( sizeof ( output ), output, "|c248|%s didn't strike armor, but landed a moderate critical strike! x2|c43| ", getName() );
						}else if ( hitArmor ) {
							damage *= 2;
							sprintf ( sizeof ( output ), output, "|c60|%s struck armor and landed a moderate critical strike! x2|c43| ", getName() );
						}
					break;

					case 4:
						// level 4 = increased damage
						//damage *= 2;
						//sprintf ( sizeof ( output ), output, "|c14|%s lands a moderate critical strike! x2|c43| ", getName() ); 
						//logDisplay ( "Level Four Crit - x2" );

						if ( !hitArmor ) {
							damage *= 2;
							sprintf ( sizeof ( output ), output, "|c248|%s didn't strike armor, but landed a moderate critical strike! x2|c43| ", getName() );
						}else if ( hitArmor ) {
							damage *= 2;
							sprintf ( sizeof ( output ), output, "|c60|%s struck armor and landed a moderate critical strike! x2|c43| ", getName() );
						}
					break;

					case 5:
						// level 5 = maximum damage
						//damage *= 3;
						//sprintf ( sizeof ( output ), output, "|c60|%s lands a heavy critical strike! x3|c43| ", getName() ); 
						//logDisplay ( "Level Five Crit - x3" );

						if ( !hitArmor ) {
							damage *= 3;
							sprintf ( sizeof ( output ), output, "|c248|%s didn't strike armor, but landed a heavy critical strike! x3|c43| ", getName() );
						}else if ( hitArmor ) {
							damage *= 3;
							sprintf ( sizeof ( output ), output, "|c60|%s struck armor and landed a heavy critical strike! x3|c43| ", getName() );
						}
					break;
				}
				
			}

   		// see if all the damage is passed through to a shield affect or not
   		affect_t *shieldAffect = NULL;
   		int absorbedDmg = 0, shieldFailed = 0;
   		char *shieldName = "";

			// make sure we still have an armor object! It may have been destroyed above

	  		if ( (shieldAffect = obj->hasAffect ( _AFF_INVULNERABLE )) ) {
   			if ( shieldAffect->value > 0 ) {
   				shieldName = "invulnerability aura";
					absorbedDmg = ((damage * shieldAffect->value) / 100) + 1;
					damage -= absorbedDmg;
					shieldAffect->value -= 2;
				
					if ( shieldAffect->value < 1 ) {
						shieldAffect->duration = 1;
						shieldFailed = 1;
					}
				}
   		} 

   		else if ( (shieldAffect = obj->hasAffect ( _AFF_GREATER_SHIELD )) ) {
				if ( shieldAffect->value > 0 ) {
					shieldName = "greater shielding aura";
					absorbedDmg = ((damage * shieldAffect->value) / 100) + 1;
					damage -= absorbedDmg;
					shieldAffect->value -= 5;
				
					if ( shieldAffect->value < 1 ) {
						shieldAffect->duration = 1;
						shieldFailed = 1;
					}
				}
   		}

   		else if ( (shieldAffect = obj->hasAffect ( _AFF_SHIELD )) ) {
				if ( shieldAffect->value > 0 ) {
					shieldName = "shielding aura";
					absorbedDmg = ((damage * shieldAffect->value) / 100) + 1;
					damage -= absorbedDmg;
					shieldAffect->value -= 10;
			  
					if ( shieldAffect->value < 1 ) {
						shieldAffect->duration = 1;
						shieldFailed = 1;
					}
				}
   		} 

			int averageDamage = std::max( (damage / numDamages), 1);//(damage / numDamages) >? 1;

			// categorize the damage 
			SET_DAMAGE ( _AFF_DAMAGE_NORMAL, damage );
			SET_DAMAGE ( _AFF_DAMAGE_FIRE, fireDamage );
			SET_DAMAGE ( _AFF_DAMAGE_COLD, coldDamage );
			SET_DAMAGE ( _AFF_DAMAGE_LIGHTNING, lightningDamage );
			SET_DAMAGE ( _AFF_DAMAGE_ACID, acidDamage );
			SET_DAMAGE ( _AFF_DAMAGE_POISON, poisonDamage );
			SET_DAMAGE ( _AFF_DAMAGE_STAMINA, staminaDamage );
			SET_DAMAGE ( _AFF_DAMAGE_STEAL_STAMINA, stealStaminaDamage );
			SET_DAMAGE ( _AFF_DAMAGE_EXPERIENCE, experienceDamage );
			SET_DAMAGE ( _AFF_DAMAGE_STEAL_EXPERIENCE, stealExperienceDamage );
			SET_DAMAGE ( _AFF_DAMAGE_STEAL_LIFE, stealLifeDamage );
			SET_DAMAGE ( _AFF_DAMAGE_RUST, rustDamage );
			SET_DAMAGE ( _AFF_DAMAGE_ETHEREAL, etherealDamage );
			SET_DAMAGE ( _AFF_DAMAGE_STUN, stunDamage );

			int showMsg = 1; 

			if ( !hitArmor ) {
				movie->putByte ( weapon.isMissile? _MOVIE_MISSILE : _MOVIE_ATTACK );
				movie->putLong ( character->servID );
				movie->putLong ( obj->servID );
				movie->putByte ( _ATTACK_HIT );
			}

			// normal impact damage goes first
			if ( damage ) 
				damage = obj->takeDamage ( _AFF_DAMAGE_NORMAL, character, damage, output, movie, 0, 1, showMsg );

			if ( absorbedDmg ) {
				char str[1024];

				if ( shieldFailed ) {
					sprintf ( sizeof ( str ), str, "|c86|%d absorbed (shield failed)|c43| ", absorbedDmg );
				} else {
					sprintf ( sizeof ( str ), str, "|c86|%d absorbed (%dSP)|c43| ", absorbedDmg, shieldAffect->value );
				}	

				strcat ( output, str );
			}

	   	// steal life damage is next
	   	if ( stealLifeDamage ) {
	   		stealLifeDamage = obj->takeDamage ( _AFF_DAMAGE_STEAL_LIFE, character, stealLifeDamage, output, movie , 0, 1, showMsg );
	   		character->takeDamage ( WorldObject::_DAMAGE_HEAL, character, -stealLifeDamage, output, movie, 0, 1 );
	   	}

	   	// fire damage is next
	   	if ( fireDamage ) 
	   		fireDamage = obj->takeDamage ( _AFF_DAMAGE_FIRE, character, fireDamage, output, movie, 0, 1, showMsg );

	   	// cold damage is next
	   	if ( coldDamage )
	   		coldDamage = obj->takeDamage ( _AFF_DAMAGE_COLD, character, coldDamage, output, movie, 0, 1, showMsg );

			// lightning damage is next
			if ( lightningDamage )
				lightningDamage = obj->takeDamage ( _AFF_DAMAGE_LIGHTNING, character, lightningDamage, output, movie, 0, 1, showMsg );

			// acid damage is next
			if ( acidDamage )
				acidDamage = obj->takeDamage ( _AFF_DAMAGE_ACID, character, acidDamage, output, movie, 0, 1, showMsg );

			// poison damage is next
			if ( poisonDamage )
				poisonDamage = obj->takeDamage ( _AFF_DAMAGE_POISON, character, poisonDamage, output, movie, 0, 1 );

			// experience damage is next
			if ( experienceDamage ) 
				experienceDamage = obj->takeDamage ( _AFF_DAMAGE_EXPERIENCE, character, experienceDamage, output, movie, 0, 1 );

		   	// experience stealing damage is next
		   	if ( stealExperienceDamage ) {
	   			stealExperienceDamage = obj->takeDamage ( _AFF_DAMAGE_STEAL_EXPERIENCE, character, stealExperienceDamage, output, movie , 0, 1, showMsg );
	   			character->takeDamage ( WorldObject::_DAMAGE_EXP_GAIN, character, -stealExperienceDamage, output, movie, 0, 1 );
	   		}	
			//if ( stealExperienceDamage ) 
			//	stealExperienceDamage = obj->takeDamage ( _AFF_DAMAGE_STEAL_EXPERIENCE, character, stealExperienceDamage, output, movie, 0, 1 );

   		// stun damage is next
   		if ( stunDamage )
   			stunDamage = obj->takeDamage ( _AFF_DAMAGE_STUN, character, stunDamage, output, movie, 0, 1, showMsg );

   		if ( rustDamage )
   			rustDamage = obj->takeDamage ( _AFF_DAMAGE_RUST, character, rustDamage, output, movie, 0, 1, showMsg );

   		int totalDamage = damage + fireDamage + coldDamage + lightningDamage + acidDamage + poisonDamage + staminaDamage + stealStaminaDamage + experienceDamage + stealExperienceDamage + stealLifeDamage + stunDamage;

   		// add the text for the amount of damage
   		char damageStr[64];

   		sprintf ( sizeof ( damageStr ), damageStr, "%d", -totalDamage );
   		movie->putByte ( _MOVIE_INFO );
   		movie->putLong ( obj->servID );
   		movie->putString ( damageStr );

   		if ( !weapon.isMissile ) {
   			// make sure the attacker takes damage from immolation
   			affect_t *affect = NULL; 
   			int immolateDamage = 0;

   			if ( (affect = obj->hasAffect ( _AFF_IMMOLATION_FIRE )) )
			{
   				int theDamage = random ( 1, 6 ) + (affect->value / 2);
   				theDamage = character->takeDamage ( _AFF_DAMAGE_FIRE, character, theDamage, output, movie  );
   				immolateDamage += theDamage;
   			}
   
   			if ( (affect = obj->hasAffect ( _AFF_IMMOLATION_COLD )) )
			{
   				int theDamage = random ( 1, 6 ) + (affect->value / 2);
   				theDamage = character->takeDamage ( _AFF_DAMAGE_COLD, character, theDamage, output, movie );
   				immolateDamage += theDamage;
   			}

   			if ( (affect = obj->hasAffect ( _AFF_IMMOLATION_LIGHTNING )) )
			{
   				int theDamage = random ( 1, 6 ) + (affect->value / 2);
   				theDamage = character->takeDamage ( _AFF_DAMAGE_LIGHTNING, character, theDamage, output, movie );
   				immolateDamage += theDamage;
   			}

			if ( immolateDamage )
			{
				sprintf ( sizeof ( damageStr ), damageStr, "%d", -immolateDamage );
				movie->putByte ( _MOVIE_INFO );
				movie->putLong ( character->servID );
				movie->putString ( damageStr );
			}
		}
		} else {
		 	movie->putByte ( weapon.isMissile? _MOVIE_MISSILE : _MOVIE_ATTACK );
	 		movie->putLong ( character->servID );
		 	movie->putLong ( obj->servID );
		 	movie->putByte ( _ATTACK_HIT_ARMOR );
	 		obj->damageArmor ( _AFF_DAMAGE_NORMAL, character, theWeapon, theShield, output, movie );
		}
	} else {
		movie->putByte ( weapon.isMissile? _MOVIE_MISSILE : _MOVIE_ATTACK );
		movie->putLong ( character->servID );
		movie->putLong ( obj->servID );
		movie->putByte ( _ATTACK_MISS );
		sprintf ( sizeof ( str ), str, "|c50|%s %s %s's attack.\n", obj->getName(), dodgeSuccessTbl[random ( 0, 5 )], getName() );
		strcat ( output, str );
	}

	putMovieText ( character, movie, "%s", output );

	character->numAttacks++;

	if ( canDodge )
		obj->numDodges++;

	// determine if retaliation is allowed...
	int canRetaliate = 1;

	// SNTODO: remove this RSN...
	if ( obj->hasAffect ( _AFF_DAMAGE_ETHEREAL ) ) {
		canRetaliate = 0;
	}

	int isOpposition = character->opposition->contains ( obj )? 1 : 0;
	int isBerserk = (character->hasAffect ( _AFF_BERSERK ) || character->hasAffect ( _AFF_CONFUSION ))? 1 : 0;

	// if my friend is attacking me, and I'm not berserk then no retaliation
	if ( !isOpposition && !isBerserk )
		canRetaliate = 0;

	// don't retaliate against summoned creatures unless target is summoned 
//	if ( character->summoned && !obj->summoned )
//		canRetaliate = 0;

	int destX = obj->combatX, destY = obj->combatY;

	if ( obj->hasAffect ( _AFF_SHIFT ) ) {

		char tries = 20;
		int destX = -1, destY = -1, bestDist = 0;

		while( (destX == -1 || destY == -1) && tries-- ) {
			int numToMove = random ( 5, 20 );

			int startX = obj->combatX - numToMove, endX = obj->combatX + numToMove;
			int startY = obj->combatY - numToMove, endY = obj->combatY + numToMove;

			for ( int theX=startX; theX<endX; theX++ ) {
				for ( int theY=startY; theY<endY; theY++ ) {
					if ( !obj->combatGroup->isOccupied ( theX, theY ) ) {
						int theDist = getDistance ( obj->combatX, obj->combatY, theX, theY );

						if ( (theDist > bestDist) || ((theDist == bestDist) && random ( 0, 1 )) ) {
							bestDist = theDist;
							destX = theX;
							destY = theY;
						}
					}
				}					
			}
		}

		if ( destX != -1 && destY != -1 ) {
			if( character->combatGroup->positionCharacter ( obj, destX, destY ) == -1 ) {
				//shift failed!
				logInfo( _LOG_ALWAYS, "%s:%d RMPlayer::attack - Shift teleport failed: character %s of %s (servID %d), current pos(%d, %d) point servID: %d ; target pos(%d, %d) point servID: %d)", __FILE__, __LINE__, obj->getName(), obj->classID, obj->servID, obj->combatX, obj->combatY, character->combatGroup->grid[obj->combatX][obj->combatY], destX, destY, character->combatGroup->grid[destX][destY] );
			} else {
                movie->putByte ( _MOVIE_COMBAT_TELEPORT );
				movie->putLong ( character->servID );
				movie->putLong ( obj->servID );
				movie->putByte ( obj->combatX );
				movie->putByte ( obj->combatY );
			}
		}
	}

	// determine if we should retaliate or not...
	if ( canRetaliate ) {
		if ( retaliate && obj->canAttack() && obj->canHit ( character ) && (obj->health > 0) ) 
			obj->player->attack ( character, movie, 0 );
	}

	return 0;
}

void RMPlayer::die ( PackedData *packet, WorldObject *killer )
{
	if ( !character || !character->player || character->hasAffect( _AFF_JAIL ) )
		return;
		
	// if character is sitting, they have to stand up to die
	if ( character->sittingOn ) {
		character->sittingOn->beStoodUpOn ( character );
		character->sittingOn = NULL;
	}

	PackedMsg response;
	PackedData *thePacket;

	if ( packet )
		thePacket = packet;
	else
		thePacket = &response;

	if ( !packet ) {
		thePacket->putLong ( character->servID );
		thePacket->putLong ( character->room->number );
	}

	// put the fact that this player died
	thePacket->putByte ( _MOVIE_DIE );
	thePacket->putLong ( character->servID );

	// update kill counts
	int playerIsKiller = 0;
	int validKiller = 0;

	if ( killer && killer->player && ( killer != character ) ) {
		validKiller = 1;
		if ( validKiller && !killer->player->isNPC ) 
			playerIsKiller = 1;
	}

	if ( playerIsKiller ) {
		BCharacter *bchar = (BCharacter *)killer->getBase ( _BCHARACTER );
		
		if ( bchar ) {
			if ( isNPC )
				bchar->npcKills++;
			else
				bchar->playerKills++;
		}
	}

	CombatGroup *pGroup = character->combatGroup;

	int allowCombat = zone? zone->allowCombat() : 0;
	int tourneyZone = allowCombat & _COMBAT_TOURNEY;
	int bDeadlyZone = pGroup && pGroup->pvp && (allowCombat & _COMBAT_ARENA);
	int bNoDropTreasure = allowCombat & _COMBAT_NO_TREASURE;
	int bNoAlignmentChange = allowCombat & _COMBAT_NO_ALIGNMENT;

	if ( !tourneyZone && character->combatGroup && !character->summoned ) {

		if ( character->combatGroup->attackers.contains ( character ) ) {
			// 
			// calculate the level of the character if it is a player
			//
			if ( character->playerControlled ) {
				BCharacter *bchar = (BCharacter *)character->getBase ( _BCHARACTER );
				int level = bchar->getLevel();

				character->combatGroup->attackerLevel += level;
			} else {
				character->combatGroup->attackerLevel += character->level;
			}
		} else {
			// 
			// calculate the level of the character if it is a player
			//
			if ( character->playerControlled ) {
				BCharacter *bchar = (BCharacter *)character->getBase ( _BCHARACTER );
				int level = bchar->getLevel();
				character->combatGroup->defenderLevel += level;
			} else {
				character->combatGroup->defenderLevel += character->level;
			}
		}

		// modify alignment of the player...
		if ( !bNoAlignmentChange && playerIsKiller ) {
			// modify killer's alignment based on mine.
			int monsterAlign = character->alignment;
			int oldAlign = killer->alignment;
			int oldC, newC;
	
			if (monsterAlign < 85 || monsterAlign > 170)
			{
				//calculate the current alignment category to see if we've
				//changed
				if      (oldAlign < 29 ) oldC = 0;
				else if (oldAlign < 57 ) oldC = 1;
				else if (oldAlign < 85 ) oldC = 2;
				else if (oldAlign < 113) oldC = 3;
				else if (oldAlign < 143) oldC = 4;
				else if (oldAlign < 171) oldC = 5;
				else if (oldAlign < 199) oldC = 6;
				else if (oldAlign < 227) oldC = 7;    
				else                     oldC = 8;
	
				//adjust the players alignment if they killed a monster
				//that is 'good' or 'evil'
				if ( monsterAlign > 170 )
					killer->alignment -= 2;
				else //monsterAlign is < 85, due to if clause above
					killer->alignment += 2;

				if (killer->alignment > 255) killer->alignment = 255;
				else if (killer->alignment < 0) killer->alignment = 0;
	
				int newAlign = killer->alignment;
				
				if      (newAlign < 29)  newC = 0;
				else if (newAlign < 57)  newC = 1;
				else if (newAlign < 85)  newC = 2;
				else if (newAlign < 113) newC = 3;
				else if (newAlign < 143) newC = 4;
				else if (newAlign < 171) newC = 5;
				else if (newAlign < 199) newC = 6;
				else if (newAlign < 227) newC = 7;
				else                     newC = 8;

				// if player's alignment has changed, tell them.
				if (oldC != newC) switch( newC ) {
					case 0: //Demonic
						roomMgr->sendPlayerText ( killer->player, "|c60|Your heart grows dark as pitch and your thoughts revel in visions of demonic mayhem.|c43|" );
						break;
					case 1: //Evil
						roomMgr->sendPlayerText ( killer->player, "|c60|Visions of evil acts linger in your mind.|c43|" );
						break;
					case 2: //Malevolent
						roomMgr->sendPlayerText ( killer->player, "|c60|Malevolent glee fills your mind.|c43|" );
						break;
					case 3: //Malignant
						roomMgr->sendPlayerText ( killer->player, "|c20|You suddenly view malignant deeds as practical.|c43|" );
						break;
					case 4: //Neutral
						roomMgr->sendPlayerText ( killer->player, "|c20|Walking in a truly neutral balance is your goal.|c43|" );
						break;
					case 5: //Benign
						roomMgr->sendPlayerText ( killer->player, "|c20|A benign approach to life appeals to you.|c43|" );
						break;
					case 6: //Benevolent
						roomMgr->sendPlayerText ( killer->player, "|c23|You view the world in a benevolent spirit.|c43|" );
						break;
					case 7: //Good
						roomMgr->sendPlayerText ( killer->player, "|c23|Visions of good acts warm your heart.|c43|" );
						break;
					case 8: //Beatific
						roomMgr->sendPlayerText ( killer->player, "|c23|Your heart rejoices in the glow of beatific love.|c43|" );
						//character->addAffect ( _AFF_MARK_ENID, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, -1, 0, NULL );
						break;
				}
			}
		}
	} 

	// only lose experience if not an NPC and my killer was not a player
	if ( character->level > 2 && !isNPC && !tourneyZone ) {
		BCharacter *bchar = (BCharacter *)character->getBase ( _BCHARACTER );

		if ( bchar ) {
			if ( !pGroup || !pGroup->pvp ) {
				bchar->gainExperience ( -1000, thePacket );
			}

			else if ( bDeadlyZone ) {
				bchar->gainExperience ( -10000, thePacket );
			}
		}
	}

	// If a player kills an innocent player, that player's group gets criminal status.
	// If an innocent player kills a criminal player, that player's group gets the 
	// bounty ( if any ) on the criminal's head and the criminal either loses criminal 
	// status ( pickpocketting ) or murder count is reduced until zero ( upon which 
	// the criminal status is removed).

	// Make sure killed character is not an npc. If character is poisoned 
	// by spell or killed by a summoned NPC that counts too.

	int poisoned = 0;

	if ( !validKiller && character->hasAffect ( _AFF_POISONED, _AFF_TYPE_NORMAL, _AFF_SOURCE_GIFT ) )
		poisoned = 1;

	if ( !tourneyZone && !isNPC && character->combatGroup && character->combatGroup->pvp && (validKiller || poisoned) ) {
		CrimeData* victimCrime = getCrimeData();

		// Is the killed character an innocent player? If so, then this was murder. 
		// Give the killers wanted status. Killer must be an attacker to become criminal, 
		// Self defense does not count

		int victimCriminal = victimCrime? victimCrime->criminal: 0;

		// not possible to murder if self defense
		if ( !victimCriminal && !character->combatGroup->attackers.contains ( character ) ) { 
			// scan through the killer's group, giving all players criminal status 
			LinkedElement *element = character->opposition->head();

			while ( element ) {

				WorldObject *killerObj = (WorldObject *)element->ptr();
				element = element->next();

				// opponent must be player controlled to get wanted status

				if ( killerObj->playerControlled ) {
					killerObj->updateWanted ( killerObj, _CRIME_MURDER );

					// put the movie command to tell the murderer what he did
					thePacket->putByte ( _MOVIE_MURDER );
					thePacket->putLong ( killerObj->servID );
					thePacket->putLong ( character->servID );
					thePacket->putLong ( 1 );
					thePacket->putLong ( 0 );
				}
			}
		}

		// if the killed character is a criminal and all opponents are 
		// innocent, remove the killed character's criminal status. In the 
		// case of murder, just remove one murder.

		int killerIsInnocent = 1;
		int bountyShareCount = 0;
		int bountyShare = 0;

		LinkedElement *element = character->opposition->head();

		while ( element ) {

			WorldObject *killerObj = (WorldObject *)element->ptr();
			element = element->next();

			// opponent must be player controlled
			if ( killerObj->playerControlled ) {
				CrimeData* crime = killerObj->player->getCrimeData();

				if ( crime->criminal ) {
					killerIsInnocent = 0;
					break;
				} else {
					bountyShareCount++;
				}
			}
		}

		if ( victimCriminal && killerIsInnocent ) {
			// distribute bounty if any
			if ( bountyShareCount ) {
				bountyShare = victimCrime->bountyOnHead / bountyShareCount;

				if ( bountyShare ) {

					LinkedElement *element = character->opposition->head();

					while ( element ) {

						WorldObject *killerObj = (WorldObject *)element->ptr();
						element = element->next();
	
						// opponent must be player controlled
						if ( killerObj->playerControlled ) {
		  					killerObj->value += bountyShare;
	  						CrimeData* crime = killerObj->player->getCrimeData();
	  						crime->bountyCollected += bountyShare;
							killerObj->player->writeCrimes();
	
	  						// put the movie command to tell the killer that he got some bounty
	  						thePacket->putByte ( _MOVIE_BOUNTY );
		  					thePacket->putLong ( killerObj->servID );
		  					thePacket->putLong ( character->servID );
	  						thePacket->putLong ( bountyShare );
						}
					}
				}
			}
			// clear wanted status and bounty
			character->updateWanted ( character, _CRIME_CLEAR );
		}
	}

	if ( !bNoDropTreasure && !tourneyZone && !character->summoned && (isNPC || character->level > 2) ) {
		int bDidDropSomething = 0;
		int nMoneyDropChance = bDeadlyZone? 25 : 5;

		// drop all of the money that this player has
		// Tom: Removed for Sacred Temple
/*		if ( character->value > 0 ) {
			if ( isNPC || (!bDidDropSomething && (random ( 1, 100 ) < nMoneyDropChance)) ) {
				if ( !bDeadlyZone )
					bDidDropSomething = 1;

				WorldObject *money = new WorldObject ( roomMgr->findClass ( "MoneyBag") );
				money->value = character->value;

				money->x = character->x;
				money->y = character->y;
				money->physicalState = _STATE_MONEY;
				money->isVisible = 1;
				money->addToDatabase();

				if ( character->combatGroup ) {
					character->combatGroup->addObject ( money );
				}

				money->hidden++;
				character->room->addObject ( money, 0 );
				character->room->sendObjInfo ( money, NULL, thePacket );
				money->hidden--;

				character->value -= money->value;
				character->calcWeight();

				thePacket->putByte ( _MOVIE_MONEY_DROP );
				thePacket->putLong ( character->servID );
				thePacket->putLong ( money->value );
				thePacket->putLong ( money->servID );
			}
		}

		if ( character->manaValue > 0 ) {
			if ( isNPC || (!bDidDropSomething && (random ( 1, 100 ) < nMoneyDropChance)) ) {
				if ( !bDeadlyZone )
					bDidDropSomething = 1;

				WorldObject *money = new WorldObject ( roomMgr->findClass ( "ManaBag") );
				money->manaValue = character->manaValue;

				money->x = character->x;
				money->y = character->y;
				money->physicalState = _STATE_MONEY;
				money->isVisible = 1;
				money->addToDatabase();

				if ( character->combatGroup ) {
					character->combatGroup->addObject ( money );
				}

				money->hidden++;
				character->room->addObject ( money, 0 );
				character->room->sendObjInfo ( money, NULL, thePacket );
				money->hidden--;

				character->manaValue -= money->manaValue;
				character->calcWeight();

				thePacket->putByte ( _MOVIE_MONEY_DROP );
				thePacket->putLong ( character->servID );
				thePacket->putLong ( money->value );
				thePacket->putLong ( money->servID );
			}
		}*/

		// take every item that is not a Head out of the character's inventory and
		// place it on the ground
		//
		BContainer *bcontain = (BContainer *)character->getBase ( _BCONTAIN );

		if ( !bcontain )
			return;

		if ( isNPC || !bDidDropSomething ) {
			LinkedList items;
			LinkedElement *element = NULL;

			if ( isNPC ) {
				character->makeItemList ( &items, -1 );
			} else {
				character->makeItemList ( &items, -1 );

				//Remove all gold and mana to drop
				element = items.head();

				while ( element ) {
					LinkedElement *pLastElement = element;

					WorldObject *pObj = (WorldObject *)element->ptr();
					element = element->next();

					if ( (pObj->physicalState & _STATE_MONEY))
						items.delElement ( pLastElement );	
				}
			}

			// pick a random item to drop
			int nDropItemIndex = random ( 0, items.size()-1 );

			element = items.head();

			// we need to be alive while dropping
			character->health = 1;

			// track the current item index...
			int nItemIndex = 0;

			while ( element ) {
				WorldObject *obj = (WorldObject *)element->ptr();
				element = element->next();
	
				BWearable *bwear = (BWearable *)obj->getBase ( _BWEAR );
				BWeapon *bweapon = (BWeapon *)obj->getBase ( _BWEAPON );
	
				int chance = 0;
				
				if ( isNPC ) {
						chance = 100;
				} else {	
					if ( bDeadlyZone ) {
						chance = 25;
					} else {
						if ( nItemIndex >= nDropItemIndex ) {
							chance = 100;
						}
					}
				}

				//do not allow drop of home decorations
				if( obj->physicalState & _STATE_HOUSE_DECOR ) {
					chance = 0;
				}
	
				//do not allow drop of items tagged as 'staff'
				else if( obj->hasAffect( _AFF_STAFF ) ) {
					chance = 0;
				}

				//if this is an NPC, we should let them be able to drop
				//containers that they are not wearing
				//if it's an npc, then the object must be a container and not wearable,
				//or a container and wearable and not worn
				
				if ( (chance > 0) && !obj->getBase ( _BHEAD ) &&
					//!obj->getBase ( _BCONTAIN ) &&
					//if i'm an NPC, and it's a container that can't be worn, or isn't being worn, OR it's not a container, it's OK
					(( isNPC && ( obj->getBase( _BCONTAIN ) && ( !bwear || obj->objectWornOn == -1 ) ) ) || !obj->getBase ( _BCONTAIN )) && (random ( 1, 100 ) <= chance ) ) {
					if ( isNPC ) {
						if ( bwear && bwear->owner ) 
							continue;
	
						if ( bweapon && bweapon->owner )
							continue;
					} else {
						if ( bwear || bweapon ) 
							character->takeOff ( obj, thePacket );
						
						// take the item out of it's inventory...
						character->take ( obj, thePacket );
					}
	
					character->drop ( obj, thePacket );
					bDidDropSomething = 1;

					// only drop one item if not an NPC...
					if ( !isNPC && !bDeadlyZone )
						break;
				}

				nItemIndex++;
			}

			items.release();

			// we can be dead again
			character->health = 0;
		}
	}

	if ( !packet ) {
		thePacket->putByte ( _MOVIE_END );
		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, thePacket->data(), thePacket->size(), character->room );
	}

	//if(character->classID == "RaidBoss")
    //roomMgr->sendPlayerInfo(this, "%s strikes a final blow on %s banishing him from the realm!", killer->player->getName(), character->player->getName());
}

int getDays( RMPlayer *player )
{
	int retVal = 0;

	switch ( player->accountType ) {

		case _TRIAL: 
	  		retVal = _TRIAL_TIME;
		  	break;
		case _90DAY:
	  		retVal = _90DAY_TIME;
		  	break;
		case _12MONTH:
	  		retVal = _EXPIRATION_TIME;
	}

	return retVal;
}

int checkLoginName ( char *loginName, char *password )
{
	if ( !loginName || !password ) {
		return 0;
	}

	int isBad = 0;
	int logLen = 0;
	int passLen = 0;

	logLen = strlen ( loginName );
	passLen = strlen ( password );

	if ( ( logLen < 1 ) || ( logLen > 17 ) || ( passLen < 1 ) || ( passLen > _MAX_PASSWORD_SIZE ) ) {
		return 0;
	}

	// can't have '/', '.', ' ', or some bogus r thing in name
	// rrrrrrrr was the signature of a lag attack program that was being used to deny service

	if ( !strcmp ( loginName, "rrrrrrrr" ) || 
		strchr (loginName, '.' ) || strchr (loginName, '/' ) || 
		strchr (loginName, ' ') )
		return 0;

	// must all be printable character

	for ( int i=0; i < logLen; i++ ) {
		if ( !isprint ( loginName[i] ) ) {
			isBad = 1;
			break;
		}
	}

	if ( !isBad ) {
		for ( int i=0; i < passLen; i++ ) {
			if ( !isprint ( password[i] ) ) {
				isBad = 1;
				break;
			}
		}
	}

	if ( isBad ) {
		return 0;
	}

	return 1;
}

int calcLoginNameSum ( char *str )
{
	if ( !str )
		return 0;

	int sum = 0, miniSum = 0, index = 0;

	while ( *str ) {
		int ch = (int)*str;
		miniSum += ch << (8 * index);

		index++;

		if ( index == 5 ) {
			sum += miniSum;
			miniSum = 0;
		}

		str++;
	}

	sum += miniSum;
	sum += strlen ( str );

	return sum;
}

void genPassword ( char *ptr )
{
	for ( int i=0; i<7; i++ ) {
		int roll = random ( 0, 99 );
		int ch = 0;

		if ( roll < 50 ) 
			ch = random ( (int)'a', (int)'z' );
		else
			ch = random ( (int)'3', (int)'9' );

		if ( ch == 'o' || ch == 'l' )
			i--;
		else
			*ptr++ = (char)ch; 
	}

	*ptr = 0;
}

void RMPlayer::handsOn ( void )
{
	if ( character ) {
		PackedMsg msg;
		msg.putLong ( character->servID );
		msg.putLong ( character->room->number );
		msg.putByte ( _MOVIE_HANDS_ON );
		msg.putByte ( _MOVIE_END );
		roomMgr->sendTo ( _IPC_PLAYER_MOVIE_DATA, msg.data(), msg.size(), this );
	}
}

void RMPlayer::disable ( const char *format, ... )
{
	char output[1024];
	va_list args;

	if ( isNPC ) 
		return;

	va_start ( args, format );
	vsprintf ( sizeof ( output ), output, format, args );
	va_end ( args );

	setAccess ( _ACCESS_DISABLED );
	gDataMgr->disable ( this );

	if ( player ) {
		BPlayer *bplayer = (BPlayer *)player->getBase ( _BPLAYER );
  		gDataMgr->logPermanent ( getLogin(), getName(), _DMGR_PLOG_DISABLED, "%s", output );
	}

	forceLogout();
}

void RMPlayer::suspend ( const char *format, ... )
{
	char output[1024];
	va_list args;

	if ( isNPC ) 
		return;

	va_start ( args, format );
	vsprintf ( sizeof ( output ), output, format, args );
	va_end ( args );

	setAccess ( _ACCESS_SUSPENDED );
	gDataMgr->suspend ( this );

	if ( player ) {
		BPlayer *bplayer = (BPlayer *)player->getBase ( _BPLAYER );
  		gDataMgr->logPermanent ( getLogin(), getName(), _DMGR_PLOG_SUSPENDED, "%s", output );
	}

	if ( room ) {
		PackedMsg response;

		response.putLong ( character->servID );
		response.putLong ( character->room->number );
		response.putByte ( _MOVIE_HANDS_OFF );
		response.putByte ( _MOVIE_END );

		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), room );
	}

	forceLogout();
}

void RMPlayer::setAccess ( int access )
{
	switch ( access ) {
		case _ACCESS_NO_GOSSIP: {
			clearAccess ( _ACCESS_GAGGED );
			gossipBanTime = getseconds() + 3600;
			gDataMgr->revoke ( this, 1 );
		}

		break;

		case _ACCESS_GAGGED: {
			clearAccess ( _ACCESS_NO_GOSSIP );
			gossipBanTime = getseconds() + 43200;
			gDataMgr->gag ( this, 1 );
		}

		break;
	}

	rights |= access;
}

void RMPlayer::clearAccess ( int access )
{
	switch ( access ) {
		case _ACCESS_NO_GOSSIP: {
			gDataMgr->revoke ( this, 0 );
		}

		break;

		case _ACCESS_GAGGED: {
			gDataMgr->gag ( this, 0 );
		}

		break;
	}

	rights &= ~access;
}

int RMPlayer::checkTimedAccess ( int access )
{
	if ( rights & access ) {
		switch ( access ) {
			case _ACCESS_NO_GOSSIP: 
			case _ACCESS_GAGGED: {
				int timeLeft = gossipBanTime - getseconds();
	
				if ( timeLeft < 1 )
					clearAccess ( access );
			}
	
			break;
		}
	}

	return rights & access;
}

int RMPlayer::checkRegistration ( void )
{
	if ( accountType ) {

		if (player)
		{
			char filename[1024];

			BPlayer *bPlayer = (BPlayer *)player->getBase ( _BPLAYER );
			sprintf ( sizeof ( filename ), filename, "../data/registrations/%s", bPlayer->login );
			strlower ( filename );

			if (!exists ( filename )) {
				return 0;
			}
		}
	}
	return 1;
}

int RMPlayer::canControl ( RMPlayer *player )
{
	if ( player == this )
		return 1;

	if ( checkAccess ( _ACCESS_MODERATOR ) && 
		( !player->checkAccess ( _ACCESS_MODERATOR ) || 
			( checkAccess( _ACCESS_DISABLE ) && !player->checkAccess ( _ACCESS_IMPLEMENTOR ) ) ) )

		return 1;

	if ( checkAccess ( _ACCESS_IMPLEMENTOR ) && !player->checkAccess ( _ACCESS_IMPLEMENTOR ) )
		return 1;

	return 0;
}

void describeObject ( WorldObject *character, WorldObject *object, const char *str, char *output, int outputSize )
{
	if ( object ) {

		BDescribed *base = (BDescribed *)object->getBase ( _BDESCRIBED );
		sprintf ( outputSize, output, "%s", str );

		// SNTODO: remove this temporary stuff...
		if ( object->hasAffect ( _AFF_DAMAGE_ETHEREAL ) ) {
			return;
		}
	
		BUse *buse = (BUse *)object->getBase ( _BUSE );
		BCarryable *bcarry = (BCarryable *)object->getBase ( _BCARRY );
		BWeapon *bweapon = (BWeapon *)object->getBase ( _BWEAPON );
		BWearable *bwear = (BWearable *)object->getBase ( _BWEAR );
		BContainer *bcontain = (BContainer *)object->getBase ( _BCONTAIN );
		BOpenable *bopen = (BOpenable *)object->getBase ( _BOPEN );
		BScroll *bscroll = (BScroll *)object->getBase ( _BSCROLL );
		BEntry *bentry =  (BEntry *)object->getBase ( _BENTRY );
	
		if ( bentry && (object->physicalState & _STATE_OCCUPIED) ) {
			char entry[1024];
	
			int placeInLine = 0;
	
			if ( character ) {
				placeInLine = character->player->zone->queueStatus( character->player );
	
				if ( placeInLine > 0 ) {
					sprintf ( sizeof ( entry ), entry, "\n\nYou are number %d in line.", placeInLine );
					strcat ( output, entry );
				}
			}
		}
	
		if ( buse && bcarry ) {
			char uses[1024];
	
			if ( buse->uses > 0 ) {
				sprintf ( sizeof ( uses ), uses, "You can use this item %d more times before recharging it.  This item has a capacity for %d charges.", buse->uses, buse->usesMax );
			} 
	
			else if ( buse->uses == 0 ) {
				sprintf ( sizeof ( uses ), uses, "This item has no more charges left and is useless until you get it recharged.  This item has a capacity for %d charges.", buse->usesMax );
			}
	
			strcat ( output, uses );
		}
	
		if ( object->player && object->player->groupLeader ) {
			char group[1024];
			int groupSize = object->player->groupLeader->group.size();
			sprintf ( sizeof ( group ), group, "  That creature is grouped with %d other %s.", groupSize - 1, (groupSize > 2)? "creatures" : "creature"  );
			strcat ( output, group );
		}
	
		int mask = bwear? ~bwear->mask : 0;
	
		if ( bwear && !bweapon ) {
			char wear[1024];
			sprintf ( sizeof ( wear ), wear, "" );
	
			if ( object->armor ) {
				char buf[512];
				sprintf ( sizeof ( buf ), buf, " This item has an armor rating of %d percent.", object->armorRating() );
	
				strcat ( wear, buf );
			}
	
			strcat ( output, wear );
		}
	
		if ( bweapon && object->health && object->healthMax > 0 ) {
	
   			char weapon[1024];
   	
   			int percent = (object->health * 100) / object->healthMax;
   	
   			sprintf ( sizeof ( weapon ), weapon, "  This is a %s weapon that causes %d to %d points of damage.", (bweapon->isMissile)? "throwing" : (bweapon->hands == 1)? "single-handed" : "two-handed", std::max( 1, bweapon->minDamage), std::max( 1, bweapon->maxDamage) );

//1 >? bweapon->minDamage, 1 >? bweapon->maxDamage );
   	
   			strcat ( output, weapon );
   	
   			if ( !bweapon->isMissile ) {
   				sprintf ( sizeof ( weapon ), weapon, "  This weapon has a range of %d combat %s.", bweapon->distance, (bweapon->distance > 1)? "squares" : "square" );
   				strcat ( output, weapon );
   			}
		}
	
		if ( bcontain && bcarry && ( !bopen || ( bopen && bopen->isOpen() ) ) &&
			bcontain->weight )
		{
			char full[1024];
	
			// remove my own weight from weight calc

			int myWeightCap = bcontain->weightCapacity - bcarry->weight;

			int weightFull = 0;
			
			if ( myWeightCap > 0 )
				weightFull = ( ( bcontain->weight - bcarry->weight ) * 100) / myWeightCap;

			if ( weightFull ) {
				sprintf ( sizeof ( full ), full, " It is holding %d percent of its weight capacity.", weightFull ); 
				strcat ( output, full );
			}

			if ( bcontain->bulk ) {

				int myBulkCap = bcontain->bulkCapacity - bcarry->bulk;

				// remove my own bulk from capacity calc

				int bulkFull = 0;

				if ( myBulkCap > 0 )
					bulkFull = ( ( bcontain->bulk - bcarry->bulk ) * 100 ) / myBulkCap;
		
				if ( bulkFull ) {
					sprintf ( sizeof ( full ), full, " It is filled to %d percent of its bulk capacity.", bulkFull ); 
					strcat ( output, full );
				}
			}
		}
	
		if ( object->player && object->player->isNPC ) {
			char buf[512];
			sprintf ( sizeof ( buf ), buf, "  This is a level %d NPC.", object->level );
			strcat ( output, buf );
		}
	
		if ( bscroll && (bscroll->skill > -1) ) {
			char buf[1024];
			sprintf ( sizeof ( buf ), buf, "  " );
	
			int didIt = 0;
	
			if ( !didIt && object->strength ) {
				if ( object->dexterity ) {
					strcat ( buf, "The attributes used when learning this skill are strength and dexterity." );
					didIt = 1;
				}
	
				else if ( object->intelligence ) {
					strcat ( buf, "The attributes used when learning this skill are strength and intelligence." );
					didIt = 1;
				}
	
				else {
					strcat ( buf, "The attribute used when learning this skill is strength." );
					didIt = 1;
				}
			}
	
			if ( !didIt && object->intelligence ) {
				if ( object->dexterity ) {
					strcat ( buf, "The attributes used when learning this skill are intelligence and dexterity." );
					didIt = 1;
				}
	
				else if ( object->endurance ) {
					strcat ( buf, "The attributes used when learning this skill are intelligence and endurance." );
					didIt = 1;
				}
	
				else {
					strcat ( buf, "The attribute used when learning this skill is intelligence." );
					didIt = 1;
				}
			}
	
			if ( !didIt && object->dexterity ) {
				strcat ( buf, "The attribute used when learning this skill is dexterity." );
				didIt = 1;
			}
	
			strcat ( output, buf );
	
			int theCost = bscroll->calcBPCost ( character );
			sprintf ( sizeof ( buf ), buf, " Based on your current unenhanced attributes, it will cost you %d build %s to learn this skill.  Remember, this cost is calculated based entirely on your base attributes.  Magical enhancements will NOT affect the build point cost.", theCost, (theCost > 1)? "points" : "point" );
	
			strcat ( output, buf );
		}
	
		if ( bcarry && bcarry->weight ) {
			int stoneWeight = bcarry->weight;

			if ( bcontain )
				stoneWeight = bcontain->calculateWeight();

			int stones = stoneWeight / 10;
			int fraction = stoneWeight - (stones * 10);
			char buf[512];
			sprintf ( sizeof ( buf ), buf, "  This item weighs %d.%d stones.", stones, fraction );
			strcat ( output, buf );
		}
	
		if ( object->health && object->healthMax ) {
			char *ratingMsgs[] = { "terrible", "poor", "fair", "good", "very good", "excellent" };
			int percent = (object->health * 100) / object->healthMax;
	
			int rating = percent / 20;
	
			if ( rating > 5 )
				rating = 5;
	
			if ( rating < 0 )
				rating = 0;
	
			char buf[512];
			sprintf ( sizeof ( buf ), buf, "  This is in %s condition (%d%%).", ratingMsgs[rating], percent );
	
			strcat ( output, buf );
		}
	
		if ( mask ) {
			strcat ( output, " (Excludes: " );
	
			LinkedList masks;
	
			if ( (mask & _WEAR_MASK_FEMALE) ) 
				masks.add ( new StringObject ( "females" ) );
	
			if ( (mask & _WEAR_MASK_MALE) ) 
				masks.add ( new StringObject ( "males" ) );
	
			if ( (mask & _WEAR_MASK_GOOD) )
				masks.add ( new StringObject ( "good" ) );
	
			if ( (mask & _WEAR_MASK_NEUTRAL) )
				masks.add ( new StringObject ( "neutral" ) );
	
			if ( (mask & _WEAR_MASK_EVIL) )
				masks.add ( new StringObject ( "evil" ) );
	
			if ( (mask & _WEAR_MASK_ADVENTURER) )
				masks.add ( new StringObject ( "adventurers" ) );
	
			if ( (mask & _WEAR_MASK_WARRIOR) )
				masks.add ( new StringObject ( "warriors" ) );
	
			if ( (mask & _WEAR_MASK_WIZARD) )
				masks.add ( new StringObject ( "wizards" ) );
	
			if ( (mask & _WEAR_MASK_THIEF) )
				masks.add ( new StringObject ( "thieves" ) );
	
			LinkedElement *element = masks.head();
	
			while ( element )
            {
                StringObject *info = (StringObject *)element->ptr();
                element = element->next();

                strcat ( output, info->data );

                if ( element )
                    strcat ( output, ", " );
            }

            strcat ( output, ")" );
        }


         // Want this check at the very end of description

        if(bwear || bweapon)
        {
            strcat(output, "\n");
        }


        char proc[1024];

        if(bwear)
        {

            if(bwear->spellProcID)
            {
                sprintf ( sizeof ( proc ), proc, "\nThis item has a (%d%%) chance to cast a spell on your attacker when hit.\n", bwear->forwardProcChance );
                strcat(output, proc);
            }
            if(bwear->reverseProcID)
            {
                sprintf ( sizeof ( proc ), proc, "\nThis item has a (%d%%) chance to cast a spell on you when hit.\n", bwear->reverseProcChance );
                strcat(output, proc);
			}
        }
        else if (bweapon)
        {
            if(bweapon->spellProcID)
            {
                sprintf ( sizeof ( proc ), proc, "\nThis item has a (%d%%) chance to cast a spell on your attacker when hit.\n", bweapon->forwardProcChance );
                strcat(output, proc);
            }
            if(bweapon->reverseProcID)
            {
                sprintf ( sizeof ( proc ), proc, "\nThis item has a (%d%%) chance to cast a spell on you when hit.\n", bweapon->reverseProcChance );
                strcat(output, proc);
            }
        }
    }
}


int isValidName ( const char* name )
{
	if ( !name )
		return 0;

	int len = strlen ( name );

	if ( len < 1 || len > _MAX_CHAR_NAME )
		return 0;

	int isBad = 0;

	for ( int i=0; i < len; i++ ) {
		if ( !isalnum ( name[i] ) ) {
			isBad = 1;
			break;
		}
	}

	if ( isBad )
		return 0;

	return 1;
}

