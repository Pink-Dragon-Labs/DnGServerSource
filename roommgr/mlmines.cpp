//
// mlmines.cpp
//
// Control logic for mine NPCs.
//
// author: Michael Nicolella
//

#include "roommgr.hpp"

// ClothBolt
// LeatherBolt
// TrollHideBolt
//{ "WoodBlock", "IronBar", "SteelBar", "TemperedSteelBar", "MythrilBar", "ObsidianiteBar", "AdmantiumBar", 0 },

const unsigned short InvMax = 3;

const char* inventory[ InvMax+1 ] = {
	"RandomMineTreasureWWood",
	"RandomMineTreasureWBeyond",
	"RandomMineTreasureFoD",
	"RandomMineTreasureWWold"
};

MinesDropper::MinesDropper() : SmartMonster()
{
	invIndex = 0;
	lastPlayers[0] = static_cast<RMPlayer* >( 0 );
	lastPlayers[1] = static_cast<RMPlayer* >( 0 );
	lastPlayers[2] = static_cast<RMPlayer* >( 0 );
}

MinesDropper::MinesDropper( unsigned short invType ) : SmartMonster()
{
	invIndex = invType;

	lastPlayers[0] = static_cast<RMPlayer* >( 0 );
	lastPlayers[1] = static_cast<RMPlayer* >( 0 );
	lastPlayers[2] = static_cast<RMPlayer* >( 0 );
}

MinesDropper::~MinesDropper( )
{
}

void MinesDropper::init( void )
{
	new MWActionChill ( 0, this );
}

int MinesDropper::normalLogic ( void )
{
	static const unsigned char MaxTreasure = 4;

	if( !character || !room ) {
		return 100;
	}

	if( character->x != -200 || !character->y != -200 ) {
		character->x = -200;
		character->y = -200;
	}

	WorldObject* treasure = 0;
	if( inventory[ invIndex ] ) {
		treasure = roomMgr->findClass( inventory[ invIndex ] );
	}
	if( !treasure ) {
		return 500;
	}

	BTreasure *btreasure = static_cast< BTreasure*>( treasure->getBase ( _BTREASURE ) );

	if( btreasure ) {
		treasure = btreasure->makeObj();
	} else {
		treasure = treasure->clone();
	}
	if( !treasure ) {
		return 500;
	}

	//we should not drop any objects if there are 3 or more on the screen already
	
	//count carryable objects
	unsigned short carryCount = 0;
	LinkedElement* objElement = room->objects.head();

	while( objElement ) {
		WorldObject* obj = static_cast< WorldObject*>( objElement->ptr() );
		objElement = objElement->next();

		if( obj && obj->getBase( _BCARRY ) ) {
			carryCount++;
		}
	}

	if( carryCount > MaxTreasure ) {
		return 100;
	}

	//lets consider dropping the treasure
	unsigned short roll = rand100();

	if( roll > 90 ) {
		//drop a treasure near a player

		if( room->size() <= 5 ) {
			//there are very few players - drop it anywhere
			treasure->x = random( 30, 600 );
			//treasure->y = random( 130, 300 );
			treasure->y = random( 130, 275 );
			treasure->addToDatabase();
			room->addObject( treasure );
			return 13;
		} else {
			//lets find a player that we haven't dropped a treasure near recently

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
					//we have a player that hasn't had a treasure dropped next to them in awhile.. lets treat them!
					if( dropNear->isNPC )
						dropNear = static_cast<RMPlayer* >( 0 ); //disregard them if they're an NPC
					else break;
				}
			}

			if( dropNear && dropNear->character ) {
				//drop the treasure near this player
				treasure->x = dropNear->character->x + random( -10, +10 );
				treasure->y = dropNear->character->y + random( -5, +5 );
				treasure->addToDatabase();
				room->addObject( treasure );

				lastPlayers[0] = lastPlayers[1];
				lastPlayers[1] = lastPlayers[2];
				lastPlayers[2] = dropNear;
				return 13;
			}
		}
	}
	else if( roll > 86 ) {
		//drop a treasure anywhere
		treasure->x = random( 30, 600 );
		treasure->y = random( 130, 275 );
		treasure->addToDatabase();
		room->addObject( treasure );
		return 13;
	}

	return 30;
}


Miner::Miner() : SmartMonster()
{
}

Miner::~Miner()
{
}

void Miner::init( void )
{
	new MWActionChill ( 0, this, true );
	new MWActionWander ( 10, this, true );
	new MWActionChangeRooms ( 5, this, true );
	new MWActionTake ( 0, this, true, 600, true );

	//randomize the head on this sucker
	const unsigned char headPool[12][13] = {
		{  2,  0,  2,  2,  4,  0,  1,  6, 14, 10, 10,  4,  1 },
		{  2,  0,  1,  1,  6,  0, 11,  5,  4,  0, 10,  5,  0 },
		{  2,  0,  0,  6,  0,  0,  5,  2, 14,  3,  4,  6,  4 },
		{  2,  0,  1,  9,  3,  0,  1,  3, 11,  4,  7,  0,  1 },
		{  2,  0,  2,  7,  7,  0, 11,  4,  1,  2,  5,  3,  1 },
		{  2,  0,  1,  8,  6,  0, 11, 11,  0,  5,  3,  2,  0 },
		{  2,  0,  1,  1,  6,  0,  8, 11,  5,  4, 10,  9,  7 },
		{  2,  0,  0,  8,  2,  0,  7,  2,  3,  4,  4,  8,  0 },
		{  2,  0,  2,  1,  8,  0,  6, 12, 13,  2,  7,  8,  6 },
		{  2,  0,  1,  2,  4,  0,  5,  6,  7,  5, 10,  2,  4 },
		{  2,  0,  3,  1,  4,  0,  8,  5,  0,  3,  4, 11,  4 },
		{  2,  0,  1,  1, 10,  0,  1,  4, 14,  8, 10, 10,  2 }
	};

	const char namePool[40][20] = {
		"Benam","Daro","Derafo","Dogit",
		"Fosken","Laheko","Legad","Nevtid",
		"Niler","Casin","Dirmeb","Kido",
		"Loko","Metdur","Rehdof","Rihnu",
		"Ruden","Targal","Degroh","Dhand",
		"Dirri","Kafi","Lared","Mirro",
		"Mrog","Nilov","Rasmir","Sigeda",
		"Skolf","Tosfit","Dahde","Dirre",
		"Dudrat","Herlis","Hor","Ladug",
		"Rafaga","Releho","Relte","Rohive"
	};

	if( character ) {

		character->setName( namePool[ random(0, 19) ] );
		character->girth = random( 108, 115 );
		character->height = random( 87, 97 );

		BHead* bhead = character->GetHead();

		const unsigned char* attrPtr = headPool[ random(0, 11) ];

		if( bhead ) {
			bhead->race = *attrPtr++;
			bhead->sex = *attrPtr++;
			if( character->character )character->character->sex = bhead->sex;
			else logDisplay( "Could not set up miner's character's sex..." );
			bhead->skinColor = *attrPtr++;
			bhead->headNumber = *attrPtr++;
			bhead->hairNumber = *attrPtr++;
			bhead->hairColor = *attrPtr++;
			bhead->browNumber = *attrPtr++;
			bhead->faceHairNumber = *attrPtr++;
			bhead->eyeNumber = *attrPtr++;
			bhead->eyeColor = *attrPtr++;
			bhead->noseNumber = *attrPtr++;
			bhead->mouthNumber = *attrPtr++;
			bhead->earNumber = *attrPtr++;
		} else {
			logDisplay( "Could not set up miner's head..." );
		}
	} else {
		logDisplay( "Could not set up miner's character..." );
	}
}

