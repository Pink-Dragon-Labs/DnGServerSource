//
// callbacks
//
// This file contains lots of callback functions.
//
// author: Stephen Nichols
//

#include "roommgr.hpp"
#include "callbacks.hpp"
#include "globals.hpp"
#include "../global/datamgrdefs.hpp"

// this function is called to teleport a WorldObject to a named house
void teleportHouse ( WorldObject *obj, char *name )
{
	// see if the house is already in memory
	Building *house = findHouse ( name );

	if ( house ) {
		// default to living room
		int roomIdx = 0;

		// if not owner, default to front yard
		if ( strcasecmp ( obj->getName(), house->_owner ) )
			roomIdx = 2;

		RMRoom *room = (RMRoom *)house->rooms.at ( roomIdx );
		int roomNumber = room? room->number : random ( 5000, 5089 );
		obj->teleport ( roomNumber, NULL );

		// Give permenant house if necessary.
		if ( roomIdx == 0 && house->homeTown == 1 && obj->player ) {
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
		// there is no house loaded, start the loading process
		loadHouse ( name, cbTeleportHouse, obj->servID );
	}
}

// this callback completes the teleport to house process
void cbTeleportHouse ( int result, int context, int houseID, int accountID, char *name, int size, char *data )
{
	Building *house = NULL;

	// find the object we're teleporting
	WorldObject *obj = roomMgr->findObject ( context );

	if ( result == _DATAMGR_OKAY ) {
		// process the loaded house
		house = processHouseData ( name, data, size, houseID, accountID );

		if ( house ) {
			// default to living room
			int roomIdx = 0;

			// if not owner, default to front yard
			if ( strcasecmp ( obj? obj->getName() : "", house->_owner ) )
				roomIdx = 2;

			RMRoom *room = (RMRoom *)house->rooms.at ( roomIdx );
			int roomNumber = room? room->number : random ( 5000, 5089 );

			if ( obj ) {
				obj->teleport ( roomNumber, NULL );

				// Give permenant house if necessary.
				if ( roomIdx == 0 && house->homeTown == 1 && obj->player ) {
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
			}
		}
	} else {
		if ( gLoadingHouses.head() ) {
			// toss the first loading house string
			StringObject *obj = (StringObject *)gLoadingHouses.head()->ptr();

			if ( obj ) {
				delete obj;
				gLoadingHouses.delElement ( gLoadingHouses.head() );
			}
		}
	}

	// if there is no house, tell the object
	if ( (house == NULL) && obj ) {
		if ( obj->player ) {
			roomMgr->sendSystemMsg ( "House Unavailable", obj->player, "Sorry, but the house that you requested could not be located." );
		}
	}
}

