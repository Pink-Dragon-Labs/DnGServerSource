/*
	CHARACTERS.CPP
	Template Character Creation Functions
*/

#include "roommgr.hpp"

WorldObject *makeTemplateCharacter ( int profession, int race, int sex, char *name, char *password, int isDemo, WorldObject *source )
{
	WorldObject *object = NULL;

	if ( profession < 0 || profession >= _MAX_PROFESSION )
		return NULL;

	if ( race == _RACE_DWARF || race < 0 || race >= _MAX_RACE )
		return NULL;

	if ( sex < 0 || sex > _MAX_SEX )
		return NULL;

	// find class that represents this kind of character
	WorldObject *super = roomMgr->findClass ( characterClassTable[profession][race][sex] );

	if ( source )
		object = source;

	if ( super ) {
		if ( !source ) {
			object = new WorldObject ( super );
			object->addToDatabase();
		}

		object->isVisible = 1;
		object->creationTime = getseconds();

		if ( !source ) {
			// give the character a head
			WorldObject *head = new WorldObject ( roomMgr->findClass ( "Head" ) );

			// synchronize the head's sex and race
			BHead *bhead = (BHead *)head->getBase ( _BHEAD );
			bhead->sex = sex;
			bhead->race = race;

			// put the head in the database
			head->addToDatabase();

			// make sure the object owns the head
			head->forceIn ( object );
			head->isVisible = 1;
		}

		switch ( profession ) {
			case _PROF_ADVENTURER:
			{
				if ( sex==1 ) object->addObject ( "NewbieLeatherSkirt", 1 );
				else object->addObject ( "NewbieLeatherPants", 1 );
				if ( sex==1 ) object->addObject ( "NewbieBoots", 1 ); //female
				else object->addObject ( "NewbieLeatherBoots", 1 );	//male
				
				object->addObject ( "NewbieLeatherShirt", 1 );
				object->addObject ( "NewbieSword", 1 );
				object->addObject ( "NewbieSmallShield", 1 );
				object->addObject ( "NewbieLeatherCowl", 1 );
				object->addObject ( "BackPack", 1 );
				object->addObject ( 2, "Bread" );
				object->addObject ( 2, "WaterBottle" );
				object->manaValue = 20;
				object->value = 2400;
			}

			break;

			case _PROF_WARRIOR:
			{

				if ( sex==1 ) object->addObject ( "NewbieBoots", 1 ); //female
				else object->addObject ( "NewbieSollerets", 1 );	//male

				object->addObject ( "NewbieClub", 1 );
				object->addObject ( "NewbieChainTunic", 1 );
				object->addObject ( "NewbieChainPants", 1 );
				object->addObject ( "NewbieChainCowl", 1 );
				object->addObject ( "NewbieBracers", 1 );
				object->addObject ( "BackPack", 1 );
				object->addObject ( 2, "Bread" );
				object->addObject ( 2, "WaterBottle" );
				object->manaValue = 20;
				object->value = 2400;
			}

			break;

			case _PROF_WIZARD:
			{
				if ( sex==1 ) object->addObject ( "NewbieSkirt", 1 );
				else object->addObject ( "NewbieRobe", 1 );
				if ( sex==1 ) object->addObject ( "NewbieBoots", 1 ); //female
				else object->addObject ( "NewbieLeatherBoots", 1 );	//male
				

				object->addObject ( "NewbiePants", 1 );
				object->addObject ( "NewbieShirt", 1 );
				object->addObject ( "NewbieCowl", 1 );
				object->addObject ( "NewbieBracers", 1 );
				object->addObject ( "NewbieThrower", 1 );
				object->addObject ( "NewbieSmallShield", 1 );
				object->addObject ( "BackPack", 1 );
				object->addObject ( 2, "Veggies" );
				object->addObject ( 2, "WaterBottle" );
				object->manaValue = 250;
				object->value = 1250;
			}

			break;

			case _PROF_THIEF: {

				if ( sex==1 ) object->addObject ( "NewbieBoots", 1 ); //female
				else object->addObject ( "NewbieLeatherBoots", 1 );	//male

				object->addObject ( "NewbieLeatherShirt", 1 );
				object->addObject ( "NewbieLeatherPants", 1 );
				object->addObject ( "NewbieBracers", 1 );
				object->addObject ( "NewbieLeatherCowl", 1 );
				object->addObject ( "NewbieThrower", 1 );
				object->addObject ( "NewbieSmallShield", 1 );
				object->addObject ( "BackPack", 1 );
				object->addObject ( 2, "Bread" );
				object->addObject ( 2, "WaterBottle" );
				object->manaValue = 20;
				object->value = 2400;
			}

			break;
		}
	}

	return object;
}

Building *makeTemplateHouse ( char *name, char *password, int firstCreation )
{
  	char title[1024];

  	// let's give this character a house
  	Building *house = new Building;
  	house->resetHouse = 1;

	if ( firstCreation ) {
		house->homeTown = _TOWN_LEINSTER_EAST;
	} else {
  		house->homeTown = random ( _TOWN_LEINSTER_EAST, _TOWN_ARIMATHOR );
	}

  	RMRoom *livingRoom = new RMRoom;
  	livingRoom->picture = 3300;
  	house->addRoom ( livingRoom );

  	// put the ATPs
  	livingRoom->addATP ( 2001, 595, 35, 0 );
  	livingRoom->addATP ( 2001, 457, 14, 0 );
  	livingRoom->addATP ( 2001, 221, 21, 0 );
  	livingRoom->addATP ( 2002, 527, 38, 0 );
  	livingRoom->addATP ( 2013, 363, 14, 0 );
  	livingRoom->addATP ( 2013, 318, 21, 0 );
  	livingRoom->addATP ( 2013, 125, 19, 0 );
  	livingRoom->addATP ( 2014, 346, 30, 0 );
  	livingRoom->addATP ( 2014, 40, 46, 0 );
  	livingRoom->addATP ( 2087, 617, 172, 0 );
  	livingRoom->addATP ( 2593, 558, 147, 0 );
  	livingRoom->addATP ( 2086, 104, 159, 0 );
  	livingRoom->addATP ( 2592, 117, 133, 0 );
  	livingRoom->addATP ( 2116, 294, 101, 0 );
  	livingRoom->addATP ( 2599, 371, 267, 0 );
  	livingRoom->addATP ( 2164, 295, 94, 0 );
  	livingRoom->addATP ( 2165, 529, 102, 0 );
  	livingRoom->addATP ( 2467, 215, 171, 40 );
  	livingRoom->addATP ( 34989, -81, 232, 0 );
  	livingRoom->addATP ( 2111, 217, 142, -35 );
  	livingRoom->addATP ( 2111, 217, 152, 0 );
  	livingRoom->addATP ( 34989, 48, 193, 0 );
  	livingRoom->addATP ( 2131, 213, 165, 0 );


  	sprintf ( sizeof ( title ), title, "%s's Livingroom", name );
  	livingRoom->setTitle ( title ); 

  	// create the doors that belong in the livingroom
  	WorldObject *entryDoorA, *entryDoorB, *bedroomDoorA, *bedroomDoorB, *basementEntry, *basementExit;

  	entryDoorA = livingRoom->addObject ( "PlankDoor", 461, 181, 1 );
  	bedroomDoorA = livingRoom->addObject ( "PlankDoor", 348, 181, 0 );
	// basement trap door add
	basementEntry = livingRoom->addObject ( "TrapDoor", 73, 294, 0 );

  	// we need some chairs
  	livingRoom->addObject ( "Chair", 325, 252, 0 );
  	livingRoom->addObject ( "Chair", 367, 249, 2 );
  	livingRoom->addObject ( "Chair", 409, 251, 1 );
  	livingRoom->addObject ( "Chair", 609, 209, 1 );

  	// and a fireplace
  	livingRoom->addObject ( "FirePlace", 1, 210, 1 );


	// Attempt to create a basement
	RMRoom *basement = new RMRoom;
  	basement->picture =4000 ;
  	house->addRoom ( basement );

	basement->addATP ( 2340, 314, 46, -100 );
  	basement->addATP ( 2339, 468, 61, -100 );
  	basement->addATP ( 2338, 164, 63, -100 );
  	basement->addATP ( 2340, 106, 63, -100 );
  	basement->addATP ( 2340, 513, 61, -100 );
  	basement->addATP ( 63, 13, 181, 0 );
  	basement->addATP ( 64, 23, 183, 10 );
  	basement->addATP ( 64, 70, 174, 10 );
  	basement->addATP ( 63, 54, 188, 15 );
  	basement->addATP ( 64, 49, 177, 10 );
  	basement->addATP ( 63, 21, 176, 0 );
  	basement->addATP ( 2340, 4, 113, -50 );
  	basement->addATP ( 2340, 617, 111, -50 );
  	basement->addATP ( 6, 78, 184, 15 );
  	basement->addATP ( 7900, 314, 165, 0 );
  	basement->addATP ( 3, 164, 171, 0 );
  	basement->addATP ( 3, 461, 171, 0 );
  	basement->addATP ( 7900, 18, 182, 0 );
  	basement->addATP ( 7900, 606, 182, 0 );
  	basement->addATP ( 32782, 164, 188, 0 );
  	basement->addATP ( 14, 469, 1888, 0 );
	basement->addATP ( 3, 139, 255, 0 );
	basement->addATP ( 3, 496, 257, 0 );
	basement->addATP ( 3, 151, 265, 0 );
	basement->addATP ( 3, 490, 271, 0 );
	basement->addATP ( 3, 151, 288, 0 );
	basement->addATP ( 3, 490, 293, 0 );
	basement->addATP ( 3, 151, 312, 0 );
	basement->addATP ( 3, 490, 314, 0 );
	basement->addATP ( 3, 139, 255, 0 );
  	basement->addATP ( 32784, 7, 388, 0 );
  	basement->addATP ( 16, 635, 340, 0 );
	basement->addATP ( 2737, 420, 69, 0 );
	basement->addATP ( 35505, 209, 69, 0 );
	basement->addATP ( 2599, 514, 265, 0 );

	// name basement
	sprintf ( sizeof ( title ), title, "%s's Basement", name );
  	basement->setTitle ( title ); 

	// link door
	basementExit = basement->addObject ( "PlankDoor", 308, 153, 2 );

	
	// add chairs
  	basement->addObject ( "Chair", 453, 257, 0 );
  	basement->addObject ( "Chair", 578, 257, 1 );
  	basement->addObject ( "Chair", 515, 243, 2 );
	// add fireplaces
	basement->addObject ( "FirePlace", 462, 165, 0 );
	basement->addObject ( "FirePlace", 174, 168, 1 );  

	


  	// create the bedroom
  	RMRoom *bedroom = new RMRoom;
  	bedroom->picture = 3300;

  	house->addRoom ( bedroom );

  	sprintf ( sizeof ( title ), title, "%s's Bedroom", name );
  	bedroom->setTitle ( title ); 

  	// add te ATPs
  	bedroom->addATP ( 2001, 595, 83, 0 );
  	bedroom->addATP ( 2002, 521, 85, 0 );
  	bedroom->addATP ( 2013, 36, 55, 0 );
  	bedroom->addATP ( 2086, 3, 177, 0 );
  	bedroom->addATP ( 2592, 16, 151, 0 );
  	bedroom->addATP ( 2165, 614, 158, 0 );
  	bedroom->addATP ( 2467, 154, 215, 25 );
  	bedroom->addATP ( 2221, 423, 210, 0 );
  	bedroom->addATP ( 2002, 428, 71, 0 );
  	bedroom->addATP ( 2425, 266, 216, 30 );
  	bedroom->addATP ( 2421, 289, 216, 2 );
  	bedroom->addATP ( 2427, 217, 217, 35 );
  	bedroom->addATP ( 2025, 316, 52, 0 );
  	bedroom->addATP ( 2026, 199, 62, 0 );
  	bedroom->addATP ( 2166, 331, 140, 0 );
  	bedroom->addATP ( 2599, 113, 273, 0 );
  	bedroom->addATP ( 2025, 138, 52, 0 );
  	bedroom->addATP ( 2112, 135, 209, 0 );
  	bedroom->addATP ( 2088, 143, 163, 0 );
  	bedroom->addATP ( 2594, 122, 147, 0 );

  	// put the chairs
  	bedroom->addObject ( "Chair", 60, 254, 0 );
  	bedroom->addObject ( "Chair", 114, 250, 2 );
  	bedroom->addObject ( "Chair", 173, 260, 1 );
  	bedroom->addObject ( "Chair", 360, 251, 0 );
  	bedroom->addObject ( "Chair", 450, 268, 1 );

  	// put the bed
  	bedroom->addObject ( "Bed", 295, 215, 2 );

  	// put the door back to the livingroom
  	bedroomDoorB = bedroom->addObject ( "PlankDoor", 548, 242, 1 );

  	RMRoom *entryRoom = new RMRoom;
  	entryRoom->picture = 3201;
  	entryRoom->south = -2;

  	house->addRoom ( entryRoom );

  	sprintf ( sizeof ( title ), title, "%s's Front Yard", name );
  	entryRoom->setTitle ( title ); 

  	// add the ATPs
  	entryRoom->addATP ( 2501, 435, 12, 0 );
  	entryRoom->addATP ( 2501, 556, 30, 0 );
  	entryRoom->addATP ( 2502, 486, 34, 0 );
  	entryRoom->addATP ( 2513, 341, 32, 20 );
  	entryRoom->addATP ( 2513, 208, 34, 0 );
  	entryRoom->addATP ( 2514, 276, 83, 50 );
  	entryRoom->addATP ( 2508, 59, 20, 0 );
  	entryRoom->addATP ( 2621, 296, 40, 20 );
  	entryRoom->addATP ( 2623, 270, 52, 30 );
  	entryRoom->addATP ( 2627, 479, 47, 30 );
  	entryRoom->addATP ( 2628, 508, 63, 40 );
  	entryRoom->addATP ( 2999, 314, 136, 0 );
  	entryRoom->addATP ( 2997, 454, 241, -20 );
  	entryRoom->addATP ( 2997, 394, 179, -20 );
  	entryRoom->addATP ( 2997, 406, 201, -20 );
  	entryRoom->addATP ( 2997, 463, 221, -20 );
  	entryRoom->addATP ( 35719, 41, 187, 0 );
  	entryRoom->addATP ( 2952, 10, 173, -20 );
  	entryRoom->addATP ( 2955, 139, 185, -20 );
  	entryRoom->addATP ( 2953, 577, 216, 0 );
  	entryRoom->addATP ( 2956, 367, 222, -20 );
  	entryRoom->addATP ( 2954, 594, 238, 0 );
  	entryRoom->addATP ( 2957, 375, 259, 0 );
  	entryRoom->addATP ( 2586, 226, 174, 20 );
  	entryRoom->addATP ( 2592, 201, 136, 0 );
  	entryRoom->addATP ( 2587, 102, 158, 0 );
  	entryRoom->addATP ( 2593, 91, 139, 0 );
  	entryRoom->addATP ( 2593, 432, 120, 0 );
  	entryRoom->addATP ( 2587, 443, 159, 20 );
  	entryRoom->addATP ( 2974, 274, 116, 0 );
  	entryRoom->addATP ( 2520, 699, 12, 0 );
  	entryRoom->addATP ( 2614, 604, 120, 100 );
  	entryRoom->addATP ( 35382, 164, 124, 100 );
  	entryRoom->addATP ( 2957, 363, 212, -20 );
  	entryRoom->addATP ( 2711, 401, 172, 0 );
  	entryRoom->addATP ( 2717, 48, 204, 0 );
  	entryRoom->addATP ( 2717, 439, 193, 0 );

  	// put a door on the house
  	entryDoorB = entryRoom->addObject ( "PWDoor", 328, 178, 0 ); 

  	BPassword *base = (BPassword *)entryDoorB->getBase ( _BPASSWORD );

  	if ( base ) 
  		sprintf ( sizeof ( base->password ), base->password, "%s", password );
	
	// add strongboxes
  	WorldObject *strongBox = bedroom->addObject ( "StrongBox", 320, 250, 0 );
	base = (BPassword *)strongBox->getBase ( _BPASSWORD );

  	if ( base ) 
  		sprintf ( sizeof ( base->password ), base->password, "%s", password );
	

	WorldObject *bigStrongBox1 = basement->addObject ( "BigStrongBox", 116, 187, 0 );
	base = (BPassword *)bigStrongBox1->getBase ( _BPASSWORD );

  	if ( base ) 
  		sprintf ( sizeof ( base->password ), base->password, "%s", password );
	WorldObject *bigStrongBox2 = basement->addObject ( "BigStrongBox", 27, 209, 0 );
	base = (BPassword *)bigStrongBox2->getBase ( _BPASSWORD );

  	if ( base ) 
  		sprintf ( sizeof ( base->password ), base->password, "%s", password );
	WorldObject *bigStrongBox3 = basement->addObject ( "BigStrongBox", 72, 198, 0 );
	base = (BPassword *)bigStrongBox3->getBase ( _BPASSWORD );

  	if ( base ) 
  		sprintf ( sizeof ( base->password ), base->password, "%s", password );

	

  	// link the doors
  	entryDoorA->linkWith ( entryDoorB );
  	bedroomDoorA->linkWith ( bedroomDoorB );
	// link basement
	basementEntry->linkWith ( basementExit );

  	house->setOwnerName ( name );

  	return house;
}

