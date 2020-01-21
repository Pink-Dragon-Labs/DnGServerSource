//
// mlpumpkindropper.cpp
//
// Control logic for pumpkin droppers.
//
// Author: Michael Nicolella
//

#include "roommgr.hpp"

void dropPumpkin( RMRoom* room, int x, int y );

PumpkinDropper::PumpkinDropper()
{
	lastPlayers[0] = static_cast<RMPlayer* >( 0 );
	lastPlayers[1] = static_cast<RMPlayer* >( 0 );
	lastPlayers[2] = static_cast<RMPlayer* >( 0 );
}

PumpkinDropper::~PumpkinDropper()
{
}

void PumpkinDropper::init( void )
{
	new MWActionChill ( 0, this );
}

int PumpkinDropper::normalLogic ( void )
{
	static const unsigned char MaxPumpkins = 2;

	if( !character || !room ) return 100;

	if( character->x != -200 || !character->y != -200 ) {
		//wander off screen
		gotoXY( -200, -200 );
	}

	//make sure pumpkins exist and are loaded
	WorldObject* pumpkin = roomMgr->findClass( "LargePumpkin" );
	if( !pumpkin ) { return 100; }

	//we should not drop any pumpkins if there are 5 or more on the screen already
	LinkedElement* objElement = room->objects.head();

	unsigned short pumpkinCount = 0;

	while( objElement ) {
		WorldObject* obj = static_cast<WorldObject* >( objElement->ptr() );
		objElement = objElement->next();

		if( obj->classNumber == pumpkin->classNumber )
			pumpkinCount++;

		if( pumpkinCount >= MaxPumpkins ) return 100;
	}

	//lets consider dropping a pumpkin
	unsigned short roll = rand100();

	if( roll > 90 ) {
		//drop a pumpkin near a player

		if( room->size() <= 5 ) {
			//there are very few players - drop it anywhere
			dropPumpkin( room, random( 0, 630 ), random( 130, 300 ) );
			return 20;
		} else {
			//lets find a player that we haven't dropped a pumpkin near recently

			RMPlayer* dropNear = static_cast<RMPlayer* >( 0 );
			unsigned short maxIndex = room->size() - 1; //subtract 1 to make maxIndex the highest index, not the size

			unsigned char tries = 6;

			while( --tries && !dropNear ) {
				//get a random player
				unsigned short randIndex = random( 0, maxIndex );

				dropNear = static_cast<RMPlayer* >( room->at( randIndex ) );

				//check it against our list
				if( lastPlayers[0] == dropNear || lastPlayers[1] == dropNear || lastPlayers[2] == dropNear ) {
					//reset to zero and loop again
					dropNear = static_cast<RMPlayer* >( 0 );
				} else {
					//we have a player that hasn't had a pumpkin dropped next to them in awhile.. lets treat them!
					if( dropNear->isNPC )
						dropNear = static_cast<RMPlayer* >( 0 ); //disregard them if they're an NPC
					else break;
				}
			}

			if( dropNear && dropNear->character ) {
				//drop the pumpkin near this player
				dropPumpkin( room, dropNear->character->x + random( -10, +10 ), dropNear->character->y + random( -5, +5 ) );

				lastPlayers[0] = lastPlayers[1];
				lastPlayers[1] = lastPlayers[2];
				lastPlayers[2] = dropNear;
				return 20;
			}
		}
	}
	else if( roll > 85 ) {
		//drop a pumpkin anywhere
		dropPumpkin( room, random( 0, 630 ), random( 130, 300 ) );
		return 20;
	}

	return 30;
}

void dropPumpkin( RMRoom* room, int x, int y ) {
	if( !room ) return;

	unsigned short roll = rand100();
	if( roll > 98 ) {
		//drop rare pumpkin
		room->addObject( "ShockJackolantern", x, y );
	}
	else {
		room->addObject( "LargePumpkin", x, y );
	}
}
