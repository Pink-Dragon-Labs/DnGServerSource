/*
	room manager process
	author: Stephen Nichols
*/

#include <sys/time.h>
#include <sys/resource.h>

#include "roommgr.hpp"
#include "../global/fatal.hpp"
#include "mysql.h"
#include "audit.hpp"
#include "globals.hpp"
#include "../global/crash.hpp"
#include "datamgrclient.hpp"
#include "friendmgr.hpp"
#include "scriptmgr.hpp"
#include "../global/msgs.hpp"

int gCrashProgress = 0, gCurMsgType = 0;

void handleCrash ( int signo );

SparseArray *gObjectArray = new SparseArray ( _SERVID_MAX, _OBJECT_CHUNK_SIZE );

// skip "howMany" lines of data in this buffer
inline void skipLines ( int howMany, char **buffer, int *size )
{
	char str[1024];

	for (int i = 0; i < howMany; ++i)
	{
		bufgets ( str, buffer, size, sizeof ( str ) );
	}
}

RMServer *roomMgr = NULL;
int sysStatsFlag = 1;
int gMaxConnections = 1, gSparseArray = 1;
RMPlayer *gActivePlayer = NULL;
Counter gZoneResetCounter;

char	gCrashMessage[1024];
char	gShutdownMessage[1024];
int		gDowntimeMessage = 1;

int gServIDTbl[_SERVID_TBL_SIZE];
int gCurServID = 0;
int gParsingClasses = 0;
int gSpawnCount = 0;
int gSpawnTotal = 0;

int gStartTime = 0;

double gExpBoost = 1.0;
int gLootBoost = 1;

char *gMsgTable[_MAX_MESSAGES];

int gAcceptCounts[_MAX_ACCEPT_COUNTS];

LinkedList gWantedList;
LinkedList gComponentObjects[_SKILL_MAX];

WorldObject *gTreasureTbls[50][1024];

int gHouseExits[] = { 5014, 5014, 5075, 7000, 7109, 5908, 5200, 7311, 5405, 5105, 5605, 5301, 5810, 90019 };

SparseArray *gRoomArray = NULL;

void handleCrash ( int signo )
{
	File file;
	
	gMaxConnections = 0;

//	char *ptr = (char *)malloc ( 100000 );

//	if ( ptr )
//		free ( ptr );
//	else {
//		logInfo ( _LOG_ALWAYS, "Out of memory on crash handler." );
//		return;
//	}

	logInfo ( _LOG_ALWAYS, "signal %d caught", signo );

	if ( gActivePlayer ) {
		logHack ( "AccountID #%d crashed the game.", gActivePlayer->accountID );
		gDataMgr->crasher( gActivePlayer->accountID );
	} else if ( gShutdownPlayer ) {
		logHack ( "%s shutting by %d.", gServerName, ((RMPlayer*) gShutdownPlayer)->accountID );
	} else {
		logHack ( "I do not know who crashed the game." );

	}

	if ( gCrashMessage[0] ) {
		logHack ( gCrashMessage );
	}

	LinkedElement *element = roomMgr->players()->head();

	logInfo ( _LOG_ALWAYS, "Saving Characters." );

	while ( element ) {
		RMPlayer *player = (RMPlayer *)element->ptr();

		if ( player->character ) {
			logInfo ( _LOG_ALWAYS, "writing character %s", player->getName() );
			player->character->writeCharacterData();
		}

		gCrashProgress++;

		element = element->next();
	}

	element = gBuildings.head();

	logInfo ( _LOG_ALWAYS, "Saving Houses." );

	while ( element ) {
		Building *building = (Building *)element->ptr();
		logInfo ( _LOG_ALWAYS, "writing house %s", building->_owner );
		building->writeHouseData();
		gCrashProgress++;

		element = element->next();
	}

    logInfo ( _LOG_ALWAYS, "Closing all the open data manager messages." );

    if ( gDataMgr->handle() != -1 ) {
       while ( gDataMgr->msgQueueSize() ) {
          int result = gDataMgr->sendNextMsg();

		  if ( result == -2 )
             break;
       }
    }

	if ( gActivePlayer ) {
		logHack ( "   which is login {%s} character {%s}.", gActivePlayer->getLogin(), gActivePlayer->getName() );
	} else if ( gShutdownPlayer ) {
		logHack ( "   which is login {%s} character {%s}.", ((RMPlayer*) gShutdownPlayer)->getLogin(), ((RMPlayer*) gShutdownPlayer)->getName() );
	}

//	**************************************************************************
	char* pMsg = IPCStats::display();

	file.open ( "../logs/msgs_crash.txt.%d", getpid() );
	file.truncate();

	file.write ( pMsg, strlen( pMsg ) );

	file.close();
	
//	**************************************************************************
	//
        element = gZones.head();

        int nPlayers = 0;
        int nNPCs = 0;

        while ( element ) {
                Zone *zone = (Zone *)element->ptr();
                element = element->next();

                nPlayers += zone->players.size();
                nNPCs += zone->npcs.size();
                nNPCs += zone->externalNPCs.size();
        }

        int upTime = getsecondsfast() - gStartTime;
        double hoursUp = (double) upTime / 3600.0;

        char output[10240];
	
        sprintf ( sizeof(output), output, "uptime = %0.6f\n\ndungeons spawned = %d\n\nmemory taken = %d bytes  (#%d) > %d dif %d\n\nfile handles = %d\nobjects = %d\naffected objects = %d\n\nhouses = %d/%d\n\nplayers = %d  NPCs = %d", hoursUp, gSpawnCount, gAllocSize, gAllocCount, gLargestAllocSize, g_nAllocations, gFileHandles.val(), roomMgr->_objects.size(), gAffectedObjects.size(), gEmptyBuildings.size(), gBuildings.size(), nPlayers, nNPCs );

        file.open ( "../logs/memoryStat.txt.%d", getpid() );
        file.append();

        file.printf ( output );
        file.printf ( "\n\n\nLine\tfile\tSize\tAlloc\tFree\n" );

        for (int nFound = 0;nFound < g_nAllocations;nFound++) {
                file.printf ( "%d\t%s\t%d\t%d\t%d\n",
                                gAllocations[ nFound ].line,
                                gAllocations[ nFound ].file,
                                gAllocations[ nFound ].nSize,
                                gAllocations[ nFound ].nAlloc,
                                gAllocations[ nFound ].nFree );
        }

        file.printf( "\n\n\n" );
        file.close();

//	**************************************************************************
	//
	logInfo ( _LOG_ALWAYS, "server dump completed..." );
}

int allocateServID ( void ) 
{
	int i = (gCurServID / 32);
	int j = (gCurServID % 32);

	while ( 1 ) {
		// handle clipping the servID to the max
		if ( gCurServID >= _SERVID_MAX ) {
			gCurServID = 0;
			i = j = 0;
		}

		int mask = 1 << j;

		if ( !(gServIDTbl[i] & mask) ) {
			gServIDTbl[i] |= mask;
			gCurServID++;

			int retVal = gCurServID - 1;
			return retVal;
		}

		j++;

		if ( j == 32 ) {
			j = 0;
			i++;
		}

		gCurServID++;
	}
		
	logCrash ( "returning servID of -1" );
	crash();

	return -1;
}

void freeServID ( long servID )
{
	if ( ( servID < -1 ) || ( servID >= _SERVID_MAX ) ) {
		logInfo ( _LOG_ALWAYS, "Invalid servID %d passed to freeServID", servID );
		return;
	}

	int i = servID / 32;
	int j = servID % 32;

	int mask = ~(1 << j);
	gServIDTbl[i] &= mask;
}

void logChatData ( const char *format, ... ) {
	char text[1024];
	va_list args;

	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );
	va_end ( args );

	File *file = new File ( "../logs/chat.log" );

	if ( file->isOpen() ) {
		file->seek ( file->size() );

		char *timeStr = timeToStr();
		file->printf ( "%s: %s\n", timeStr, text );
		free ( timeStr );

		file->close();
	}

	delete file;
}

void loadTreasure ( void )
{
	logDisplay ( "loading treasure tables..." );
	int index = 0;

	for ( int i=0; i<50; i++ ) 
		for ( int j=0; j<1024; j++ )
			gTreasureTbls[i][j] = NULL;

	while ( 1 ) {
		char filename[1024];
		sprintf ( sizeof ( filename ), filename, "../data/treasure.%03d", index );

		FILE *file = fopen ( filename, "rb" );

		if ( file ) {
			char str[1024];
			int j = 0;

			LinkedList objects;

			while ( !feof ( file ) && fgets ( str, 1024, file ) ) {
				char *ptr = strrchr ( str, '\n' );

				if ( ptr )
					*ptr = 0;

				ptr = str;

				while ( *ptr == ' ' )
					ptr++;

				WorldObject *obj = roomMgr->findClass ( ptr );

				if ( !obj )
					logDisplay ( "can't find %s", ptr );

				LinkedElement *element = objects.head();
				int value = obj->netWorth ( 50 );

				while ( element ) {
					WorldObject *theObj = (WorldObject *)element->ptr();
					element = element->next();

					if ( value < theObj->netWorth ( 50 ) ) {
						objects.addBefore ( theObj, obj );
						break;
					}
				}

				if ( !element )
					objects.add ( obj );
			}

			fclose ( file );

			LinkedElement *element = objects.head();

			while ( element ) {
				WorldObject *obj = (WorldObject *)element->ptr();
				element = element->next();

				gTreasureTbls[index][j++] = obj;
			}

			objects.release();

			index++;
		} else {
			return;
		}
	}
}

void loadMessages ( void )
{
	FILE *file = NULL;
	int i;

	logDisplay ( "loading text messages..." );

	for (i=0; i<_MAX_MESSAGES; i++ ) 
		gMsgTable[i] = "no message";

	// load the text message file
	file = fopen ( "../data/lib/text/messages.txt", "rb" );

	char str[1024];
	i = 0;

	if ( file ) {
		while ( !feof ( file ) && fgets ( str, 1024, file ) ) {
			char *ptr = strrchr ( str, '\n' );

			if ( ptr )
				*ptr = 0;

			gMsgTable[i] = strdup ( str );	
			i++;
		}

		fclose ( file );
	}
}

void loadQuests ( void )
{
	FILE *file = NULL;
	char filename[1024], path[1024], fname[1024];
	int retVal = 0;

	logDisplay ( "loading quests..." );

	// load all of the talk tree specification files
	file = fopen ( "../data/lib/quests/quests.dir", "rb" );

	if ( file ) {
		while ( !feof ( file ) && fgets ( fname, 1024, file ) ) {
			char *ptr = strrchr ( fname, '\n' );

			if ( ptr )
				*ptr = 0;

			sprintf ( sizeof ( filename ), filename, "../data/lib/quests/%s.tok", fname );

			File talkFile ( filename );

			if ( talkFile.isOpen() ) {
				int lineNum = 0;

				// load the data and setup the input
				char *buffer = (char *)malloc ( talkFile.size() );
				talkFile.read ( buffer, talkFile.size() );
				int bufferSize = talkFile.size();

				QuestParser parser;
				parser.input = buffer;
				parser.index = 0;

				while ( retVal != -1 && (parser.index < bufferSize) ) {
					lineNum++;

					switch ( parser.parseLine() ) {
						case -1: {
							logInfo ( _LOG_ALWAYS, "\"%s\", line %d: %s", filename, lineNum, parser.error() );
							exit ( 1 );
						}

						break;
					}
				}

				free ( buffer );
			}
		}

		fclose ( file );
	} else {
		fatal ( "cannot access %s!\n", path );
	}
}

void loadTalkTrees ( void )
{
	FILE *file = NULL;
	char filename[1024], path[1024], fname[1024];
	int retVal = 0;

	logDisplay ( "loading talk tree data..." );

	// load all of the talk tree specification files
	file = fopen ( "../data/lib/talktree/talktree.dir", "rb" );

	if ( file ) {
		while ( !feof ( file ) && fgets ( fname, 1024, file ) ) {
			char *ptr = strrchr ( fname, '\n' );

			if ( ptr )
				*ptr = 0;

			sprintf ( sizeof ( filename ), filename, "../data/lib/talktree/%s.tok", fname );

			File talkFile ( filename );

			if ( talkFile.isOpen() ) {
				TalkTree *tree = new TalkTree ( "How can I help you?", 0 );
				int lineNum = 0;

				// load the data and setup the input
				char *buffer = (char *)malloc ( talkFile.size() );
				talkFile.read ( buffer, talkFile.size() );
				int bufferSize = talkFile.size();

				TalkTreeParser parser;
				parser.tree = tree;
				parser.input = buffer;
				parser.index = 0;

				while ( retVal != -1 && (parser.index < bufferSize) ) {
					lineNum++;

					switch ( parser.parseLine() ) {
						case -1: {
							logInfo ( _LOG_ALWAYS, "\"%s\", line %d: %s", filename, lineNum, parser.error() );
							exit ( 1 );
						}

						break;
					}
				}

				free ( buffer );
			}
		}

		fclose ( file );
	} else {
		fatal ( "cannot access %s!\n", path );
	}
}

void saveState(bool crash)
{
	LinkedElement *element = roomMgr->players()->head();

	if(crash)
	{
		logInfo ( _LOG_ALWAYS, "Saving Characters." );
	}

	while ( element ) {
		RMPlayer *player = (RMPlayer *)element->ptr();

		if ( player->character ) {
			if(crash)
			{
				logInfo ( _LOG_ALWAYS, "writing character %s", player->getName() );
			}
			player->character->writeCharacterData();
		}

		if(crash){
			gCrashProgress++;
		}

		element = element->next();
	}

	element = gBuildings.head();

	if(crash)
	{
		logInfo ( _LOG_ALWAYS, "Saving Houses." );
	}

	while ( element ) {
		Building *building = (Building *)element->ptr();
		
		if(crash)
		{
			logInfo ( _LOG_ALWAYS, "writing house %s", building->_owner );
		}
		
		building->writeHouseData();

		if(crash){
			gCrashProgress++;
		}

		element = element->next();
	}
}

void RMServer::handleMessage ( IPCMessage *message )
{
	clock_t nStartTime = clock();

	IPCClient *client = (IPCClient *)message->from();

	switch ( message->type() ) {
		case _IPC_CLIENT_CONNECTED: {
			if ( message->to() != (void *)-1 ) 
				break;

			// let the base class handle the message too
			IPCServer::handleMessage ( message );	

			if ( isValidPtr ( client ) && getPtrType ( client ) == _MEM_CLIENT ) {
				RMPlayer *player = new RMPlayer;

				client->player = player;
				player->owner = client;

				roomMgr->addPlayer ( player );

				client->secure();
			}
		}

		break;

		case _IPC_SECURITY: {
			if ( isValidPtr ( client ) && getPtrType ( client ) == _MEM_CLIENT ) {

				RMPlayer *player = (RMPlayer *)client->player;

				if ( player ) {
					player->secured = TRUE;
				}
			}
		}

		break;

		case _IPC_INN_LOGIN: {
			if ( isValidPtr ( client ) && getPtrType ( client ) == _MEM_CLIENT ) {
				RMPlayer *player = (RMPlayer *)client->player;
	
				if ( player ) {
					if ( player->secured  ) {
		   				PackedData *packet = new PackedData ( message->data(), message->size() );
			   
		   				player->servID = packet->getLong();
		   				player->pkgInfo = packet->getWord();
			   
		   				delete packet;
			
						gActivePlayer = player;
						player->processRequest ( message );
						gActivePlayer = NULL;
					} else {
  						sendSystemMsg ( "System Error", player, "I'm sorry, but you are playing from an unsecured client.  Please download a more recent version of the software.  Press ALT-F4 to exit the game." );
  						player->forceLogout();
					}
				}
			}
		}

		break;

		case _IPC_CLIENT_HUNG_UP: {
			if ( isValidPtr ( client ) && getPtrType ( client ) == _MEM_CLIENT ) {
				RMPlayer *player = (RMPlayer *)client->player;

				if ( player ) {
					gActivePlayer = player;
					player->processRequest ( message );
					gActivePlayer = NULL;

					player->tossing = 1;
					delete player;
				}
			}

			// let the base class handle the message too
			IPCServer::handleMessage ( message );	
		}

		break;

		case _IPC_SERVER_PING: {
			if ( isValidPtr ( client ) && getPtrType ( client ) == _MEM_CLIENT ) {
				RMPlayer *player = (RMPlayer *)client->player;

				if ( player ) {
					player->ping = 1;

					int nTime = getseconds();
					player->pingTime = nTime - player->lastWriteTime;

	   				PackedData *packet = new PackedData ( message->data(), message->size() );
					player->pingClientTime = packet->getWord();

					delete packet;

					// if the ping message comes too fast, hang up...
					if ( player->pingTime < 30 && player->badPingCount > 1 ) {	// WAY too fast just hang up instantly.
						logInfo ( _LOG_ALWAYS, "character %s pinged WAY too fast %d seconds", player->getName(), player->pingTime );
						player->forceLogout();
					} else if ( player->pingTime < 50 ) {	//	Consistantly fast give them a bit of a leaway
						player->badPingCount++;

					if ( player->badPingCount == 5 ) {
						logInfo ( _LOG_ALWAYS, "character %s pinged too fast %d seconds", player->getName(), player->pingTime );
							player->forceLogout();
						}
					}

					player->lastWriteTime = nTime;

					sendACK ( _IPC_SERVER_PING, player );
				}
			}
		}
	
		break;

		case _IPC_PLAYER_CHECK_LOGIN: {
			PackedMsg packet ( message->data(), message->size() );

			unsigned short loginLen=0;
			unsigned short passLen =0;
			char* pLoginName = NULL;
			char* pPassword  = NULL;
			char* pThird     = NULL;

			int nOSID =0;
			int nCount=0;

			bool isValid = true;

			loginLen = packet.getWord();
			if( loginLen && loginLen < 17 )
				pLoginName = packet.getString( loginLen );
			else isValid = false;
			
			if( isValid ) {
				passLen = packet.getWord();
				if( passLen && passLen < 17 ) pPassword = packet.getString( passLen );
				else isValid = false;
			} 

			if( isValid ) {
				nOSID = packet.getLong();
				nCount = packet.getByte();
			
				if ( nCount )
					pThird = packet.getString();

				if ( packet.getWord() != 1031 ) {
					logInfo ( _LOG_ALWAYS, "Old interpretor trying to log in on account %s", pLoginName );

					PackedData ack;
					ack.putLong ( -1 );
					ack.putLong ( 0 );
					ack.putLong ( -1 );

					// calculate the security seed
					int seed = random ( 1, 55000 );
					unsigned int offset = rand();

					ack.putWord ( seed );
					ack.putLong ( offset );

					ack.putByte ( 1 ); //player->checkRegistration() );
					ack.putString ( "You are running on an old interpretor, please do not hack your client!!" );

					roomMgr->sendTo( _IPC_PLAYER_CHECK_LOGIN, ack.data(), ack.size(), (RMPlayer *)client->player );
				} else {
					if ( pLoginName && pPassword && loginLen && passLen ) {

						char* testCh = pLoginName;
						while( isValid && loginLen-- ) {
							if ( ! isprint( *(testCh++) ) ) isValid = false;
						}

						testCh = pPassword;
						while( isValid && passLen-- ) {
							if ( ! isprint( *(testCh++) ) ) isValid = false;
						}

						if ( isValid ) {
							//force logout if this player is logged in and last ping time is
							//more than 4 minutes ago.
							TreeNode *node = gLoginTree.find ( pLoginName );

							if ( node ) {
								RMPlayer* target = (RMPlayer*) node->data;

								if ( target ) {
									int nTime = getseconds() - target->lastWriteTime;

									if ( nTime > ( 180 ) ) { 
										logInfo ( _LOG_ALWAYS, "Forced logout of %s for ghosting %s for %d seconds", pLoginName, target->character? "" : "at character selection screen ", nTime );
										target->forceLogout();

										// Send message to player that we are un-ghosting them and to try again.
										PackedData ack;
										ack.putLong( -1 );
										ack.putLong( 0 );
										ack.putLong( -1 );

										ack.putByte( 1 ); //player->checkRegistration() );
										ack.putLong( 0 );
										ack.putString( "Your account has gone linkdead / ghosted. It has been forced offline, please try logging in again." );
										roomMgr->sendTo( _IPC_PLAYER_CHECK_LOGIN, (char *)ack.data(), ack.size(), (RMPlayer *)client->player );
									} else {
										// create a new login context.
										new LoginContext ( pLoginName, pPassword, nOSID, nCount, pThird, (RMPlayer *)client->player );										                                                  
									}
								}
							} else {
								// create a new login context.
								new LoginContext ( pLoginName, pPassword, nOSID, nCount, pThird, (RMPlayer *)client->player ); 
							}

						} else {
							//either login or password contains non printable characters
							int clientIP = client->IP();
							logHack("got binary data in login/password in _IPC_PLAYER_CHECK_LOGIN - IP is %d.%d.%d.%d", (clientIP & 0xFF000000) >> 24, (clientIP & 0x00FF0000) >> 16, (clientIP & 0x0000FF00) >> 8, (clientIP & 0x000000FF) );
							if( pLoginName ) delete pLoginName;
							if( pPassword  ) delete pPassword;
						}
					} else {
						int clientIP = client->IP();
						logHack("got bad data on _IPC_PLAYER_CHECK_LOGIN - IP is %d.%d.%d.%d", (clientIP & 0xFF000000) >> 24, (clientIP & 0x00FF0000) >> 16, (clientIP & 0x0000FF00) >> 8, (clientIP & 0x000000FF) );
						if( pLoginName ) delete pLoginName;
						if( pPassword  ) delete pPassword;
					}
				}
			} else {

				//apparently there is one case that is valid
				if( ! (loginLen == 0 && passLen == 0 && !pLoginName && !pPassword) ) {

					//corrupt length values in packet
					int clientIP = client->IP();
					logHack("got corrupted length values in _IPC_PLAYER_CHECK_LOGIN - IP is %d.%d.%d.%d", (clientIP & 0xFF000000) >> 24, (clientIP & 0x00FF0000) >> 16, (clientIP & 0x0000FF00) >> 8, (clientIP & 0x000000FF) );
					logHack(" - loginLen = %d  %s", loginLen, pLoginName?"login valid":"login invalid" );
					logHack(" - passLen = %d  %s", passLen, pPassword?"pass valid":"pass invalid" );
				}

				if( pLoginName ) delete pLoginName;
				if( pPassword  ) delete pPassword;
			}
		}

		break;

		default: {
			if ( isValidPtr ( client ) && getPtrType ( client ) == _MEM_CLIENT ) {
				RMPlayer *player = (RMPlayer *)client->player;

				if ( isValidPtr ( player ) && player->secured ) {
					gActivePlayer = player;
					player->processRequest ( message );
					gActivePlayer = NULL;
				} else {
					logHack ( "Got unknown msg %d of %d from %s", message->type(), message->size(), csIPtoChar( client->IP() ) );
				}
			} else {
    			if ( message->to() == (void *)-1 ) {
					logHack ( " invalid message to in timer event!" );
				}

				// toss old houses
				else if ( message->type() == _IPC_TOSS_DEAD_HOUSES ) {
					LinkedElement *element = gEmptyBuildings.head();

					while ( element ) {
						Building *building = (Building *)element->ptr();
						LinkedElement *next = element->next();

						if ( building->disposeDelay < 1 ) {
							int startTime = getsecondsfast();

							if ( building->changed )
								building->writeHouseData();

							int elapsedTime = getsecondsfast() - startTime;

							if ( elapsedTime > 1 ) {
								logInfo ( _LOG_ALWAYS, "writing house '%s' took %d seconds", building->_owner, elapsedTime );
							}

							delete building;
						} else {
							building->disposeDelay--;
						}

						element = next;
					}
				}

				// clean up crowded rooms 
				else if ( message->type() == _IPC_MAINTENANCE_PULSE ) {
					LinkedElement *element = gCrowdedRooms.head();

					while ( element ) {
						RMRoom *room = (RMRoom *)element->ptr();
						element = element->next();

						LinkedElement *elementA = room->objects.head();
						int time = getseconds();

						while ( elementA ) {
							WorldObject *obj = (WorldObject *)elementA->ptr();
							elementA = elementA->next();

							if ( !obj->isZoneObject && ((time - obj->creationTime) >= (15 * 60)) ) {
								roomMgr->destroyObj ( obj, 1, __FILE__, __LINE__ );
							}

							if ( room->objects.size() < 64 )
								break;
						}
					}
				}

				// handle ambushing players...
				else if ( message->type() == _IPC_AMBUSH_PULSE ) {
					LinkedElement *element = gZones.head();

					while ( element ) {
						Zone *zone = (Zone *)element->ptr();
						element = element->next();

						zone->ambushPlayers();
					}
				}

				else if ( message->type() == _IPC_SAVE_STATE ) {
					saveState(false);
				}

				// generate monsters for all my server's zones
				else if ( message->type() == _IPC_MONSTER_PULSE ) {
					LinkedElement *element = gZones.head();

					while ( element ) {
					    Zone *zone = (Zone *)element->ptr();
						element = element->next();

						if ( !zone->isDungeon ) 
							zone->generateMonsters();
					}
				}

				// process the dungeon queue entry and set the reset pulse

				else if ( message->type() == _IPC_DUNGEON_QUEUE_PULSE ) {
					LinkedElement *element = gZones.head();

					while ( element ) {
						Zone *zone = (Zone *)element->ptr();
						LinkedElement *next = element->next();

						if ( zone->isDungeon && zone->entrance ) {
							
							//lets see if this dungeon just became empty
							if ( zone->didClose && !zone->players.size() && !zone->externalNPCs.size() && zone->dungeonShutdownTimer == 0 ) {
								//start the shutdown timer
								zone->dungeonShutdownTimer = 60; //DUNGEON_QUEUE_PULSE is 15 seconds, so 60 = 15minutes
								//logDisplay("DUNGEON_PULSE: starting shutdown timer" );
							}

							//if this dungeoon is in limbo waiting to shut down, decrement the shutdown timer, and shut it down if it reaches zero.
							else if ( zone->didClose && !zone->players.size() && !zone->externalNPCs.size() && zone->dungeonShutdownTimer > 0 ) {
								zone->dungeonShutdownTimer--;

								//logDisplay("DUNGEON_PULSE: decremented shutdown timer, it's now %d", zone->dungeonShutdownTimer );

								if( zone->dungeonShutdownTimer == 0 ) {
									//shut this dungeon down!
									//logDisplay("DUNGEON_PULSE: shutting down dungeon" );
									zone->tossRooms();
									gZones.delElement ( element );

									delete zone;
									gSpawnCount--;
								}
							}

							//if this dungeoon is waiting to shutdown and someone has entered, stop the shutdown
							else if( zone->didClose && zone->dungeonShutdownTimer && ( zone->players.size() || zone->externalNPCs.size() ) ) {
								//logDisplay("DUNGEON_PULSE: stopping shutdown");
								zone->dungeonShutdownTimer = 0;
							}

							// send in characters from queue
							else if ( !zone->didClose && zone->enterFromQueue() ) {
								// close the dungeon, and spawn it
								zone->entrance->beClosed ( NULL );
								zone->spawn();
								zone->didClose = 1;
							}
						} else {
							if ( zone->isDungeon ) {
								logInfo ( _LOG_ALWAYS, "dungeon has no entrance" );
							}
						}

						element = next;
					}
				}

				// process all my character's heals
				else if ( message->type() == _IPC_HEAL_PULSE ) {
					LinkedElement *element = gZones.head();

					while ( element ) {
						Zone *zone = (Zone *)element->ptr();
						element = element->next();

						LinkedElement *elementA = zone->players.head();

						while ( elementA ) {
							RMPlayer *player = (RMPlayer *)elementA->ptr();
							elementA = elementA->next();

							if( player ) {
								if ( player->character && !player->character->combatGroup )
									player->character->heal();
							}
						}
					}
				}

				// process all my character's doits
				else if ( message->type() == _IPC_CHAR_DOIT_PULSE ) {
					LinkedElement *element = gZones.head();

					while ( element ) {
						Zone *zone = (Zone *)element->ptr();
						element = element->next();

						LinkedElement *elementA = zone->players.head();

						while ( elementA ) {
							RMPlayer *player = (RMPlayer *)elementA->ptr();
							elementA = elementA->next();

							if ( player->character && !player->character->combatGroup )
								player->character->doit();
						}
					}
				}

				// process the affected states for all of my objects
				else if ( message->type() == _IPC_PROCESS_AFFECT_PULSE ) {
					LinkedElement *element = gAffectedObjects.head();

					while ( element ) {
						WorldObject *obj = (WorldObject *)element->ptr();
						element = element->next();

						if ( !obj->combatGroup )
							obj->processAffects();
					}
				}

				else if ( message->type() == _IPC_COMBAT_PULSE ) {
					LinkedElement *element = gPendingCombats.head();

					while ( element ) {
						CombatGroup *group = (CombatGroup *)element->ptr();

						LinkedElement *next = element->next();
						gPendingCombats.delElement ( element );

						if ( isValidPtr ( group ) && getPtrType ( group ) == _MEM_COMBAT_GROUP )
							group->processActions();

						element = next;
					}
				}

				else if ( message->type() == _IPC_RESET_PULSE ) {
					Zone *zone = (Zone *)message->to();
					logDisplay ( "zone reset %s", zone->name() );

					zone->reset();
				}

				else if ( message->type() == _IPC_ZONE_RESET_CMD ) {
				}

				else if ( message->type() == _IPC_NPC_PULSE ) {
					// let the scripts do something...
//					if ( g_pScriptMgr ) {
//						g_pScriptMgr->Doit();
//					}

					LinkedElement *element = gDeadCombats.head();

					while ( element ) {
						CombatGroup *group = (CombatGroup *)element->ptr();
						element = element->next();

						gDeadCombats.del ( group );

						if ( isValidPtr ( group ) && getPtrType ( group ) == _MEM_COMBAT_GROUP )
							delete group;
					}

					// an NPC doit message?
					element = gNpcList.head();

					while ( element ) {
						NPC *npc = (NPC *)element->ptr();
						element = element->next();

						if ( npc->aiReady && npc->delay && !npc->tossing ) {
							npc->delay--;

							if ( npc->lastAmbushTime == -1 ) {
								npc->tossing = 1;
								delete npc;
							} else {
								if ( !npc->delay ) {
									gActivePlayer = npc;
									int delay = npc->doit();
									gActivePlayer = NULL;

									if ( (delay == -1) ) {
										npc->tossing = 1;
										delete npc;
									} else {
										npc->delay = delay;
									}
								}
							}
						}
					}
				} else {
					logHack ( "Got an unclaimed NON-CLIENT msg %d of %d", message->type(), message->size() );
				}
			}
		}
		break;
	}

	if ( ( message->type() > -1 ) && ( message->type() < _IPC_MAX_MESSAGES ) ) {
		clock_t nEndTime = clock() - nStartTime;

		IPCStats::Msgs[ message->type() ].addExecTime( nEndTime, message );
	}
}

int RMServer::send ( int type, char *buffer, int size, IPCClient *client )
{
	if ( !client )
		return 0;

	return IPCServer::send ( type, buffer, size, client );
}
	
RMRoom *RMServer::sendRoomInfo ( int number, RMPlayer *player )
{
	RMRoom *room = findRoom ( number );

	if ( room && isValidPtr ( player ) ) {

		PackedData response;

		/* put to and from fields */
		response.putLong ( player->servID );
		response.putLong ( 0 );

		char *titlePtr = room->title? room->title : room->zone->title;

		/* see if the title is needed */
		if ( !player->roomTitle || strcmp ( player->roomTitle, titlePtr ) ) {
			response.putByte ( 1 );
			response.putString ( titlePtr );
		} else {
			response.putByte ( 0 );
		}

		if ( room->zone ) {
			response.putByte ( room->zone->allowCombat() );
		} else {
			response.putByte ( 0 );
		}

		if ( number > _DBR_THRESHOLD ) {
			response.putByte ( random ( 15, 20 ) );
		} else {
			response.putByte ( room->midiFile );
		}

		/* put room information */
		room->buildPacket ( &response );

		sendTo ( _IPC_PLAYER_NEW_ROOM, response.data(), response.size(), player );
	}

	return room;
}

RMRoom *RMServer::sendRoomInfo ( int number, long servID )
{
	Player *player = findPlayer ( servID );

	if ( isValidPtr ( player ) ) {

		RMPlayer *tPlayer = (RMPlayer *)player;
	
		if ( !tPlayer->isNPC )
			return sendRoomInfo ( number, tPlayer );
	}

	return NULL;
}

void RMServer::sendToRoom ( int command, void *msg, int size, RMRoom *room )
{
	if ( room ) 
		sendToRoom ( command, msg, size, room, (RMPlayer *)NULL );
}

void RMServer::sendToRoom ( int command, void *msg, int size, RMRoom *room, RMPlayer *exclusion )
{
	if ( room ) {
		LinkedElement *element = room->head();
	
		while ( element ) {
			RMPlayer *player = (RMPlayer *)element->ptr();
	
			if ( !player->isNPC && player != exclusion )
				sendTo ( command, msg, size, player );
	
			element = element->next();
		}
	}
}

void RMServer::sendToRoom ( int command, void *msg, int size, int number )
{
	RMRoom *room = findRoom ( number );

	if ( room )
		sendToRoom ( command, msg, size, room );
}

void RMServer::sendToList ( int command, void *msg, int size, LinkedList *list, RMPlayer *exclusion )
{
	LinkedElement *element = list->head();

	while ( element ) {
		RMPlayer *player = (RMPlayer *)element->ptr();

		if ( !player->isNPC && player != exclusion && player->character )
			sendTo ( command, msg, size, player );

		element = element->next();
	}
} 

RMRoom *RMServer::findRoom ( int number )
{
	RMRoom *retVal = NULL;

	if ( number < 0 || number > 900000000 )
		return 0;

	retVal = (RMRoom *)gRoomArray->lookup ( number );

	if ( number > _DBR_THRESHOLD ) {
		if ( retVal && retVal->building )
			retVal->building->disposeDelay = 10;
	}

	return retVal;
}

void RMServer::addZone ( Zone *zone )
{
	_zones.add ( zone );
}

void RMServer::delZone ( Zone *zone )
{
	_zones.del ( zone );
}

void RMServer::destroyObj ( WorldObject *object, int notify, char *file, int line )
{
	if ( !object || object->destructing )
		return;

	if ( !object->isWorldObject() ) {
		logInfo ( _LOG_ALWAYS, "%s:%d - invalid object passed to RMServer::destroyObj (0x%x)", file, line, object );
		return;
	}

	if ( object->getBase ( _BHEAD ) )
		logInfo ( _LOG_ALWAYS, "destroying head object %s:%d", file, line );

	object->allowVisible = FALSE;

	// save the state of this object's building's changed flag
	WorldObject *baseOwner = object->getBaseOwner();
	Building *building = baseOwner->room? baseOwner->room->building : NULL;
	int oldChanged = building? building->changed : -1;

	object->forceOff();
	object->forceOut();

	// restore the changed flag
	if ( building )
		building->changed = oldChanged;

	if ( object->ownerID != -1 ) {
		logInfo ( _LOG_ALWAYS, "%s, %s (%d) could not be forced of it's owner", object->classID, object->super, object->servID );
		return;
	}

	if ( object->objectWornOn != -1 || object->objectWieldedOn != -1 ) {
		logInfo ( _LOG_ALWAYS, "%s, %s (%d) worn/wielded object could not be forced off of it's owner %s:%d", object->classID, object->super, object->servID, file, line );
		return;

		if ( object->objectWieldedOn != -1 ) {
			WorldObject *owner = roomMgr->findObject ( object->objectWieldedOn );

			logInfo ( _LOG_ALWAYS, "Adjusting the curWeapon pointer." );

			if ( owner ) 
				owner->curWeapon = NULL;
		}
	}

	if ( object->servID > -1 ) {
		WorldObject *obj = (WorldObject *)gObjectArray->lookup ( object->servID );

		if ( obj != object ) {
			logInfo ( _LOG_ALWAYS, "WorldObject (%s) is not in gObjectArray", object->getName() );
		} else
			gObjectArray->del ( object->servID );
	}
	
	object->deleteFromRoom ( notify );

	// restore the changed flag
	if ( building )
		building->changed = oldChanged;

	delete object;
}

void RMServer::destroyObj ( long servID, int notify, char *file, int line )
{
	WorldObject *object = findObject ( servID );
	destroyObj ( object, notify, file, line );
} 

void RMServer::addObject ( WorldObject *object )
{
	if ( ( object->servID < 0 ) || ( object->servID >= _SERVID_MAX ) ) {
		logInfo ( _LOG_ALWAYS, "adding object with invalid servID %d", object->servID );
		crash();
	}

	if ( !gObjectArray->lookup ( object->servID ) ) {
		if ( !object->objElement )
			object->objElement = _objects.add ( object );

		gObjectArray->add ( object, object->servID );
	}
}
 
void RMServer::deleteObject ( WorldObject *object )
{
	// new
	if ( !object || object->destructing )
		return;

	if ( !object->isWorldObject() ) {
		logInfo ( _LOG_ALWAYS, "invalid object passed to RMServer::deleteObject (0x%x)", object );
		return;
	}

	if ( object->objElement ) {
		_objects.delElement ( object->objElement );
		object->objElement = NULL;
	}

	if ( object->characterElement ) {
		_characters.delElement ( object->characterElement );
		object->characterElement = NULL;
	}

	if ( object->servID > -1 ) {
		WorldObject *obj = (WorldObject *)gObjectArray->lookup ( object->servID );

		if ( obj == object ) 
			gObjectArray->del ( object->servID );
	}
}

void stripTokens ( LinkedList *tokens )
{
	char *tossTbl[] = {
		"of",
		"and",
		"a",
		"an",
		"the",
	};

	int size = sizeof ( tossTbl ) / sizeof ( char * );

	LinkedElement *element = tokens->head();

	while ( element ) {
		StringObject *string = (StringObject *)element->ptr();
		element = element->next();

		for ( int i=0; i<size; i++ ) {
			if ( !strcasecmp ( tossTbl[i], string->data ) ) {
				tokens->del ( string );
				delete string;

				break;
			}
		}
	}
}

WorldObject *RMServer::findComponentObject ( int table, char *text )
{
	if ( !text )
		return NULL;

	LinkedList *objList = &gComponentObjects[table];

	LinkedElement *element = objList->head();
	int anyMatch = 0;

	while ( element ) {
		WorldObject *object = (WorldObject *)element->ptr();
		element = element->next();

		if ( !strcmp ( object->name, text ) ) 
			return object;
	}

	if ( anyMatch ) 
		return (WorldObject *)-1;

	return NULL;
}
 
WorldObject *RMServer::findObject ( long servID )
{
	WorldObject *retVal = (WorldObject *)gObjectArray->lookup ( servID );
	return retVal;
}

WorldObject *RMServer::findObjectLast ( long servID )
{
	// new
//	if ( servID < 0 ) {
	if ( ( servID < -1 ) || ( servID >= _SERVID_MAX ) ) {
		logDisplay ( "Find Object Last passed invalid servID %d.", servID );
		return NULL;
	}

	LinkedElement *element = _objects.head();

	while ( element ) {
		WorldObject *object = (WorldObject *)element->ptr();
		element = element->next();
	}

	return NULL;
}

WorldObject *RMServer::findObject ( const char *name )
{
	WorldObject *retVal = NULL;

	// new
	if ( !name )
		return retVal;

	LinkedElement *element = _objects.head();

	while ( element ) {
		WorldObject *object = (WorldObject *)element->ptr();

		if ( !strcmp ( object->name, name ) ) {
			retVal = object;
			break;
		}

		element = element->next();
	}

	return retVal;
}

WorldObject *RMServer::findObjectByClass ( const char *name )
{
	WorldObject *retVal = NULL;

	// new
	if ( !name )
		return retVal;

	LinkedElement *element = _objects.head();

	while ( element ) {
		WorldObject *object = (WorldObject *)element->ptr();

		if ( !strcmp ( object->classID, name ) ) {
			retVal = object;
			break;
		}

		element = element->next();
	}

	return retVal;
}

WorldObject *RMServer::findClass ( const char *name )
{
	WorldObject *retVal = NULL;

	// new
	if ( !name )
		return retVal;

	retVal = textToObject ( name );

	if ( retVal )
		return retVal;

	TreeNode *node = classTree.find ( name );

	if ( node )
		return (WorldObject *)node->data;

	return NULL;
}

void RMServer::addClass ( WorldObject *object )
{
	static int classCount = 0;

	if ( !classTree.find ( object->classID ) ) {

		//if this is a test object and we're not a test server, don't add it, but
		//we need to preserve the class numbers
		bool isTestObj = ((object->physicalState & _STATE_TESTSERVER_OBJ) != 0);
		if( !isTestObj || ( isTestObj && IsThisATestServer() ) ) {
			classTree.add ( object->classID, object );
			_classes.add ( object );
		}

		object->classNumber = classCount;

		if ( !(object->physicalState & _STATE_SPECIAL) ) 
			classCount++;
	}	
}

WorldObject *RMServer::getObject ( long servID )
{
	if ( servID == -1 )
		return NULL;

	return findObject ( servID );
}

void RMServer::sendSystemMsg ( const char *format, ... )
{
	char text[1024] = "";
	
	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	PackedMsg msg;
	msg.putString ( "Important System Message" );
	msg.putString ( text );

	sendToAll ( _IPC_PLAYER_SYSTEM_MSG, msg.data(), msg.size() );
}

void RMServer::sendSystemMsg ( const char *title, RMPlayer *player, const char *format, ... )
{
	char text[20480] = "";
	
	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	PackedMsg msg;
	msg.putString ( title );
	msg.putString ( text );

	// new
	if ( player )
		sendTo ( _IPC_PLAYER_SYSTEM_MSG, msg.data(), msg.size(), player );
}

void RMServer::sendRoomText ( RMRoom *room, const char *format, ... )
{
	char text[1024] = "";
	
	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	int size = sizeof ( IPCPMMessage ) + strlen ( text ) + 1;

	IPCPMMessage *response = (IPCPMMessage *)malloc ( size );
	memcpy ( (char *)response + sizeof ( IPCPMMessage ), text, size - sizeof ( IPCPMMessage ) );

	sendToRoom ( _IPC_PLAYER_TEXT, response, size, room );

	free ( response );
}

void RMServer::sendPlayerChat ( RMPlayer *source, RMPlayer *player, const char *format, ... )
{
	if ( !player || !source )
		return;

	char text[1024] = "";
	
	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	int size = sizeof ( IPCPMMessage ) + strlen ( text ) + 1;

	IPCPMMessage *response = (IPCPMMessage *)malloc ( size );
	memcpy ( (char *)response + sizeof ( IPCPMMessage ), text, size - sizeof ( IPCPMMessage ) );

	player->addText ( source, text );
 
	sendTo ( _IPC_PLAYER_TEXT, response, size, player );

	free ( response );
}

void RMServer::sendPlayerText ( RMPlayer *player, const char *format, ... )
{
	// new
	if ( !player )
		return;

	char text[1024] = "";
	
	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	int size = sizeof ( IPCPMMessage ) + strlen ( text ) + 1;

	IPCPMMessage *response = (IPCPMMessage *)malloc ( size );
	memcpy ( (char *)response + sizeof ( IPCPMMessage ), text, size - sizeof ( IPCPMMessage ) );

	sendTo ( _IPC_PLAYER_TEXT, response, size, player );

	free ( response );
}

void RMServer::sendListText ( LinkedList *list, const char *format, ... )
{
	char text[1024] = "";
	
	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	int size = sizeof ( IPCPMMessage ) + strlen ( text ) + 1;

	IPCPMMessage *response = (IPCPMMessage *)malloc ( size );
	memcpy ( (char *)response + sizeof ( IPCPMMessage ), text, size - sizeof ( IPCPMMessage ) );
 
	sendToList ( _IPC_PLAYER_TEXT, response, size, list );

	free ( response );
}

void RMServer::sendListChat ( RMPlayer *source, LinkedList *list, const char *format, ... )
{
	char text[1024] = "";
	
	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	int size = sizeof ( IPCPMMessage ) + strlen ( text ) + 1;

	IPCPMMessage *response = (IPCPMMessage *)malloc ( size );
	memcpy ( (char *)response + sizeof ( IPCPMMessage ), text, size - sizeof ( IPCPMMessage ) );
 
	LinkedElement *element = list->head();

	while ( element ) {
		RMPlayer *player = (RMPlayer *)element->ptr();
		element = element->next();
		if( source ) player->addText ( source, text );
	}
 
	sendToList ( _IPC_PLAYER_TEXT, response, size, list );

	free ( response );
}

void RMServer::sendPlayersText ( RMPlayer *obj, const char *format, ... )
{
	char text[1024] = "";
	
	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	int size = sizeof ( IPCPMMessage ) + strlen ( text ) + 1;

	IPCPMMessage *response = (IPCPMMessage *)malloc ( size );
	memcpy ( (char *)response + sizeof ( IPCPMMessage ), text, size - sizeof ( IPCPMMessage ) );
 
	sendToList ( _IPC_PLAYER_TEXT, response, size, roomMgr->players(), obj );

	free ( response );
}

void RMServer::sendRoomInfo ( RMRoom *room, const char *format, ... )
{
	char text[1024] = "";
	
	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	int size = sizeof ( IPCPMMessage ) + strlen ( text ) + 1;

	IPCPMMessage *response = (IPCPMMessage *)malloc ( size );
	memcpy ( (char *)response + sizeof ( IPCPMMessage ), text, size - sizeof ( IPCPMMessage ) );
 
	sendToRoom ( _IPC_PLAYER_INFO, response, size, room );

	free ( response );
}

void RMServer::sendPlayerInfo ( RMPlayer *player, const char *format, ... )
{
	// new

	if ( !player )
		return;

	char text[1024] = "";
	
	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	int size = sizeof ( IPCPMMessage ) + strlen ( text ) + 1;

	IPCPMMessage *response = (IPCPMMessage *)malloc ( size );
	memcpy ( (char *)response + sizeof ( IPCPMMessage ), text, size - sizeof ( IPCPMMessage ) );
 
	sendTo ( _IPC_PLAYER_INFO, response, size, player );

	free ( response );
}

void RMServer::sendListInfo ( LinkedList *list, const char *format, ... )
{
	char text[1024] = "";
	
	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	int size = sizeof ( IPCPMMessage ) + strlen ( text ) + 1;

	IPCPMMessage *response = (IPCPMMessage *)malloc ( size );
	memcpy ( (char *)response + sizeof ( IPCPMMessage ), text, size - sizeof ( IPCPMMessage ) );
 
	sendToList ( _IPC_PLAYER_INFO, response, size, list );

	free ( response );
}

void RMServer::sendPlayersInfo ( RMPlayer *obj, const char *format, ... )
{
	char text[1024] = "";
	
	va_list args;
	va_start ( args, format );
	vsprintf ( sizeof ( text ), text, format, args );

	int size = sizeof ( IPCPMMessage ) + strlen ( text ) + 1;

	IPCPMMessage *response = (IPCPMMessage *)malloc ( size );
	memcpy ( (char *)response + sizeof ( IPCPMMessage ), text, size - sizeof ( IPCPMMessage ) );
 
	sendToList ( _IPC_PLAYER_INFO, response, size, roomMgr->players(), obj );

	free ( response );
}

void RMServer::forceLogout ( void )
{
	LinkedElement *element = _players.head();

	while ( element ) {
		RMPlayer *player = (RMPlayer *)element->ptr();
		player->forceLogout();
		element = element->next();
	}
}

void RMServer::addPlayer ( Player *player )
{
	//MIKE cleanup RMPlayer *thePlayer = (RMPlayer *)player;

	PlayerRegistry::addPlayer ( player );
}

void RMServer::deletePlayer ( Player *player )
{
	PlayerRegistry::deletePlayer ( player );
	_gms.del ( player );
	_guides.del ( player );
	_hosts.del ( player );

	if ( bEventHappening ) {
		gEventPlayers.del ( player );
	}
}

// process the world library
int RMServer::processLibrary ( char *directory )
{
	FILE *file = NULL;
	char filename[1024], path[1024], fname[1024];
	int retVal = 0;

	logDisplay ( "loading object files..." );

	gParsingClasses = 1;

	// load all of the object specification files
	sprintf ( sizeof ( path ), path, "%s%s", directory, "obj/" );
	sprintf ( sizeof ( filename ), filename, "%sobjects.dir", path );
	file = fopen ( filename, "rb" );

	if ( file ) {
		while ( !feof ( file ) && fgets ( fname, 1024, file ) ) {
			char *ptr = strrchr ( fname, '\n' );

			if ( ptr )
				*ptr = 0;

			sprintf ( sizeof ( filename ), filename, "%s%s.tok", path, fname );

			if ( parseObjectFile ( filename ) == -1 ) 
				exit ( 1 );
		}

		fclose ( file );
	} else {
		fatal ( "cannot access %s!\n", path );
	}

	gParsingClasses = 0;

	FILE *dupeRoomFile = fopen ( "../logs/duplicaterooms.txt", "w" );

	if( !dupeRoomFile ) {
		logDisplay ( "Error opening /logs/duplicaterooms.txt for writing.");
	}
	else {
		fprintf ( dupeRoomFile, "Duplicate Room Report, generated at %s\n", timeToStr() );
		fprintf ( dupeRoomFile, "-----------------------------------------\n" );
		fclose(dupeRoomFile);
	}

	logDisplay ( "loading zone files..." );

	// load all of the zone specification files
	sprintf ( sizeof ( path ), path, "%s%s", directory, "zones/" );
	sprintf ( sizeof ( filename ), filename, "%szones.dir", path );
	file = fopen ( filename, "rb" );

	if ( file ) {
		while ( !feof ( file ) && fgets ( fname, 1024, file ) ) {
			if ( fname[0] != '#' ) {
				char *ptr = strrchr ( fname, '\n' );

				if ( ptr )
					*ptr = 0;

				sprintf ( sizeof ( filename ), filename, "%s%s.tok", path, fname );

				if ( parseZoneFile ( filename ) == -1 ) 
					exit ( 1 );
			}
		}

		fclose ( file );
	} else {
		fatal ( "cannot access %s!\n", path );
	}

	return retVal;
}

// parse an object file
int RMServer::parseObjectFile ( char *name )
{
	File file ( name );

	// load the data and setup the input
	char *buffer = (char *)malloc ( file.size() );
	file.read ( buffer, file.size() );
	objectInfoParser.input = buffer;
	objectInfoParser.index = 0;

	int bufferSize = file.size();

	int retVal = 0, lineNum = 0;

	if ( file.isOpen() ) {
		while ( retVal != -1 && (objectInfoParser.index < bufferSize) ) {
			lineNum++;

			switch ( objectInfoParser.parseLine() ) {
				case -1: {
					logInfo ( _LOG_ALWAYS, "\"%s\", line %d: %s", name, lineNum, objectInfoParser.error() );
					retVal = -1;
				}

				break;

				case 1: {
					WorldObject *object = objectInfoParser.object;
					WorldObject *super = roomMgr->findClass ( object->classID );

					if ( super ) {
						logInfo ( _LOG_ALWAYS, "\"%s\", line %d: '%s' has already been defined", name, lineNum, object->classID );
						retVal = -1;
					} else {
						roomMgr->addClass ( object );

						if( (object->physicalState & _STATE_TESTSERVER_OBJ) == 0 || IsThisATestServer()  ) {

							while ( object->physicalState & _STATE_SPECIAL ) {
								object->classNumber = object->superObj->classNumber;
								object = object->superObj;
							}

						} else delete object;
					}
				}

				break;
			}
		}
	}

	free ( buffer );

	return retVal;
}

// parse a zone file
int RMServer::parseZoneFile ( char *name )
{
	int retVal = 0, lineNum = 0;
	ZoneInfoParser zoneInfoParser;

	File file ( name );

	// load the data and setup the input
	char *buffer = (char *)malloc ( file.size() );
	file.read ( buffer, file.size() );
	zoneInfoParser.input = buffer;
	zoneInfoParser.index = 0;
	int bufferSize = file.size();

	if ( file.isOpen() ) {
		while ( retVal != -1 && (zoneInfoParser.index < bufferSize) ) {
			lineNum++;

			switch ( zoneInfoParser.parseLine() ) {
				case -1: {
					logInfo ( _LOG_ALWAYS, "\"%s\", line %d: %s", name, lineNum, zoneInfoParser.error() );
					retVal = -1;
				}

				break;
			}
		}

		if ( retVal == 0 ) {
			Zone *zone = zoneInfoParser.zone;

			if ( zone ) {
				addZone ( zone );
				zone->initDatabase();
			}
		}
	}

	free ( buffer );

	return retVal;
}

int drandom ( int start, int end )
{
	static int initted = 0;

	if ( !initted ) {
		srand48 ( getseconds() );
		initted = 1;
	}

	int range = (end - start) + 1;

	if ( range )
		return (lrand48() % range) + start;

	return start;
}

int opposedRoll ( int a, int b )
{
	int rollA = drandom ( 1, a * 1000 );
	int rollB = drandom ( 1, b * 1000 );

	if ( rollA > rollB )
		return 1;

	return 0;
}

int main ( int argc, char **argv )
{
	gTouchDistance = 30;

	gAppStarted = 1;

	gRoomArray = new SparseArray ( 900000000, 32768 );

	for (int nCount = 0;nCount < _MAX_ACCEPT_COUNTS;nCount++) {
		gAcceptCounts[ nCount ] = 0;
	}

	FILE* acceptFile = fopen ( "../logs/counts.bin", "rb" );

	if ( acceptFile ) {
		fread( &gAcceptCounts, sizeof( int ), _MAX_ACCEPT_COUNTS, acceptFile );
		fclose( acceptFile );
	}

	initSignalHandlers();

	// handle argument usage display
	if ( argc < 2 ) {
		logDisplay ( "usage %s [config file] options..." );
		return 0;
	}

	ConfigMgr config;
	config.load ( argv[1] );

	char *dataMgrHost = config.get ( "datamgrHost" );
	char *dataMgrPort = config.get ( "datamgrPort" );
	char *portName = config.get ( "portName" );
	gMaxConnections = atoi ( config.get ( "maxConnections" ) );
	gServerID = atoi ( config.get ( "serverID" ) );
	gServerName = config.get ( "serverName", 0 );
	char* mmOff = config.get( "magicMailOff", 0 );

	if ( mmOff )
		gMagicMailOff = atoi( mmOff );
	else
		gMagicMailOff = 0;

	char* testServer = config.get( "TestServer", 1 );

	if ( testServer ) {
		if( atoi( testServer ) == 1 ) gTestServer = true;
		else gTestServer = false;
	} else {
		gTestServer = false;
	}

	gMaxConnectCount = 4500;

	appCrashHandler = handleCrash;

	printf ( "-------------------------------------------\n" );
	printf ( "Dwarves and Giants - Server v0.1\n" );
	printf ( "startup mode: %s server.\n", IsThisATestServer() ? "\bTEST" : "MAIN" );
	printf ( "-------------------------------------------\n\n" );

	sysLogAppName = "roommgr";
	sysLogLevel = _LOG_ALWAYS;

	struct rlimit limit;
	int nRet1, nRet2;

	nRet1 = getrlimit ( RLIMIT_NOFILE, &limit );
	limit.rlim_cur = limit.rlim_max;
	nRet2 = setrlimit ( RLIMIT_NOFILE, &limit );

	int i;

	for ( i=0; i<_SERVID_TBL_SIZE; i++ )
		gServIDTbl[i] = 0;

	for ( i=0; i<_ROOM_ID_TBL_SIZE; i++ )
		gRoomIDTbl[i] = 0;

	for ( i=0; i<_MAX_CHANNEL; i++ ) {
		gChannels[i] = new Channel;
		gChannels[i]->number = i;
	}

	gCrashMessage[0] = 0;
	gShutdownMessage[0] = 0;

	gChannels[4]->setName ( "General" );
	gChannels[4]->setTopic ( "|c85|Welcome to Dwarves and Giants! (Name not final)" );
	gChannels[4]->isSystem = 1;

	//gChannels[0]->setName ( "Help" );
	//gChannels[0]->setTopic ( "This channel is for requesting help." );
	//gChannels[0]->isSystem = 1;

	roomMgr = new RMServer;
	roomMgr->initted = 0;

	// create the script manager
	g_pScriptMgr = new CScriptMgr;

	if ( roomMgr->processLibrary ( "../data/lib/" ) == -1 )
		exit ( 1 );

	loadTreasure();

	gBlackBaldric = roomMgr->findClass ( "BlackBaldric" );
	gRealBlackBaldric = roomMgr->findClass ( "RealBlackBaldric" );
	gPurpleBaldric = roomMgr->findClass ( "PurpleBaldric" );
	gSatoriBaldric = roomMgr->findClass ( "SatoriBaldric" );
	gLiveChristmasTree = roomMgr->findClass ( "ChristmasTree" );
	gDeadChristmasTree = roomMgr->findClass ( "DeadChristmasTree" );

	gStaffItems[0] = roomMgr->findClass( "MentorBaldric" );
	gStaffItems[1] = roomMgr->findClass( "EventsBaldric" );
	gStaffItems[2] = roomMgr->findClass( "SentinelBaldric" );
	gStaffItems[3] = roomMgr->findClass( "ImplementorBaldric" );
	//gStaffItems[4] = roomMgr->findClass( "SentinelHelm" );
	//gStaffItems[5] = roomMgr->findClass( "MentorHelmet" );
	//gStaffItems[6] = roomMgr->findClass( "BishopHat" );
	//gStaffItems[7] = roomMgr->findClass( "DuachsAura" );
	//gStaffItems[8] = roomMgr->findClass( "DespothesCrown" );
	//gStaffItems[9] = roomMgr->findClass( "RareCrown" );
	gStaffItems[10] = roomMgr->findClass( "SMHolidayBaldric" );

	if ( argc > 2 && !strcmp ( argv[2], "-buildvalue" ) ) {
		LinkedElement *element = roomMgr->_classes.head();

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			element = element->next();

			if ( obj->components ) {
				LinkedElement *elementA = obj->components->head();
				int theValue = 0;

				logDisplay ( "%s", obj->classID );

				while ( elementA ) {
					WorldObject *objA = (WorldObject *)elementA->ptr();
					elementA = elementA->next();

					logDisplay ( "      %s (%d gold) %s", objA->classID, objA->netWorth(), (objA->physicalState & _STATE_WHOLESALE)? "" : "(non-wholesale!)" ); 

					theValue += objA->netWorth();
				}

				logDisplay ( "   component value = %d, defined value = %d", theValue, obj->value );
			}
		}

		exit ( 1 );
	}

	if ( argc > 2 && !strcmp ( argv[2], "-items" ) ) {
		LinkedList itemLists[_SKILL_MAX][5];
		LinkedElement *element = roomMgr->_classes.head();

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			element = element->next();

			if ( obj->components ) {
				LinkedList *list = &itemLists[obj->skill][obj->strength];

				LinkedElement *elementA = list->head();

				while ( elementA ) {
					WorldObject *theObj = (WorldObject *)elementA->ptr();

					if ( strcmp ( obj->name, theObj->name ) < 0 ) {
						list->addBefore ( theObj, obj );
						break;
					}

					elementA = elementA->next();
				}

				if ( !elementA )
					list->addToEnd ( obj );
			}
		}

		for ( int i=0; i<_SKILL_MAX; i++ ) {
			for ( int j=0; j<5; j++ ) {
				if ( itemLists[i][j].size() ) {
					element = itemLists[i][j].head();

					while ( element ) {
						WorldObject *obj = (WorldObject *)element->ptr();	
						element = element->next();

						logDisplay ( "{%d, %d, \"%s\"},", obj->skill, obj->strength, obj->name );
					}
				}
			}
		}

		exit ( 1 );
	}

	if ( argc > 2 && !strcmp ( argv[2], "-objvalue" ) ) {
		LinkedList objects;

		LinkedElement * element = roomMgr->_classes.head();

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			element = element->next();

			if ( !(obj->physicalState & _STATE_SPECIAL) && obj->value && obj->getBase ( _BCARRY ) ) {
				LinkedElement *elementA = objects.head();

				while ( elementA ) {
					WorldObject *objA = (WorldObject *)elementA->ptr();
					elementA = elementA->next();

					if ( obj->value <= objA->value ) {
						objects.addBefore ( objA, obj );
						break;
					}
				}

				if ( !elementA )
					objects.addToEnd ( obj );
			}
		}

		element = objects.head();

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			element = element->next();

			logDisplay ( "%32s     %d gold", obj->classID, obj->value );
		}

		objects.release();

		exit ( 1 );
	}

	if ( argc > 2 && !strcmp ( argv[2], "-objdump" ) ) {
		LinkedElement *element = roomMgr->_classes.head();
		unlink ( "objdump.csv" );

		FILE *file = fopen ( "objdump.csv", "wb" );

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			fprintf ( file, "%s,%s\n", obj->getName(), obj->classID ); 
			element = element->next();
		}

		delete file;

		exit ( 1 );
	}

	if ( argc > 2 && !strcmp ( argv[2], "-treasure" ) ) {
		LinkedElement *element = roomMgr->_classes.head();

		unlink ( "treasure.txt" );

		FILE *file = fopen ( "treasure.txt", "wb" );

		// this is used as a kludge variable when the roomMgr is initted... see 
		// btreasure.cpp
		//
		roomMgr->initted = 1;

		while ( element ) {
			WorldObject *obj = (WorldObject *)element->ptr();
			element = element->next();

			if ( obj->getBase ( _BNPC ) ) {
				if ( argc == 3 && strcmp ( obj->classID, argv[3] ) )
					continue;

				int value = 0, level = 0;
				int i;

				for (i=0; i<1000; i++ ) {
					WorldObject *testObj = new WorldObject;
					testObj->addToDatabase();

					testObj->copy ( obj );

					NPC *npc = makeNPC ( testObj );

					value += testObj->sellWorth();	
					level += testObj->level;

					delete npc;
				}

				fprintf ( file, "%32s     $%6d     %d\n", obj->classID, value / i, level / i );
				fflush ( file );

				logDisplay ( "%32s     %6d gold      lvl %d", obj->classID, value / i, level / i );
			}
		}

		fclose ( file );

		exit ( 1 );
	}

	loadMessages();
	loadQuests();
	loadTalkTrees();

	if ( argc > 2 && !strcmp ( argv[2], "-makecode" ) ) {
		// step through all of the classes and write each one out
		LinkedElement *root = roomMgr->_classes.head();
		int count = 250, number = 0;

makecode:
		char name[1024];
		sprintf ( sizeof ( name ), name, "../data/stkobj%d.sc", number );
		FILE *file = fopen ( name, "wb" );

		if ( file ) {
			fprintf ( file, ";;\n" );
			fprintf ( file, ";; STKOBJ%d.SC\n", number );
			fprintf ( file, ";;\n" );
			fprintf ( file, ";; This file contains SCI representations of all objects found in the server's\n" );
			fprintf ( file, ";; object class data files.  This file was generated by the SPARCStation server\n" );
			fprintf ( file, ";; and is maintained by the SPARCStation server.  You SHOULD NOT manually\n" );
			fprintf ( file, ";; modify this file for ANY reason.  The system will most probably cease\n" );
			fprintf ( file, ";; to function if you did.\n" );
			fprintf ( file, ";;\n" );
			fprintf ( file, ";; Author: SPARCStation (%s)\n", timeToStr () );
			fprintf ( file, ";;\n\n" );
			fprintf ( file, "(module# STOCKOBJ%d)\n", number );

			if ( !number ) {
				fprintf ( file, "\n(public StockObjList 0)\n" );
			} else {
				fprintf ( file, "\n(public StockObjInitter%d 0)\n(define StockObjList (ModuleID STOCKOBJ0 0))\n", number );
			}

			fprintf ( file, "\n(include \"wobject.sh\")\n" );

			LinkedElement *element = root;

			while ( element && count ) {
				WorldObject *obj = (WorldObject *)element->ptr();

				if ( !(obj->physicalState & _STATE_SPECIAL) ) {
					fprintf ( file, "\n" );
					obj->writeSCIData ( file );

					count -= 1;
				}

				element = element->next();
			}

			count = 250;

			LinkedElement *start = root;
			root = element;

			if ( !number ) {
				fprintf ( file, "\n(instance StockObjList of Set\n" );
				fprintf ( file, "\t(method (init)\n" );
			} else {
				fprintf ( file, "\n(instance StockObjInitter%d of Code\n", number );
				fprintf ( file, "\t(method (doit)\n" );
			}	

			element = start;

			while ( element != root ) {
				WorldObject *obj = (WorldObject *)element->ptr();

				if ( !(obj->physicalState & _STATE_SPECIAL) ) {
					fprintf ( file, "\t\t(%s add: SOBJ%s)\n", number? "StockObjList" : "self", obj->classID );
				}

				element = element->next();
			}

			fprintf ( file, "\t)\n" );
			fprintf ( file, ")\n" );

			fclose ( file );

			if ( element ) {
				number++;
				goto makecode;
			}

			exit ( 1 );
		}
	}

	sysLogDisplay = 1;

	// this is used as a kludge variable when the roomMgr is initted... see 
	// btreasure.cpp
	//
	printf ( "Initializing network subsystem...\n" );
	roomMgr->initted = TRUE;
	roomMgr->init ( portName );

	if ( roomMgr->handle() == -1 ) {
		logDisplay ( "error binding to port %s(%d)", portName, errno );
		exit ( 1 );
	}

	printf ( "Network subsystem is up and running...\n" );

	// create the friend manager
	g_pFriendMgr = new CFriendMgr;

	//mike- this seems unused
	//if ( sysLogDisplay ) {
	//	windowMgr = new WindowManager;
	//}

	// reset the zones
	LinkedElement *element = roomMgr->_zones.head();
	int dungeonID = 0;

	while ( element ) {
		Zone *zone = (Zone *)element->ptr();
		element = element->next();

		if ( zone->isDungeon ) {
			gSpawnCount++;
			zone->id = gSpawnTotal++;

			logDisplay ( "zone entrance = '%s'", zone->entranceName );
			TreeNode *node = gObjectTree.find ( zone->entranceName );
			WorldObject *entrance = node? (WorldObject *)node->data : NULL; 

			if ( entrance && entrance->servID ) {
				entrance->strength = dungeonID;
				Zone *entranceZone = entrance->room->zone;

				LinkedList *playerQueue = new LinkedList;
				entranceZone->dungeonEntranceArray[dungeonID] = playerQueue;

				dungeonID++;
			}
		}

		logDisplay ( "resetting zone %s...", zone->name() );

		gZones.add ( zone );

                logDisplay ( "added zone %s...", zone->name() );
		zone->reset();

                logDisplay ( "reset zone." );
	}

	if ( !gMaxConnections )
		gMaxConnections = 1;

	// get the starting execution time
	gStartTime = getsecondsfast();

	IPCStats::init();

	// install all the timers
	installTimer ( 1, _IPC_NPC_PULSE );
	installTimer ( 1, _IPC_COMBAT_PULSE );
	installTimer ( 5, _IPC_AMBUSH_PULSE );
	installTimer ( 15, _IPC_DUNGEON_QUEUE_PULSE );
	installTimer ( 30, _IPC_TOSS_DEAD_HOUSES );
	installTimer ( 60, _IPC_PROCESS_AFFECT_PULSE );
	installTimer ( 60, _IPC_SAVE_STATE );
	installTimer ( 120, _IPC_HEAL_PULSE );
	installTimer ( 120, _IPC_MONSTER_PULSE );
	installTimer ( 120, _IPC_MAINTENANCE_PULSE );
	installTimer ( 900, _IPC_CHAR_DOIT_PULSE );

	// display server ready message
	logDisplay ( "server running" );

	WorldObject *super = roomMgr->findClass ( "Baldric" );
	WorldObject *obj = new WorldObject ( super );

	obj->bePutIn ( obj );

	int shutdownElapsed = 0;

    element = gZones.head();

    int nPlayers = 0;
    int nNPCs = 0;

    while ( element ) {
            Zone *zone = (Zone *)element->ptr();
            element = element->next();

            nPlayers += zone->players.size();
            nNPCs += zone->npcs.size();
	nNPCs += zone->externalNPCs.size();
    }

    int upTime = getsecondsfast() - gStartTime;
    double hoursUp = (double) upTime / 3600.0;

    char output[10240];

    sprintf ( sizeof(output), output, "uptime = %0.6f\n\ndungeons spawned = %d\n\nmemory taken = %d bytes  (#%d) > %d dif %d\n\nfile handles = %d\nobjects = %d\naffected objects = %d\n\nhouses = %d/%d\n\nplayers = %d  NPCs = %d", hoursUp, gSpawnCount, gAllocSize, gAllocCount, gLargestAllocSize, g_nAllocations, gFileHandles.val(), roomMgr->_objects.size(), gAffectedObjects.size(), gEmptyBuildings.size(), gBuildings.size(), nPlayers, nNPCs );

    char filename [ 1024 ];
    sprintf ( sizeof ( filename ), filename, "../logs/memoryStat.txt.%d", getpid() );

    File file ( filename );
    file.append();

    file.printf ( output );
    file.printf ( "\n\n\nLine\tfile\tSize\tAlloc\tFree\n" );

    for (int nFound = 0;nFound < g_nAllocations;nFound++) {
		file.printf ( "%d\t%s\t%d\t%d\t%d\n",
			gAllocations[ nFound ].line,
			gAllocations[ nFound ].file,
			gAllocations[ nFound ].nSize,
			gAllocations[ nFound ].nAlloc,
			gAllocations[ nFound ].nFree );
	}
	
	file.printf( "\n\n\n" );
	file.close();

	// execute the main game loop
	while ( 1 ) {
		if ( gDataMgr->handle() == -1 ) {
			logDisplay ( "connecting to data manager..." );

			while ( gDataMgr->handle() == -1 ) {
				gDataMgr->makeConnection ( dataMgrHost, dataMgrPort );

				if ( gDataMgr->handle() != -1 ) {
					logDisplay( "connected to data manager..." );
					gIPCPollMgr.addClient ( gDataMgr );
					gDataMgr->hello();
				}
			}
		}

		// process any messages
		gIPCPollMgr.doit();
		gDataMgr->doit();
		roomMgr->doit();	

		// process all of the timers
		int nElapsed = processTimers();

		// process the shutdown system
		if ( nElapsed && (gShutdownTimer > -1) ) {
			gShutdownTimer -= nElapsed;

			if ( gShutdownTimer < 1 ) {
				crash();
			} else {
				// time for downtime message
				if ( gDowntimeMessage && gShutdownTimer < 300 && gShutdownMessage[0] ) {
					gDataMgr->DowntimeMessage( gShutdownMessage );
					gDowntimeMessage = 0;
				}

				if ( (gShutdownTimer % 60) == 0 ) {

					unsigned int timeHours = gShutdownTimer / 3600;
					unsigned int timeMinutes = ( gShutdownTimer - ( timeHours * 3600) ) / 60;
					unsigned int timeSeconds = gShutdownTimer - ( timeHours * 3600) - ( timeMinutes * 60);

					char timeText[40] = {0};
					char scratch[40] = {0};

					if( timeHours ) {
						sprintf( 40, scratch, "%dh", timeHours );
						strcat( timeText, scratch );
					}
					if( timeMinutes ) {
						sprintf( 40, scratch, "%dm", timeMinutes );
						if( timeHours ) strcat( timeText, " " );
						strcat( timeText, scratch );
					}
					if( timeSeconds || ( !timeHours && !timeMinutes ) ) {
						sprintf( 40, scratch, "%ds", timeSeconds );
						if( timeHours || timeMinutes ) strcat( timeText, " " );
						strcat( timeText, scratch );
					}

					roomMgr->sendPlayersInfo ( NULL, "Shutdown> %s remain.\n", timeText );
				}

			}
		}
	}

	saveState(true);

	logInfo ( _LOG_ALWAYS, "Closing all the open data manager messages." );

	if ( gDataMgr->handle() != -1 ) {
		while ( gDataMgr->msgQueueSize() ) {
			int result = gDataMgr->sendNextMsg();

			if ( result == -2 )  
				break;
		}
	}

	return 0;
}

// Check for a test server??
bool IsThisATestServer() {
	return gTestServer;
}
