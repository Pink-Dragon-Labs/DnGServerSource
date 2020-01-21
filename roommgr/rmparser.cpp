#include <algorithm>
#include "roommgr.hpp"
#include "rmparser.hpp"
#include "globals.hpp"
#include "friendmgr.hpp"
 
extern LinkedList gOpenFileList;
extern int validName ( char * name );
LinkedList gComplaintList;
LinkedList gGmsComplaintList;

char *profaneTable[] = {
	"nigger",
	NULL
};

int isProfane ( const char* str ) 
{
	int index = 0;

	if ( !str )
		return TRUE;

	char *newStr = strdup ( str );
	strlower ( newStr );

	while ( profaneTable[index] ) {
		if ( strstr ( newStr, profaneTable[index] ) ) {
			free ( newStr );
			return TRUE;
		}

		index++;
	}

	if ( newStr )
		free ( newStr );

	return FALSE;
}

RMPlayer *findPlayer ( char *name, RMPlayer *player ) 
{
	if ( !name ) {
		logInfo ( _LOG_ALWAYS, "Invalid findPlayer: (NULL) from %s:", player->getName() );
		return NULL;
	}

	char theName[1024];

	strcpy ( theName, name );
	strlower ( theName );

	TreeNode *node = gCharacterTree.find ( theName );

	if ( !node && player && player->checkAccess ( _ACCESS_MODERATOR ) )
		node = gLoginTree.find ( theName );

	return (RMPlayer*) ( node ? node->data : NULL );
}

//this is a bit rediculous...
class ListedNumber : public ListObject
{
public:
	int num;
	ListedNumber( int numb ) { num = numb; }
	~ListedNumber() {}
};

void addToChunk( LinkedList* chunk, RMRoom* room )
{
	if( !chunk || !room )
		return;

	//if this room is already in the chunk, get out.
	LinkedElement* roomElement = chunk->head();

	while( roomElement ) {
		ListedNumber* roomNum = (ListedNumber*) roomElement->ptr();
		roomElement = roomElement->next();

		if( room->number == roomNum->num ) return;
	}

	//add this room to the chunk
	chunk->add( new ListedNumber(room->number) );

	//add all our surrounding rooms to the chunk, if they're in the same zone.
	RMRoom* northRoom = NULL;
	RMRoom* southRoom = NULL;
	RMRoom* eastRoom  = NULL;
	RMRoom* westRoom  = NULL;

	if ( room->north != -1 ) northRoom = roomMgr->findRoom( room->north );
	if ( room->south != -1 ) southRoom = roomMgr->findRoom( room->south );
	if ( room->east  != -1 )  eastRoom = roomMgr->findRoom( room->east );
	if ( room->west  != -1 )  westRoom = roomMgr->findRoom( room->west );
	
	//test if any of the destinations are in a different zone, if so,
	//don't travel there.
	if( northRoom && northRoom->zone != room->zone ) northRoom = NULL; 
	if( southRoom && southRoom->zone != room->zone ) southRoom = NULL;
	if( eastRoom  &&  eastRoom->zone != room->zone )  eastRoom = NULL;
	if( westRoom  &&  westRoom->zone != room->zone )  westRoom = NULL;

	//add them to the chunk
	if( northRoom ) addToChunk( chunk, northRoom );
	if( southRoom ) addToChunk( chunk, southRoom );
	if( eastRoom  ) addToChunk( chunk,  eastRoom );
	if( westRoom  ) addToChunk( chunk,  westRoom );
}

int generateZoneValidationReport ( Zone* zone )
{
	LinkedList* chunkList = new LinkedList(); //list of LinkedList*

	if(!zone) {
		logDisplay( "ZoneValidator> Invalid zone." );
		return -1;
	}

	LinkedElement* roomElement = zone->rooms.head();

	while( roomElement ) {
		RMRoom* room = (RMRoom*)roomElement->ptr();
		roomElement = roomElement->next();

		bool foundRoom = false;

		//see what chunk roomA is in. If a chunk is not found with roomA
		//in it, we make a new chunk and scavenge for rooms.
		LinkedElement* chunkListElement = chunkList->head();
		while( !foundRoom && chunkListElement ) {
			LinkedList* chunk = (LinkedList*) chunkListElement->ptr();
			chunkListElement = chunkListElement->next();

			//search this chunk for the room
			LinkedElement* roomElement = chunk->head();

			while( !foundRoom && roomElement ) {
				ListedNumber* roomNum = (ListedNumber*) roomElement->ptr();
				roomElement = roomElement->next();

				if( room->number == roomNum->num ) foundRoom = true;
			}

		}

		//we have searched all the chunks, and possibly found the room in one of them.
		//but if we have NOT found the room, we need to make a new chunk from roomA!

		if( !foundRoom ) {
			LinkedList* newChunk = new LinkedList();
			addToChunk( newChunk, room );
			if( newChunk->size() > 0 ) chunkList->add( newChunk );
		}
	}

	//report the chunks
	char filename[255] = "../logs/zonevalidation-";
	strcat( filename, zone->name() );
	strcat( filename, ".txt" );

	FILE* outfile = fopen( filename, "w+" );

	if( !outfile ) {
		logDisplay( "ZoneValidator> Unable to open output file." );
		return -1;
	}


	fprintf( outfile, "----------------------------\r\n" );
	fprintf( outfile, "Zone validation report for zone %s (%s) - %d rooms.\r\n", zone->name(), zone->title, zone->rooms.size() );
	fprintf( outfile, "----------------------------\r\n\r\n" );
	
	fprintf( outfile, "Number of isolated chunks: %d\r\n\r\n", chunkList->size() );

	short chunkNum = 1;

	LinkedElement* chunkListElement = chunkList->head();
	while( chunkListElement ) {
		LinkedList* chunk = (LinkedList*) chunkListElement->ptr();
		chunkListElement = chunkListElement->next();

		fprintf( outfile, "\r\n----------------------------\r\n" );
		fprintf( outfile, "Chunk Num: %d\r\n", chunkNum++ );
		fprintf( outfile, "Number of rooms: %d\r\n", chunk->size() );
		fprintf( outfile, "Room List:\r\n" );

		LinkedElement* roomElement = chunk->head();

		while( roomElement ) {
			ListedNumber* roomNum = (ListedNumber*) roomElement->ptr();
			roomElement = roomElement->next();
			fprintf( outfile, "\t%d\r\n", roomNum->num );
		}

		chunk->release();
	}
	fclose( outfile );

	chunkList->release();
	delete chunkList;

	return (chunkNum-1);
}

void deleteObjects(StringObject *objectToDelete, RMPlayer *player, StringObject *distance)
{
		LinkedElement *element = player->room->objects.head();
		WorldObject *wobjectToDelete = NULL;
		double shortestDistance = 10000.0; //10k

		//validate distance
		if(strcmp(distance->data, "*") && strcmp(distance->data, "closest"))
		{
			roomMgr->sendPlayerText ( player, "|c2|Info> Usage: /delete <objectName> [*, closest]\n");
			return;
		}

		while ( element )
		{
			WorldObject *target = (WorldObject *)element->ptr();

			if(!strcmp(target->name, objectToDelete->data))
			{
				if(!strcmp(distance->data, "closest"))
				{
					double dist = sqrt(((target->x - player->character->x) * (target->x - player->character->x)) + ((target->y - player->character->y) * (target->y - player->character->y)));
					if(dist < shortestDistance)
					{
						shortestDistance = dist;
						wobjectToDelete = target;
					}
				}
				else if(!strcmp(distance->data, "*"))
				{
					player->room->delObject(target);
					roomMgr->sendPlayerText ( player, "|c60|Info> Deleting object %s. It is gone forever!\n", target->name);
					deleteObjects(objectToDelete, player, distance);
					return;
				}
				else
				{
					roomMgr->sendPlayerText ( player, "|c2|Info> Usage: /delete <objectName> [*, closest]\n");
					return;
				}
			}

			element = element->next();
		}

		if(wobjectToDelete != NULL)
		{
			player->room->delObject(wobjectToDelete);
			roomMgr->sendPlayerText ( player, "|c60|Info> Deleting the closest object %s. It is gone forever!\n", wobjectToDelete->name);
			return;
		}
}

void cmdValidateZone ( LinkedList* tokens, char* str, RMPlayer* player )
{
	bool doAllZones = false;

	Zone* zone = player->zone;

	if ( tokens->size() == 2 ) {
		int roomNum = atoi ( ((StringObject *)tokens->at ( 1 ))->data );
		
		if( roomNum == 0 ) doAllZones = true;
		
		RMRoom* rm;
		if( !doAllZones ) {
			rm = roomMgr->findRoom( roomNum );
			if( rm ) zone = rm->zone;
			else zone = NULL;
		}
	}

	if(!doAllZones && !zone) {
		roomMgr->sendPlayerText( player, "ZoneValidator> Invalid zone." );
		return;
	}

	if( doAllZones ) {
		LinkedElement* zoneElement = gZones.head();

		while( zoneElement ) {
			zone = (Zone*) zoneElement->ptr();
			zoneElement = zoneElement->next();

			if( !zone ) {
				roomMgr->sendPlayerText( player, "ZoneValidator> Invalid zone." );
				continue;
			}

			int numChunks = generateZoneValidationReport( zone );

			if( numChunks == -1 ) {
				roomMgr->sendPlayerText( player, "ZoneValidator> Zone '%s'(%s) validation failed.", zone->name(), zone->title );
			}
			else {
				roomMgr->sendPlayerText( player, "ZoneValidator> Zone '%s' - %d chunks reported.", zone->name(), numChunks );
			}
		}
	}
	else {
		int numChunks = generateZoneValidationReport( zone );

		if( numChunks == -1 ) {
			roomMgr->sendPlayerText( player, "ZoneValidator> Zone '%s'(%s) validation failed.", zone->name(), zone->title );
		}
		else {
			roomMgr->sendPlayerText( player, "ZoneValidator> Zone '%s' - %d chunks reported.", zone->name(), numChunks );
		}
	}

	roomMgr->sendPlayerText( player, "ZoneValidator> Zone validation complete." );
}

void cmdValidateRooms ( LinkedList* tokens, char* str, RMPlayer* player )
{
	//logDisplay("Generating Linkage Report for %d zones...", roomMgr->_zones.size() );
	roomMgr->sendPlayerText ( player, "RoomValidator> Validating %d Zones...", roomMgr->_zones.size() );

	FILE *file = fopen ( "../logs/roomlinks.txt", "wb" );

	if( !file ) {
		roomMgr->sendPlayerText ( player, "RoomValidator> Error opening /logs/roomlinks.txt for writing.");
		return;
	}

	fprintf ( file, "-------------------\r\n" );
	fprintf ( file, "Room Linkage Report\r\n" );
	fprintf ( file, "-------------------\r\n\r\n" );

	fprintf ( file, "Validating %d Zones...\r\n\r\n", roomMgr->_zones.size() );

	LinkedElement* zoneElement = roomMgr->_zones.head();

	int tLinkErrors  = 0;
	int tExistErrors = 0;
	int tIsolationErrors = 0;
	int tZoneIsolationErrors = 0;

	while( zoneElement ) {
		int linkErrors  = 0;
		int existErrors = 0;
		int isolationErrors = 0;
		int zoneIsolationErrors = 0;

		Zone* zone = (Zone*)zoneElement->ptr();
		zoneElement = zoneElement->next();

		fprintf ( file, "------------------------------------\r\n" );
		fprintf ( file, "Zone: %s\r\n", zone->name() );
		fprintf ( file, "Validating %d rooms...\r\n", zone->rooms.size() );
		fprintf ( file, "------------------------------------\r\n" );
		
		LinkedElement* roomElement = zone->rooms.head();

		while( roomElement ) {
			bool zoneIsolated = true;

			RMRoom* northRoom = NULL;
			RMRoom* southRoom = NULL;
			RMRoom* eastRoom  = NULL;
			RMRoom* westRoom  = NULL;
			RMRoom* room = (RMRoom*)roomElement->ptr();
			
			roomElement = roomElement->next();

			if( room->north != -1 ) {
				northRoom = roomMgr->findRoom( room->north );

				if( northRoom ) {
					if( northRoom->south != room->number ) {
						fprintf ( file, "\tRoom linkage error - Room %d north = %d  Room %d south ", room->number, room->north, northRoom->number );
						
						if( northRoom->south != -1 ) fprintf ( file, "= %d\r\n", northRoom->south );
						else fprintf ( file, "is not an exit.\r\n" );
						
						linkErrors++;
					}

					if( room->zone == northRoom->zone ) zoneIsolated = false;
				}
				else {
					fprintf ( file, "\tRoom error - Room north of %d (%d) does not exist.\r\n", room->number, room->north );
					existErrors++;
				}
			}

			if( room->south != -1 ) {
				southRoom = roomMgr->findRoom( room->south );

				if( southRoom ) {
					if( southRoom->north != room->number ) {
						fprintf ( file, "\tRoom linkage error - Room %d south = %d  Room %d north ", room->number, room->south, southRoom->number );

						if( southRoom->north != -1 ) fprintf ( file, "= %d\r\n", southRoom->north );
						else fprintf ( file, "is not an exit.\r\n" );

						linkErrors++;
					}
					if( room->zone == southRoom->zone ) zoneIsolated = false;
				}
				else {
					fprintf ( file, "\tRoom error - Room south of %d (%d) does not exist.\r\n", room->number, room->south );
					existErrors++;
				}
			}

			if( room->east != -1 ) {
				eastRoom = roomMgr->findRoom( room->east );

				if( eastRoom ) {
					if( eastRoom->west != room->number ) {
						fprintf ( file, "\tRoom linkage error - Room %d east  = %d  Room %d west ", room->number, room->east, eastRoom->number );

						if( eastRoom->west != -1 ) fprintf ( file, "= %d\r\n", eastRoom->west );
						else fprintf ( file, "is not an exit.\r\n" );

						linkErrors++;
					}

					if( room->zone == eastRoom->zone ) zoneIsolated = false;
				}
				else {
					fprintf ( file, "\tRoom error - Room east  of %d (%d) does not exist.\r\n", room->number, room->east );
					existErrors++;
				}
			}

			if( room->west != -1 ) {
				westRoom = roomMgr->findRoom( room->west );

				if( westRoom ){
					if( westRoom->east != room->number ) {
						fprintf ( file, "\tRoom linkage error - Room %d west  = %d  Room %d east ", room->number, room->west, westRoom->number );

						if( westRoom->east != -1 ) fprintf ( file, "= %d\r\n", westRoom->east );
						else fprintf ( file, "is not an exit.\r\n" );

						linkErrors++;
					}
					if( room->zone == westRoom->zone ) zoneIsolated = false;
				}
				else {
					fprintf ( file, "\tRoom error - Room west  of %d (%d) does not exist.\r\n", room->number, room->west );
					existErrors++;
				}
			}

			if( room->north == -1
			 && room->south == -1
			 && room->east  == -1
			 && room->west  == -1) {
				fprintf( file, "\tRoom isolated - %d\r\n", room->number );
				isolationErrors++;
			 } else if( zoneIsolated ) {
				fprintf( file, "\tRoom zone isolated - %d\r\n", room->number );
				zoneIsolationErrors++;
			 }

		}

		if( linkErrors || existErrors || isolationErrors || zoneIsolationErrors ) {
			fprintf ( file, "\r\n" );
			fprintf ( file, "\tRoom link errors      : %d\r\n", linkErrors );
			fprintf ( file, "\tRoom existance errors : %d\r\n", existErrors );
			fprintf ( file, "\tRoom isolation errors : %d\r\n", isolationErrors );
			fprintf ( file, "\tRoom zone isolation errors : %d\r\n", zoneIsolationErrors );
			fprintf ( file, "\t------------------------------\r\n" );
			fprintf ( file, "\tTotal zone errors     : %d\r\n", linkErrors + existErrors + isolationErrors );
		}
		else {
			fprintf ( file, "Zone OK! No errors.\r\n" );
		}

		tLinkErrors += linkErrors;
		tExistErrors += existErrors;
		tIsolationErrors += isolationErrors;
		tZoneIsolationErrors += zoneIsolationErrors;

		fprintf ( file, "------------------------------------\r\n\r\n");
	}
	
	fprintf ( file, "\r\n" );
	if( tLinkErrors || tExistErrors || tIsolationErrors || tZoneIsolationErrors ) {
		roomMgr->sendPlayerText ( player, "RoomValidator> Total link errors      : %d", tLinkErrors );
		roomMgr->sendPlayerText ( player, "RoomValidator> Total existance errors : %d", tExistErrors );
		roomMgr->sendPlayerText ( player, "RoomValidator> Total isolation errors : %d", tIsolationErrors );
		roomMgr->sendPlayerText ( player, "RoomValidator> Total zone isolation errors : %d", tZoneIsolationErrors );
		roomMgr->sendPlayerText ( player, "RoomValidator> ------------------------------" );
		roomMgr->sendPlayerText ( player, "RoomValidator> Total errors     : %d", tLinkErrors + tExistErrors + tIsolationErrors + tZoneIsolationErrors );
		fprintf ( file, "Total link errors      : %d\r\n", tLinkErrors );
		fprintf ( file, "Total existance errors : %d\r\n", tExistErrors );
		fprintf ( file, "Total isolation errors : %d\r\n", tIsolationErrors );
		fprintf ( file, "Total zone isolation errors : %d\r\n", tZoneIsolationErrors );
		fprintf ( file, "------------------------------\r\n" );
		fprintf ( file, "Total errors     : %d\r\n", tLinkErrors + tExistErrors + tIsolationErrors );
	}
	else {
		roomMgr->sendPlayerText ( player, "RoomValidator> Game OK! No errors. (Good job!)" );
		fprintf ( file, "Game OK! No errors. (Good job!)\r\n" );
	}

	fclose( file );

	roomMgr->sendPlayerText ( player, "RoomValidator> Validation Complete, full report saved to /logs/roomlinks.txt" );
}

void cmdBring ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	RMPlayer *target = player;

	if ( tokens->size() == 2 ) {
		StringObject *name = (StringObject *)tokens->at ( 1 );

		target = findPlayer ( name->data, player );

		if ( target && target->room && target->character ) {
			if ( !target->isTeleporting ) {
				gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_GMCMD, "bring %s %d", target->getName(), player->character->room->number );
				gDataMgr->logPermanent ( target->getLogin(), target->getName(), _DMGR_PLOG_GMCMD, "%s bring %d", player->getName(), player->character->room->number );
				roomMgr->sendPlayerText ( player, "|c71|Info> You are bringing %s here.\n", target->getName() );
				roomMgr->sendPlayerText ( target, "|c71|Info> You are being teleported by %s.\n", player->getName() );

				PackedMsg response;
		
				response.putLong ( target->character->servID );
				response.putLong ( target->character->room->number );

				target->character->teleport ( player->character->room->number, &response );
				response.putByte ( _MOVIE_END );
				
				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), target->room );
				return;
			} else {
				roomMgr->sendPlayerText ( player, "|c60|Info> Teleporting of %s failed, already teleporting.\n", target->getName() );
				return;
			}
		}
	}
	roomMgr->sendPlayerText ( player, "|c71|Info> Who do you want to teleport?\n" );
}

void cmdGoto ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	RMPlayer *target = player;

	if ( tokens->size() == 2 ) {
		StringObject *name = (StringObject *)tokens->at ( 1 );

		target = findPlayer ( name->data, player );

		if ( target && target->room ) {
			if ( !player->isTeleporting ) {
				gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_GMCMD, "goto %s %d", target->getName(), target->character->room->number );
				roomMgr->sendPlayerText ( player, "|c71|Info> You are going to %s.\n", target->getName() );

				PackedMsg response;
		
				response.putLong ( player->character->servID );
				response.putLong ( player->character->room->number );

				player->character->teleport ( target->character->room->number, &response );
				response.putByte ( _MOVIE_END );
				
				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), player->room );
				return;
			} else {
				roomMgr->sendPlayerText ( player, "|c71|Info> You are already teleporting.\n" );
				return;
			}
		}
	}
	roomMgr->sendPlayerText ( player, "|c71|Info> Who do you want to go to?\n" );
}

void cmdHideout ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	int roomNum = 4060;

	roomMgr->sendPlayerText ( player, "|c71|Info> You are going to the hideout.\n" );

	if ( !player->isTeleporting ) {
		PackedMsg response;

		response.putLong ( player->character->servID );
		response.putLong ( player->character->room->number );

		player->character->teleport ( roomNum, &response );
		response.putByte ( _MOVIE_END );
		
		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), player->room );
	} else {
		roomMgr->sendPlayerText ( player, "|c60|Info> Teleport to hideout failed, already teleporting.\n" );
	}
}

void cmdOasis ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	int roomNum = 1050;

	roomMgr->sendPlayerText ( player, "|c71|Info> You are going to the Oasis.\n" );

	if ( !player->isTeleporting ) {
		PackedMsg response;

		response.putLong ( player->character->servID );
		response.putLong ( player->character->room->number );

		player->character->teleport ( roomNum, &response );
		response.putByte ( _MOVIE_END );
		
		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), player->room );
	} else {
		roomMgr->sendPlayerText ( player, "|c60|Info> Teleport to Oasis failed, already teleporting.\n" );
	}
}

void cmdAmbush ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	int roomNum = 98000;

	roomMgr->sendPlayerText ( player, "|c71|Info> You are going to the ambush arena.\n" );

	if ( !player->isTeleporting ) {
		PackedMsg response;

		response.putLong ( player->character->servID );
		response.putLong ( player->character->room->number );

		player->character->teleport ( roomNum, &response );
		response.putByte ( _MOVIE_END );
		
		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), player->room );
	} else {
		roomMgr->sendPlayerText ( player, "|c60|Info> Teleport to arena failed, already teleporting.\n" );
	}
}

void cmdCrime ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;

	CrimeData* crime = player->getCrimeData();

	if ( crime->criminal ) {
		if ( crime->murders ) 
			roomMgr->sendPlayerText ( player, "|c60|Info> You are wanted for %d %s.\n", crime->murders, crime->murders > 1? "murders": "murder" );

	 	if ( crime->pickedPockets ) 
			roomMgr->sendPlayerText ( player, "|c60|Info> You are wanted for picking %d %s.\n", crime->pickedPockets, crime->pickedPockets > 1? "pockets": "pocket" );
	} else {
		roomMgr->sendPlayerText ( player, "|c67|Info> You are not wanted for any crime.\n" );
	}

	if ( crime->bountyOnHead )
		roomMgr->sendPlayerText ( player, "|c60|Info> You have a bounty of %d on your head.\n", crime->bountyOnHead );
	else
		roomMgr->sendPlayerText ( player, "|c67|Info> You have no bounty on your head.\n" );
}

void cmdCrash ( LinkedList *tokens, char *str, RMPlayer *player )
{
	crash();
}

void cmdBounty ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;

	// get the name of the target 
	StringObject *name = (StringObject *)tokens->at ( 1 );	

	// get the amount of money to put
	StringObject *amount = (StringObject *)tokens->at ( 2 );

	// must have both string objects to proceed
	if ( amount && name ) {

		RMPlayer* target = findPlayer ( name->data, player );
		// verify amount and set the bounty on character
		long bounty = atoi ( amount->data );

		// On Line Player
		if ( target ) {
			if ( bounty < 100 ) {
				roomMgr->sendPlayerText ( player, "|c71|Info> The bounty must be least 100 gold.\n" );
				return;
			}

			if ( player->character->canAfford ( bounty ) ) {

				player->character->value -= bounty;

				CrimeData* crime = target->getCrimeData();
				crime->bountyOnHead += bounty;
				target->writeCrimes();

				roomMgr->sendPlayerText ( player, "|c71|Info> You have placed a bounty of %d gold on the head of %s. The gold will be withdrawn from your inventory\n", bounty, name->data );

			} else 
				roomMgr->sendPlayerText ( player, "|c60|Info> You haven't enough gold.\n" );
		} else {
                        if ( bounty < 100 ) {
                                roomMgr->sendPlayerText ( player, "|c60|Info> The bounty must be least 100 gold.\n" );
                                return;
                        }

                        if ( player->character->canAfford ( bounty ) ) {

                                player->character->value -= bounty;

				gDataMgr->placeBounty( player->character, name->data, bounty );
                        } else
                                roomMgr->sendPlayerText ( player, "|c60|Info> You haven't enough gold.\n" );
		}
	} else 
		roomMgr->sendPlayerText ( player, "|c71|Usage: /bounty <character name> <amount>.\n" );
}

void cmdEvict ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	RMRoom *room = player->room;

	if ( room ) {

		Building *building = room->building;

		if ( building && !strcasecmp ( player->character->getName(), building->_owner ) ) {

			// step through everyone in the room and toss them out
			LinkedElement *element = building->players.head();

			while ( element ) {
				RMPlayer *squatter = (RMPlayer *)element->ptr();
				element = element->next();

				if (squatter != player ) {
					if ( squatter->checkAccess ( _ACCESS_IMPLEMENTOR ) ) {
						roomMgr->sendPlayerText ( squatter, "|c71|Info> %s tried to evict you.\n", player->getName() );
						roomMgr->sendPlayerText ( player, "|c71|Info> You can not evict Implementor %s from your house!!\n", squatter->getName() );
					} else if ( squatter->character && !squatter->isNPC ) {
						roomMgr->sendPlayerText ( squatter, "|c60|Info> %s has evicted you from %s house.\n", player->getName(), player->character->getPronoun ( _PRONOUN_HIS ) );
						roomMgr->sendPlayerText ( player, "|c60|Info> You have evicted %s from your house.\n", squatter->getName() );

						if ( !squatter->isTeleporting ) {

							PackedMsg response;

							response.putLong ( squatter->character->servID );
							response.putLong ( squatter->character->room->number );

							squatter->character->teleport ( random ( 5000, 5089 ), &response );

							response.putByte ( _MOVIE_END );

							roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), squatter->room );
						}
					}
				}
			}

			roomMgr->sendPlayerText ( player, "|c12|Info> Your house is now clear of squatters!\n" );
		} else {
			roomMgr->sendPlayerText ( player, "|c71|Info> You can not use the /evict command unless you are in your house.\n" );
		}
	}
}

void cmdComplain ( LinkedList *tokens, char *str, RMPlayer *player )
{
	static int sendCount = 0;

	if ( !player->checkAccess ( _ACCESS_GUIDE ) ) {
		roomMgr->sendSystemMsg ( "Information", player, "We have improved the method that we use to track issues within the game.  To that end, we no longer support the general /complain feature.  Please use the following guidelines to get your message to us:\n\n1. If you wish to report someone for rude behavior, profanity, harassment or any other chat-realated offense, please use the /report command  (For example, to file a report on Bob type '/report bob').\n\n2. If you wish to report a bug, please visit http://www.realmserver.com/support and follow the on-screen instructions for contacting our Technical Support staff.\n\n3. If you have a general customer support issue (i.e. lost password, missing characters, hacked account, billing problems, etc.) visit http://www.realmserver.com/support and follow the on-screen instructions for contacting our Customer Support staff.\n\n3. If you wish to make a suggestion for improving the game, please visit http://www.realmserver.com/support and follow the on-screen instructions for making a product suggestion." );
		return;
	}

	if ( tokens->size() == 1 ) {
		roomMgr->sendSystemMsg ( "Explanation Required", player, "You must provide an explanation for this /complain." );
		return;
	}

	player->reportText( NULL, str, "COMPLAIN" );

	roomMgr->sendSystemMsg ( "GM Report Filed", player, "Your GM report has been filed." );
}

void cmdReport ( LinkedList *tokens, char *str, RMPlayer *player )
{
	static int sendCount = 0;

	if ( !player || !player->character )
		return;

	if ( tokens->size() < 2 ) {
		roomMgr->sendSystemMsg ( "Unable To File A Report", player, "You must provide the name of the character that you wish to file a report on  (i.e. '/report bob').\n\nThe last 100 lines of chat that you have seen is included in the report for our reference while dealing with the offense.  If the last 100 lines of chat text do not include any messages from the person you are reporting, the report will not be filed.\n\nPlease only file reports when a player policy violation has occured.  You can find the current player policy at http://www.realmserver.com/policy.  Be aware that repeated filing of false reports is cause for disciplinary action.  Please only report offensive actions.");
		return;
	}

	StringObject *name = (StringObject *)tokens->at ( 1 );

	player->reportText( name->data, str, "Report" );

	roomMgr->sendSystemMsg ( "Report Filed", player, "Your report against %s has been filed.  The last 100 lines of chat text has been sent along with the report for investigation.", name->data );
}

void cmdGold ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_IMPCMD, "gold" );

	player->character->value += 100000000;
	player->character->manaValue += 100000000;
	roomMgr->sendPlayerText ( player, "|c71|Info> You have been given 100000000 gold coins and 100000000 mana crystals.\n" );
}

void cmdHouse ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	if ( tokens->size() > 1 ) {

		StringObject *name = (StringObject *)tokens->at ( 1 );
		StringObject *password =  new StringObject ( "password" );

		int crest = 0;

		if ( tokens->size() > 2 )
			password = (StringObject *)tokens->at ( 2 );

		if ( tokens->size() > 3 )
			crest = 1;

		if ( name->data ) {
			makeHouse( name->data, strlower ( password->data ), crest );
			roomMgr->sendPlayerText ( player, "|c71|New house constructed for %s.\n", name->data );
		} else {
			roomMgr->sendPlayerText ( player, "|c71|Usage /house <character name> <password> [crest].\n" );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c71|Usage /house <character name> [password - optional] [crest - optional].\n" );
	}
}

void cmdStamina ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( player && player->character )
		roomMgr->sendPlayerText ( player, "|c71|Info> Your stamina is %d of %d.\n", player->character->stamina, 250 );
}

void cmdWeight ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	player->character->calcWeightCap();

	BContainer *bcontain = (BContainer *)player->character->getBase ( _BCONTAIN );

	if ( bcontain ) {
		int theWeight = bcontain->calculateWeight();
		roomMgr->sendPlayerText ( player, "|c71|Info> You are carrying %d stones in equipment.  Your carrying capacity is %d stones.  You are %d%% encumbered.\n", theWeight / 10, bcontain->weightCapacity / 10, ( theWeight * 100) / bcontain->weightCapacity );
//		roomMgr->sendPlayerText ( player, "|c43|Info> You are carrying %d stones in equipment.  Your carrying capacity is %d stones.  You are %d%% encumbered.\n", theWeight bcontain->weight / 10, bcontain->weightCapacity / 10, (bcontain->weight * 100) / bcontain->weightCapacity );
	}
}

void cmdAccountInfo ( LinkedList *tokens, char *str, RMPlayer *player )
{
	time_t expireTime = player->billingDate;
	tm* expire = localtime( &expireTime );
						
	roomMgr->sendSystemMsg ( "Account Information", player, "Your account type is: %s\nYour billing date is: %d-%d-%d\n\nIf you have not paid for this account using the new payment system, your account will expire on the date specified above.  Otherwise, if you have paid using a credit card, the date above reflects when your credit card will be automatically billed in the future.  If you paid with a check or money order, the date above reflects when your account will expire.", player->accountTypeStr, (expire->tm_mon + 1 ), expire->tm_mday, ( expire->tm_year + 1900 ) );
}

void cmdNoGrace ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	WorldObject *obj = player->character;

	if ( obj ) {
		if ( obj->pvpGrace > 0 ) {
			obj->pvpGrace = 0;
			roomMgr->sendPlayerText ( player, "|c60|Info> You have forefitted your player-killer grace period.\n" );
		}

		if ( obj->pvpGrace == -1 ) {
			roomMgr->sendPlayerText ( player, "|c60|Info> You are already not accepting player-killer grace periods.\n" );
		} else {
			roomMgr->sendPlayerText ( player, "|c71|Info> You are now not accepting player-killer grace periods.\n" );
			obj->pvpGrace = -1;
		}
	}
}

void cmdYesGrace ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	WorldObject *obj = player->character;

	if ( obj ) {
		if ( obj->pvpGrace == -1 ) {
			roomMgr->sendPlayerText ( player, "|c71|Info> You are now accepting player-killer grace periods.\n" );
			obj->pvpGrace = 0;
		} else {
			roomMgr->sendPlayerText ( player, "|c71|Info> You are already accepting player-killer grace periods.\n" );
		}
	}
}

void cmdYesCombat ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	if ( player->character->character->peaceful ) {
		roomMgr->sendSystemMsg ( "Player Combat Active!", player, "You are now able to be attacked by other players." );
		player->character->character->peaceful = 0;
	} else {
		roomMgr->sendSystemMsg ( "Player Combat Already Active!", player, "You are already able to be attacked by other players." );
	}
}

void cmdNoCombat ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	if ( player->character->character->peaceful ) {
		roomMgr->sendSystemMsg ( "Player Combat Already Off!", player, "You are already immune from player-killers." );
	} else {
		int lastCrimeTime = 2592000 - (getseconds() - player->character->lastCrimeTime);

		if ( lastCrimeTime > 0 ) {
			int upDays = lastCrimeTime / 86400;
			lastCrimeTime -= upDays * 86400;
			int upHours = lastCrimeTime / 3600;
			lastCrimeTime -= upHours * 3600;
			int upMinutes = lastCrimeTime / 60;
			lastCrimeTime -= upMinutes * 60;
			int upSeconds = lastCrimeTime;

			roomMgr->sendSystemMsg ( "As If!", player, "You can't do that until you've refrained from attacking players and picking pockets for %d:%d:%d:%d.", upDays, upHours, upMinutes, upSeconds );
		} else {
			roomMgr->sendSystemMsg ( "Player Combat Off!", player, "You are now immune from player-killers until you either attempt to kill a player or pick a pocket." );
			player->character->character->peaceful = 1;
		}
	}
}

void cmdCreate ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !str || !player || !player->character )
		return;

	if ( tokens->size() > 1 ) {
		WorldObject *object = roomMgr->findComponentObject ( _SKILL_WEAPONSMITH, str + 7 );

		if ( object == (WorldObject *)-1 ) {
			roomMgr->sendPlayerText ( player, "|c71|Info> You'll have to be more specific than that.\n" );
		}

		else if ( object == NULL ) {
			roomMgr->sendPlayerText ( player, "|c60|Info> You don't know how to make that.\n" );
		}

		else {
			roomMgr->sendPlayerText ( player, "|c71|Info> Search found %s.\n", object->classID );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c71|Info> You must provide an object description.\n" );
	}
}

void cmdShutdown ( LinkedList *tokens, char *str, RMPlayer *player )
{
	LinkedElement *element = tokens->head()->next();

	unsigned int shutTimer = 300;

	if ( element ) 
		shutTimer = atoi ( ((StringObject *)element->ptr())->data );

	char* pStr = &str[ 9 ];

	while ( *pStr && *pStr == ' ' )
		pStr++;

	while ( *pStr && *pStr != ' ' )
		pStr++;

	while ( *pStr && *pStr == ' ' )
		pStr++;

	unsigned int timeHours = shutTimer / 3600;
	unsigned int timeMinutes = ( shutTimer - ( timeHours * 3600) ) / 60;
	unsigned int timeSeconds = shutTimer - ( timeHours * 3600) - ( timeMinutes * 60);

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

	if ( shutTimer ) {
		if ( pStr ) {
			roomMgr->sendSystemMsg ( "The game will be shutting down in %s.\n%s", timeText, pStr );
			roomMgr->sendListChat ( player, roomMgr->players(), "|c60|Shutdown> The game will be shutting down in %s.\n%s", timeText, pStr );
			strcpy( gShutdownMessage, pStr );
		} else {
			roomMgr->sendSystemMsg ( "The game will be shutting down in %s. It will be back up shortly.", timeText );
			roomMgr->sendListChat ( player, roomMgr->players(), "|c60|Shutdown> The game will be shutting down in %s.  It will be back up shortly.", timeText );
			gShutdownMessage[ 0 ] = 0;
		}
	}

	gShutdownTimer = shutTimer;
}

void EmoteFunc ( LinkedList *tokens, char *str, RMPlayer *player, char *str1, char *str2, char *str3, char *str4, char *str5, char *str6, char *str7, char *str8 ) 
{ 
	if ( !player || !player->character || !player->room )
		return;

	if ( player->checkTimedAccess ( _ACCESS_GAGGED ) ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You are not allowed to emote while gagged.\n" );
		return;
	}

	LinkedList *players = player->room->copy(); 

	if ( players->contains ( player ))
		players->del ( player ); 

	// strip out any players that are being ignored
	LinkedElement *element = players->head();

	while ( element ) {
		RMPlayer *thePlayer = (RMPlayer *)element->ptr();
		element = element->next();

		if ( thePlayer->isIgnoring ( player->getName() ) )
			players->del ( thePlayer );
	}

	if ( tokens->size() > 1 ) { 
		RMPlayer *targetPlayer = player->room->findPlayerByName ( ((StringObject *)tokens->at ( 1 ))->data );  

		if ( targetPlayer ) {
			if ( targetPlayer->checkAccess ( _ACCESS_GUIDE ) ) {
				roomMgr->sendPlayerText ( player, str6, ((StringObject *)tokens->at ( 1 ))->data ); 
			} else if ( targetPlayer == player ) { 
				roomMgr->sendPlayerText ( player, str1 ); 
				if ( players->size() )
					roomMgr->sendListText ( players, str2, player->getName(), player->getPronoun() ); 
			} else { 
				if ( targetPlayer->isIgnoring ( player->getName() ) ) {
					roomMgr->sendPlayerText ( player, "|c60|Info> %s is ignoring you!\n", targetPlayer->getName() );
				} else {
					players->del ( targetPlayer );  
					roomMgr->sendPlayerText ( player, str3, targetPlayer->getName() ); 
					roomMgr->sendPlayerText ( targetPlayer, str4, player->getName() ); 

					if ( players->size() )
						roomMgr->sendListText ( players, str5, player->getName(), targetPlayer->getName() ); 
				}
			} 
		} else { 
			roomMgr->sendPlayerText ( player, str6, ((StringObject *)tokens->at ( 1 ))->data ); 
		} 
	} else { 
		roomMgr->sendPlayerText ( player, str7 );	
		if ( players->size() )
			roomMgr->sendListText ( players, str8, player->getName() ); 
	} 

	players->release();
	delete players;
}

// extern rwlock_t gMsgProcLock;

void cmdSmile ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You smile at yourself, cuz you crazy.\n", "%s smiles at %sself.\n", "You smile at %s.\n", "%s smiles at you.\n", "%s smiles at %s.\n", "You can't find '%s' to smile at.\n", "You smile.\n", "%s smiles.\n" );
}

void cmdGrin ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You grin at yourself.\n", "%s grins at %sself.\n", "You grin at %s.\n", "%s grins at you.\n", "%s grins at %s.\n", "You can't find '%s' to grin at.\n", "You grin.\n", "%s grins.\n" );
}

void cmdGrimace ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You grimace at yourself.\n", "%s grimaces at %sself.\n", "You grimace at %s.\n", "%s grimaces at you.\n", "%s grimaces at %s.\n", "You can't find '%s' to grimace at.\n", "You grimace.\n", "%s grimaces.\n" );
}

void cmdComfort ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You comfort yourself.\n", "%s comforts %sself.\n", "You comfort %s.\n", "%s comforts you.\n", "%s comforts %s.\n", "You can't find '%s' to comfort.\n", "Who do you want to comfort?\n", "%s starts to do something but stops.\n" );
}

void cmdTickle ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You tickle yourself.\n", "%s tickles %sself.\n", "You tickle %s.\n", "%s tickles you.\n", "%s tickles %s.\n", "You can't find '%s' to tickle.\n", "Who do you want to tickle?", "%s starts to do something but stops.\n" );
}

void cmdPat ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You pat yourself on the head.\n", "%s pats %sself on the head.\n", "You pat %s on the head.\n", "%s pats you on the head.\n", "%s pats %s on the head.\n", "You can't find '%s' to pat.\n", "Who do you pat?\n", "%s starts to do something but stops.\n" );
}

void cmdEye ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You eye yourself up and down.\n", "%s eyes %sself up and down.\n", "You eye %s up and down.\n", "%s eyes you up and down.\n", "%s eyes %s up and down.\n", "You can't find '%s' to eye at.\n", "Who do you want to eye?\n", "%s starts to do something but stops.\n" );
}

void cmdStare ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You stare at yourself.\n", "%s stares at %sself.\n", "You stare at %s.\n", "%s stares at you.\n", "%s stares at %s.\n", "You can't find '%s' to stare at.\n", "You stare off into space.\n", "%s stares off into space.\n" );
}

void cmdWorship ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You worship yourself (how vain).\n", "%s worships %sself (how vain).\n", "You worship %s.\n", "%s worships you.\n", "%s worships %s.\n", "You can't find '%s' to worship.\n", "You worship The Realm gods.\n", "%s worships The Realm gods.\n" );
}

void cmdHug ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You hug yourself.\n", "%s hugs %sself.\n", "You hug %s.\n", "%s hugs you.\n", "%s hugs %s.\n", "You can't find '%s' to hug.\n", "Who do you want to hug?\n", "%s starts to do something but stops.\n" );
}

void cmdDismiss ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You wave yourself away.\n", "%s waves %sself away.\n", "You wave %s away.\n", "%s waves you away.\n", "%s waves %s away.\n", "You can't find '%s' to dismiss.\n", "You wave everyone away.\n", "%s waves everyone away.\n" );
}

void cmdAgree ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You agree with yourself.\n", "%s agrees with %sself.\n", "You agree with %s.\n", "%s agrees with you.\n", "%s agrees with %s.\n", "You can't find '%s' to agree with.\n", "You agree wholeheartedly.\n", "%s agrees.\n" );
}

void cmdLaugh ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You laugh at yourself.\n", "%s laughs at %sself.\n", "You laugh at %s.\n", "%s laughs at you.\n", "%s laughs at %s.\n", "You can't find '%s' to laugh at.\n", "You laugh out loud!\n", "%s laughs out loud!\n" );
}

void cmdWink ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You wink at yourself.\n", "%s winks at %sself.\n", "You wink at %s.\n", "%s winks at you.\n", "%s winks at %s.\n", "You can't find '%s' to wink at.\n", "You wink.\n", "%s winks.\n" );
}

void cmdKiss ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You kiss yourself.\n", "%s kisses %sself.\n", "You kiss %s.\n", "%s kisses you.\n", "%s kisses %s.\n", "You can't find '%s' to kiss.\n", "You blow a kiss.\n", "%s blows a kiss.\n" );
}

void cmdCry ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You cry.\n", "%s cries.\n", "You cry on %s's shoulder.\n", "%s cries on your shoulder.\n", "%s cries on %s's shoulder.\n", "You can't find '%s' to cry on.\n", "You cry.\n", "%s cries.\n" );
}

void cmdFrown ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You frown at yourself.\n", "%s frowns at %sself.\n", "You frown at %s.\n", "%s frowns at you.\n", "%s frowns at %s.\n", "You can't find '%s' to frown at.\n", "You frown.\n", "%s frowns.\n" );
}

void cmdBow ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You bow in your own honor.\n", "%s bows in his own honor.\n", "You bow before %s.\n", "%s bows before you.\n", "%s bows before %s.\n", "You can't find '%s' to bow before.\n", "You bow deeply.\n", "%s bows deeply.\n" );
}

void cmdNod ( LinkedList *tokens, char *str, RMPlayer *player )
{
	EmoteFunc ( tokens, str, player, "You nod at yourself.\n", "%s nods at %sself.\n", "You nod at %s.\n", "%s nods at you.\n", "%s nods at %s.\n", "You can't find '%s' to nod at.\n", "You nod your head.\n", "%s nods.\n" );
}

void cmdWho ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	PackedData packet;

	// put the packet header
	packet.putLong ( 0 );
	packet.putLong ( 0 );

	packet.putByte ( 1 );

	// go through the list of players and put their names in the packet
	LinkedElement *element = roomMgr->players()->head();

	while ( element ) {
		RMPlayer * rPlayer = (RMPlayer *)element->ptr();

		if (!rPlayer->checkAccess( _ACCESS_GUIDE ) ) {
			WorldObject *character = rPlayer->character;

			if ( character ) {
				BCharacter *base = (BCharacter *)character->getBase ( _BCHARACTER );

				if ( base ) {
					packet.putString ( base->properName );
					packet.putString ( base->title );
				}
			}
		}

		element = element->next();
	}

	packet.putWord( 0 );

	roomMgr->sendTo ( _IPC_PLAYER_WHO, (IPCPMMessage *)packet.data(), packet.size(), player );
}

void cmdEventWho ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;

	PackedData packet;

	// put the packet header
	packet.putLong ( 0 );
	packet.putLong ( 0 );
	
	packet.putByte ( 4 );
	
	// go through the list of players and put their names in the packet
	LinkedElement *element = roomMgr->_hosts.head();

	while ( element ) {
		RMPlayer * rPlayer = (RMPlayer *)element->ptr();
		element = element->next();
		
		if ( rPlayer->checkAccess( _ACCESS_ONLY_EVENT ) ) {
			WorldObject *character = rPlayer->character;
		
			if ( character ) {
				BCharacter *base = (BCharacter *)character->getBase ( _BCHARACTER );
			
				if ( base ) {
					packet.putString( base->properName );
					packet.putString( base->title );
				}
			}
		}
	}

	packet.putWord( 0 );
	
	roomMgr->sendTo ( _IPC_PLAYER_WHO, (IPCPMMessage *)packet.data(), packet.size(), player );
}

void cmdChannels ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	PackedData packet;
	packet.putLong ( 0 );
	packet.putLong ( 0 );
	packet.putByte ( 0 );
	packet.putLong ( 0 );

	int count = 0;

	for ( int i=0; i<_MAX_CHANNEL; i++ ) {
		Channel *channel = gChannels[i];
		char str[1024];

		if ( channel->isSystem || channel->members.size() ) {
			char *pModName = channel->getTopModeratorName();

			sprintf ( sizeof ( str ), str, "%d", channel->number );

			packet.putString( str );
			packet.putString( channel->name );

			if ( channel->getPassword() )
				packet.putString( "no" );
			else
				packet.putString( "yes" );

			packet.putString( pModName );

			sprintf ( sizeof ( str ), str, "%d", ( channel->members.size() - channel->gms.size() ) ); 
			packet.putString( str );

			count++;
		}
	}

	packet.setLong( 9, count );

	roomMgr->sendTo ( _IPC_PLAYER_WHO, (IPCPMMessage *)packet.data(), packet.size(), player );
}

void cmdChannelMembers ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	if ( player->channel != NULL ) {
		PackedData packet;
		packet.putLong ( 0 );
		packet.putLong ( 0 );

		packet.putByte ( 2 );

		player->channel->listMembers( &packet );

		roomMgr->sendTo ( _IPC_PLAYER_WHO, (IPCPMMessage *)packet.data(), packet.size(), player );
	} else {
		roomMgr->sendPlayerText ( player, "|c60|Info> You are not a member of any gossip channel.\n" );
	}
}

void cmdChannelBannedMembers ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	if ( player->channel != NULL ) {
		PackedData packet;
		packet.putLong ( 0 );
		packet.putLong ( 0 );

		packet.putByte ( 3 );

		player->channel->listBanned( &packet );

		roomMgr->sendTo ( _IPC_PLAYER_WHO, (IPCPMMessage *)packet.data(), packet.size(), player );
	} else {
		roomMgr->sendPlayerText ( player, "|c60|Info> You are not a member of any gossip channel.\n" );
	}
}

void cmdInfo ( LinkedList *tokens, char *str, RMPlayer *player )
{
	char *cmd = ((StringObject *)tokens->at ( 0 ))->data;

	if ( !str || !cmd )
		return;

	roomMgr->sendListChat ( player, roomMgr->players(), "-i|c62|Info> %s: %s\n", player->getName(), str + strlen ( cmd )  );
}

void cmdWorldQuake ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;

	PackedMsg response;

	// put ACK info
	response.putACKInfo ( _IPC_ROCKING );

	roomMgr->sendToList ( _IPC_PLAYER_ACK, response.data(), response.size(), roomMgr->players() );
}

void cmdZoneQuake ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character || !player->zone )
		return;

	PackedMsg response;

	// put ACK info
	response.putACKInfo ( _IPC_ROCKING );

	roomMgr->sendToList( _IPC_PLAYER_ACK, response.data(), response.size(), &player->zone->players );
}

void cmdRoomQuake ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character || !player->room )
		return;

	PackedMsg response;

	// put ACK info
	response.putACKInfo ( _IPC_ROCKING );

	roomMgr->sendToRoom( _IPC_PLAYER_ACK, response.data(), response.size(), player->room );
}

void cmdGodWindow ( LinkedList *tokens, char *str, RMPlayer *player )
{
	char *cmd = ((StringObject *)tokens->at ( 0 ))->data;

	if ( !str || !cmd )
		return;

	roomMgr->sendListChat ( player, roomMgr->players(), "-7|c62|%s: %s\n", player->getName(), str + strlen ( cmd )  );
}

int bEventHappening = 0;
int bEventReadOnly = 0;

char* gEventTitle = NULL;
char* gEventInformation = NULL;

LinkedList gEventPlayers;
LinkedList gBadEventPlayers;

void cmdOpenEvent ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( tokens->size() == 3 ) {
		if ( bEventHappening ) {
			roomMgr->sendPlayerText( player, "-8t%s", gEventTitle );
			roomMgr->sendPlayerText( player, "|c60|Info> There is already an event going on.\n" );
		} else {
			bEventHappening = 1;

			gEventPlayers.copy( roomMgr->players() );
		
			gEventTitle = strdup( ((StringObject *)tokens->at ( 1 ))->data );
			gEventInformation = strdup( ((StringObject *)tokens->at ( 2 ))->data );
		
			roomMgr->sendListChat ( player, &gEventPlayers, "-8t%s", gEventTitle );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|Info> Event open failed.\n" );
	}
}

void cmdEventInformation( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;
	
	if ( bEventHappening ) {
		roomMgr->sendPlayerText( player, "-8t%s", gEventTitle );
		roomMgr->sendSystemMsg ( gEventTitle, player, gEventInformation );
	}
}

void cmdEvent ( LinkedList *tokens, char *str, RMPlayer *player ) {
	static int nLines = 0;

	char *cmd = ((StringObject *)tokens->at ( 0 ))->data;

	if ( !str || !cmd )
		return;

	if ( bEventHappening ) {
		if ( player->checkAccess( _ACCESS_EVENT | _ACCESS_PUBLICRELATIONS ) ) {
			roomMgr->sendListChat ( player, &gEventPlayers, "-8 |c255|%s: |c19|%s\n", player->getName(), str + strlen ( cmd )  );
		} else {
			if ( gEventPlayers.contains( player ) && !bEventReadOnly ) {
				int nColor = ( (nLines++) & 0x00000001) ? 85 : 86;

				roomMgr->sendListChat ( player, &gEventPlayers, "-8 |c255|%s: |c%d|%s\n", player->getName(), nColor, str + strlen ( cmd )  );
			}
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|Info> There is no event going on right now.\n" );
	}
}

void cmdEventClose ( LinkedList *tokens, char *str, RMPlayer *player )
{
	char *cmd = ((StringObject *)tokens->at ( 0 ))->data;

	if ( !str || !cmd )
		return;

	if ( bEventHappening ) {
		if ( player->checkAccess( _ACCESS_EVENT | _ACCESS_PUBLICRELATIONS ) ) {
			StringObject *name = (StringObject *)tokens->at ( 1 );

			if ( name ) {
				RMPlayer *target = findPlayer ( name->data, player );

				if ( target && !target->checkAccess( _ACCESS_EVENT | _ACCESS_PUBLICRELATIONS ) ) {
					gEventPlayers.del ( target );

					roomMgr->sendPlayerText ( target, "-8c" );
					gBadEventPlayers.add( (ListObject*) target->accountID );
				} else {
					roomMgr->sendPlayerText ( player, "-8 Can not find %s to kick!", name->data );
				}
			} else {
				roomMgr->sendListChat ( player, &gEventPlayers, "-8c" );
				bEventHappening = 0;

				gEventPlayers.release();
				gBadEventPlayers.release();

				delete gEventTitle;
				delete gEventInformation;
				
				gEventTitle = NULL;
				gEventInformation = NULL;
			}
		} else {
			gEventPlayers.del( player );
			roomMgr->sendPlayerText ( player, "-8c" );
		}
	}
}


void cmdGossip ( LinkedList *tokens, char *str, RMPlayer *player )
{
	static int nLines[ 1000 ] = { 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	if ( !str || !player || !player->character )
		return;

//	if ( !player->checkAccess( _ACCESS_IMPLEMENTOR ) ) {
		int len = strlen ( str );
	
		if ( len < 1 || len > 320 ) {
			return;
		}
//	}

	if ( strchr ( str, '|' ) ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> No text color changes or phony fonts!\n" );
		return;
	}

	if ( player->checkTimedAccess ( _ACCESS_NO_GOSSIP ) ) {
		int timeLeft = player->gossipBanTime - getseconds();

		if ( timeLeft > 0 ) {
			timeLeft /= 60;
			timeLeft = std::max( 1, timeLeft );
//1 >? timeLeft;

			roomMgr->sendPlayerText ( player, "|c60|Info> You are not allowed to send gossip messages for %d %s.\n", timeLeft, (timeLeft > 1)? "minutes" : "minute" );
			return;
		}
	}

	if ( player->checkTimedAccess ( _ACCESS_GAGGED ) ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You are not allowed to send gossip messages while gagged.\n" );
		return;
	}

	if ( player->channel == NULL ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You are not a member of any gossip channel.\n" );
		return;
	}

	Channel *channel = player->channel;

	if ( channel->isReadOnly ) {

		if ( channel->number == 4 ) {
			if ( !player->checkAccess ( _ACCESS_GUIDE ) ) {
				roomMgr->sendPlayerChat ( player, player, "|c60|Info> We're sorry, but this gossip channel is currently read-only, try again later.\n" );
				return;
			}
		} else {
 			if ( !player->checkAccess ( _ACCESS_MODERATOR ) ) {
 				roomMgr->sendPlayerChat ( player, player, "|c60|Info> We're sorry, but this gossip channel is currently read-only, try again later.\n" );
				return;
			}
		}
	}

	LinkedList list;
	LinkedList gmList;

	int totalMembers = channel->members.size();
	int ignoringMembers = 0;

	// filter all players that are ignoring me
	LinkedElement *element = channel->members.head();

	while ( element ) {

		RMPlayer *thePlayer = (RMPlayer *)element->ptr();
		element = element->next();

		if ( isValidPtr ( thePlayer ) && (getPtrType ( thePlayer ) == _MEM_PLAYER) ) {
			if ( thePlayer->checkAccess( _ACCESS_MODERATOR ) )
				gmList.add ( thePlayer);
			else 
				if ( !thePlayer->isIgnoring ( player->getName() ) )
					list.add ( thePlayer );
				else
					ignoringMembers++;
		}
	}

	int tGMListSize = FALSE;

	if ( gmList.size() ) {
		tGMListSize = TRUE;
	}

	// at least half of the gossip channel that I'm on is ignoring me... auto revoke
	if ( !player->checkAccess ( _ACCESS_MODERATOR ) && channel->isSystem && totalMembers > 25 && (ignoringMembers >= totalMembers / 3) ) {
		player->setAccess ( _ACCESS_NO_GOSSIP );

		gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_IMPCMD, "auto-revoked by channel members" );

		roomMgr->sendSystemMsg ( "You Must Have Annoyed Someone", player, "Your gossip rights have been revoked because %d of the %d people on this channel are ignoring you.  I guess that means they don't want to hear from you anymore.  This revoke will last for sixty minutes.\n", ignoringMembers, totalMembers );

		list.release();
		gmList.release();
		return;
	}

	// check for flood
	int gossipTime = getseconds() - player->badGossipTime;
	player->badGossipTime = getseconds();

	if ( channel->isSystem ) {
		if ( gossipTime <= 5 )
			player->badGossipCount++;

		else if ( gossipTime > 2400 && player->badGossipCount > 0 )
			player->badGossipCount = 0;

		else if ( gossipTime > 15 && player->badGossipCount > 0 )
			player->badGossipCount--;

		if ( player->badGossipCount >= 15 ) {
			player->setAccess ( _ACCESS_NO_GOSSIP );

			gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_IMPCMD, "auto-revoked for channel spam" );

			roomMgr->sendSystemMsg ( "Gossip Revoked", player, "Your gossip rights have been revoked for misuse of the gossip channel.  You either sent too many gossip messages too quickly or you tried to send profane messages.  This revoke will last for one hour.\n" );

			list.release();
			gmList.release();

			return;
		}
	}

	char *cmd = ((StringObject *)tokens->at ( 0 ))->data;

	if ( !cmd ) {
		list.release();
		gmList.release();
		return;
	}

	logChatData ( "%s:%s:G%03d:%s", player->getLogin(), player->getName(), channel->number, str + strlen ( cmd ) );
//	logChatData ( "%s: GOSSIP(%d):%s", player->getName(), channel->number, str + strlen ( cmd ) );

	char *txt = str + strlen ( cmd );

	if ( !strlen ( txt ) ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> No blank gossip messages allowed.\n" );

	} else {

		char *login = "<unknown>";

		if ( tGMListSize ) {
			BPlayer *base = (BPlayer *)player->player->getBase ( _BPLAYER );

			if ( base ) 
				login = base->login;

			if ( !login )
				login = "<bad base login>";
		}

		char *tempText = strdup ( txt );
		strlower ( tempText );

		int index = 0;

		while ( channel->isSystem && profaneTable[index] ) {
			if ( strstr ( tempText, profaneTable[index] ) ) {
				roomMgr->sendPlayerText ( player, "|c60|Info>  That gossip message contains what appears to be profanity.  Please try saying that a different way.\n" );
				player->badGossipCount += 2;
				goto end;
			}

			index++;
		}

		if ( player->checkAccess ( _ACCESS_IMPLEMENTOR ) ) {
			roomMgr->sendListChat ( player, &list, "|c56|%s>|c20|%s: %s\n", channel->name, player->getName(), str + strlen ( cmd )  );

			if ( tGMListSize ) 
				roomMgr->sendListChat ( player, &gmList, "|c56|%s>|c20|%s: %s\n", channel->name, player->getName(), str + strlen ( cmd )  );

		} else if ( player->checkAccess( _ACCESS_PUBLICRELATIONS ) ) {
			roomMgr->sendListChat ( player, &list, "|c56|%s>|c5|%s: %s\n", channel->name, player->getName(), str + strlen ( cmd )  );

            if ( tGMListSize )
				roomMgr->sendListChat ( player, &gmList, "|c56|%s>|c5|%s: %s\n", channel->name, player->getName(), str + strlen ( cmd )  );

		} else if ( player->checkAccess ( _ACCESS_MODERATOR ) ) {
			roomMgr->sendListChat ( player, &list, "|c56|%s>|c65|%s: %s\n", channel->name, player->getName(), str + strlen ( cmd )  );
			if ( tGMListSize ) 
				roomMgr->sendListChat ( player, &gmList, "|c56|%s>|c65|(%s) %s: %s\n", channel->name, login, player->getName(), str + strlen ( cmd )  );

		} else if ( player->checkAccess ( _ACCESS_GUIDE ) ) {
			roomMgr->sendListChat ( player, &list, "|c56|%s>|c62|%s: %s\n", channel->name, player->getName(), str + strlen ( cmd )  );
			
			if ( tGMListSize ) 
				roomMgr->sendListChat ( player, &gmList, "|c56|%s>|c62|(%s) %s: %s\n", channel->name, login, player->getName(), str + strlen ( cmd )  );
		} else if ( player->checkAccess ( _ACCESS_EVENT ) ) {
			roomMgr->sendListChat ( player, &list, "|c56|%s>|c248|%s: %s\n", channel->name, player->getName(), str + strlen ( cmd )  );
			
			if ( tGMListSize )
				roomMgr->sendListChat ( player, &gmList, "|c56|%s>|c248|(%s) %s: %s\n", channel->name, login, player->getName(), str + strlen ( cmd )  );

		} else {
			int nColor = ( (nLines[ channel->number ]++) & 0x00000001) ? 85 : 86;

			roomMgr->sendListChat ( player, &list, "|c56|%s>|c%d|%s: %s\n", channel->name, nColor, player->getName(), str + strlen ( cmd )  );

			if ( tGMListSize ) 
				roomMgr->sendListChat ( player, &gmList, "|c56|%s>|c%d|(%s) %s: %s\n", channel->name, nColor, login, player->getName(), str + strlen ( cmd )  );
		}				

end:
		if ( tempText )
			free ( tempText );
	}

	list.release();
	gmList.release();
}

void cmdTopic ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !str || !player || !player->character )
		return;

	int len = strlen ( str );
	
	if ( len < 1 || len > 320 ) {
		return;
	}

	if ( player->checkTimedAccess ( _ACCESS_NO_GOSSIP ) ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You can't change the topic while revoked!" );
		return;
	}

	if ( !player->channel ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You are not a member of any gossip channel." );
		return;
	}
	
	char *topic = str + strlen ( ((StringObject *)tokens->at ( 0 ))->data );
	
	if ( strchr ( topic, '|' ) ) {
		return;
	}
	
	if ( strchr ( topic, '%' ) ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> Topics can not contain any '%%' characters.\n" );
		return;
	}

	LinkedList *topicTokens = buildTokenList ( topic );
	
	if ( topicTokens->size() ) {
		Channel *pChannel = player->channel;

		if ( pChannel->isModerator ( player ) ) {
			player->channel->setTopic ( topic );	
			player->channel->sendText ( "|c71|Info> New channel topic: %s\n", topic );
		} else {
			roomMgr->sendPlayerText ( player, "|c60|Info> You can not change the topic.\n" );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c43|Info> Topic: %s\n", player->channel->topic );
	}
	delete topicTokens;
}

void cmdName ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !str || !player || !player->character )
		return;

	int len = strlen ( str );
	
	if ( len < 1 || len > 320 ) {
		return;
	}

	if ( player->checkTimedAccess ( _ACCESS_NO_GOSSIP ) ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You can't change the name while revoked!" );
		return;
	}

  	if ( !player->channel ) {
  		roomMgr->sendPlayerText ( player, "|c60|Info> You are not a member of any gossip channel." );
  		return;
  	}
   
  	char *name = str + strlen ( ((StringObject *)tokens->at ( 0 ))->data );	

	if ( !name )
		return;
   
   	if ( strchr ( name, '|' ) ) {
   		return;
   	}
   
   	if ( strchr ( name, '%' ) ) {
   		roomMgr->sendPlayerText ( player, "|c60|Info> Channel names can not contain any '%%' characters.\n" );
   		return;
   	}
   
   	LinkedList *nameTokens= buildTokenList ( name );
   
   	if ( nameTokens->size() ) {
			Channel *pChannel = player->channel;

   		if ( pChannel->isModerator ( player ) ) {
   			name += ((StringObject *)nameTokens->at ( 0 ))->index;
   
   			if ( strlen ( name ) > 12 ) {
   				roomMgr->sendPlayerText ( player, "|c60|Info> Channel names can be no longer than 12 letters.\n" );
   				return;
   			}
   
   			player->channel->setName ( name );	
   			player->channel->sendText ( "|c71|Info> New channel name: %s\n", name );
   		} else {
   			roomMgr->sendPlayerText ( player, "|c60|Info> You can not change the channel name.\n" );
   		}
   	} else {
   		roomMgr->sendPlayerText ( player, "|c71|Info> Channel name: %s\n", player->channel->name );
   	}
   	delete nameTokens;
}

void cmdGetChannel ( LinkedList *tokens, char *str, RMPlayer *player ) 
{
	if ( !player || !player->character )
		return;

	Channel *channel = NULL;

	// find the first free channel
	for ( int i=0; i<_MAX_CHANNEL; i++ ) {
		if ( !gChannels[i]->isSystem && !gChannels[i]->members.size() ) {
			channel = gChannels[i];	
			break;
		}
	}

	if ( !player->channel && channel ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You are not a member of any channel.  The first available channel is %d(%s)\n", channel->number, channel->name );
	}

	else if ( !player->channel && !channel ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You are not a member of any channel. There are no other available channels at this time.\n" );
	}

	else if ( player->channel && !channel ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You are currently listening to channel %d(%s). There are no other available channels at this time.\n", player->channel->number, player->channel->name );
	}

	else if ( player->channel && channel ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You are currently listening to channel %d(%s). The first available channel is %d(%s).\n", player->channel->number, player->channel->name, channel->number, channel->name );
	}
}

void cmdGods ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	PackedData packet;

	// put the packet header
	packet.putLong ( 0 );
	packet.putLong ( 0 );

	packet.putByte ( 5 );

	if ( player->checkAccess( _ACCESS_MODERATOR ) ) {
		LinkedElement *element = roomMgr->_gms.head();

		while ( element ) {
			RMPlayer *thePlayer = (RMPlayer *)element->ptr();
			element = element->next();

			if ( thePlayer->character ) {
				char *name = thePlayer->getName();

				if ( name ) {
					if ( thePlayer->player->physicalState & _STATE_BUSY ) {
						if ( !thePlayer->checkAccess ( _ACCESS_IMPLEMENTOR ) ) {
							packet.putString ( name );

                	                                if ( thePlayer->checkAccess( _ACCESS_IMPLEMENTOR ) ) {
        	                                                packet.putString( "Hiding Implementor" );
	                                                } else if ( thePlayer->checkAccess( _ACCESS_PUBLICRELATIONS ) ) {
                	                        	        packet.putString( "Hiding Community Relations" );
                        	        	        } else if ( thePlayer->checkAccess( _ACCESS_MODERATOR ) ) {
                        		                        packet.putString( "Hiding Sentinel" );
                	                	        } else {
        	                                	        packet.putString( "Hiding Mentor" );
		                                        }
						}
					} else {
						packet.putString ( name );

                                                if ( thePlayer->checkAccess( _ACCESS_IMPLEMENTOR ) ) {
                                                        packet.putString( "Implementor" );
                                                } else if ( thePlayer->checkAccess( _ACCESS_PUBLICRELATIONS ) ) {
                                        	        packet.putString( "Community Relations" );
                                	        } else if ( thePlayer->checkAccess( _ACCESS_MODERATOR ) ) {
                        	                        packet.putString( "Sentinel" );
                	                        } else {
        	                                        packet.putString( "Mentor" );
	                                        }
					}
				}
			}
		}

		element = roomMgr->_guides.head();

		while ( element ) {
			RMPlayer *thePlayer = (RMPlayer *)element->ptr();
			element = element->next();

			if ( thePlayer->character && !thePlayer->checkAccess ( _ACCESS_MODERATOR ) ) {
				char *name = thePlayer->getName();

				if ( name ) {
					packet.putString ( name );

                                        if ( thePlayer->checkAccess( _ACCESS_PUBLICRELATIONS ) ) {
                                                packet.putString( "Community Relations" );
                                        } else if ( thePlayer->checkAccess( _ACCESS_MODERATOR ) ) {
                                                packet.putString( "Sentinel" );
                                        } else {
                                                packet.putString( "Mentor" );
                                        }
				}
			}
		}
	} else {
		LinkedElement *element = roomMgr->_gms.head();

		while ( element ) {
			RMPlayer *thePlayer = (RMPlayer *)element->ptr();
			element = element->next();

			if ( thePlayer->character && !thePlayer->checkAccess ( _ACCESS_NORMAL ) && !( thePlayer->player->physicalState & _STATE_BUSY ) ) {
				char *name = thePlayer->getName();

				if ( name ) {
					packet.putString ( name );

                                        if ( thePlayer->checkAccess( _ACCESS_IMPLEMENTOR ) ) {
                                                        packet.putString( "Implementor" );
                                                } else if ( thePlayer->checkAccess( _ACCESS_PUBLICRELATIONS ) ) {
                                        	        packet.putString( "Community Relations" );
                                	        } else if ( thePlayer->checkAccess( _ACCESS_MODERATOR ) ) {
                        	                        packet.putString( "Sentinel" );
                	                        } else {
        	                                        packet.putString( "Mentor" );
	                                        }
				}
			}
		}

		element = roomMgr->_guides.head();

		while ( element ) {
			RMPlayer *thePlayer = (RMPlayer *)element->ptr();
			element = element->next();

			if ( thePlayer->character && !thePlayer->checkAccess ( _ACCESS_MODERATOR ) ) {
				char *name = thePlayer->getName();

				if ( name ) {
					packet.putString ( name );

					if ( thePlayer->checkAccess( _ACCESS_PUBLICRELATIONS ) ) {
						packet.putString( "Community Relations" ); 
					} else if ( thePlayer->checkAccess( _ACCESS_MODERATOR ) ) {
						packet.putString( "Sentinel" ); 
					} else {
						packet.putString( "Mentor" ); 
					}
				}
			}
		}
	}

	packet.putWord( 0 );

	roomMgr->sendTo ( _IPC_PLAYER_WHO, (IPCPMMessage *)packet.data(), packet.size(), player );
}

void cmdHostGossip ( LinkedList *tokens, char *str, RMPlayer *player )
{
	static int nLines = 0;

	if ( !str || !player || !player->character )
		return;

	LinkedElement *element = roomMgr->_hosts.head();

	char *cmd = ((StringObject *)tokens->at ( 0 ))->data;
	char *txt = str + strlen ( cmd );

	int nColor = ( (nLines++) & 0x00000001) ? 86 : 11;

	while ( element ) {
		RMPlayer *thePlayer = (RMPlayer *)element->ptr();
		element = element->next();

		if ( thePlayer && thePlayer->character )
			roomMgr->sendPlayerChat ( player, thePlayer, "-9|c%d| %s: %s\n", nColor, player->getName(), txt );
	}

	logChatData ( "%s:%s:h:%s", player->getLogin(), player->getName(), txt );
}

void cmdGuideGossip ( LinkedList *tokens, char *str, RMPlayer *player )
{
	static int nLines = 0;

	if ( !str || !player || !player->character )
		return;

	LinkedElement *element = roomMgr->_guides.head();

	char *cmd = ((StringObject *)tokens->at ( 0 ))->data;
	char *txt = str + strlen ( cmd );

	int nColor = ( (nLines++) & 0x00000001) ? 86 : 11;

	while ( element ) {
		RMPlayer *thePlayer = (RMPlayer *)element->ptr();
		element = element->next();

		roomMgr->sendPlayerChat ( player, thePlayer, "-d |c%d| %s: %s\n", nColor, player->getName(), txt );
	}

	logChatData ( "%s:%s:g:%s", player->getLogin(), player->getName(), txt );
}

void cmdGodGossip ( LinkedList *tokens, char *str, RMPlayer *player )
{
	static int nLines = 0;

	if ( !str || !player || !player->character )
		return;

	LinkedElement *element = player->checkAccess ( _ACCESS_MODERATOR )? roomMgr->_gms.head() : roomMgr->_guides.head();

	char *cmd = ((StringObject *)tokens->at ( 0 ))->data;
	char *txt = str + strlen ( cmd );

	int nColor = ( (nLines++) & 0x00000001) ? 86 : 11;

	if ( player->checkAccess ( _ACCESS_MODERATOR ) ) {
		while ( element ) {
			RMPlayer *thePlayer = (RMPlayer *)element->ptr();
			element = element->next();

			roomMgr->sendPlayerChat ( player, thePlayer, "-m |c%d| %s: %s\n", nColor, player->getName(), txt );
		}

		logChatData ( "%s:%s:M:%s", player->getLogin(), player->getName(), txt );
	} else {
		while ( element ) {
			RMPlayer *thePlayer = (RMPlayer *)element->ptr();
			element = element->next();

			roomMgr->sendPlayerChat ( player, thePlayer, "-d |c%d| %s: %s\n", nColor, player->getName(), txt );
		}

		logChatData ( "%s:%s:g:%s", player->getLogin(), player->getName(), txt );
	}
}

void cmdGossipOff ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	if ( player->channel != NULL ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You have just left channel %d(%s).\n", player->channel->number, player->channel->name );
		player->channel->delPlayer ( player );
	}
}

void cmdGossipOn ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	if ( player->channel == NULL ) {
		gChannels[0]->addPlayer ( player );
	} else {
		roomMgr->sendPlayerText ( player, "|c71|Info> You are already listening to gossip messages.\n" );
	}
}

void cmdOpenGroup ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	if ( !player->groupLeader || player->groupLeader == player ) {
		if ( !player->allowJoin ) {
			player->allowJoin = 1;
			roomMgr->sendPlayerText ( player, "-3O|c71|You are now accepting new group members.\n" );
		} else {
			roomMgr->sendPlayerText ( player, "-3O|c71|You are already accepting new group members.\n" );
		}
	} else {
		roomMgr->sendPlayerText ( player, "-3O|c248|Sorry, only the group leader can open the group.\n" );
	}
}

void cmdCloseGroup ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	if ( !player->groupLeader || player->groupLeader == player ) {
		if ( player->allowJoin ) {
			player->allowJoin = 0;
			roomMgr->sendPlayerText ( player, "-3C|c71|You are no longer accepting new group members.\n" );
		} else {
			roomMgr->sendPlayerText ( player, "-3C|c71|You are already not accepting new group members.\n" );
		}
	} else {
		roomMgr->sendPlayerText ( player, "-3O|c248|Sorry, only the group leader can close the group.\n" );
	}
}

void cmdAutoGive ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	if ( !player->autoGive ) {
		player->autoGive = 1;
		roomMgr->sendPlayerText ( player, "|c71|Info> You will now accept items given to you.\n" );
	} else {
		player->autoGive = 0;
		roomMgr->sendPlayerText ( player, "|c60|Info> You will now reject items given to you.\n" );
	}
}

void cmdInvite ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	// get the current channel...
	Channel *pChannel = player->channel;

	if ( pChannel == NULL ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You are not in a chat channel.\n" );
		return;
	}

	// handle not being a moderator...
	if ( !pChannel->isModerator ( player ) ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You must be a channel moderator to do that.\n" );
		return;
	}

	StringObject *name = (StringObject *)tokens->at ( 1 );	

	if ( name && name->data ) {
		int len = strlen ( name->data ); 
		
		if ( len < 1 || len > 16 ) {
			roomMgr->sendPlayerText ( player, "|c60|Info> Invalid name.\n" );
			return;
		}

		// check to see if the target name is banned
		if ( pChannel->isBanned ( name->data ) ) {
			roomMgr->sendPlayerText ( player, "|c71|Info> Channel ban lifted for '%s'.\n", name->data );
			pChannel->unbanPlayer ( name->data );

			RMPlayer *target = findPlayer ( name->data, player);

			if ( target ) {
				roomMgr->sendPlayerText ( target, "|c71|Info> Your channel ban was lifted by '%s' for channel %d.  You may now join the channel.\n", player->getName(), pChannel->number );
			}
		} else {
			roomMgr->sendPlayerText ( player, "|c71|Info> That name is not on the banned list for your channel.\n" );
			return;
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c71|Info> Usage: /invite <player name>\n" );
	}
}

void cmdKick ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );	

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player);

		if ( target && !target->checkAccess( _ACCESS_GUIDE ) ) {
			Channel *pChannel = player->channel;

			if ( !pChannel ) {
				roomMgr->sendPlayerText ( player, "|c60|Info> You are not a member of a channel.\n" );
				return;
			}

			if ( pChannel->isModerator ( player ) ) {
				if ( target->channel == pChannel ) {
					if ( !player->checkAccess ( _ACCESS_MODERATOR ) && pChannel->isModerator ( target ) ) {
						roomMgr->sendPlayerText ( player, "|c60|Info> You can not kick a moderator out of the channel.\n" );
						return;
					}

					roomMgr->sendPlayerText ( player, "|c248|Info> You have kicked '%s' out of your gossip channel.\n", name->data );
					roomMgr->sendPlayerText ( target, "|c60|Info> You have been kicked out of gossip channel %d(%s) by '%s'.\n", player->channel->number, player->channel->name, player->getName() );

					pChannel->delPlayer ( target, FALSE );

					// add the player to the banned list...
					pChannel->banPlayer ( target );
				} else {
					roomMgr->sendPlayerText ( player, "|c60|Info> '%s' is not a member of your channel.\n", target->getName() );
				}
			} else {
				roomMgr->sendPlayerText ( player, "|c60|Info> You can not kick '%s' because you are not a channel moderator.\n", name->data );
			}
		} else {
			roomMgr->sendPlayerText ( player, "|c60|Info> There is nobody by the name of '%s' to kick.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c71|Info> Usage: /kick <player name>\n" );
	}
}

void cmdMakeMod ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	// get channel pointer...
	Channel *pChannel = player->channel;

	if ( (pChannel == NULL) || !pChannel->isModerator ( player ) ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You must be a channel moderator to do that.\n" );
		return;
	}

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target ) {
			if ( target->channel == pChannel ) {
				if ( pChannel->isModerator ( target ) ) {
					roomMgr->sendPlayerText ( player, "|c60|Info> '%s' is already a moderator for your channel.\n", name->data );
				} else {
					pChannel->makeModerator ( target );

					pChannel->sendText ( "|c71|Info> '%s' has granted '%s' moderator rights over the channel.\n", player->getName(), target->getName() );
					roomMgr->sendPlayerText ( target, "|c71|Info> '%s' has granted you moderator rights to the channel.\n", player->getName() );
				}
			} else {
				roomMgr->sendPlayerText ( player, "|c60|Info> '%s' is not in your channel.\n", name->data );
			}
		} else {	
			roomMgr->sendPlayerText ( player, "|c60|Info> '%s' is not logged in.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c71|Info> usage /makemod <player name>\n" );
	}
}

void cmdRevokeMod ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	// get channel pointer...
	Channel *pChannel = player->channel;

	if ( (pChannel == NULL) || !pChannel->isModerator ( player ) ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You must be a channel moderator to do that.\n" );
		return;
	}

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target ) {
			roomMgr->sendPlayerText ( target, "|c60|Info> Your moderator rights to channel %d have been revoked by '%s'.\n", pChannel->number, player->getName() );
			pChannel->sendText ( "|c60|Info> '%s' has revoked moderator rights from '%s'.\n", player->getName(), target->getName() );
			pChannel->removeModerator ( target );	
		} else {
			pChannel->removeModerator ( name->data );	
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c71|Info> usage /revokemod <player name>\n" );
	}
}

void cmdJoin ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );
	StringObject *pPassword = (StringObject *)tokens->at ( 2 );

	if ( name ) {
		int number = atoi ( name->data );	

		if ( number < 0 || number >= _MAX_CHANNEL ) {
			roomMgr->sendPlayerText ( player, "|c43|Info> Channel '%s' does not exist.\n", name->data );
			return;
		}

		Channel *channel = gChannels[number];

		if ( player->channel == channel ) {
			roomMgr->sendPlayerText ( player, "|c43|Info> You are already a member of channel %d(%s).\n", number, channel->name ); 
			return;
		}

		// check the password...
		if ( !player->checkAccess ( _ACCESS_MODERATOR ) && channel->getPassword() ) {
			if ( pPassword ) {
				if ( !channel->checkPassword ( pPassword->data ) ) {
					roomMgr->sendPlayerText ( player, "|c43|Info> The provided password is not valid for that channel.\n" );
					return;
				}
			} else {
				roomMgr->sendPlayerText ( player, "|c43|Info> A password is required to join that channel.\n" );
				return;
			}
		}

		if ( player->channel ) {
			roomMgr->sendPlayerText ( player, "|c43|Info> You have left channel %d(%s).\n", player->channel->number, player->channel->name );
			player->channel->delPlayer ( player );
		}

		// check to see that the player is not banned...
		if ( channel->isBanned ( player ) ) {
			roomMgr->sendPlayerText ( player, "|c43|Info> You have been banned from that channel and cannot join.\n" );
		} else {
			channel->addPlayer ( player );

			roomMgr->sendPlayerText ( player, "|c43|Info> You have joined channel %d(%s)%s.\n", channel->number, channel->name, channel->isModerator ( player )? " as a moderator" : "" );
			roomMgr->sendPlayerText ( player, "|c43|Info> Topic: %s\n", channel->topic );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c43|Info> Usage: /join <channel number> <optional password>\n" );
	}
}

void cmdPrivate ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	// get the channel pointer
	Channel *pChannel = player->channel;

	if ( !pChannel ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You must be a channel member to issue that command.\n" );
		return;
	}

	// get the password to assign...
	StringObject *pPassword = (StringObject *)tokens->at ( 1 );

	// handle no password being assigned...
	if ( pPassword == NULL ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You must specify a password for the channel.\n" );
		return;
	}

	// only moderators can do this...
	if ( pChannel->isModerator ( player ) ) {
		//
		// if the channel is already private, let the players know that the
		// password changed
		//
		if ( pChannel->getPassword() ) {
			pChannel->sendText ( "|c71|Info> '%s' has changed the channel password.\n", player->getName() );
		} else {
			pChannel->sendText ( "|c71|Info> '%s' has password protected the channel.\n", player->getName() );
		}

		pChannel->setPassword ( pPassword->data );
	} else {
		roomMgr->sendPlayerText ( player, "|c60|Info> Only a channel moderator can make the channel private.\n" );
	}
}

void cmdPublic ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	// get the channel pointer
	Channel *pChannel = player->channel;

	if ( !pChannel ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> You must be a channel member to issue that command.\n" );
		return;
	}

	if ( pChannel->isModerator ( player ) ) {
		if ( pChannel->getPassword() ) {
			pChannel->sendText ( "|c71|Info> '%s' has removed the password on this channel.\n", player->getName() );
			pChannel->setPassword ( NULL );
		} else {
			roomMgr->sendPlayerText ( player, "|c60|Info> The channel is already public.\n" );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|Info> Only a channel moderator can make the channel public.\n" );
	}
}

void cmdRevoke ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target ) {
			int revoked = 0;

			// guide revoke
			if ( player->checkAccess ( _ACCESS_GUIDE ) && !player->checkAccess ( _ACCESS_MODERATOR ) ) {
				if ( target->checkAccess ( _ACCESS_MODERATOR ) ) {
					roomMgr->sendPlayerText ( player, "|c60|Info> Now wouldn't that be cute!" );
					return;
				}

				if ( target->channel && (target->channel->number == 4) ) {
					roomMgr->sendListText ( &roomMgr->_guides, "|c60|Info> Gossip rights revoked for %s by %s for one hour.\n", target->getName(), player->getName() );
					roomMgr->sendPlayerChat ( player, player, "|c248|Info> You revoked %s.\n", target->getName() );
					revoked = 1;
				} else {
			  		roomMgr->sendPlayerText ( player, "|c60|Info> You cannot revoke a player's gossip who is outside of the Help channel.\n" );
				}

				// gm revoke 
			} else {
				roomMgr->sendListText ( &roomMgr->_gms, "|c60|Info> Gossip rights revoked for %s by %s for one hour.\n", target->getName(), player->getName() );
				roomMgr->sendPlayerChat ( player, player, "|c248|Info> You revoked %s.\n", target->getName() );
				revoked = 1;
			}
		
			if ( revoked ) {
				gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_GMCMD, "revoke %s", target->getName() );
				gDataMgr->logPermanent ( target->getLogin(), target->getName(), _DMGR_PLOG_GMCMD, "%s revoke", player->getName() );

				target->setAccess ( _ACCESS_NO_GOSSIP );

				roomMgr->sendPlayerChat ( player, target, "|c60|Info> Your gossip rights have been revoked by %s.\n", player->getName() );

				player->reportText( name->data, str, "revoked" );
			}

		} else 
			roomMgr->sendPlayerText ( player, "|c60|Info> '%s' could not be found.\n", name->data );
	} else {
		roomMgr->sendPlayerText ( player, "|c60|You must specify the person you want to revoke.\n" );
	}
}

void cmdReadOnly ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character || !player->channel )
		return;

	if ( player->checkAccess ( _ACCESS_GUIDE ) && (player->channel->number == 4) ) {
		player->channel->isReadOnly ^= 1;
		roomMgr->sendListText ( &roomMgr->_guides, "|c71|Info> Channel %d is now %s.\n", player->channel->number, player->channel->isReadOnly? "read-only" : "full access" ); 
		gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_GMCMD, "rdonly %d %d", player->channel->number, player->channel->isReadOnly );
	} else if ( player->checkAccess ( _ACCESS_MODERATOR ) ) {
		player->channel->isReadOnly ^= 1;
		roomMgr->sendListText ( &roomMgr->_gms, "|c71|Info> Channel %d is now %s.\n", player->channel->number, player->channel->isReadOnly? "read-only" : "full access" ); 
		gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_GMCMD, "rdonly %d %d", player->channel->number, player->channel->isReadOnly );
	}
}

void cmdEventReadOnly ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	if ( bEventHappening ) {
		bEventReadOnly ^= 1;
		roomMgr->sendListChat ( player, &gEventPlayers, "-8 |c255|%s: |c67|The channel is now %s\n", player->getName(), bEventReadOnly ? "read only" : "open to all" );
	} else {
		roomMgr->sendPlayerText ( player, "|c71|Info> There is no event going on right now.\n" );
	}
}

void cmdRestore ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target ) {

			if ( target->checkTimedAccess ( _ACCESS_NO_GOSSIP ) ) {

				int restored = 0;

				if ( player->checkAccess ( _ACCESS_MODERATOR ) ) {
					roomMgr->sendListText ( &roomMgr->_gms, "|c71|Info> Gossip rights restored for %s by %s.\n", target->getName(), player->getName() );
					restored = 1;
				}
				else {
					if ( target->channel && (target->channel->number == 4) ) {
						roomMgr->sendListText ( &roomMgr->_guides, "|c71|Info> Gossip rights restored for %s by %s.\n", target->getName(), player->getName() );
						roomMgr->sendPlayerChat ( player, player, "|c71|Info> You restored %s.\n", target->getName() );
						restored = 1;
					}
					else
						roomMgr->sendPlayerText ( player, "|c60|Info> You cannot restore a player's gossip outside of the Help channel.\n" );
				}
		
				if ( restored ) {
					gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_GMCMD, "restore %s", target->getName() );
					gDataMgr->logPermanent ( target->getLogin(), target->getName(), _DMGR_PLOG_GMCMD, "%s restore", player->getName() );

					target->clearAccess ( _ACCESS_NO_GOSSIP );
					target->badGossipCount = 0;

					roomMgr->sendPlayerChat ( player, player, "|c71|Info> You restored %s.\n", target->getName() );
					roomMgr->sendPlayerChat ( player, target, "|c71|Info> Your gossip rights have been restored by %s.\n", player->getName() );

					player->reportText( name->data, str, "restored" );
				}
			}
			else
				roomMgr->sendPlayerText ( player, "|c71|Info> Gossip rights already granted to '%s'.\n", target->getName() );

		} else {
			roomMgr->sendPlayerText ( player, "|c60|Info> '%s' could not be found.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|You must specify the person you want to restore.\n" );
	}
}

void cmdEmote ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !str || !player || !player->character )
		return;
	
	int len = strlen ( str );
	
	if ( len < 1 || len > 320 ) {
		return;
	}
	
  	if ( strchr ( str, '|' ) ) {
  		roomMgr->sendPlayerText ( player, "|c60|Info> No embedded color changes allowed.\n" );
  		return;
  	}
   
  	if ( player && player->checkTimedAccess ( _ACCESS_GAGGED ) ) {
  		roomMgr->sendPlayerText ( player, "|c60|Info> You are not allowed to use emote commands while gagged.\n" );
  		return;
  	}
   
  	char *cmd = ((StringObject *)tokens->at ( 0 ))->data;
	logChatData ( "%s:%s:E: %s", player->getLogin(), player->getName(), str + strlen ( cmd ) );
//  	logChatData ( "%s: EMOTE: %s", player->getName(), str + strlen ( cmd ) );

	player->sendRoomChat ( "%s%s\n", player->getName(), str + strlen ( cmd ) );
}

void cmdKill ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		if ( *name->data == '*' ) {
			LinkedElement* element = player->room->head();

			while ( element ) {
				RMPlayer *target = (RMPlayer *)element->ptr();
				element = element->next();

				if ( !target->checkAccess ( _ACCESS_GUIDE ) ) {
					target->character->changeHealth ( -target->character->health, NULL );
				}
			}
		} else {
			RMPlayer *target = findPlayer ( name->data, player );

			if ( target && target->character ) {
				roomMgr->sendListText ( &roomMgr->_gms, "|c60|Info> %s has just been killed by %s.\n", target->getName(), player->getName() );
				target->character->changeHealth ( -target->character->health, NULL );		
			} else {
				roomMgr->sendPlayerChat ( player, player, "|c60|Info> You can not find %s to kill.\n", name->data );
			}
		}
	}
}

void cmdLogout ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target && player->canControl ( target ) ) {
			gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_GMCMD, "logged out %s", target->getName() );
			gDataMgr->logPermanent ( target->getLogin(), target->getName(), _DMGR_PLOG_GMCMD, "logged out by %s", player->getName() );
			roomMgr->sendListText ( &roomMgr->_gms, "|c60|Info> %s has just been logged out by %s.\n", target->getName(), player->getName() );
			target->forceLogout();

			player->reportText( name->data, str, "logged out" );
		} else {
			roomMgr->sendPlayerChat ( player, player, "|c60|Info> You can not find %s to logout.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|You must specify the person you want to logout.\n" );
	}
}

void cmdWarn ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target && target->character && target->room && player->canControl ( target ) ) {
			BCharacter *bchar = (BCharacter *)target->character->getBase ( _BCHARACTER );
			bchar->warnCount++;

			gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_GMCMD, "warn %s %d", target->getName(), bchar->warnCount );
			gDataMgr->logPermanent ( target->getLogin(), target->getName(), _DMGR_PLOG_GMCMD, "%s warn %d", target->getName(), player->character->room->number );
			roomMgr->sendListText ( &roomMgr->_gms, "|c60|Info> %s has just been warned by %s.\n", target->getName(), player->getName() );

			roomMgr->sendSystemMsg ( "Official GM Warning", target, "This is an official warning from the GM named %s!  You are not conforming to the set rules of conduct for The Realm.  If you do not follow the set guidelines of behavior, more drastic measures may be taken.  This is warning number %d for you.", player->getName(), bchar->warnCount );

			roomMgr->sendSystemMsg ( "Heads Up", player, "You have issued official warning number %d to %s.", bchar->warnCount, target->getName() );

			char output[1024];
			sprintf ( sizeof ( output ), output, "warn #%d", bchar->warnCount );

			player->reportText( name->data, str, output );
		} else {
			roomMgr->sendPlayerChat ( player, player, "|c60|Info> You can not find %s to warn.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|You must specify the person you want to warn.\n" );
	}
}

void cmdNoGag ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target && target->room ) { //&& player->canControl ( target ) ) {
			gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_IMPCMD, "nogag %s", target->getName() );
			gDataMgr->logPermanent ( target->getLogin(), target->getName(), _DMGR_PLOG_IMPCMD, "%s nogag", player->getName() );

			roomMgr->sendListChat ( player, &roomMgr->_gms, "|c71|Info> %s has had a gag removed by %s.\n", target->getName(), player->getName() );
			roomMgr->sendPlayerChat ( player, target, "|c71|Info> Your gag has been lifted by %s.\n", player->getName() );

			target->clearAccess ( _ACCESS_GAGGED );

			player->reportText( name->data, str, "UNgagged" );
		} else {
			roomMgr->sendPlayerChat ( player, player, "|c60|Info> You can not find %s to lift their gag.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|You must specify the person you want to lift a gag on.\n" );
	}
}

void cmdGag ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target && target->room ) {   // && player->canControl ( target ) ) {
			gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_IMPCMD, "gag %s", target->getName() );
			gDataMgr->logPermanent ( target->getLogin(), target->getName(), _DMGR_PLOG_IMPCMD, "gagged by %s", player->getName() );

			roomMgr->sendListChat ( player, &roomMgr->_gms, "|c60|Info> %s has been gagged by %s.\n", target->getName(), player->getName() );
			roomMgr->sendPlayerChat ( player, target, "|c60|Info> You have been gagged by %s.\n", player->getName() );

			target->setAccess ( _ACCESS_GAGGED );

			player->reportText( name->data, str, "gagged" );
		} else {
			roomMgr->sendPlayerChat ( player, player, "|c60|Info> You can not find %s to gag.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|You must specify the person you want to gag.\n" );
	}
}

void cmdSuspend ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target && player->canControl ( target ) ) {
			gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_IMPCMD, "suspended %s", target->getName() );
			gDataMgr->logPermanent ( target->getLogin(), target->getName(), _DMGR_PLOG_IMPCMD, "suspended by %s", player->getName() );

			roomMgr->sendListText ( &roomMgr->_gms, "|c60|Info> %s has just been suspended by %s.\n", target->getName(), player->getName() );
			roomMgr->sendPlayerChat ( player, player, "|c60|Info> You suspended %s.\n", target->getName() );
			target->suspend ( "%s", player->getName() );

			player->reportText( name->data, str, "suspended" );
		} else {
			roomMgr->sendPlayerChat ( player, player, "|c60|Info> You can not find %s to suspend.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|You must specify the person you want to suspend.\n" );
	}
}

void cmdShowReport ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target && target->room ) {
			if ( target->m_pLastComplain ) {
				roomMgr->sendSystemMsg( target->getName(), player, "%s", target->m_pLastComplain );

				gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_IMPCMD, "viewed %s's report", target->getName() );

				roomMgr->sendListChat ( player, &roomMgr->_gms, "|c71|Info> %s's report has been reviewed by %s.\n", target->getName(), player->getName() );

				player->reportText( name->data, str, "report checked" );
			} else {
				roomMgr->sendPlayerChat ( player, player, "|c60|Info> %s has no report.\n", name->data );
			}
		} else {
			roomMgr->sendPlayerChat ( player, player, "|c60|Info> You can not find %s to get the report.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|You must specify the person you want to check the report.\n" );
	}
}

void cmdDisable ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target && player->canControl ( target ) ) {
			if ( target->accountType == _TRIAL ) {
				target->writeSerialNumber();
			}

			gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_IMPCMD, "disabled %s", target->getName() );
			gDataMgr->logPermanent ( target->getLogin(), target->getName(), _DMGR_PLOG_IMPCMD, "disabled by %s", player->getName() );

			roomMgr->sendListText ( &roomMgr->_gms, "|c60|Info> %s has just been disabled by %s.\n", target->getName(), player->getName() );
			roomMgr->sendPlayerChat ( player, player, "|c60|Info> You disabled %s.\n", target->getName() );
			target->disable ( "account disabled by implementor %s", player->getName() );

			player->reportText( name->data, str, "disabled" );
		} else {
			roomMgr->sendPlayerChat ( player, player, "|c60|Info> You can not find %s to disable.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|You must specify the person you want to disable.\n" );
	}
}

void cmdGroupTell ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !str || !player || !player->character )
		return;
		
	int len = strlen ( str );
	
	if ( len < 1 || len > 320 ) {
		return;
	}
	
	if ( strchr ( str, '|' ) ) {
		roomMgr->sendPlayerText ( player, "|c60|Info> No embedded color changes allowed.\n" );
		return;
	}
	
   	char *cmd = ((StringObject *)tokens->at ( 0 ))->data;
   	char *txt = str + strlen ( cmd );
	
   	if ( player->groupLeader ) {
   		player->sendGroupChat ( "-3F|c82|%s: %s\n", player->getName(), txt );
   	} else {
   		roomMgr->sendPlayerText ( player, "-3F|c60|You are not a member of a group.\n" );
   	}
}

void cmdMonsterHead ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name =   static_cast<StringObject*>(tokens->at ( 1 ));
	StringObject *number = static_cast<StringObject*>(tokens->at ( 2 ));

	if (!name || !number ) {
		roomMgr->sendPlayerText ( player, "|c71|Info> Usage: /monsterhead <playername> <head#>\n", name->data );
	}
	else if ( name && number ) {
		RMPlayer *target = findPlayer ( name->data, player );
		int val = atoi ( number->data );

		if ( val < 0 || val > 20 )
			val = 0;

		if ( target && target->character ) {
			BContainer *bcontain = static_cast<BContainer*>(target->character->getBase ( _BCONTAIN ));
			
			if( !bcontain ) return;

			//find a head
			LinkedElement* element = bcontain->contents.head();
			WorldObject *obj = 0;

			while( element ) {
				obj = static_cast<WorldObject *>(element->ptr());
				element = element->next();

				if( obj->getBase( _BHEAD ) ) break;
			}

			if( obj ) {
				BHead *bhead = static_cast<BHead*>( obj->getBase( _BHEAD ) );

				if ( bhead ) {
					bhead->eyeNumber = 31;
					bhead->headNumber = val;
					roomMgr->sendPlayerText ( player, "|c71|Info> %s given monster head %d.\n", name->data, val );
				}
				else {
					roomMgr->sendPlayerText ( player, "|c71|Info> %s has no head.\n", name->data );
				}
			}
			else {
				roomMgr->sendPlayerText ( player, "|c71|Info> %s has no head.\n", name->data );
			}
		}
		else {
			roomMgr->sendPlayerText ( player, "|c60|Info> Could not find %s.\n", name->data );
		}
	}
}

void cmdNoHead ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = static_cast<StringObject*>( tokens->at( 1 ) );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target && target->character ) {
			BContainer *bcontain = static_cast<BContainer*>( target->character->getBase( _BCONTAIN ) );

			if( bcontain ) {

				//destroy any heads found
				unsigned int nHeads = 0;
				LinkedElement* element = bcontain->contents.head();

				while( element ) {
					WorldObject *obj = static_cast<WorldObject*>( element->ptr() );
					element = element->next();

					if( obj->getBase( _BHEAD ) ) {
						roomMgr->destroyObj( obj, 1, __FILE__, __LINE__ );
						++nHeads;
					}
				}

				if( nHeads ) {
					roomMgr->sendPlayerText ( target, "|c71|Info> Your head has been removed.\n" );
					roomMgr->sendPlayerText ( player, "|c71|Info> %d head%s removed, master.\n", nHeads, nHeads == 1 ? "" : "s" );
				}
				else {
					roomMgr->sendPlayerText ( player, "|c60|Info> There was no head to remove from %s.\n", name->data );
				}
			}
		}
	}
}

void cmdCopyRoom ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *room1 = (StringObject *)tokens->at ( 1 );
	StringObject *room2 = (StringObject *)tokens->at ( 2 );

	if ( room1 && room2 ) {
		int nRoom1 = atoi( room1->data );
		int nRoom2 = atoi( room2->data );

		RMRoom *pRoom1 = roomMgr->findRoom ( nRoom1 );
		RMRoom *pRoom2 = roomMgr->findRoom ( nRoom2 );

		if ( pRoom1 && pRoom2 ) {
			pRoom1->copyOther( pRoom2 );
		}
	}
}

void cmdStartRoom ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		// update a character in memory
		if ( target && target->character ) {
			target->character->roomNumber = -1;

			roomMgr->sendPlayerText ( target, "|c71|Info> Your starting room has been reset.\n" );
			roomMgr->sendPlayerText ( player, "|c71|Info> %s's starting room has been reset.\n", target->getName() );
		}
	}
}

void cmdTell ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( strchr ( str, '|' ) ) {
		roomMgr->sendPlayerText ( player, "-taNo embedded color changes allowed.\n" );
		return;
	}

	if ( player && player->checkTimedAccess ( _ACCESS_GAGGED ) ) {
		roomMgr->sendPlayerText ( player, "-taYou are not allowed to send tell messages while gagged.\n" );
		return;
	}

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		char* pMessage = str + name->index + name->length;

		if ( ! *pMessage ) {
			roomMgr->sendPlayerText ( player, "-taWhat would you like to tell %s?\n", name->data );
			return;
		}

		if ( target && target->character ) {
			if ( target == player ) {
				roomMgr->sendPlayerText ( player, "-taYou can't tell yourself something... that would be silly.\n" );
			} else {
				if ( !player->canControl ( target ) ) {
					if ( target->isIgnoring ( player->getName() ) ) {
						roomMgr->sendPlayerText ( player, "-tb%s  This player is ignoring you.\n", target->getName() );			
						return;
					}

					if ( !player->checkAccess ( _ACCESS_GUIDE ) && target->player->physicalState & _STATE_BUSY ) {
						if ( target->checkAccess ( _ACCESS_GUIDE) )
							roomMgr->sendPlayerText ( player, "-tb%s  This player can not be found to tell that to.\n", name->data );
						else
							roomMgr->sendPlayerText ( player, "-tb%s  This player is busy right now, try again later.\n", target->getName() );
						return;
					}
				}

				char pType = 'p';

				if ( player->checkAccess( _ACCESS_IMPLEMENTOR ) )
					pType = 'i';
				else if ( player->checkAccess ( _ACCESS_PUBLICRELATIONS ) )
					pType = 'c';
				else if ( player->checkAccess ( _ACCESS_MODERATOR ) )
					pType = 'm';
				else if ( player->checkAccess ( _ACCESS_GUIDE ) )
					pType = 'g';

				if ( target->checkAccess ( _ACCESS_MODERATOR ) && !player->checkAccess ( _ACCESS_MODERATOR ) ) {
					char *login = "";

					BPlayer *base = (BPlayer *)player->player->getBase ( _BPLAYER );

					if ( base ) 
						login = base->login;

					if (*login)
						roomMgr->sendPlayerChat ( player, target, "-t%c%s (%s) %s\n", pType, player->getName(), login, str + name->index + name->length );
					else
						roomMgr->sendPlayerChat ( player, target, "-t%c%s (Unknown) %s\n", pType, player->getName(), str + name->index + name->length );
				} else {
					roomMgr->sendPlayerChat ( player, target, "-t%c%s %s\n", pType, player->getName(), str + name->index + name->length );	
				}

				roomMgr->sendPlayerChat ( player, player, "-tr%s %s\n", target->getName(), str + name->index + name->length );

				logChatData ( "%s:%s:T %s:%s\n", player->getLogin(), player->getName(), target->getName(), str + name->index + name->length );
//				logChatData ( "%s: TELL: %s,%s\n", player->getName(), target->getName(), str + name->index + name->length );
			}	
		} else {
			roomMgr->sendPlayerText ( player, "-tb%s  This player can not be found to tell that to.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "-taYou must specify the person you want to tell.\n" );
	}
}

void cmdRoom ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	int number = player->character->room->number; 
	
	roomMgr->sendPlayerText ( player, "|c71|Info> You are currently in room %d.\n", number );
}

void cmdRubber ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	player->room->bRubberWalled ^= 1;

	roomMgr->sendPlayerText ( player, "|c71|Info> You have turned rubber walling %s.\n", player->room->bRubberWalled ? "on" : "off" );
}

void cmdEmpty ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	LinkedList *list = player->room->copy();

	// get the list of players in this room.
	LinkedElement *element = list->head();

	if ( player->room->isDungeonEntrance ) {
		while ( element ) {
			RMPlayer *target = (RMPlayer *)element->ptr();
			element = element->next();

			if ( target && target->isNPC ) {
				target->character->changeHealth ( -target->character->health, NULL );
			}
		}

		roomMgr->sendPlayerText ( player, "|c71|Info> Cleared room of all NPCs" );
	} else {
		while ( element ) {
			RMPlayer *target = (RMPlayer *)element->ptr();
			element = element->next();

			if ( target && !target->isNPC && target != player && !target->checkAccess( _ACCESS_MODERATOR ) ) {
				RMRoom *newRoom = player->room;
				Zone *zone = newRoom? newRoom->zone : NULL;

				if ( zone ) {
					if ( !target->isTeleporting ) {
						int nCount = 0;

						do {
							newRoom = (RMRoom *) zone->rooms.at ( random ( 0, zone->rooms.size() - 1 ) );
							nCount++;
						} while ( ( newRoom->size() > 20 && nCount < 10 ) || newRoom == player->room );

						PackedMsg response;
				
						response.putLong ( target->character->servID );
						response.putLong ( target->character->room->number );

						target->character->teleport ( newRoom->number, &response );
						response.putByte ( _MOVIE_END );
						
						roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), target->room );

						roomMgr->sendPlayerText ( target, "|c60|Info> You are being randomly teleported by %s from this room.\n", player->getName() );
					} else {
						roomMgr->sendPlayerText ( player, "|c60|Info> Teleporting of %s failed, already teleporting.\n", target->getName() );
						roomMgr->sendPlayerText ( target, "|c60|Info> Teleport failed, already teleporting.\n" );
					}
				}
			}
		}
	}

	list->release();
	delete list;
}

void cmdTeleport ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	RMPlayer *target = player;
	int roomNum = -1;

	if ( tokens->size() == 1 ) {
		roomMgr->sendPlayerText ( player, "|c71|Info> You are currently in room %d.\n", player->character->room->number );
	} 

	else if ( tokens->size() == 2 ) {
		roomNum = atoi ( ((StringObject *)tokens->at ( 1 ))->data );
		roomMgr->sendPlayerText ( player, "|c71|Info> You are teleporting to room %d.\n", roomNum );
		if ( player->character->hasAffect ( _AFF_JAIL, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL ) ) {
			roomMgr->sendPlayerText ( player, "|c60|Info> You can not teleport because you are in jail.\n" );
			return;
		}
	} else {
		roomNum = atoi ( ((StringObject *)tokens->at ( 2 ))->data );
		StringObject *name = (StringObject *)tokens->at ( 1 );

		target = findPlayer ( name->data, player );

		if ( target && target->room ) {
			gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_GMCMD, "teleport %s %d", target->getName(), roomNum );
			gDataMgr->logPermanent ( target->getLogin(), target->getName(), _DMGR_PLOG_GMCMD, "%s teleport %d", player->getName(), roomNum );
			roomMgr->sendPlayerText ( player, "|c71|Info> You are teleporting %s to room %d.\n", target->getName(), roomNum );
			roomMgr->sendPlayerText ( target, "|c71|Info> You are being teleported by %s.\n", player->getName() );
		} else {
			roomMgr->sendPlayerText ( player, "|c71|Info> Who do you want to teleport?\n" );
			roomNum = -1;
		}
	}

	if ( roomNum != -1 && target && target->room ) {

		if ( !target->isTeleporting ) {
			PackedMsg response;
	
			response.putLong ( target->character->servID );
			response.putLong ( target->character->room->number );

			target->character->teleport ( roomNum, &response );
			response.putByte ( _MOVIE_END );
			
			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), target->room );
		} else {
			roomMgr->sendPlayerText ( player, "|c60|Info> Teleporting of %s failed, already teleporting.\n", target->getName() );
			roomMgr->sendPlayerText ( target, "|c60|Info> Teleport failed, already teleporting.\n" );
		}
	}
}

void cmdHeal ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	player->character->changeHealth ( ( player->character->healthMax - player->character->health ), NULL );
}

void cmdLearned ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	RMPlayer *target = player;

	if ( tokens->size() == 2 ) {
		StringObject *name = (StringObject *)tokens->at ( 1 );
		target = findPlayer ( name->data, player );
	}

	if ( target && target->room && target->character ) {
		BCharacter *bchar = (BCharacter *)target->character->getBase ( _BCHARACTER );

		int i;

		for ( i=0; i<_SKILL_MAX; ++i ) 
			bchar->skills[i] = _SKILL_LVL_GRAND_MASTER;

		for ( i=0; i < _SPELL_MAX; ++i )
			if ( i != _SPELL_HEAD_OF_DEATH || target->checkAccess( _ACCESS_IMPLEMENTOR ) )  // do not grant head of death.
				bchar->learnSpell ( i );

		gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_IMPCMD, "learned %s", target->getName() );
		gDataMgr->logPermanent ( target->getLogin(), target->getName(), _DMGR_PLOG_IMPCMD, "%s learned", player->getName() );

		roomMgr->sendPlayerText ( player, "|c71|Info> %s is now learned in all areas of magic and skill.\n", target->getName() );
		roomMgr->sendPlayerText ( target, "|c71|Info> You are now learned in all areas of magic and skill.\n" );
	} else {
			roomMgr->sendPlayerText ( player, "|c71|Info> Who do you want to make learned?\n" );
	}
}

void cmdPower ( LinkedList *tokens, char *str, RMPlayer *player )
{
}

void cmdTeleportHouse ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	RMPlayer *target = player;
	int roomNum = -1;

	if ( tokens->size() == 1 ) {
		roomMgr->sendPlayerText ( player, "|c71|Info> You are currently in room %d.\n", player->character->room->number );
	} 

	else if ( tokens->size() == 2 ) {
		roomNum = atoi ( ((StringObject *)tokens->at ( 1 ))->data ) + _DBR_OFFSET;
		roomMgr->sendPlayerText ( player, "|c71|Info> You are teleporting to room %d.\n", roomNum );
	} else {
		roomNum = atoi ( ((StringObject *)tokens->at ( 2 ))->data ) + _DBR_OFFSET;
		StringObject *name = (StringObject *)tokens->at ( 1 );

		target = findPlayer ( name->data, player );

		if ( target && target->room ) {
			gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_GMCMD, "teleport %s %d", target->getName(), roomNum );
			gDataMgr->logPermanent ( target->getLogin(), target->getName(), _DMGR_PLOG_GMCMD, "%s teleport %d", player->getName(), roomNum );
			roomMgr->sendPlayerText ( player, "|c71|Info> You are teleporting  %s to room %d.\n", target->getName(), roomNum );
			roomMgr->sendPlayerText ( target, "|c71|Info> You are being teleported by %s.\n", player->getName() );
		} else {
			roomMgr->sendPlayerText ( player, "|c71|Info> Who do you want to teleport?\n" );
			roomNum = -1;
		}
	}

	if ( roomNum != -1 && target && target->room ) {
		PackedMsg response;

		target->exitCombat();

		response.putLong ( target->character->servID );
		response.putLong ( target->character->room->number );
	
		target->character->teleport ( roomNum, &response );

		response.putByte ( _MOVIE_END );

		roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), target->room );
	}
}

void cmdFreeze ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	if ( tokens->size() > 1 ) {
		StringObject *name = (StringObject *)tokens->at ( 1 );
	
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target && target->character ) {
			roomMgr->sendSystemMsg ( "Wait Right Here", target, "You have been frozen.  Please wait for a game master." );
			roomMgr->sendPlayerText ( player, "|c60|Info> %s has been frozen.\n", target->getName() );

			PackedMsg response;

			response.putLong ( target->character->servID );
			response.putLong ( target->character->room->number );
			response.putByte ( _MOVIE_HANDS_OFF );
			response.putByte ( _MOVIE_END );

			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), target->room );
		} else {
			roomMgr->sendPlayerText ( player, "|c60|Info> '%s' could not be found.\n", name->data );
		}		
	}
}

void cmdThaw ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	if ( tokens->size() > 1 ) {
		StringObject *name = (StringObject *)tokens->at ( 1 );
	
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target && target->character ) {
			roomMgr->sendSystemMsg ( "You Are Free To Go", target, "You have been thawed." );
			roomMgr->sendPlayerText ( player, "|c71|Info> %s has been thawed.\n", target->getName() );

			PackedMsg response;

			response.putLong ( target->character->servID );
			response.putLong ( target->character->room->number );
			response.putByte ( _MOVIE_HANDS_ON );
			response.putByte ( _MOVIE_END );

			roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), target->room );
		} else {
			roomMgr->sendPlayerText ( player, "|c60|Info> '%s' could not be found.\n", name->data );
		}
	}
}

void cmdCredit ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;                                                                                                       

	if ( tokens->size() == 3 ) {
		int nCredits = 0;
		
		StringObject *name = (StringObject *)tokens->at ( 1 );

		RMPlayer *target = findPlayer ( name->data, player );

		if ( target && target->character ) {
               		StringObject *numItems = (StringObject *)tokens->at ( 2 );

	                if ( numItems ) {
	                         nCredits = atoi( numItems->data );
	                }

			if ( nCredits > 0 ) {
				target->nCredits += nCredits;

				gDataMgr->credit( target, target->nCredits );					
				gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_IMPCMD, "creditted %s with %d days", target->getName(), nCredits );
				gDataMgr->logPermanent ( target->getLogin(), target->getName(), _DMGR_PLOG_IMPCMD, "%s creditted %d days", player->getName(), nCredits );
								
				roomMgr->sendPlayerText ( target, "|c71|Info> You were just given %d free days of play.\n", nCredits );
				roomMgr->sendPlayerText ( player, "|c71|Info> You just gave '%s' %d free days of play.\n", name->data, nCredits );
			} else {
				roomMgr->sendPlayerText ( player, "|c71|Info> How many credits to give '%s'?\n", name->data );
			}
		} else {
			roomMgr->sendPlayerText ( player, "|c60|Info> '%s' could not be found.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|Info> /credit [player names] [# of days]\n" );
	}
}   

void cmdMMOff ( LinkedList *tokens, char *str, RMPlayer *player ) {
        if ( !player || !player->character )                                                                                                  return;

	gMagicMailOff = 1;

	roomMgr->sendPlayerText ( player, "|c60|Info>Magic Mail has now been disabled!\n" );
}

void cmdCopper ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;                                                                                                       

	unsigned int nTokens = tokens->size();

	if ( nTokens < 3 ) {
		roomMgr->sendPlayerText ( player, "|c71|Info> Usage: /copper <player name> <# of coppers>|c43|\n" );
		roomMgr->sendPlayerText ( player, "|c71|Info> Usage: /copper <list of player names> <# of coppers>|c43|\n" );
		roomMgr->sendPlayerText ( player, "|c71|Info> Usage: /copper room <# of coppers> --- coppers the room you are standing in|c43|\n" );
		roomMgr->sendPlayerText ( player, "|c71|Info> Usage: /copper room <room number> <# of coppers> --- coppers the specified room|c43|\n" );
	} else {
		//get # of coppers
		unsigned char nCoppers = 0;
		unsigned int  nTargets = nTokens - 2;

		StringObject *tokCoppers = static_cast< StringObject*>( tokens->at(nTokens - 1) );

		if ( tokCoppers ) {
			nCoppers = atoi( tokCoppers->data );
		} else
			return;

		if( nCoppers < 1000 ) {
			//check for special functions
			StringObject *tokFunc = static_cast< StringObject*>(tokens->at( 1 ));

			if( !strcmp( tokFunc->data, "room") ) {
				switch( nTokens ) {
					case 3: {//copper the current room
						RMRoom* pRoom = NULL;

						if( (pRoom = player->room) ) {
							unsigned int nCoppered = 0;
							char playerList[8192] = {0};

							LinkedElement *playerElement = pRoom->head();

							while ( playerElement ) {
								RMPlayer *pPlayer = static_cast< RMPlayer*>( playerElement->ptr() );
								playerElement = playerElement->next();

								if( pPlayer && pPlayer->character && !pPlayer->isNPC && !pPlayer->checkAccess( _ACCESS_PRIVILEGED ) ) {
									//copper this player
									pPlayer->nCoppers += nCoppers;
									gDataMgr->copper( pPlayer, pPlayer->nCoppers );
									gDataMgr->logPermanent (  player->getLogin(),  player->getName(), _DMGR_PLOG_EVENTS, "%s[%s] got %d coppers [/copper room %d]", pPlayer->getName(), pPlayer->getLogin(), nCoppers, pRoom->number );
									gDataMgr->logPermanent ( pPlayer->getLogin(), pPlayer->getName(), _DMGR_PLOG_EVENTS, "%s gave %d coppers [/copper room %d]", player->getName(), nCoppers, pRoom->number );

									roomMgr->sendPlayerText ( pPlayer, "|c71|Info> You were just given %d coppers.\n", nCoppers );
									strcat( playerList, pPlayer->getName() );
									strcat( playerList, ", " );
									++nCoppered;
								}
							}

							roomMgr->sendPlayerText ( player, "|c71|Info> %d coppers awarded to %d players (%s) in room %d. (total %d coppers)\n", nCoppers, nCoppered, playerList, pRoom->number, (nCoppers * nCoppered) );

						} else {
							roomMgr->sendPlayerText ( player, "|c60|Info> room not found to copper.\n" );
							return;
						}

						}break;
					case 4: {//copper the specified room
						unsigned int nRoom = 0;
						StringObject *tokRoom = static_cast< StringObject*>( tokens->at ( 2 ) );

						if ( tokRoom ) {
							nRoom = atoi( tokRoom->data );
						} else return;

						RMRoom* pRoom = roomMgr->findRoom( nRoom );

						if( pRoom ) {
							unsigned int nCoppered = 0;
							char playerList[8192] = {0};

							LinkedElement *playerElement = pRoom->head();

							while ( playerElement ) {
								RMPlayer *pPlayer = static_cast< RMPlayer*>( playerElement->ptr() );
								playerElement = playerElement->next();

								if( pPlayer && pPlayer->character && !pPlayer->isNPC && !pPlayer->checkAccess( _ACCESS_PRIVILEGED ) ) {
									//copper this player
									pPlayer->nCoppers += nCoppers;
									gDataMgr->copper( pPlayer, pPlayer->nCoppers );
									gDataMgr->logPermanent (  player->getLogin(),  player->getName(), _DMGR_PLOG_EVENTS, "%s[%s] got %d coppers [/copper room %d]", pPlayer->getName(), pPlayer->getLogin(), nCoppers, pRoom->number );
									gDataMgr->logPermanent ( pPlayer->getLogin(), pPlayer->getName(), _DMGR_PLOG_EVENTS, "%s gave %d coppers [/copper room %d]", player->getName(), nCoppers, pRoom->number );

									roomMgr->sendPlayerText ( pPlayer, "|c71|Info> You were just given %d coppers.\n", nCoppers );
									strcat( playerList, pPlayer->getName() );
									strcat( playerList, ", " );
									++nCoppered;
								}
							}

							roomMgr->sendPlayerText ( player, "|c71|Info> %d coppers awarded to %d players (%s) in room %d.\n", nCoppers, nCoppered, playerList, pRoom->number );

						} else {
							roomMgr->sendPlayerText ( player, "|c60|Info> room %d not found to copper.\n", nRoom );
							return;
						}

						}break;
					default:
						roomMgr->sendPlayerText ( player, "|c60|Info> Invalid parameters to /copper\n" );
						return;
				};
			} else if( nTargets ) { //standard list of players to copper.
				unsigned short nPlayers = 0;
				unsigned short nErrors = 0;
				for( unsigned int targetNum = 0; targetNum < nTargets; ++targetNum ) {
					StringObject *targetName = static_cast< StringObject*>(tokens->at ( targetNum + 1 ));

					if( targetName && targetName->data ) {
						RMPlayer *pPlayer = findPlayer ( targetName->data, player );

						if ( pPlayer && pPlayer->character && !pPlayer->checkAccess( _ACCESS_PRIVILEGED ) ) {

							if ( nCoppers < 4 ) {
								pPlayer->nCoppers += nCoppers;
								gDataMgr->copper( pPlayer, pPlayer->nCoppers );
								gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_EVENTS, "%s got %d coppers.", pPlayer->getName(), nCoppers );
								gDataMgr->logPermanent ( pPlayer->getLogin(), pPlayer->getName(), _DMGR_PLOG_EVENTS, "%s gave %d coppers.", player->getName(), nCoppers );
												
								roomMgr->sendPlayerText ( pPlayer, "|c71|Info> You were just given %d coppers.\n", nCoppers );
								roomMgr->sendPlayerText ( player, "|c71|Info> %s was given %d coppers.\n", targetName->data, nCoppers );
								++nPlayers;
							}
						} else if ( pPlayer && pPlayer->checkAccess( _ACCESS_PRIVILEGED ) ) {
							roomMgr->sendPlayerText ( player, "|c17|Info> '%s' skipped - they are staff.\n", targetName->data );
						} else {
							roomMgr->sendPlayerText ( player, "|c60|Info> '%s' could not be found.\n", targetName->data );
							++nErrors;
						}
					} else {
						roomMgr->sendPlayerText ( player, "|c60|Info> Error in command.\n" );
					}
				}

				roomMgr->sendPlayerText ( player, "|c71|Info> %d players coppered, %d players not found.\n", nPlayers, nErrors );

			} else roomMgr->sendPlayerText ( player, "|c60|Info> Invalid number of targets. Please check syntax\n" );

		} else {
			if( nTargets ) roomMgr->sendPlayerText ( player, "|c60|Info> Number of coppers must be less than 1000\n" );
		}
	}
}   

void cmdLoadingHouses ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;

	char output[1024];
	sprintf ( sizeof ( output ), output, "There are %d houses loading...\n\n", gLoadingHouses.size() );

	LinkedElement *element = gLoadingHouses.head();

	while ( element ) {
		StringObject *obj = (StringObject *)element->ptr();
		element = element->next();

		strcat ( output, obj->data );
		strcat ( output, "\n" );
	}

	roomMgr->sendSystemMsg ( "Loading Houses", player, output );
}

void cmdLoadAllHouses ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	logDisplay( "loading..." );

	gDataMgr->loadAllHouses();

	logDisplay( "loaded..." );

}

void cmdDisplayCombatGrid( LinkedList *tokens, char *str, RMPlayer *player )
{
	if(player && player->character && player->character->combatGroup )
		player->character->combatGroup->displayGrid();
}

void cmdDump ( LinkedList *tokens, char *str, RMPlayer *player )
{
	File file ( "memorydump.txt" );
	file.truncate();

	malloc_t *ptr = gAllocList;
	int count = 0;
	int size = 0;

	while ( ptr ) {
		const char* name = strrchr ( gAllocations[ptr->nAlloc].file, '/' );

		if ( name )
			name++;
		else
			name = gAllocations[ptr->nAlloc].file;

		file.printf ( "%d %s(%d) %d\n", ptr->size, name, gAllocations[ptr->nAlloc].line, ptr->type );
		size += ptr->size;
		count++;
		ptr = ptr->next;
	}

	roomMgr->sendSystemMsg ( "Memory Dumped", player, "%d items dumped for %d bytes", count, size );

	file.close();
}

void cmdAddFriend ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;

	char* pName = player->getName();

	LinkedElement *pElement = tokens->head()->next();

	while ( pElement ) {
		StringObject *name = (StringObject *)pElement->ptr();
		pElement = pElement->next();

		if ( name ) {
			if ( player->IsFriend( name->data ) ) {
				roomMgr->sendPlayerText ( player, "|c60|Info> %s is already on your friend's list.\n", name->data );
			} else if ( strcasecmp( pName, name->data ) ) {
				gDataMgr->AddFriend( player, name->data );
			} else {
				roomMgr->sendPlayerText ( player, "|c60|Info> You can't be a friend to yourself... that would be silly.\n" );
			}
		}
	}
}

void cmdDelFriend ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;

	LinkedElement *pElement = tokens->head()->next();

	while ( pElement ) {
		StringObject *name = (StringObject *)pElement->ptr();
		pElement = pElement->next();

		if ( name ) {
			player->DelFriend ( name->data );
		}
	}
}

void cmdLoginMsg ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;

	char* pStr = &str[9];

	while ( *pStr && *pStr == ' ' )
		pStr++;

	gDataMgr->LoginMessage( pStr );

	roomMgr->sendPlayerText ( player, "|c71|Info> You set the login message to '%s'.", pStr );
}

void cmdStartup ( LinkedList *tokens, char *str, RMPlayer *player )
{
	gDataMgr->DowntimeMessage( "" );

	roomMgr->sendPlayerText ( player, "|c71|Info> You have opened the game to the masses" );
}

void cmdSetAttrs ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );
	StringObject *strength = (StringObject *)tokens->at ( 2 );
	StringObject *intelligence = (StringObject *)tokens->at ( 3 );
	StringObject *dexterity = (StringObject *)tokens->at ( 4 );
	StringObject *endurance = (StringObject *)tokens->at ( 5 );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target && target->character ) {
			
			if ( strength ) {
				target->character->strength = atoi ( strength->data );
			}

			if ( intelligence ) {
				target->character->intelligence = atoi ( intelligence->data );
			}

			if ( dexterity ) {
				target->character->dexterity = atoi ( dexterity->data );
			}

			if ( endurance ) {
				target->character->endurance = atoi ( endurance->data );
			}

			gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_IMPCMD, "attrs %s", target->getName() );
	                gDataMgr->logPermanent ( target->getLogin(), target->getName(), _DMGR_PLOG_IMPCMD, "%s attrs", player->getName() );

			roomMgr->sendSystemMsg ( "Attributes", player, "%s:\nstr: %d\nint: %d\ndex: %d\nend: %d", target->character->getName(), target->character->strength, target->character->intelligence, target->character->dexterity, target->character->endurance );
		} else {
			roomMgr->sendPlayerText ( player, "|c60|Info> '%s' could not be found.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|Info> You need to specify a name.\n" );
	}
}

void cmdSpellList ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	char output[10240] = "";

	File file ( "../logs/spelluse.txt" );
	file.truncate();

	for ( int i=0; i<_SPELL_MAX; i++ ) {
		char text[1024];
		sprintf ( sizeof ( text ), text, "%03d: %d ** ", i, gSpellTable[i].castCount );
		strcat ( output, text );

		file.printf ( "%d %d\n", i, gSpellTable[i].castCount );
	}

	roomMgr->sendSystemMsg ( "Spell Usage", player, "%s", output );
}

void cmdMsgs ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	char* pMsg = IPCStats::display();

	File file ( "../logs/msgstats.txt" );
	file.truncate();

	file.write ( pMsg, strlen( pMsg ) );

	file.close();

	roomMgr->sendSystemMsg ( "Message Statistics", player, pMsg );
}

void cmdLag( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;

	char lagMsg[10240] = "";

	RMPlayer* pClientLowest = NULL;
	RMPlayer* pClientHighest = NULL;
	RMPlayer* pServerLowest = NULL;
	RMPlayer* pServerHighest = NULL;
	
	int nCount = 0;
	unsigned int nClientLag = 0;
	unsigned int nServerLag = 0;
	
	// go through the list of players and put their names in the packet
	LinkedElement *element = roomMgr->players()->head();

	while ( element ) {
		RMPlayer * rPlayer = (RMPlayer *)element->ptr();
		element = element->next();

		if ( rPlayer->pingTime ) {
			if ( !pClientLowest || rPlayer->pingClientTime < pClientLowest->pingClientTime ) 
				pClientLowest = rPlayer;

			if ( !pClientHighest || rPlayer->pingClientTime > pClientHighest->pingClientTime ) 
				pClientHighest = rPlayer;

			if ( !pServerLowest || rPlayer->pingTime < pServerLowest->pingTime ) 
				pServerLowest = rPlayer;

			if ( !pServerHighest || rPlayer->pingTime > pServerHighest->pingTime ) 
				pServerHighest = rPlayer;

			nClientLag += rPlayer->pingClientTime;
			nServerLag += rPlayer->pingTime;
			nCount++;
		}
	}

	if ( nCount ) {
		sprintf( sizeof( lagMsg ), lagMsg, "Server Side\n\nAverage lag = %d seconds\nLowest player = %s with %d seconds and %d.%d client lag\nHighest player = %s with %d seconds and %d.%d client lag\n\n\nClient Side\n\nAverage lag = %d.%d seconds\nLowest player = %s with %d.%d seconds and %d server lag\nHighest player = %s with %d.%d seconds and %d server lag\n", 
			( nServerLag / nCount ),
			pServerLowest->getName(), pServerLowest->pingTime, ( pServerLowest->pingClientTime / 60 ), ( ( ( pServerLowest->pingClientTime % 60 ) * 166 ) / 100 ),
			pServerHighest->getName(), pServerHighest->pingTime, ( pServerHighest->pingClientTime / 60 ), ( ( ( pServerHighest->pingClientTime % 60 ) * 166 ) / 100 ),
			( ( nClientLag / nCount ) / 60 ), ( ( ( ( nClientLag / nCount ) % 60 ) * 166 ) / 100 ),
			pClientLowest->getName(), ( pClientLowest->pingClientTime / 60 ), ( ( ( pClientLowest->pingClientTime % 60 ) * 166 ) / 100 ), pClientLowest->pingTime, 
			pClientHighest->getName(), ( pClientHighest->pingClientTime / 60 ), ( ( ( pClientHighest->pingClientTime % 60 ) * 166 ) / 100 ), pClientHighest->pingTime
		);

		roomMgr->sendSystemMsg ( "Lag Statistics", player, lagMsg );
	} else {
		roomMgr->sendSystemMsg ( "Lag Statistics", player, "No data yet" );
	}
}

void cmdStatus ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	int nPlayers = 0;
	int nNPCs = 0;

	unsigned int dungeonCount_shuttingDown = 0;
	unsigned int dungeonCount_active = 0;
	unsigned int dungeonCount_error = 0;

	LinkedElement* zoneElement = gZones.head();

	while ( zoneElement ) {
		Zone* zone = static_cast< Zone*>( zoneElement->ptr() );
		zoneElement = zoneElement->next();

		if( !zone ) continue;

		nPlayers += zone->players.size();
		nNPCs += zone->npcs.size();
		nNPCs += zone->externalNPCs.size();

		if ( zone->isDungeon && zone->entrance ) {
			if( zone->dungeonShutdownTimer > 0 ) {
				dungeonCount_shuttingDown++;
			} else if ( zone->dungeonShutdownTimer == 0 ) {
				dungeonCount_active++;
			} else {
				dungeonCount_error++;
			}
		}
	}

	int upTime = getsecondsfast() - gStartTime;
	unsigned int upHours = upTime / 3600;
	unsigned int upMinutes = ( upTime - (upHours * 3600) ) / 60;
	unsigned int upSeconds = upTime - (upHours * 3600) - (upMinutes * 60);

	char output[10240];
	
	sprintf ( sizeof(output), output, "Dwarves and Giants Server v0.1\nBuild Date: %s %s\n----------------------------------\nuptime = %.2d:%.2d:%.2d\n\ndungeons active = %d\ndungeons disposing = %d\ndungeons in error = %d\n\nmemory taken\n #%d allocations for %d bytes\nlargest single %d current allocs %d\n\nfile handles = %d\nobjects = %d\naffected objects = %d\n\nhouses -  %d total, %d disposing\n\nplayers = %d  NPCs = %d", __DATE__, __TIME__, upHours, upMinutes, upSeconds, dungeonCount_active, dungeonCount_shuttingDown, dungeonCount_error, gAllocCount, gAllocSize, gLargestAllocSize, g_nAllocations, gFileHandles.val(), roomMgr->_objects.size(), gAffectedObjects.size(), gBuildings.size(), gEmptyBuildings.size(), nPlayers, nNPCs );

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

	roomMgr->sendSystemMsg ( "Server Status", player, output );
}

void cmdState ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( player && player->character && player->character->character ) {
		BCharacter *pChar = player->character->character;
		
		roomMgr->sendSystemMsg ( "Your State", player, "|c13|Build Points: %d\nMelee Armor Pierce: %d\nEvil MDM Mod: %d\nGood MDM Mod: %d\nCast Resistance: S:%d E:%d M:%d T:%d N:%d\nSpell Resistance: S:%d E:%d M:%d T:%d N:%d\nSDM: S:%d E:%d M:%d T:%d N:%d\nPlayer Kills: %d\nNPC Kills: %d\n",  pChar->buildPoints, pChar->m_nMeleeArmorPiercing, pChar->m_nEvilMDMMod, pChar->m_nGoodMDMMod, pChar->m_anCastResistance[0], pChar->m_anCastResistance[1], pChar->m_anCastResistance[2], pChar->m_anCastResistance[3], pChar->m_anCastResistance[4], pChar->m_anSpellResistance[0], pChar->m_anSpellResistance[1], pChar->m_anSpellResistance[2], pChar->m_anSpellResistance[3], pChar->m_anSpellResistance[4], pChar->m_anSDM[0], pChar->m_anSDM[1], pChar->m_anSDM[2], pChar->m_anSDM[3], pChar->m_anSDM[4], pChar->playerKills, pChar->npcKills  );
	}
}

void cmdAbout ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	if ( tokens->size() > 1 ) {
		StringObject *name = (StringObject *)tokens->at ( 1 );
	
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target ) {
			char *login = "<unknown>";

			if ( target->checkAccess ( _ACCESS_IMPLEMENTOR ) ) 
				login = "Implementor";
			else if ( target->checkAccess ( _ACCESS_MODERATOR ) && !player->checkAccess ( _ACCESS_IMPLEMENTOR ) ) 
				login = "Moderator";
			else if ( target->checkAccess ( _ACCESS_GUIDE ) && !player->checkAccess ( _ACCESS_IMPLEMENTOR ) ) 
				login = "Guide";
			else {
				login = target->getLogin();
			}

			if ( target->character && target->character->room ) {
				roomMgr->sendPlayerText ( player, "-m |c71|---- Account Info for (%s) %s is %s\n---- room number = %d  bad gossip count = %d  level = %d, last Ping %d seconds", 
					login, 
					target->getName(), 
					target->character->super, 
					target->character->room->number, 
					target->badGossipCount, 
					target->character->level,
					( getseconds() - target->lastWriteTime ) );
			} else {
				roomMgr->sendPlayerText ( player, "-m |c71|---- Account Info for (%s) is at the character selection/creation screen, last Ping %d seconds\n", login, ( getseconds() - target->lastWriteTime ) );
			}
		} else {
			roomMgr->sendPlayerText ( player, "-m |c60| Info on %s is not found.\n", name->data );
		}
	}
}

void cmdAboutRoom ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	if ( tokens->size() > 1 ) {

		StringObject *numStr = (StringObject *)tokens->at ( 1 );
		int number = atoi ( numStr->data );

		RMRoom *room = roomMgr->findRoom ( number );

		if ( room ) {
			Zone *zone = room->zone;

			char players[10240];

			players[0] = 0;

			if ( room->size() ) {
				LinkedElement *element = room->head();

				while ( element ) {
					RMPlayer *roomPlayer = (RMPlayer *)element->ptr();

					if ( players[0] )
						strcat( players, ", " );

					strcat( players, roomPlayer->getName() );

					if ( roomPlayer->isNPC )
						strcat( players, "(NPC)" );

					element = element->next();
				}

				if ( players[0] )
					strcat( players, "\n" );
			}

			char sTitle[1024];

			sprintf( 1023, sTitle, "Room #%d of %s", number, zone ? zone->name() : "<unknown>" );

			roomMgr->sendSystemMsg ( sTitle, player, "Exits = $%08x     Type = %d     Picture = %d\nFlags = $%08x     Rubbered = %s\n #%d object(s)        #%d (%d) player(s)\n%s%s\nZone:   #%d player(s)     #%d npc(s)     #%d External Npc(s)", 
				room->exits, 
				room->type, 
				room->picture, 
				room->flags,
				room->bRubberWalled ? "Yes" : "No",
				room->objects.size(),
				room->numPlayers,
				room->size(),
				players[ 0 ] ? "\nPlayers:\n\n" : "",
				players,
				zone ? zone->players.size() : 0, 
				zone ? zone->npcs.size() : 0, 
				zone ? zone->externalNPCs.size() : 0
			);
		} else {
			roomMgr->sendPlayerText ( player, "|c60|Info> Room number %d could not be found.\n", number );
		}
	}
	else
		roomMgr->sendPlayerText ( player, "|c60|Info> You must specify a Room number.\n" );

}

void cmdCandy( LinkedList *tokens, char *str, RMPlayer *player ) {
	LinkedElement* element = player->room->head();
	WorldObject* pCandy1 = roomMgr->findClass( "CandyHeal" );
	WorldObject* pCandy2 = roomMgr->findClass( "CandyInvis" );
	WorldObject* pCandy3 = roomMgr->findClass( "CandySeeInvis" );

	if ( !pCandy1 || !pCandy2 || !pCandy3 ) {
		roomMgr->sendPlayerText ( player, "Can not find the candy!!" );
		return;
	}
	
	int nCount = 0;

	while ( element ) {
		nCount++;

		WorldObject* object = new WorldObject;

		switch ( random( 1, 3 ) ) {
			case 1:
				object->copy( pCandy1 );
				break;
			case 2:
				object->copy( pCandy2 );
				break;
			case 3:
				object->copy( pCandy3 );
				break;
		}
		
		object->addToDatabase();

		RMPlayer* target = (RMPlayer*) element->ptr();
		element = element->next();

		object->forceIn( target->character );
		object->makeVisible ( 1 );

		roomMgr->sendPlayerText( target, "-0You just caught some candy that was thrown to you!" );
	}

	gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_EVENTS, "gave out %d candies", nCount );
}

void cmdPumpkin( LinkedList *tokens, char *str, RMPlayer *player ) {
	LinkedElement* element = player->room->head();
	WorldObject* pPumpkin1 = roomMgr->findClass( "Pumpkin" );
	WorldObject* pPumpkin2 = roomMgr->findClass( "LargePumpkin" );

	if ( !pPumpkin1 || !pPumpkin2 ) {
		roomMgr->sendPlayerText ( player, "Can not find the pumpkins!!" );
		return;
	}

	int nCount = 0;

	while ( element ) {
		nCount++;

		WorldObject* object = new WorldObject;

		switch ( random( 1, 2 ) ) {
			case 1:
				object->copy( pPumpkin1 );
				break;
			case 2:
				object->copy( pPumpkin2 );
				break;
		}
		
		object->addToDatabase();

		RMPlayer* target = (RMPlayer*) element->ptr();
		element = element->next();

		object->forceIn( target->character );
		object->makeVisible ( 1 );

		roomMgr->sendPlayerText( target, "-0You just caught a prize that was thrown to you!" );
	}

	gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_EVENTS, "gave out %d pumpkins", nCount );
}

int nPlaceRoom[50] = { 5014, 5013, 5012, 5011, 5010, 5010, 5009, 5009, 5008, 5007, 5007, 5006, 5005, 5004, 5003, 5002, 5001, 5000, 5000, 5000,5015, 5030, 5045, 5060, 5075, 5076, 5076, 5077, 5078, 5078, 5079, 5080, 5080, 5081, 5082, 5083, 5083, 5084, 5085, 5085, 5086, 5087, 5088, 5089, 5089, 5074, 5059, 5044, 5029, 5029 };
int nPlaceX[50] = { 59, 419, 436, 377, 535, 86, 558, 86, 312, 543, 144, 55, 292, 544, 419, 54, 458, 171, 349, 559, 514, 507, 76, 404, 105, 228, 581, 276, 76, 541, 79, 195, 573, 315, 319, 165, 477, 307, 62, 574, 324, 359, 314, 80, 253, 553, 35, 214, 61, 414 };
int nPlaceY[50] = { 225, 219, 248, 178, 139, 139, 142, 142, 183, 163, 163, 178, 222, 204, 216, 197, 204, 215, 141, 169, 162, 168, 225, 253, 275, 205, 228, 245, 211, 212, 232, 170, 164, 140, 209, 120, 120, 158, 119, 119, 147, 136, 149, 112, 112, 158, 114, 225, 195, 214 };

void cmdPlace( LinkedList *tokens, char *str, RMPlayer *player ) {
	RMRoom *room = roomMgr->findRoom ( nPlaceRoom[0] );
	WorldObject* super = roomMgr->findClass ( "Wheatbale" );

	if ( room ) {
		LinkedElement* element = room->objects.head();
		WorldObject* object = NULL;
		int nFound = 0;

		while ( !nFound && element ) {
			object = (WorldObject *)element->ptr();

			if ( object->classNumber == super->classNumber ) {
				nFound = 1;
			}

			element = element->next();
		}

		if ( nFound ) {	//	Remove the markers
			for (int nItem = 0;nItem < 50;nItem++) {
				room = roomMgr->findRoom( nPlaceRoom[nItem] );

				if ( room ) {
					element = room->objects.head();

					while ( element ) {
						object = (WorldObject *) element->ptr();
						element = element->next();

						if ( object->classNumber == super->classNumber ) {
							room->delObject( object );
						}
					}
				}
			}
		} else {		//	Place the markers.
			for (int nItem = 0;nItem < 50;nItem++) {
				room = roomMgr->findRoom( nPlaceRoom[nItem] );

				if ( room )
					room->addObject( "Wheatbale", nPlaceX[nItem], nPlaceY[nItem], 0, 1 );
			}
		}
	}

	gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_EVENTS, "placed the markers" );
}

int nPlaceTownSnow[285] = {5000, 5001, 5002, 5003, 5004, 5005, 5006, 5007, 5008, 5009, 5010, 5011, 5012, 5013, 5014, 5015, 5016, 5017, 5018, 5019, 5020, 5021, 5022, 5023, 5024, 5025, 5026, 5027, 5028, 5029, 5030, 5031, 5032, 5033, 5034, 5035, 5036, 5037, 5038, 5039, 5040, 5041, 5042, 5043, 5044, 5045, 5046, 5047, 5048, 5049, 5050, 5051, 5052, 5053, 5054, 5055, 5056, 5057, 5058, 5059, 5060, 5061, 5062, 5063, 5064, 5065, 5066, 5067, 5068, 5069, 5070, 5071, 5072, 5073, 5074, 5075, 5076, 5077, 5078, 5079, 5080, 5081, 5082, 5083, 5084, 5085, 5086, 5087, 5088, 5089, 5100, 5101, 5102, 5103, 5104, 5105, 5106, 5107, 5108, 5109, 5110, 5111, 5112, 5113, 5114, 5200, 5201, 5202, 5203, 5204, 5205, 5206, 5207, 5208, 5209, 5210, 5211, 5212, 5213, 5214, 5215, 5216, 5217, 5218, 5219, 5300, 5301, 5302, 5303, 5304, 5305, 5306, 5307, 5308, 5309, 5310, 5311, 5312, 5313, 5314, 5400, 5401, 5402, 5403, 5404, 5405, 5406, 5407, 5408, 5409, 5410, 5411, 5412, 5413, 5414, 5500, 5501, 5502, 5503, 5504, 5505, 5506, 5507, 5508, 5509, 5510, 5511, 5600, 5601, 5602, 5603, 5604, 5605, 5606, 5607, 5608, 5609, 5610, 5611, 5612, 5613, 5614, 5800, 5801, 5802, 5803, 5804, 5805, 5806, 5807, 5808, 5809, 5810, 5811, 5812, 5813, 5814, 5815, 5816, 5817, 5900, 5901, 5902, 5903, 5904, 5905, 5906, 5907, 5908, 5909, 5910, 5911, 6000, 6022, 6023, 6024, 6041, 6089, 6117, 6816, 6912, 6914, 7000, 7001, 7002, 7003, 7004, 7005, 7006, 7007, 7008, 7009, 7010, 7011, 7012, 7013, 7014, 7015, 7016, 7017, 7018, 7019, 7020, 7021, 7022, 7023, 7100, 7101, 7102, 7103, 7104, 7105, 7106, 7107, 7108, 7109, 7110, 7111, 7112, 7113, 7114, 7300, 7301, 7302, 7303, 7304, 7305, 7306, 7307, 7308, 7309, 7310, 7311, 7312, 7313, 7314, 26250, 26251, 26252, 26253, 26254, 26255, 26256, 26257, 26258};
int nPlaceWildSnow[3880] = {1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023, 1024, 1025, 1026, 1027, 1028, 1029, 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1037, 1038, 1039, 1040, 1041, 1042, 1043, 1044, 1045, 1046, 1047, 1100, 1101, 1102, 1103, 1104, 1105, 1106, 1107, 1108, 1109, 1110, 1111, 1112, 1113, 1114, 1115, 1116, 1117, 1118, 1119, 1120, 1121, 1122, 1123, 1124, 1125, 1126, 1127, 1128, 1129, 1130, 1131, 1132, 1133, 1134, 1135, 1136, 1137, 1138, 1139, 1140, 1141, 1142, 1143, 1144, 1145, 1146, 1147, 1148, 1150, 1151, 1152, 1153, 1154, 1155, 1156, 1157, 1158, 1159, 1160, 1161, 1162, 1163, 1164, 1165, 1166, 1167, 1168, 1169, 1170, 1171, 1172, 1173, 1174, 1175, 1176, 1177, 1178, 1179, 1180, 1181, 1182, 1183, 1184, 1185, 1186, 1187, 1188, 1189, 1190, 1191, 1192, 1193, 1194, 1195, 1196, 1197, 1200, 1201, 1202, 1203, 1204, 1205, 1206, 1207, 1208, 1209, 1210, 1211, 1212, 1213, 1214, 1215, 1216, 1217, 1218, 1219, 1220, 1221, 1222, 1223, 1224, 1225, 1226, 1227, 1228, 1229, 1230, 1231, 1232, 1233, 1234, 1235, 1239, 1240, 1241, 1242, 1243, 1244, 1245, 1246, 1247, 1250, 1251, 1252, 1253, 1254, 1255, 1256, 1257, 1258, 1259, 1260, 1261, 1262, 1263, 1264, 1265, 1266, 1267, 1268, 1269, 1270, 1271, 1272, 1273, 1274, 1275, 1276, 1277, 1278, 1279, 1280, 1281, 1282, 1283, 1284, 1285, 1286, 1287, 1288, 1289, 1290, 1291, 1292, 1293, 1294, 1295, 1296, 1300, 1301, 1302, 1303, 1304, 1305, 1306, 1307, 1308, 1309, 1310, 1311, 1312, 1313, 1314, 1315, 1316, 1317, 1318, 1319, 1320, 1321, 1322, 1323, 1324, 1325, 1326, 1327, 1328, 1329, 1330, 1331, 1332, 1333, 1334, 1335, 1336, 1337, 1338, 1339, 1340, 1341, 1342, 1343, 1344, 1345, 1346, 1347, 1350, 1351, 1352, 1353, 1354, 1355, 1356, 1357, 1358, 1359, 1360, 1361, 1362, 1363, 1364, 1365, 1366, 1367, 1368, 1369, 1370, 1371, 1372, 1373, 1374, 1375, 1376, 1377, 1378, 1379, 1380, 1381, 1382, 1383, 1384, 1385, 1386, 1387, 1388, 1389, 1390, 1391, 1392, 1393, 1394, 1395, 1396, 1397, 1398, 1399, 1400, 1401, 1402, 1403, 1404, 1405, 1406, 1407, 1408, 1409, 1410, 1411, 1412, 1413, 1414, 1415, 1416, 1417, 1418, 1419, 1420, 1421, 1422, 1423, 1424, 1425, 1426, 1427, 1428, 1429, 1430, 1431, 1432, 1433, 1434, 1435, 1436, 1437, 1438, 1439, 1440, 1441, 1442, 1443, 1444, 1445, 1446, 1447, 1448, 1449, 1450, 1451, 1452, 1453, 1454, 1455, 1456, 1457, 1458, 1459, 1460, 1461, 1462, 1463, 1464, 1465, 1466, 1467, 1468, 1469, 1470, 1471, 1472, 1473, 1474, 1475, 1476, 1477, 1478, 1479, 1480, 1481, 1482, 1483, 1484, 1485, 1486, 1487, 1488, 1489, 1490, 1491, 1492, 1493, 1494, 1495, 1496, 1497, 1500, 1501, 1502, 1503, 1504, 1505, 1506, 1507, 1508, 1509, 1510, 1511, 1512, 1513, 1514, 1515, 1516, 1517, 1518, 1519, 1520, 1521, 1522, 1523, 1524, 1525, 1526, 1527, 1528, 1529, 1530, 1531, 1532, 1533, 1534, 1535, 1536, 1537, 1538, 1539, 1540, 1541, 1542, 1543, 1544, 1545, 1546, 1547, 1548, 1549, 1550, 1551, 1552, 1553, 1554, 1555, 1556, 1557, 1558, 1559, 1560, 1561, 1562, 1563, 1564, 1565, 1566, 1567, 1568, 1569, 1570, 1571, 1572, 1573, 1574, 1575, 1576, 1577, 1578, 1579, 1580, 1581, 1582, 1583, 1584, 1585, 1586, 1587, 1588, 1589, 1590, 1591, 1592, 1593, 1594, 1595, 1596, 1597, 1598, 1600, 1601, 1602, 1603, 1604, 1605, 1606, 1607, 1608, 1609, 1610, 1611, 1612, 1613, 1614, 1615, 1616, 1617, 1618, 1619, 1620, 1621, 1622, 1623, 1624, 1625, 1626, 1627, 1628, 1629, 1630, 1631, 1632, 1633, 1634, 1635, 1636, 1637, 1638, 1639, 1640, 1641, 1642, 1643, 1644, 1645, 1646, 1647, 1650, 1651, 1652, 1653, 1654, 1655, 1656, 1657, 1658, 1659, 1660, 1661, 1662, 1663, 1664, 1665, 1666, 1667, 1668, 1669, 1670, 1671, 1672, 1673, 1674, 1675, 1676, 1677, 1678, 1679, 1680, 1681, 1682, 1683, 1684, 1685, 1686, 1687, 1688, 1689, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024, 2025, 2026, 2027, 2028, 2029, 2030, 2031, 2032, 2033, 2034, 2035, 2036, 2037, 2038, 2039, 2040, 2041, 2042, 2043, 2044, 2045, 2046, 2047, 2048, 2049, 2050, 2051, 2052, 2053, 2054, 2055, 2056, 2057, 2058, 2059, 2100, 2101, 2102, 2103, 2104, 2105, 2106, 2107, 2108, 2109, 2110, 2111, 2112, 2113, 2114, 2115, 2119, 2120, 2121, 2122, 2123, 2124, 2125, 2126, 2127, 2128, 2129, 2130, 2131, 2132, 2133, 2134, 2135, 2136, 2137, 2138, 2139, 2140, 2141, 2142, 2143, 2144, 2145, 2146, 2147, 2148, 2149, 2150, 2151, 2152, 2153, 2154, 2155, 2156, 2157, 2158, 2159, 2160, 2161, 2162, 2163, 2164, 2165, 2166, 2167, 2168, 2169, 2170, 2171, 2172, 2173, 2174, 2175, 2176, 2177, 2178, 2179, 2180, 2181, 2182, 2183, 2184, 2185, 2186, 2187, 2188, 2189, 2190, 2191, 2192, 2193, 2194, 2195, 2196, 2197, 2198, 2199, 2200, 2201, 2202, 2203, 2204, 2205, 2206, 2207, 2208, 2209, 2210, 2211, 2212, 2213, 2214, 2215, 2216, 2217, 2218, 2219, 2220, 2221, 2222, 2223, 2224, 2225, 2226, 2227, 2228, 2229, 2230, 2231, 2232, 2233, 2234, 2235, 2236, 2237, 2238, 2239, 2240, 2241, 2242, 2243, 2244, 2245, 2246, 2247, 2248, 2249, 2250, 2251, 2252, 2253, 2254, 2255, 2256, 2257, 2258, 2259, 2260, 2261, 2262, 2263, 2264, 2265, 2266, 2267, 2268, 2269, 2270, 2271, 2272, 2273, 2274, 2275, 2276, 2277, 2278, 2279, 2280, 2281, 2282, 2283, 2284, 2285, 2286, 2287, 2288, 2289, 2290, 2291, 2292, 2293, 2294, 2295, 2296, 2297, 2298, 2299, 2500, 2501, 2502, 2503, 2504, 2505, 2506, 2507, 2508, 2509, 2510, 2511, 2512, 2513, 2514, 2515, 2516, 2517, 2518, 2519, 2520, 2521, 2522, 2523, 2524, 2525, 2526, 2527, 2528, 2529, 2900, 2901, 2902, 2903, 2904, 2905, 2906, 2907, 2908, 2909, 2910, 2911, 2912, 2913, 2914, 2915, 2916, 2917, 2918, 2919, 2920, 2921, 2922, 2923, 2924, 3000, 3001, 3002, 3003, 3004, 3005, 3006, 3007, 3008, 3009, 3010, 3011, 3012, 3013, 3014, 3015, 3016, 3017, 3018, 3019, 3020, 3021, 3022, 3023, 3024, 3025, 3026, 3027, 3028, 3029, 3030, 3031, 3032, 3033, 3034, 3035, 3036, 3037, 3038, 3039, 3040, 3041, 3042, 3043, 3044, 3045, 3046, 3047, 3100, 3101, 3102, 3103, 3104, 3105, 3106, 3107, 3108, 3109, 3110, 3111, 3112, 3113, 3114, 3115, 3116, 3117, 3118, 3119, 3120, 3121, 3122, 3123, 3124, 3125, 3126, 3127, 3128, 3129, 3130, 3131, 3132, 3133, 3134, 3135, 3136, 3137, 3138, 3139, 3140, 3141, 3142, 3143, 3144, 3145, 3146, 3147, 3148, 3150, 3151, 3152, 3153, 3154, 3155, 3156, 3157, 3158, 3159, 3160, 3161, 3162, 3163, 3164, 3165, 3166, 3167, 3168, 3169, 3170, 3171, 3172, 3173, 3174, 3175, 3176, 3177, 3178, 3179, 3180, 3181, 3182, 3183, 3184, 3185, 3186, 3187, 3188, 3189, 3190, 3191, 3192, 3193, 3194, 3195, 3196, 3197, 3198, 3199, 3200, 3201, 3202, 3203, 3204, 3205, 3206, 3207, 3208, 3209, 3210, 3211, 3212, 3213, 3214, 3215, 3216, 3217, 3218, 3219, 3220, 3221, 3222, 3223, 3224, 3225, 3226, 3227, 3228, 3229, 3230, 3231, 3232, 3233, 3234, 3235, 3236, 3237, 3238, 3239, 3240, 3241, 3242, 3243, 3244, 3245, 3246, 3247, 3248, 3250, 3251, 3252, 3253, 3254, 3255, 3256, 3257, 3258, 3259, 3260, 3261, 3262, 3263, 3264, 3265, 3266, 3267, 3268, 3269, 3270, 3271, 3272, 3273, 3274, 3275, 3276, 3277, 3278, 3279, 3280, 3281, 3282, 3283, 3284, 3285, 3286, 3287, 3288, 3289, 3290, 3291, 3292, 3300, 3301, 3302, 3303, 3304, 3305, 3306, 3307, 3308, 3309, 3310, 3311, 3312, 3313, 3314, 3315, 3316, 3317, 3318, 3319, 3320, 3321, 3322, 3323, 3324, 3328, 3329, 3330, 3331, 3332, 3333, 3334, 3335, 3336, 3337, 3338, 3339, 3340, 3341, 3342, 3343, 4000, 4001, 4002, 4003, 4004, 4005, 4006, 4007, 4008, 4009, 4010, 4011, 4012, 4013, 4014, 4015, 4016, 4017, 4018, 4019, 4020, 4021, 4022, 4023, 4024, 4025, 4026, 4027, 4028, 4029, 4030, 4031, 4032, 4033, 4034, 4035, 4036, 4037, 4038, 4039, 4040, 4041, 4042, 4043, 4044, 4045, 4046, 4047, 4048, 4049, 4050, 4051, 4052, 4053, 4054, 4055, 4056, 4057, 4058, 4059, 4100, 4101, 4102, 4103, 4104, 4105, 4106, 4107, 4108, 4109, 4110, 4111, 4112, 4113, 4114, 4115, 4116, 4117, 4118, 4119, 4120, 4121, 4122, 4123, 4124, 4125, 4126, 4127, 4128, 4129, 4130, 4131, 4132, 4133, 4134, 4135, 4136, 4137, 4138, 4139, 4140, 4141, 4142, 4143, 4144, 4145, 4146, 4147, 4148, 4150, 4151, 4152, 4153, 4154, 4155, 4156, 4157, 4158, 4159, 4160, 4161, 4162, 4163, 4164, 4165, 4166, 4167, 4168, 4169, 4170, 4171, 4172, 4173, 4174, 4175, 4176, 4177, 4178, 4179, 4180, 4181, 4182, 4183, 4184, 4185, 4186, 4187, 4188, 4189, 4190, 4191, 4192, 4193, 4194, 4195, 4196, 4197, 4198, 4200, 4201, 4202, 4203, 4204, 4205, 4206, 4207, 4208, 4209, 4210, 4211, 4212, 4213, 4214, 4215, 4216, 4217, 4218, 4219, 4220, 4221, 4222, 4223, 4224, 4225, 4226, 4227, 4228, 4229, 4230, 4231, 4232, 4233, 4234, 4235, 4236, 4237, 4238, 4239, 4240, 4241, 4242, 4243, 4244, 4245, 4246, 4247, 4300, 4301, 4302, 4303, 4304, 4305, 4306, 4307, 4308, 4309, 4310, 4311, 4312, 4313, 4314, 4315, 4316, 4317, 4318, 4319, 4320, 4321, 4322, 4323, 4324, 4325, 4326, 4327, 4328, 4329, 4330, 4331, 4332, 4333, 4334, 4335, 4336, 4350, 4351, 4352, 4353, 4354, 4355, 4356, 4357, 4358, 4359, 4360, 4361, 4362, 4363, 4364, 4365, 4366, 4367, 4368, 4369, 4370, 4371, 4372, 4373, 4374, 4375, 4376, 4377, 4378, 4379, 4380, 4381, 4382, 4383, 4384, 4385, 4386, 4387, 4400, 4401, 4402, 4403, 4404, 4405, 4406, 4407, 4408, 4409, 4410, 4411, 4412, 4413, 4414, 4415, 4416, 4417, 4418, 4419, 4420, 4421, 4422, 4423, 4424, 4425, 4426, 4427, 4428, 4429, 4430, 4431, 4432, 4433, 4434, 4435, 4436, 4437, 4438, 4442, 4443, 4444, 4445, 4446, 4447, 4448, 4449, 4450, 4451, 4452, 4453, 4454, 4455, 4456, 4457, 4458, 4459, 4460, 4461, 4462, 4463, 4464, 4465, 4466, 4467, 4468, 4469, 4470, 4471, 4472, 4473, 4474, 4475, 4476, 4477, 4478, 4479, 4480, 4481, 4482, 4483, 4484, 4485, 4486, 4487, 4488, 4489, 4490, 4491, 4492, 4493, 4494, 4495, 4496, 4497, 4498, 4499, 4500, 4501, 4502, 4503, 4504, 4505, 4506, 4507, 4508, 4509, 4510, 4511, 4512, 4513, 4514, 4515, 4516, 4517, 4518, 4519, 4520, 4521, 4522, 4523, 4524, 4525, 4526, 4527, 4528, 4529, 4530, 4531, 4532, 4533, 4534, 4535, 4536, 4537, 4538, 4539, 4540, 4541, 4542, 4543, 4544, 4545, 4546, 4547, 4548, 4549, 4550, 4551, 4552, 4553, 4554, 4555, 4556, 4557, 4558, 4559, 4560, 4561, 4562, 4563, 4564, 4565, 4566, 4567, 4568, 4569, 4570, 4571, 4572, 4573, 4574, 4575, 4576, 4577, 4578, 4579, 4580, 4581, 4582, 4583, 4584, 4585, 4586, 4587, 4588, 4589, 4590, 4591, 4592, 4593, 4594, 4595, 4596, 4597, 4598, 4599, 4600, 4601, 4602, 4603, 4604, 4605, 4606, 4607, 4608, 4609, 4610, 4611, 4612, 4613, 4614, 4615, 4616, 4617, 4618, 4619, 4620, 4621, 4622, 4623, 4624, 4625, 4626, 4627, 4628, 4629, 4630, 4631, 4632, 4633, 4634, 4635, 4636, 4637, 4638, 4639, 4640, 4641, 4642, 4643, 4644, 4645, 4646, 4647, 4648, 4649, 4650, 4651, 4652, 4653, 4654, 4655, 4656, 4657, 4658, 4659, 4660, 4661, 4662, 4663, 4664, 4665, 4666, 4667, 4668, 4669, 4670, 4671, 4672, 4673, 4674, 4675, 4676, 4677, 4678, 4679, 4680, 4681, 4682, 4683, 4684, 4685, 4686, 4687, 4688, 4689, 4690, 4691, 4692, 4693, 4694, 4695, 4696, 4697, 4698, 4699, 4700, 4701, 4702, 4703, 4704, 4705, 4706, 4707, 4708, 4709, 4710, 4711, 4712, 4713, 4714, 4715, 4716, 4717, 4718, 4719, 4720, 4721, 4722, 4723, 4724, 4725, 4726, 4727, 4728, 4729, 4730, 4731, 4732, 4733, 4734, 4735, 4736, 4737, 4738, 4739, 4740, 4741, 4742, 4743, 4744, 4745, 4746, 4747, 4748, 4749, 4750, 4751, 4752, 4753, 4754, 4755, 4756, 4757, 4758, 4759, 4760, 4761, 4762, 4763, 4764, 4765, 4766, 4767, 4768, 4769, 4770, 4771, 4772, 4773, 4774, 4775, 4776, 4777, 4778, 4779, 4780, 4781, 4782, 4783, 4784, 4785, 4786, 4787, 4788, 4789, 4790, 4791, 4792, 4793, 4794, 4795, 4796, 4797, 4798, 4799, 4800, 4801, 4802, 4803, 4804, 4805, 4806, 4807, 4808, 4809, 4810, 4811, 4812, 4813, 4814, 4815, 4816, 4817, 4818, 4819, 4820, 4821, 4822, 4823, 4824, 4825, 4826, 4827, 4828, 4829, 4830, 4831, 4832, 4833, 4834, 4835, 4836, 4837, 4838, 4839, 4840, 4841, 4842, 4843, 4844, 4845, 4846, 4847, 4848, 4849, 9019, 9053, 9054, 9055, 9056, 9057, 9058, 9059, 9061, 9062, 9063, 9064, 9065, 9109, 9200, 9201, 9202, 9203, 9204, 9205, 9206, 9207, 9208, 9209, 9210, 9211, 9212, 9213, 9214, 9215, 9216, 9217, 9218, 9219, 9220, 9221, 9222, 9223, 9224, 9225, 9226, 9227, 9228, 9229, 9230, 9231, 9232, 9233, 9234, 9235, 9300, 9301, 9302, 9303, 9304, 9305, 9306, 9307, 9308, 9309, 9310, 9311, 9312, 9313, 9314, 9315, 9316, 9317, 9318, 9319, 9320, 9321, 9322, 9323, 9324, 9325, 9326, 9327, 9328, 9329, 9330, 9331, 9332, 9333, 9334, 9335, 10000, 10001, 10002, 10003, 10004, 10005, 10006, 10007, 10008, 10009, 10010, 10011, 10012, 10013, 10014, 10015, 10016, 10017, 10018, 10019, 10020, 10021, 10022, 10023, 10024, 10025, 10026, 10027, 10028, 10029, 10030, 10031, 10032, 10033, 10034, 10035, 10036, 10037, 10038, 10039, 10040, 10041, 10042, 10043, 10044, 10045, 10046, 10047, 11000, 11001, 11002, 11003, 11004, 11005, 11006, 11007, 11008, 11009, 11010, 11011, 12000, 12001, 12002, 12003, 12004, 12005, 12006, 12007, 12008, 12009, 12010, 12011, 12012, 12013, 12014, 12015, 12016, 12017, 12018, 12019, 12020, 12021, 12022, 12023, 12024, 12025, 12026, 12027, 12028, 12029, 12030, 12031, 12032, 12033, 12034, 12035, 12036, 12037, 12038, 12039, 12040, 12041, 12042, 12043, 12044, 12045, 12046, 12047, 12048, 12049, 12100, 12101, 12102, 12103, 12104, 12105, 12106, 12107, 12108, 12109, 12110, 12111, 12112, 12113, 12114, 12115, 12116, 12117, 12118, 12119, 12120, 12121, 12122, 12123, 12124, 12125, 12126, 12127, 12128, 12129, 12130, 12131, 12132, 12133, 12134, 12135, 12136, 12137, 12138, 12139, 12140, 12141, 12142, 12143, 12144, 12145, 12146, 12147, 12148, 12149, 12200, 12201, 12202, 12203, 12204, 12205, 12206, 12207, 12208, 12209, 12210, 12211, 12212, 12213, 12214, 12215, 12216, 12217, 12218, 12219, 12220, 12221, 12222, 12223, 12224, 12225, 12226, 12227, 12228, 12229, 12230, 12231, 12232, 12233, 12234, 12235, 12236, 12237, 12238, 12239, 12240, 12241, 12242, 12243, 12244, 12245, 12246, 12247, 12248, 12250, 12251, 12252, 12253, 12254, 12255, 12256, 12257, 12258, 12259, 12260, 12261, 12262, 12263, 12264, 12265, 12266, 12267, 12268, 12269, 12270, 12271, 12272, 12273, 12274, 12275, 12276, 12277, 12278, 12279, 12280, 12281, 12282, 12283, 12284, 12285, 12286, 12287, 12288, 12289, 12290, 12291, 12292, 12293, 12294, 12295, 12300, 12301, 12302, 12303, 12304, 12305, 12306, 12307, 12308, 12309, 12310, 12314, 12315, 12316, 12317, 12318, 12319, 12320, 12321, 12322, 12323, 12324, 12325, 12326, 12327, 12328, 12329, 12330, 12331, 12332, 12333, 12334, 12335, 12336, 12337, 12338, 12339, 12340, 12341, 12342, 12343, 12344, 12345, 12346, 12347, 12348, 12349, 12350, 12351, 12352, 12353, 12354, 12355, 12356, 12357, 12358, 12359, 12360, 12361, 12362, 12363, 12364, 12365, 12366, 12367, 12368, 12369, 12370, 12371, 12372, 12373, 12374, 12375, 12376, 12377, 12378, 12379, 12380, 12381, 12382, 12383, 12384, 12385, 12386, 12387, 12388, 12389, 12390, 12391, 12392, 12393, 12394, 12395, 12396, 12397, 12398, 12399, 12400, 12401, 12402, 12403, 12404, 12405, 12406, 12407, 12408, 12409, 12410, 12411, 12412, 12413, 12414, 12415, 12416, 12417, 12418, 12419, 12420, 12421, 12422, 12423, 12424, 12425, 12426, 12427, 12428, 12429, 12430, 12431, 12432, 12433, 12434, 12435, 12436, 12437, 12438, 12439, 12440, 12441, 13000, 13001, 13002, 13003, 13004, 13005, 13006, 13007, 13008, 13009, 13010, 13011, 13012, 13013, 13014, 13100, 13101, 13102, 13103, 13104, 13105, 13106, 13107, 13108, 13109, 13110, 13111, 13112, 13113, 13114, 13115, 13116, 13117, 13200, 13201, 13202, 13203, 13204, 13205, 13206, 13207, 13208, 13209, 13210, 13211, 13212, 13213, 13214, 13215, 13219, 13220, 13221, 13222, 13223, 13224, 13225, 13226, 13227, 13228, 13229, 13230, 13231, 13232, 13233, 13234, 13235, 13250, 13251, 13252, 13253, 13254, 13255, 13256, 13257, 13258, 13259, 13260, 13261, 13262, 13263, 13264, 13265, 13266, 13267, 13268, 13269, 13270, 13271, 13272, 13273, 13274, 13275, 13276, 13277, 13278, 13279, 13280, 13281, 13282, 13283, 13284, 16000, 16001, 16002, 16003, 16004, 16005, 16006, 16007, 16008, 16009, 16010, 16011, 16012, 16013, 16014, 16015, 16016, 16017, 16018, 16019, 16020, 16021, 16022, 16023, 16024, 16025, 16026, 16027, 16028, 16029, 16030, 16031, 16032, 16033, 16034, 16035, 16036, 16037, 16038, 16039, 16040, 16041, 16042, 16043, 16044, 16045, 16046, 16047, 16048, 16049, 16050, 16051, 16052, 16053, 16054, 16055, 16056, 16057, 16058, 16059, 16060, 16061, 16062, 16063, 16064, 16065, 16066, 16067, 16068, 16069, 16070, 16071, 16072, 16073, 16074, 16075, 16076, 16077, 16078, 16079, 16080, 16081, 16082, 16083, 16084, 16085, 16086, 16087, 16088, 16089, 16090, 16091, 16092, 16093, 16094, 16095, 16096, 16097, 16098, 16099, 16100, 16101, 16102, 16103, 16104, 16105, 16106, 16107, 16108, 16109, 16110, 16111, 16112, 16113, 16114, 16115, 16116, 16117, 16118, 16119, 16120, 16121, 16122, 16123, 16124, 16125, 16126, 16127, 16128, 16129, 16130, 16131, 16132, 16133, 16134, 16135, 16136, 16137, 16138, 16139, 16140, 16141, 16142, 16143, 16144, 16145, 16146, 16147, 16148, 16149, 16150, 16151, 16152, 16153, 16154, 16155, 16156, 16157, 16158, 16159, 16160, 16161, 16162, 16163, 16164, 16165, 16166, 16167, 16168, 16169, 16170, 16171, 16172, 16173, 16174, 16175, 16176, 16177, 16178, 16179, 16180, 16181, 16182, 16183, 16184, 16185, 16186, 16187, 16188, 16189, 16190, 16191, 16192, 16193, 16194, 16195, 16196, 16197, 16198, 16199, 16500, 16501, 16502, 16503, 16504, 16505, 16506, 16507, 16508, 16509, 16510, 16511, 16512, 16513, 16514, 16515, 16516, 16517, 16518, 16519, 16520, 16521, 16522, 16523, 16524, 16525, 16526, 16527, 16528, 16529, 16530, 16531, 16532, 16533, 16534, 16535, 16536, 16537, 16538, 16539, 16540, 16541, 16542, 16543, 16544, 16545, 16546, 16547, 16548, 16549, 16550, 16551, 16552, 16553, 16554, 16555, 16556, 16557, 16558, 16559, 16560, 16561, 16562, 16563, 16564, 16565, 16566, 16567, 16568, 16569, 16570, 16571, 16572, 16573, 16574, 16575, 16576, 16577, 16578, 16579, 16580, 16581, 16582, 16583, 16584, 16585, 16586, 16587, 16588, 16589, 16590, 16591, 16592, 16593, 16594, 16595, 16596, 16597, 16598, 16599, 16600, 16601, 16602, 16603, 16604, 16605, 16606, 16607, 16608, 16609, 16610, 16611, 16612, 16613, 16614, 16615, 16616, 16617, 16618, 16619, 16620, 16621, 16622, 16623, 16624, 16625, 16626, 16627, 16628, 16629, 16630, 16631, 16632, 16633, 16634, 16635, 16636, 16637, 16638, 16639, 16640, 16641, 16642, 16643, 16644, 16645, 16646, 16647, 16648, 16649, 16650, 16651, 16652, 16653, 16654, 16655, 16656, 16657, 16658, 16659, 16660, 16661, 16662, 16663, 16664, 16665, 16666, 16667, 16668, 16669, 16670, 16671, 16672, 16673, 16674, 16675, 16676, 16677, 16678, 16679, 16680, 16681, 16682, 16683, 16684, 16685, 16686, 16687, 16688, 16689, 16690, 16691, 16692, 16693, 16694, 16695, 16696, 16697, 16698, 16699, 20000, 20001, 20002, 20003, 20004, 20005, 20006, 20007, 20008, 20009, 20010, 20011, 20012, 20013, 20014, 20015, 20016, 20017, 20018, 20019, 20020, 20021, 20022, 20023, 20024, 20025, 20026, 20027, 20028, 20029, 20030, 20031, 20032, 20033, 20034, 20035, 20036, 20037, 20038, 20039, 20040, 20041, 20042, 20043, 20044, 20045, 20046, 20047, 20048, 20049, 20050, 20051, 20052, 20053, 20054, 20055, 20056, 20057, 20058, 20059, 20060, 20061, 20062, 20063, 20064, 20065, 20066, 20067, 20068, 20069, 20070, 20071, 20072, 20073, 20074, 20075, 20076, 20077, 20078, 20079, 20080, 20082, 20083, 20084, 20085, 20086, 20087, 20088, 20089, 20090, 20091, 20092, 20093, 20094, 20095, 20096, 20097, 20098, 20099, 20100, 20101, 20102, 20103, 20104, 20105, 20106, 20107, 20108, 20109, 20111, 20112, 20113, 20114, 20115, 20116, 20117, 20118, 20119, 20120, 20121, 20122, 20123, 20124, 20125, 20126, 20127, 20128, 20129, 20130, 20131, 20132, 20133, 20134, 20135, 20136, 20137, 20138, 20139, 20140, 20141, 20142, 20143, 20144, 20145, 20146, 20147, 20148, 20149, 20150, 20151, 20152, 20153, 20154, 20155, 20156, 20157, 20158, 20159, 20160, 20161, 20162, 20163, 20164, 20165, 20166, 20167, 20168, 20169, 20170, 20171, 20172, 20173, 20174, 20175, 20176, 20177, 20178, 20179, 20180, 20181, 20182, 20183, 20184, 20185, 20186, 20187, 20188, 20189, 20190, 20191, 20192, 20193, 20194, 20195, 20196, 20197, 20198, 20199, 20200, 20201, 20202, 20203, 20204, 20205, 20206, 20207, 20208, 20209, 20210, 20211, 20212, 20213, 20214, 20215, 20216, 20217, 20218, 20219, 20220, 20221, 20222, 20223, 20224, 20225, 20226, 20227, 20228, 20229, 20230, 20231, 20232, 20233, 20234, 20235, 20236, 20237, 20238, 20239, 20240, 20241, 20242, 20243, 20244, 20245, 20246, 20247, 20248, 21150, 25050, 25051, 25052, 25053, 25054, 25055, 25056, 25057, 25058, 25059, 25060, 25061, 25062, 25063, 25064, 25065, 25066, 25067, 25068, 25069, 25070, 25071, 25072, 25073, 25074, 25075, 25076, 25077, 25078, 25079, 25080, 25081, 25082, 25083, 25084, 25085, 25086, 25087, 25088, 25089, 25090, 25091, 25092, 25093, 25094, 25095, 25096, 25097, 25098, 25099, 25100, 25101, 25102, 25103, 25104, 25105, 25106, 25107, 25108, 25109, 25110, 25111, 25112, 25113, 25114, 25115, 25116, 25117, 25118, 25119, 25120, 25121, 25122, 25123, 25124, 25125, 25126, 25127, 25128, 25129, 25130, 25131, 25132, 25133, 25134, 25135, 25136, 25137, 25138, 25139, 25140, 25141, 25142, 25143, 25144, 25145, 25146, 25147, 25148, 25149, 25150, 25151, 25152, 25153, 25154, 25155, 25156, 25157, 25158, 25159, 25160, 25161, 25162, 25163, 25164, 25165, 25166, 25167, 25168, 25169, 25170, 25171, 25172, 25173, 25174, 25175, 25176, 25177, 25178, 25179, 25180, 25181, 25182, 25183, 25184, 25185, 25186, 25187, 25188, 25189, 25190, 25191, 25192, 25193, 25194, 25195, 25196, 25197, 25198, 25199, 26000, 26001, 26002, 26003, 26004, 26005, 26006, 26007, 26008, 26009, 26010, 26011, 26012, 26013, 26014, 26015, 26016, 26017, 26018, 26019, 26020, 26021, 26022, 26023, 26024, 26025, 26026, 26027, 26028, 26029, 26030, 26031, 26032, 26033, 26034, 26035, 26036, 26037, 26038, 26039, 26040, 26041, 26042, 26043, 26044, 26045, 26046, 26047, 26048, 26049, 26050, 26051, 26052, 26053, 26054, 26055, 26056, 26057, 26058, 26059, 26060, 26061, 26062, 26063, 26064, 26065, 26066, 26067, 26068, 26069, 26070, 26071, 26072, 26073, 26074, 26075, 26076, 26077, 26078, 26079, 26080, 26081, 26082, 26083, 26084, 26085, 26086, 26087, 26088, 26089, 26090, 26091, 26092, 26093, 26094, 26095, 26096, 26097, 26098, 26099, 26100, 26101, 26102, 26103, 26104, 26105, 26106, 26107, 26108, 26109, 26110, 26111, 26112, 26113, 26114, 26115, 26116, 26117, 26118, 26119, 26120, 26121, 26122, 26123, 26124, 26125, 26126, 26127, 26128, 26129, 26130, 26131, 26132, 26133, 26134, 26135, 26136, 26137, 26138, 26139, 26140, 26141, 26142, 26143, 26144, 26145, 26146, 26147, 26148, 26149, 26150, 26151, 26152, 26153, 26154, 26155, 26156, 26157, 26158, 26159, 26160, 26161, 26162, 26163, 26164, 26165, 26166, 26167, 26168, 26169, 26170, 26171, 26172, 26173, 26174, 26175, 26176, 26177, 26178, 26179, 26180, 26181, 26182, 26183, 26184, 26185, 26186, 26187, 26188, 26189, 26190, 26191, 26192, 26193, 26194, 26195, 26196, 26197, 26198, 26199, 26200, 26201, 26202, 26203, 26204, 26205, 26206, 26207, 26208, 26209, 26210, 26211, 26212, 27000, 27001, 27003, 27004, 27007, 27008, 27011, 27012, 27013, 27014, 27015, 27016, 27018, 27019, 27022, 27023, 27024, 27025, 27026, 27027, 27028, 27029, 27030, 27031, 27032, 27033, 27034, 27035, 27036, 27037, 27038, 27039, 27040, 27041, 27042, 27043, 27044, 27045, 27046, 27047, 27048, 27049, 27050, 27051, 27052, 27053, 27054, 27055, 27056, 27057, 27058, 27059, 27060, 27061, 27062, 27063, 27064, 27065, 27066, 27067, 27068, 27069, 27070, 27071, 27072, 27073, 27074, 27075, 27076, 27077, 27078, 27081, 27082, 27084, 27085, 27086, 27087, 27088, 50000, 50001, 50002, 50003, 50004, 50005, 50006, 50007, 50008, 50009, 50010, 50011, 50012, 50013, 50014, 50015, 50016, 50017, 50018, 50019, 50020, 50021, 50022, 50023, 50024, 50025, 50026, 50027, 50028, 50029, 50030, 50031, 50032, 50033, 50034, 50035, 50036, 50037, 50038, 50039, 50040, 50041, 50042, 50043, 50044, 50045, 50046, 50047, 50048, 50049, 50050, 50051, 50052, 50053, 50054, 50055, 50056, 50057, 50058, 50059, 50060, 50061, 50062, 50063, 50064, 50065, 50066, 50067, 50068, 50069, 50070, 50071, 50072, 50073, 50074, 50075, 50076, 50077, 50078, 50079, 50080, 50081, 50082, 50083, 50084, 50085, 50086, 50087, 50088, 50089, 50090, 50091, 50092, 50093, 50094, 50095, 50096, 50097, 50098, 50099, 50100, 50101, 50102, 50103, 50104, 50105, 50106, 50107, 50108, 50109, 50110, 50111, 50112, 50113, 50114, 50115, 50116, 50117, 50118, 50119, 50120, 50121, 50122, 50123, 50124, 50125, 50126, 50127, 50128, 50129, 50130, 50131, 50132, 50133, 50134, 50135, 50136, 50137, 50138, 50139, 50140, 50141, 50142, 50143, 50144, 50145, 50146, 50147, 50148, 50149, 50150, 50151, 50152, 50153, 50154, 50155, 50156, 50157, 50158, 50159, 50160, 50161, 50162, 50163, 50164, 50165, 50166, 50167, 50168, 50169, 50170, 50171, 50172, 50173, 50174, 50175, 50176, 50177, 50178, 50179, 50180, 50181, 50182, 50183, 50184, 50185, 50186, 50187, 50188, 50189, 50190, 50191, 50192, 50193, 50194, 50195, 50196, 50197, 50198, 50199, 50200, 50201, 50202, 50203, 50204, 50205, 50206, 50207, 50208, 50209, 50210, 50211, 50212, 50213, 50214, 50215, 50216, 50217, 90000, 90001, 90002, 90003, 90004, 90005, 90006, 90007, 90008, 90009, 90010, 90011, 90012, 90013, 90014, 90015, 90016, 90017, 90018, 90019, 90020, 90021, 92000, 92001, 92002, 92003, 93000, 93001, 93002, 93003, 93004, 93005, 93006, 93007, 93008, 93009, 93010, 93011, 93012, 93013, 93020, 95000, 95001, 95002, 95003, 95010, 95020, 95021, 95025, 96001, 96003, 96030 };
int nPlaceSnowX = 1;
int nPlaceWildSnowY = 370;
int nPlaceTownSnowY = 318;

void cmdSnow( LinkedList *tokens, char *str, RMPlayer *player ) {
    RMRoom *room = roomMgr->findRoom ( nPlaceRoom[0] );
    WorldObject* super = roomMgr->findClass ( "FallingSnow" );
 
    if ( room ) {
        LinkedElement* element = room->objects.head();
        WorldObject* object = NULL;
        int nFound = 0;
 
        while ( !nFound && element ) {
            object = (WorldObject *)element->ptr();
 
            if ( object->classNumber == super->classNumber ) {
                nFound = 1;
            }
 
            element = element->next();
        }
 
        if ( nFound ) {
			//Delete Town
            for (int nItem = 0;nItem < 285; nItem++) {
                room = roomMgr->findRoom( nPlaceTownSnow[nItem] );
 
                if ( room ) {
                    element = room->objects.head();
 
                    while ( element ) {
                        object = (WorldObject *) element->ptr();
                        element = element->next();
 
                        if ( object->classNumber == super->classNumber ) {
                            room->delObject( object );
                        }
                    }
                }
            }
			//Delete Wild
			for (int nItem = 0;nItem < 3880; nItem++) {
                room = roomMgr->findRoom( nPlaceWildSnow[nItem] );
 
                if ( room ) {
                    element = room->objects.head();
 
                    while ( element ) {
                        object = (WorldObject *) element->ptr();
                        element = element->next();
 
                        if ( object->classNumber == super->classNumber ) {
                            room->delObject( object );
                        }
                    }
                }
            }
        } else {
			//Place Town
            for (int nItem = 0;nItem < 285;nItem++) {
                room = roomMgr->findRoom( nPlaceTownSnow[nItem] );
 
                if ( room )
                    room->addObject( "FallingSnow", nPlaceSnowX, nPlaceTownSnowY, 0, 1 );
            }
			//Place Wild
			for (int nItem = 0;nItem < 3880;nItem++) {
                room = roomMgr->findRoom( nPlaceWildSnow[nItem] );
 
                if ( room )
                    room->addObject( "FallingSnow", nPlaceSnowX, nPlaceWildSnowY, 0, 1 );
            }
        }
    }
 
    gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_EVENTS, "started snow." );
}

void cmdMark ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( player->room ) {
		WorldObject* super = roomMgr->findClass ( "EventMarker" );
		LinkedElement* element = player->room->objects.head();
		WorldObject* object = NULL;
		int nFound = 0;

		while ( !nFound && element ) {
			object = (WorldObject *)element->ptr();

			if ( object->classNumber == super->classNumber )
				nFound = 1;
				
			element = element->next();
		}
													
		if ( nFound ) {
			player->room->delObject( object );
			gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_EVENTS, "removed a marker" );
		} else {
			player->room->addObject( "EventMarker", player->character->x, player->character->y, 0, 1 );
			gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_EVENTS, "placed a marker" );
		}
	}
}

void cmdConjure ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;

	int count = 1;

	if ( tokens->size() == 3 ) {
		StringObject *numItems = (StringObject *)tokens->at ( 2 );

		if (numItems) {
			count = atoi ( numItems->data );
			if ( !count )
				count = 1;
		}
	}

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		StringObject *classID = (StringObject *)tokens->at ( 1 );

		char *theClass = classID->data;
		WorldObject *super = roomMgr->findClass ( theClass );

		if ( super ) {
			gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_IMPCMD, "conjure %s %d", theClass, count );

			while (count) {
				WorldObject *object = super->clone();
				object->superObj = super;

				object->setClassID( theClass );
				object->setSuper( super->classID );
                
				BCarryable *bcarry = (BCarryable *) object->getBase ( _BCARRY );

				if ( bcarry ) {
					bcarry->sLastOwner = strdup( player->getName() );
				}

				// if this has NPC base
				if ( object->getBase ( _BNPC ) )  {

					//note - it wont work in a house and other places
					object->x = player->character->x;
					object->y = player->character->y;

					object->addToDatabase();

					NPC* npc = makeNPC ( object );

					npc->newRoom( player->room );
					npc->aiReady = 1;
					roomMgr->sendPlayerText ( player, "|c71|Info> %s created.", theClass );
					break;
				} else {
					object->addToDatabase();

					object->x = player->character->x;
					object->y = player->character->y;
						
					player->character->room->addObject ( object, TRUE );
				}

				count--;
			}
		} else {
			roomMgr->sendPlayerText ( player, "|c60|Info> %s is not a valid class.\n", theClass );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|You must specify an object.\n" );
	}
}

void cmdTestMail( LinkedList *tokens, char *str, RMPlayer *player ) {
	char testMessage[] = {
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789A"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789B"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789C"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789D"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789E"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789F"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789G"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789H"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789I"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789J"	\

		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789A"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789B"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789C"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789D"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789E"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789F"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789G"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789H"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789I"	\
		"12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678JJ"	\

		"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<A"	\
		"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<B"	\
		"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<C"	\
		"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<D"	\
		"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<E"	\
		"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<F"	\
		"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<G"	\
		"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<H"	\
		"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<I"	\
		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567JJJ"	\

		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789A"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789B"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789C"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789D"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789E"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789F"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789G"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789H"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789I"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456JJJJ"	\

		">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>A"	\
		">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>B"	\
		">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>C"	\
		">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>D"	\
		">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>E"	\
		">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>F"	\
		">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>G"	\
		">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>H"	\
		">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>I"	\
		"12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345JJJJJ"	\

		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789A"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789B"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789C"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789D"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789E"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789F"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789G"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789H"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789I"	\
		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234JJJJJJ"	\

		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789A"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789B"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789C"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789D"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789E"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789F"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789G"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789H"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789I"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123JJJJJJJ"	\

		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789A"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789B"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789C"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789D"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789E"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789F"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789G"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789H"	\
		"123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789I"	\
		"12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012JJJJJJJJ"
	};

	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	int nCount = 0;
	char* pName = NULL;

	if ( name ) {
		pName = name->data;

		StringObject *count = (StringObject *)tokens->at ( 2 );

		if ( !count ) {
			roomMgr->sendPlayerText ( player, "|c60|Info> You must specify how many magic mail(s) you want to send to %s.\n", name->data );
			return;
		}

		nCount = atoi ( count->data );

		if ( nCount ) {
			if ( nCount == 1 ) {
				roomMgr->sendPlayerText ( player, "|c60|Info> You must specify more than 1 magic mail you want.\n" );
			} else {
				gDataMgr->sendMail( player->character, pName, testMessage, nCount );
			}
		} else {
			roomMgr->sendPlayerText ( player, "|c60|Info> You must specify how many magic mail(s) you want.\n" );
			return;
		}
	}
}

// This sets the alignment of the specified toon and clears god curses and marks.
void cmdAlignment ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target && target->character ) {
			StringObject* alignment = (StringObject *)tokens->at ( 2 );

			if ( !alignment ) {
				roomMgr->sendPlayerText ( player, "|c60|Info> You must specify what value to set your alignment 0 - 255.\n" );
				return;
			}

			target->character->alignment = atoi( alignment->data );

			target->character->delAffect ( _AFF_CURSE_ENID, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, (PackedData*) NULL );
			target->character->delAffect ( _AFF_CURSE_DESPOTHES, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, (PackedData*) NULL );
			target->character->delAffect ( _AFF_CURSE_DUACH, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, (PackedData*) NULL );

			target->character->delAffect ( _AFF_MARK_ENID, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, (PackedData*) NULL );
			target->character->delAffect ( _AFF_MARK_DESPOTHES, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, (PackedData*) NULL );
			target->character->delAffect ( _AFF_MARK_DUACH, _AFF_TYPE_NORMAL, _AFF_SOURCE_SPELL, (PackedData*) NULL );

			roomMgr->sendPlayerText ( player, "|c71|You set their alignment.\n" );
		} else {
			roomMgr->sendPlayerText ( player, "|c60|You can't find '%s' to set their alignment.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|You must specify the person you want to set alignment of.\n" );
	}
}

void cmdDelete ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *objectToDelete = (StringObject *)tokens->at ( 1 );
	StringObject *distance = (StringObject *)tokens->at ( 2 );

	if(objectToDelete && distance)
	{
		deleteObjects(objectToDelete, player, distance);
	}
	else
	{
		roomMgr->sendPlayerText ( player, "|c2|Info> Usage: /delete <objectName> [*, closest]\n");
	}
}

void cmdListObjects ( LinkedList *tokens, char *str, RMPlayer *player )
{
	LinkedElement *element = player->room->objects.head();
	roomMgr->sendPlayerText ( player, "|c2|Info> This room contains the following objects: \n");
	int count = 1;

	while ( element )
	{
		WorldObject *target = (WorldObject *)element->ptr();
		roomMgr->sendPlayerText ( player, "|c86|  %d) %s\n", count, target->name);
		element = element->next();
		count++;
	}
}

void cmdExp ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target && target->character ) {
			StringObject *exp = (StringObject *)tokens->at ( 2 );

			if ( !exp ) {
				roomMgr->sendPlayerText ( player, "|c60|Info> You must specify how many experience points to give.\n" );
				return;
			}

			BCharacter *bchar = (BCharacter *)target->character->getBase ( _BCHARACTER );

			if ( bchar ) {
				PackedMsg response;

				response.putLong ( target->character->servID );
				response.putLong ( target->character->room->number );

				bchar->gainExperience ( atoi ( exp->data ), &response );
				response.putByte ( _MOVIE_END );

				gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_IMPCMD, "exp %s %d", target->getName(), atoi ( exp->data ) );

				if ( player != target )
					gDataMgr->logPermanent ( target->getLogin(), target->getName(), _DMGR_PLOG_IMPCMD, "%s exp %d", player->getName(), atoi ( exp->data ) );

				roomMgr->sendPlayerText ( player, "|c71|Info> %s has been given %d experience points.\n", target->getName(), atoi ( exp->data ) );

				roomMgr->sendToRoom ( _IPC_PLAYER_MOVIE_DATA, response.data(), response.size(), target->room );
			} else {
				roomMgr->sendPlayerText ( player, "|c60|Info> You can't give experience to that.\n" );
			}
		} else {
			roomMgr->sendPlayerText ( player, "|c60|You can't find '%s' to give experience to.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|You must specify the person you want to give experience to.\n" );
	}
}

void cmdDiskID ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target ) {
			roomMgr->sendPlayerText ( player, "|c71|Info> Disk serial number for %s is 0x%x\n", target->getName(), target->serial );
		} else {
			roomMgr->sendPlayerText ( player, "|c60|You can't find '%s' to query.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|You must specify the person you want to query.\n" );
	}
}

void cmdClass ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		RMPlayer *target = findPlayer ( name->data, player );

		if ( target && target->character ) {
			StringObject *classID = (StringObject *)tokens->at ( 2 );

			if ( classID ) {
				char *theClass = classID->data;

				if ( roomMgr->findClass ( theClass ) ) {
					target->character->setClassID ( theClass );
					target->character->setSuper ( theClass );

					roomMgr->sendPlayerText ( player, "|c71|Info> %s's class is now %s.\n", target->getName(), target->character->classID );
				} else {
					roomMgr->sendPlayerText ( player, "|c60|Info> %s is not a valid class.\n", theClass );
				}
			} else {
				roomMgr->sendPlayerText ( player, "|c71|Info> %s's class is %s.\n", target->getName(), target->character->classID );
			}
		} else {
			roomMgr->sendPlayerText ( player, "|c60|You can't find '%s' to query.\n", name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|You must specify the person you want to query.\n" );
	}
}

void cmdIgnore( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;

        char* pName = player->getName();

        LinkedElement *pElement = tokens->head()->next();

	if ( pElement ) {
	        while ( pElement ) {
        	        StringObject *name = (StringObject *)pElement->ptr();
        	        pElement = pElement->next();
	
                	if ( name ) {
				if ( player->isIgnoring( name->data ) ) {
					roomMgr->sendPlayerText ( player, "|c71|Info> You are already ignoring %s\n", name->data );
	                        } else if ( strcasecmp( pName, name->data ) ) {
                        	        gDataMgr->CheckFoe( player, name->data );
        	                } else {
                	                roomMgr->sendPlayerText ( player, "|c60|Info> You can't ignore yourself... that would be silly.\n" );
                        	}
	                }
		}
        } else {
		roomMgr->sendPlayerText ( player, "|c60|Info> You must specify the person you want to ignore to.\n" );
	}
}

void cmdIgnorePermenant( LinkedList *tokens, char *str, RMPlayer *player ) {
        if ( !player || !player->character )
                return;

        char* pName = player->getName();

        LinkedElement *pElement = tokens->head()->next();

        if ( pElement ) {
                while ( pElement ) {
                        StringObject *name = (StringObject *)pElement->ptr();
                        pElement = pElement->next();

			if ( name ) {
                                if ( strcasecmp( pName, name->data ) ) {
                                        gDataMgr->AddFoe( player, name->data );
                                } else {
                                        roomMgr->sendPlayerText ( player, "|c60|Info> You can't ignore yourself... that would be silly.\n" );
                                }
                        }
                }
        } else {
                roomMgr->sendPlayerText ( player, "|c60|Info> You must specify the person you want to ignore to.\n" );
        }
}

void cmdListen ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );

	if ( name ) {
		if ( !player->isIgnoring( name->data ) ) {
			roomMgr->sendPlayerText ( player, "|c71|Info> You are not ignoring %s.\n", name->data );
		} else {
			player->unignore( name->data );
		}
	} else {
		roomMgr->sendPlayerText ( player, "|c60|Info> You must specify the person you want to listen to.\n" );
	}
}

void cmdBusy ( LinkedList *tokens, char *str, RMPlayer *player ) {
	if ( !player || !player->character )
		return;

	WorldObject *obj = player->player;

	if ( obj->physicalState & _STATE_BUSY ) {
		roomMgr->sendPlayerText ( player, "|c17|Info> You are already marked as busy.\n" );
		return;
	}

	obj->physicalState |= _STATE_BUSY;
	roomMgr->sendPlayerText ( player, "|c71|Info> You are now marked as busy.\n" );

	// tell this player's friends that he is now busy...
	CFriend *pFriend = player->GetFriendEntry();

	if ( pFriend )
		pFriend->SendBusy();
}

void cmdUnBusy ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	WorldObject *obj = player->player;

	if ( !(obj->physicalState & _STATE_BUSY) ) {
		roomMgr->sendPlayerText ( player, "Info> You are already not busy.\n" );
		return;
	}

	obj->physicalState &= ~_STATE_BUSY;
	roomMgr->sendPlayerText ( player, "Info> You are no longer marked as busy.\n" );

	// tell this player's friends that he is now unbusy...
	CFriend *pFriend = player->GetFriendEntry();

	if ( pFriend )
		pFriend->SendUnbusy();
}


void cmdRoomEmote ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !str || !player || !player->character )
		return;
	
	int len = strlen ( str );
	
	if ( len < 1 || len > 320 ) {
		return;
	}
	
  	if ( strchr ( str, '|' ) ) {
  		roomMgr->sendPlayerText ( player, "|c60|Info> No embedded color changes allowed.\n" );
  		return;
  	}
   
  	char *cmd = ((StringObject *)tokens->at ( 0 ))->data;

	player->sendRoomChat ( "|c2|%s\n", str + strlen ( cmd ) );
}

void cmdRandom ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	int max = 100;
	StringObject *maxS = (StringObject *)tokens->at ( 1 );

	if(maxS){

		int numberToRoll = atoi ( ((StringObject *)tokens->at ( 1 ))->data );

		if(numberToRoll){
			if(numberToRoll > 32000){
				roomMgr->sendPlayerText ( player, "|c60|Info> You cannot roll higher than 32000.\n", numberToRoll);
				return;
			}
			else if (numberToRoll < 1)
			{
				roomMgr->sendPlayerText ( player, "|c60|Info> You cannot have a roll with a max lower than 1.\n" );
				return;
			}
			else{
				max = numberToRoll;	
			}
		}
	}

	int roll = random ( 0, max );

	player->sendRoomChat ( "|c2|Info> %s rolls a %d out of %d.\n", player->getName(), roll, max );
}

void cmdExpBoost( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	//We can only set if we are an implementor.
	if(player->checkAccess ( _ACCESS_MODERATOR )){
		StringObject *expB = ((StringObject *)tokens->at ( 1 ));

		//If we actually passed in a multiplier
		if(expB){
			double xp = atof ( expB->data );
			int maxExpBoost = 10;

			if(IsThisATestServer())
			{
				maxExpBoost = 10000;
			}

			//Prevent from going higher than 10 for now.
			if (xp > maxExpBoost){
				roomMgr->sendPlayerText ( player, "|60|Info> You cannot set the exp boost higher than %d.\n", maxExpBoost );
				return;
			}

			//No Negative experience, but you can set to 0.
			if(xp < 0){
				roomMgr->sendPlayerText ( player, "|c60|Info> You cannot have a negative exp boost.\n" );
				return;
			}

			//If we go up, we rejoice. If we go down (even if higher than 1), we get sad.
			if(xp > gExpBoost)
			{
				roomMgr->sendSystemMsg ( "Rejoice and clear your schedule! Experience has been raised by %.2f times normal.", xp );
				roomMgr->sendListChat ( player, roomMgr->players(), "|c67|Info> Rejoice and clear your schedule! Experience has been raised by %.2f times normal.", xp );
			}
			else if (xp < gExpBoost)
			{
				roomMgr->sendSystemMsg ( "Life and existance as we know it may be at an end. Experience has been lowered to %.2f times normal.", xp );
				roomMgr->sendListChat ( player, roomMgr->players(), "|c67|Info> Life and existance as we know it may be at an end. Experience has been lowered to %.2f times normal.", xp );
			}

			//Set Global boost variable if all validates properly.
			gExpBoost = xp;
			return;
		}
	}
	
	roomMgr->sendPlayerText ( player, "|c71|Info> Current Exp Modification is %.2f times normal.", gExpBoost );
}

void cmdLootBoost( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	//We can only set if we are an implementor.
	if(player->checkAccess ( _ACCESS_IMPLEMENTOR )){
		StringObject *lootB = ((StringObject *)tokens->at ( 1 ));

		//If we actually passed in a multiplier
		if(lootB){
			int loot = atoi ( lootB->data );

			//Prevent from going higher than 10 for now.
			if (loot > 2){
				roomMgr->sendPlayerText ( player, "|c60|Info> You cannot set the loot boost higher than 2.\n" );
				return;
			}

			//No Negative loot, but you can set to 0.
			if(loot < 0){
				roomMgr->sendPlayerText ( player, "|c60|Info> You cannot have a negative loot boost.\n" );
				return;
			}

			//If we go up, we rejoice. If we go down (even if higher than 1), we get sad.
			if(loot > gLootBoost)
			{
				roomMgr->sendSystemMsg ( "Rejoice and clear your schedule! Loot has been raised by %d times normal.", loot );
				roomMgr->sendListChat ( player, roomMgr->players(), "|c67|Info> Rejoice and clear your schedule! Loot has been raised by %d times normal.", loot );
			}
			else if (loot < gLootBoost)
			{
				roomMgr->sendSystemMsg ( "Life and existance as we know it may be at an end. Loot has been lowered to %d times normal.", loot );
				roomMgr->sendListChat ( player, roomMgr->players(), "|c67|Info> Life and existance as we know it may be at an end. Loot has been lowered to %d times normal.", loot );
			}

			//Set Global boost variable if all validates properly.
			gLootBoost = loot;
			return;
		}
	}
	
	roomMgr->sendPlayerText ( player, "|c71|Info> Current Loot Modification is %d times normal.", gLootBoost );
}

void cmdUnghost ( LinkedList *tokens, char *str, RMPlayer *player )
{
	if ( !player || !player->character )
		return;

	StringObject *name = (StringObject *)tokens->at ( 1 );
	StringObject *password = (StringObject *)tokens->at ( 2 );

	//If we entered two parameters
	if ( name && password ) {
		RMPlayer *target = findPlayer ( name->data, player );

		//If we find a target, and have the right password
		if ( target ) {
			if(strcmp(target->getPassword(), password->data) == 0) {
				gDataMgr->logPermanent ( player->getLogin(), player->getName(), _DMGR_PLOG_GMCMD, "unghosted out %s", target->getName() );
				gDataMgr->logPermanent ( target->getLogin(), target->getName(), _DMGR_PLOG_GMCMD, "unghosted out by %s", player->getName() );
				target->forceLogout();

				roomMgr->sendPlayerText ( player, "|c71|Info> You have successfully unghosted %s.\n", target->getName() );
				roomMgr->sendPlayerText ( target, "|c71|Info> %s has unghosted you.\n", player->getName() );
			}
			else
			{
				roomMgr->sendPlayerText ( player, "|c60|Info> You are not authorized to unghost %s.\n", target->getName() );
			}
		} 
		else 
		{
			roomMgr->sendPlayerChat ( player, player, "|c60|Info> You can not find %s to unghost.\n", name->data );
		}
	} 
	else 
	{
		roomMgr->sendPlayerText ( player, "|c71|Use: /unghost toonname acctpassword.\n"  );
	}
}

// Proc test 
void cmdAddForwardProcToEquipedWeapon(LinkedList *tokens, char *str, RMPlayer *player)
{
    if(tokens->size() < 2)
    {
        roomMgr->sendPlayerText ( player, "|c71|Info> Use: /zProc spellID\n" );
        return;
    }

    if ( !player || !player->character )
        return;

    StringObject *name = (StringObject *)tokens->at ( 1 );

    int procid = strtol(name->data, (char **)NULL, 10);

    int valid = isValidForwardProcID(procid);

    roomMgr->sendPlayerText ( player, "|c71|Info> ForwardProcID %d.\n", valid );

    if(valid != procid)
    {
        roomMgr->sendPlayerText ( player, "|c60|Info> Invalid ForwardProcID %d.\n", procid );
        return;
    }
   // Weapon *weapon = player->character->hands;
    BWeapon *bweapon = (BWeapon *)player->character->curWeapon->getBase(_BWEAPON);

    if(bweapon)
    {
        spell_info *spell = &gSpellTable[procid];

        if(spell)
        {
            if(spell->verbal == NULL)
                return;

            bweapon->spellProcID = procid;

            roomMgr->sendPlayerChat ( player, player, "|c71|Info> You stare in wonder as a disembodied hand etches the words %s into your %s.\n", spell->verbal, bweapon->self->getName() );
        }
    }
} 

ParseCommand parseCommands[] = {
	{"/house",			cmdHouse,					_ACCESS_IMPLEMENTOR,	_SENSITIVE_CMD},
	{"/kill",			cmdKill,					_ACCESS_IMPLEMENTOR,	_SENSITIVE_CMD}, 
	{"/crash",			cmdCrash,					_ACCESS_IMPLEMENTOR,	_SENSITIVE_CMD},
	{"/diskID",			cmdDiskID,					_ACCESS_IMPLEMENTOR,	_SENSITIVE_CMD},
	{"/status",			cmdStatus,					_ACCESS_IMPLEMENTOR,	_INSENSITIVE_CMD},
	{"/lag",			cmdLag,						_ACCESS_IMPLEMENTOR,	_INSENSITIVE_CMD},
	{"/msgs",			cmdMsgs,					_ACCESS_IMPLEMENTOR,	_INSENSITIVE_CMD},
	{"/dump",			cmdDump,					_ACCESS_IMPLEMENTOR,	_INSENSITIVE_CMD},
	{"/loadingHouses",	cmdLoadingHouses,			_ACCESS_IMPLEMENTOR,	_INSENSITIVE_CMD},
	{"/loadallhouses",	cmdLoadAllHouses,			_ACCESS_IMPLEMENTOR,	_INSENSITIVE_CMD},
	{"/rubber",			cmdRubber,					_ACCESS_IMPLEMENTOR,	_SENSITIVE_CMD},
	{"/freeze",			cmdFreeze,					_ACCESS_IMPLEMENTOR,	_SENSITIVE_CMD},
	{"/thaw",			cmdThaw,					_ACCESS_IMPLEMENTOR,	_SENSITIVE_CMD},
	{"/copyroom",		cmdCopyRoom,				_ACCESS_IMPLEMENTOR,	_SENSITIVE_CMD},
	{"/startroom",		cmdStartRoom,				_ACCESS_IMPLEMENTOR,	_SENSITIVE_CMD},
	{"/spells",			cmdSpellList,				_ACCESS_IMPLEMENTOR,	_INSENSITIVE_CMD},
	{"/testmail",		cmdTestMail,				_ACCESS_IMPLEMENTOR,	_INSENSITIVE_CMD},
	{"/align",			cmdAlignment,				_ACCESS_IMPLEMENTOR,	_INSENSITIVE_CMD},
	{"/delete",			cmdDelete,					_ACCESS_IMPLEMENTOR,	_INSENSITIVE_CMD},
	{"/listobjects",	cmdListObjects,				_ACCESS_IMPLEMENTOR,	_INSENSITIVE_CMD},

	{"/validatezone",	cmdValidateZone,			_ACCESS_IMPLEMENTOR,	_INSENSITIVE_CMD},
	{"/validaterooms",	cmdValidateRooms,			_ACCESS_IMPLEMENTOR,	_INSENSITIVE_CMD},
	{"/combatgrid",		cmdDisplayCombatGrid,		_ACCESS_IMPLEMENTOR,	_SENSITIVE_CMD},
	{"/credit",		cmdCredit,			_ACCESS_IMPLEMENTOR,	_SENSITIVE_CMD},
	{"/mmOff",		cmdMMOff,			_ACCESS_IMPLEMENTOR,	_SENSITIVE_CMD},
	{"/mmoff",		cmdMMOff,			_ACCESS_IMPLEMENTOR,	_SENSITIVE_CMD},

	{"/gold",			cmdGold,					_ACCESS_IMPLEMENTOR,		_INSENSITIVE_CMD},
	{"/conjure",		cmdConjure,					_ACCESS_IMPLEMENTOR,		_INSENSITIVE_CMD},

	{"/shutdown",		cmdShutdown,				_ACCESS_IMPLEMENTOR,		_INSENSITIVE_CMD},
	{"/loginmsg",		cmdLoginMsg,				_ACCESS_SHUTDOWN | _ACCESS_PUBLICRELATIONS,		_SENSITIVE_CMD},
	{"/startup",		cmdStartup,					_ACCESS_SHUTDOWN,		_SENSITIVE_CMD},

	{"/disable",		cmdDisable,					_ACCESS_DISABLE,		_SENSITIVE_CMD},

	{"/heal",			cmdHeal,					_ACCESS_HEAL,			_INSENSITIVE_CMD},

	{"/class",			cmdClass,					_ACCESS_TOON_MODIFY,	_SENSITIVE_CMD},
	{"/exp",			cmdExp,						_ACCESS_TOON_MODIFY,	_SENSITIVE_CMD},
	{"/nohead",			cmdNoHead,					_ACCESS_TOON_MODIFY,	_SENSITIVE_CMD},
	{"/monsterhead",	cmdMonsterHead,				_ACCESS_TOON_MODIFY,	_SENSITIVE_CMD},
	{"/learned",		cmdLearned,					_ACCESS_TOON_MODIFY,	_SENSITIVE_CMD},
	{"/attrs",			cmdSetAttrs,				_ACCESS_TOON_MODIFY,	_INSENSITIVE_CMD},

	{"/god",			cmdGodWindow,				_ACCESS_PROPHET,   		_GOSSIP_READ_CMD},

	{"/quake",			cmdWorldQuake,				_ACCESS_PROPHET,		_SENSITIVE_CMD}, 
	{"/zonequake",		cmdZoneQuake,				_ACCESS_PROPHET,		_SENSITIVE_CMD}, 
	{"/roomquake",		cmdRoomQuake,				_ACCESS_PROPHET,		_SENSITIVE_CMD}, 
	{"/roomemote",		cmdRoomEmote,				_ACCESS_PROPHET,		_SENSITIVE_CMD}, 

	{"/9",				cmdHostGossip,				_ACCESS_PROPHET | _ACCESS_EVENT | _ACCESS_GUIDE,		_SENSITIVE_CMD},
	{"/oe",				cmdOpenEvent,				_ACCESS_EVENT | _ACCESS_PUBLICRELATIONS,		_SENSITIVE_CMD},
	{"/closeEvent",		cmdEventClose,				_ACCESS_NORMAL,   		_GOSSIP_READ_CMD},
	{"/cE",				cmdEventClose,				_ACCESS_NORMAL,   		_GOSSIP_READ_CMD},
	{"/ce",				cmdEventClose,				_ACCESS_NORMAL,   		_GOSSIP_READ_CMD},
	{"/infoEvent",		cmdEventInformation,		_ACCESS_NORMAL,   		_GOSSIP_READ_CMD},
	{"/cERO",			cmdEventReadOnly,			_ACCESS_EVENT | _ACCESS_PUBLICRELATIONS,		_SENSITIVE_CMD},
	{"/cero",			cmdEventReadOnly,			_ACCESS_EVENT | _ACCESS_PUBLICRELATIONS,		_SENSITIVE_CMD},
	{"/candy",			cmdCandy,					_ACCESS_EVENT,		_SENSITIVE_CMD}, 
	{"/pumpkin",		cmdPumpkin,					_ACCESS_EVENT,		_SENSITIVE_CMD}, 
	{"/place",			cmdPlace,					_ACCESS_EVENT,		_SENSITIVE_CMD},
	{"/snow",			cmdSnow,					_ACCESS_EVENT,		_SENSITIVE_CMD},  
	{"/mark",			cmdMark,					_ACCESS_EVENT,		_SENSITIVE_CMD}, 
	{"/copper",			cmdCopper,					_ACCESS_EVENT,		_SENSITIVE_CMD},

	{"/suspend",		cmdSuspend,					_ACCESS_MODERATOR,		_SENSITIVE_CMD},
	{"/a",				cmdAbout,					_ACCESS_MODERATOR,		_SENSITIVE_CMD},
	{"/about",			cmdAbout,					_ACCESS_MODERATOR,		_SENSITIVE_CMD},
	{"/info",			cmdInfo,					_ACCESS_MODERATOR,		_GOSSIP_READ_CMD},
	{"/logout",			cmdLogout,					_ACCESS_MODERATOR,		_SENSITIVE_CMD}, 
	{"/revoke",			cmdRevoke,					_ACCESS_MODERATOR,		_SENSITIVE_CMD},
	{"/restore",		cmdRestore,					_ACCESS_MODERATOR,		_SENSITIVE_CMD},
	{"/revokemod",		cmdRevokeMod,				_ACCESS_MODERATOR,		_GOSSIP_WRITE_CMD},
	{"/warn",			cmdWarn,					_ACCESS_MODERATOR,		_SENSITIVE_CMD},
	{"/gag",			cmdGag,						_ACCESS_MODERATOR,		_SENSITIVE_CMD},
	{"/nogag",			cmdNoGag,					_ACCESS_MODERATOR,		_SENSITIVE_CMD},
	{"/showReport",		cmdShowReport,				_ACCESS_MODERATOR,		_SENSITIVE_CMD},
	{"/showreport",		cmdShowReport,				_ACCESS_MODERATOR,		_SENSITIVE_CMD},
	{"/sr",				cmdShowReport,				_ACCESS_MODERATOR,		_SENSITIVE_CMD},

	{"/aboutrm",		cmdAboutRoom,				_ACCESS_TELEPORT | _ACCESS_PROPHET | _ACCESS_EVENT,		_SENSITIVE_CMD},
	{"/empty",			cmdEmpty,					_ACCESS_TELEPORT,										_SENSITIVE_CMD},
	{"/tph",			cmdTeleportHouse,			_ACCESS_TELEPORT,										_SENSITIVE_CMD},
	{"/teleport",		cmdTeleport,				_ACCESS_TELEPORT,		_SENSITIVE_CMD},
	{"/bring",			cmdBring,					_ACCESS_TELEPORT,		_SENSITIVE_CMD},
	{"/goto",			cmdGoto,					_ACCESS_TELEPORT,		_SENSITIVE_CMD},

	{"/rdonly",			cmdReadOnly,				_ACCESS_GUIDE,			_SENSITIVE_CMD},
	{"/gm",				cmdGodGossip,				_ACCESS_GUIDE,			_GOSSIP_READ_CMD},
	{"/m",				cmdGodGossip,				_ACCESS_GUIDE,			_GOSSIP_READ_CMD},
	{"/guide",			cmdGuideGossip,				_ACCESS_GUIDE,			_GOSSIP_READ_CMD},
	{"/d",				cmdGuideGossip,				_ACCESS_GUIDE,			_GOSSIP_READ_CMD},

	{"/hideout",		cmdHideout,					_ACCESS_BASIC_TELEPORT | _ACCESS_EVENT | _ACCESS_PROPHET,	_SENSITIVE_CMD},
	{"/hide",			cmdHideout,					_ACCESS_BASIC_TELEPORT | _ACCESS_EVENT | _ACCESS_PROPHET,	_SENSITIVE_CMD},
	{"/oasis",			cmdOasis,					_ACCESS_BASIC_TELEPORT | _ACCESS_EVENT | _ACCESS_PROPHET,	_SENSITIVE_CMD},
	{"/arena",			cmdAmbush,					_ACCESS_BASIC_TELEPORT | _ACCESS_EVENT | _ACCESS_PROPHET,	_SENSITIVE_CMD},



	{"/smile",			cmdSmile,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/smi",			cmdSmile,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/grin",			cmdGrin,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/grimace",		cmdGrimace,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/comfort",		cmdComfort,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/tickle",			cmdTickle,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/pat",			cmdPat,						_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/eye",			cmdEye,						_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/stare",			cmdStare,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/worship",		cmdWorship,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/hug",			cmdHug,						_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/dismiss",		cmdDismiss,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/agree",			cmdAgree,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/laugh",			cmdLaugh,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/wink",			cmdWink,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/kiss",			cmdKiss,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/cry",			cmdCry,						_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/frown",			cmdFrown,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/fro",			cmdFrown,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/nod",			cmdNod,						_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/bow",			cmdBow,						_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/8",				cmdEvent,					_ACCESS_NORMAL,			_SENSITIVE_CMD},
	{"/who",			cmdWho,						_ACCESS_NORMAL,			_SENSITIVE_CMD},
	{"/whoEvent",		cmdEventWho,				_ACCESS_NORMAL,			_SENSITIVE_CMD},
	{"/gossip",			cmdGossip,					_ACCESS_NORMAL,			_GOSSIP_READ_CMD},
	{"/complain",		cmdComplain,				_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/report",			cmdReport,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/gos",			cmdGossip,					_ACCESS_NORMAL,			_GOSSIP_READ_CMD},
	{"/g",				cmdGossip,					_ACCESS_NORMAL,			_GOSSIP_READ_CMD},
	{"/emote",			cmdEmote,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/emo",			cmdEmote,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/e",				cmdEmote,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/tell",			cmdTell,					_ACCESS_NORMAL,			_SENSITIVE_CMD},
	{"/tel",			cmdTell,					_ACCESS_NORMAL,			_SENSITIVE_CMD},
	{"/t",				cmdTell,					_ACCESS_NORMAL,			_SENSITIVE_CMD},
	{"/ignore",			cmdIgnore,					_ACCESS_NORMAL,			_SENSITIVE_CMD},
	{"/pignore",		cmdIgnorePermenant,			_ACCESS_NORMAL,			_SENSITIVE_CMD},
	{"/mute",			cmdIgnore,					_ACCESS_NORMAL,			_SENSITIVE_CMD},
	{"/listen",			cmdListen,					_ACCESS_NORMAL,			_SENSITIVE_CMD},
	{"/unmute",			cmdListen,					_ACCESS_NORMAL,			_SENSITIVE_CMD},
	{"/unignore",		cmdListen,					_ACCESS_NORMAL,			_SENSITIVE_CMD},
	{"/nogossip",		cmdGossipOff,				_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/yesgossip",		cmdGossipOn,				_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/gossipoff",		cmdGossipOff,				_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/gossipon",		cmdGossipOn,				_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/crime",			cmdCrime,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/bounty",			cmdBounty,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/evict",			cmdEvict,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/room",			cmdRoom,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/gms",			cmdGods,					_ACCESS_NORMAL,			_SENSITIVE_CMD},
	{"/busy",			cmdBusy,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/unbusy",			cmdUnBusy,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/gt",				cmdGroupTell,				_ACCESS_NORMAL,			_GOSSIP_READ_CMD},
	{"/gtell",			cmdGroupTell,				_ACCESS_NORMAL,			_GOSSIP_READ_CMD},
	{"/join",			cmdJoin,					_ACCESS_NORMAL,			_GOSSIP_WRITE_CMD},
	{"/makemod",		cmdMakeMod,					_ACCESS_NORMAL,			_GOSSIP_WRITE_CMD},
	{"/leave",			cmdGossipOff,				_ACCESS_NORMAL,			_GOSSIP_WRITE_CMD},
	{"/invite",			cmdInvite,					_ACCESS_NORMAL,			_GOSSIP_WRITE_CMD},
	{"/kick",			cmdKick,					_ACCESS_NORMAL,			_GOSSIP_WRITE_CMD},
	{"/private",		cmdPrivate,					_ACCESS_NORMAL,			_GOSSIP_WRITE_CMD},
	{"/public",			cmdPublic,					_ACCESS_NORMAL,			_GOSSIP_WRITE_CMD},
	{"/topic",			cmdTopic,					_ACCESS_NORMAL,			_GOSSIP_WRITE_CMD},
	{"/name",			cmdName,					_ACCESS_NORMAL,			_GOSSIP_WRITE_CMD},
	{"/channel",		cmdGetChannel,				_ACCESS_NORMAL,			_GOSSIP_WRITE_CMD},
	{"/list",			cmdChannels,				_ACCESS_NORMAL,			_GOSSIP_READ_CMD},
	{"/channels",		cmdChannels,				_ACCESS_NORMAL,			_GOSSIP_READ_CMD},
	{"/close",			cmdCloseGroup,				_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/open",			cmdOpenGroup,				_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/weight",			cmdWeight,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/stamina",		cmdStamina,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/account",		cmdAccountInfo,				_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/nograce",		cmdNoGrace,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/yesgrace",		cmdYesGrace,				_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/nocombat",		cmdNoCombat,				_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/yescombat",		cmdYesCombat,				_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/give",			cmdAutoGive,				_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/addfriend",		cmdAddFriend,				_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/delfriend",		cmdDelFriend,				_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/state",			cmdState,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/chMem",			cmdChannelMembers,			_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/chBan",			cmdChannelBannedMembers,	_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/random",			cmdRandom,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/r",				cmdRandom,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/roll",			cmdRandom,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/expboost",		cmdExpBoost,				_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/lootboost",		cmdLootBoost,				_ACCESS_NORMAL,			_INSENSITIVE_CMD},
	{"/unghost",		cmdUnghost,					_ACCESS_NORMAL,			_INSENSITIVE_CMD},


	// Custom Commands for MORTALS - Only to be used by Implementor's on mortal accounts.
	{"/zAlign",		cmdAlignment,						_ACCESS_NORMAL,		_INSENSITIVE_CMD},
	{"/zDel",		cmdDelete,							_ACCESS_NORMAL,		_INSENSITIVE_CMD},
	{"/zList",		cmdListObjects,						_ACCESS_NORMAL,		_INSENSITIVE_CMD},
	{"/zOasis",		cmdOasis,							_ACCESS_NORMAL,		_SENSITIVE_CMD},
	{"/zArena",		cmdAmbush,							_ACCESS_NORMAL,		_SENSITIVE_CMD},
	{"/zBr",		cmdBring,							_ACCESS_NORMAL,		_SENSITIVE_CMD},
	{"/zGo",		cmdGoto,							_ACCESS_NORMAL,		_SENSITIVE_CMD},
	{"/zMoney",		cmdGold,							_ACCESS_NORMAL,		_INSENSITIVE_CMD},
	{"/zMake",		cmdConjure,							_ACCESS_NORMAL,		_INSENSITIVE_CMD},
    {"/zProc",		cmdAddForwardProcToEquipedWeapon,	_ACCESS_NORMAL,		_INSENSITIVE_CMD},
	{"/zTel",		cmdTeleport,						_ACCESS_NORMAL,		_SENSITIVE_CMD},
	{NULL, NULL, 0}
};

ParseCommand *matchCommand ( char *str )
{
	int index = 0;

	while ( parseCommands[index].name != NULL ) {
		if ( !strcmp ( parseCommands[index].name, str ) )
			return &parseCommands[index];

		index++;
	}

	return NULL;
}

int parseCommand ( char *str, RMPlayer *player )
{
	int retVal = 0;

	if ( !str )
		return retVal;

	if ( player && player->isNPC ) {
		logInfo ( _LOG_ALWAYS, "Invalid Command: Redirected from NPC!" );
		return retVal;
	}

	LinkedList *tokens = buildTokenList ( str );

	if ( tokens->size() ) {
		ParseCommand *command = matchCommand ( ((StringObject *)tokens->at ( 0 ))->data );

		if ( command && ( !command->type || (command->type & player->rights) ) ) {
			command->routine ( tokens, str, player );
			retVal = 1;
		}
	}

	delete tokens;

	return retVal;
}
